/**
 * Copyright 2014 Open Connectome Project (http://openconnecto.me)
 * Written by Da Zheng (zhengda1936@gmail.com)
 *
 * This file is part of FlashGraph.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "vertex_compute.h"
#include "graph_engine.h"
#include "worker_thread.h"
#include "vertex_index_reader.h"

class empty_page_byte_array: public page_byte_array
{
public:
	virtual void lock() {
	}
	virtual void unlock() {
	}

	virtual off_t get_offset_in_first_page() const {
		return 0;
	}
	virtual thread_safe_page *get_page(int idx) const {
		return NULL;
	}
	virtual size_t get_size() const {
		return 0;
	}
};

request_range vertex_compute::get_next_request()
{
	// Get the next vertex.
	const ext_mem_vertex_info info = requested_vertices.top();
	requested_vertices.pop();
	data_loc_t loc(graph->get_file_id(), info.get_off());
	num_issued++;
	return request_range(loc, info.get_size(), READ, this);
}

void vertex_compute::request_vertices(vertex_id_t ids[], size_t num)
{
	num_requested += num;
	issue_thread->get_index_reader().request_vertices(ids, num, *this);
}

void vertex_compute::request_num_edges(vertex_id_t ids[], size_t num)
{
	num_edge_requests += num;
	issue_thread->get_index_reader().request_num_edges(ids, num, *this);
}

void vertex_compute::run_on_vertex_size(vertex_id_t id, vsize_t size)
{
	vsize_t num_edges = issue_thread->get_graph().cal_num_edges(size);
	assert(!graph->get_graph_header().has_edge_data());
	vertex_header header(id, num_edges);
	issue_thread->start_run_vertex(v);
	issue_thread->get_vertex_program(v.is_part()).run_on_num_edges(*v, header);
	issue_thread->finish_run_vertex(v);
	num_edge_completed++;
	if (get_num_pending() == 0)
		issue_thread->complete_vertex(v);
}

vertex_id_t vertex_compute::get_id() const
{
	return v->get_id();
}

void vertex_compute::issue_io_request(const ext_mem_vertex_info &info)
{
	// If the vertex compute has been issued to SAFS, SAFS will get the IO
	// request from the interface of user_compute. In this case, we only
	// need to add the I/O request to the queue.
	if (issued_to_io()) {
		requested_vertices.push(info);
	}
	else {
		// Otherwise, we need to issue the I/O request to SAFS explicitly.
		data_loc_t loc(graph->get_file_id(), info.get_off());
		io_request req(this, loc, info.get_size(), READ);
		num_issued++;
		issue_thread->issue_io_request(req);
	}
}

void vertex_compute::run(page_byte_array &array)
{
	page_undirected_vertex pg_v(array);
	worker_thread *t = (worker_thread *) thread::get_curr_thread();
	vertex_program &curr_vprog = t->get_vertex_program(v.is_part());
	issue_thread->start_run_vertex(v);
	curr_vprog.run(*v, pg_v);
	issue_thread->finish_run_vertex(v);
	complete_request();
}

void vertex_compute::complete_request()
{
	num_complete_fetched++;
	// We need to notify the thread that initiate processing the vertex
	// of the completion of the vertex.
	// TODO is this a right way to complete a vertex?
	if (get_num_pending() == 0)
		issue_thread->complete_vertex(v);
}

void directed_vertex_compute::run_on_page_vertex(page_directed_vertex &pg_v)
{
	worker_thread *t = (worker_thread *) thread::get_curr_thread();
	vertex_program &curr_vprog = t->get_vertex_program(v.is_part());
	issue_thread->start_run_vertex(v);
	curr_vprog.run(*v, pg_v);
	issue_thread->finish_run_vertex(v);
	complete_request();
}

void directed_vertex_compute::run(page_byte_array &array)
{
	// If the combine map is empty, we don't need to merge
	// byte arrays.
	if (combine_map.empty()) {
		page_directed_vertex pg_v(array,
				(size_t) array.get_offset() < graph->get_in_part_size());
		run_on_page_vertex(pg_v);
		return;
	}

	vertex_id_t id = page_directed_vertex::get_id(array);
	combine_map_t::iterator it = combine_map.find(id);
	// If the vertex isn't in the combine map, we don't need to
	// merge byte arrays.
	if (it == combine_map.end()) {
		page_directed_vertex pg_v(array,
				(size_t) array.get_offset() < graph->get_in_part_size());
		run_on_page_vertex(pg_v);
		return;
	}
	else if (it->second == NULL) {
		page_byte_array *arr_copy = array.clone();
		assert(arr_copy);
		it->second = arr_copy;
	}
	else {
		page_byte_array *in_arr;
		page_byte_array *out_arr;
		if (it->second->get_offset() < get_graph().get_in_part_size()) {
			in_arr = it->second;
			out_arr = &array;
			assert(array.get_offset() >= get_graph().get_in_part_size());
		}
		else {
			out_arr = it->second;
			in_arr = &array;
			assert(array.get_offset() < get_graph().get_in_part_size());
		}
		page_directed_vertex pg_v(*in_arr, *out_arr);
		run_on_page_vertex(pg_v);
		page_byte_array::destroy(it->second);
		combine_map.erase(it);
	}
}

#if 0
void directed_vertex_compute::complete_empty_part(
		const ext_mem_vertex_info &info)
{
	worker_thread *t = (worker_thread *) thread::get_curr_thread();
	assert(t == issue_thread);
	vertex_program &curr_vprog = t->get_vertex_program(v.is_part());

	empty_page_byte_array array;
	page_directed_vertex pg_v(info.get_id(), 0, array,
			(size_t) info.get_off() < graph->get_in_part_size());
	issue_thread->start_run_vertex(v);
	curr_vprog.run(*v, pg_v);
	issue_thread->finish_run_vertex(v);
	complete_request();
}
#endif

void directed_vertex_compute::request_vertices(vertex_id_t ids[], size_t num)
{
	stack_array<directed_vertex_request> reqs(num);
	for (size_t i = 0; i < num; i++)
		reqs[i] = directed_vertex_request(ids[i], edge_type::BOTH_EDGES);
	request_partial_vertices(reqs.data(), num);
}

void directed_vertex_compute::request_partial_vertices(
		directed_vertex_request reqs[], size_t num)
{
	num_requested += num;
	issue_thread->get_index_reader().request_vertices(reqs, num, *this);
}

void directed_vertex_compute::run_on_vertex_size(vertex_id_t id,
		size_t in_size, size_t out_size)
{
	vsize_t num_in_edges = issue_thread->get_graph().cal_num_edges(in_size);
	vsize_t num_out_edges = issue_thread->get_graph().cal_num_edges(out_size);
	assert(!graph->get_graph_header().has_edge_data());
	directed_vertex_header header(id, num_in_edges, num_out_edges);
	issue_thread->start_run_vertex(v);
	issue_thread->get_vertex_program(v.is_part()).run_on_num_edges(*v, header);
	issue_thread->finish_run_vertex(v);
	num_edge_completed++;
	if (get_num_pending() == 0)
		issue_thread->complete_vertex(v);
}

void directed_vertex_compute::issue_io_request(const ext_mem_vertex_info &in_info,
		const ext_mem_vertex_info &out_info)
{
	assert(in_info.get_id() == out_info.get_id());
	// Otherwise, we need to issue the I/O request to SAFS explicitly.
	data_loc_t loc1(graph->get_file_id(), in_info.get_off());
	io_request req1(this, loc1, in_info.get_size(), READ);
	issue_thread->issue_io_request(req1);

	data_loc_t loc2(graph->get_file_id(), out_info.get_off());
	io_request req2(this, loc2, out_info.get_size(), READ);
	issue_thread->issue_io_request(req2);

	combine_map.insert(combine_map_t::value_type(in_info.get_id(), NULL));
	num_issued++;
}

void directed_vertex_compute::request_num_edges(vertex_id_t ids[], size_t num)
{
	num_edge_requests += num;
	issue_thread->get_index_reader().request_num_directed_edges(ids,
			num, *this);
}
