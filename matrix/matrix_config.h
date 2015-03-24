#ifndef __MATRIX_CONFIG_H__
#define __MATRIX_CONFIG_H__

/**
 * Copyright 2014 Open Connectome Project (http://openconnecto.me)
 * Written by Da Zheng (zhengda1936@gmail.com)
 *
 * This file is part of FlashMatrix.
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

#include "common.h"
#include "log.h"
#include "config_map.h"

#include "graph_exception.h"

namespace fm
{

/**
 * The data structure contains the configurations for matrix operations.
 */
class matrix_config
{
	int num_threads;
	std::string prof_file;
	bool _in_mem_matrix;
	// With 1D partition, a matrix is partitioned into row blocks.
	int row_block_size;
	// For 1D partition, each matrix I/O contains multiple row blocks.
	// The matrix I/O size in row blocks.
	int rb_io_size;
	// For 1D partition, the size of a matrix I/O stolen from another thread.
	int rb_steal_io_size;
	// The size of CPU cache that can be used by a thread. It affects
	// the super block size.
	int cpu_cache_size;
	// Indicate whether the hilbert order is enabled.
	bool hilbert_order;
	// The number of NUMA nodes.
	int num_nodes;
public:
	/**
	 * \brief The default constructor that set all configurations to
	 * their default values.
	 */
	matrix_config() {
		num_threads = 4;
		_in_mem_matrix = false;
		row_block_size = 1024;
		rb_io_size = 1024;
		rb_steal_io_size = 1;
		cpu_cache_size = 1024 * 1024;
		hilbert_order = true;
		num_nodes = 1;
	}

	/**
	 * \brief Print the explanations of all configurations.
	 */
	void print_help();
	/**
	 * \brief Print the current values of all configurations.
	 */
	void print();

	/**
	 * \brief Set the configurations to the user-defined values.
	 */
	void init(config_map::ptr map);

	/**
	 * \brief Get the output file containing CPU profiling.
	 * \return the file name.
	 */
	const std::string &get_prof_file() const {
		return prof_file;
	}

	/**
	 * \brief Get the number of worker threads.
	 * \return The number of worker threads.
	 */
	int get_num_threads() const {
		return num_threads;
	}

	/**
	 * \brief Determine whether to use in-mem matrix data.
	 * \return true if we loads the entire matrix data in memory
	 * in advance.
	 */
	bool use_in_mem_matrix() const {
		return _in_mem_matrix;
	}

	/**
	 * \brief The size of a row block (the number of rows).
	 */
	int get_row_block_size() const {
		return row_block_size;
	}

	/**
	 * \brief The size of a matrix I/O in 1D partitioning
	 * (the number of row blocks).
	 */
	int get_rb_io_size() const {
		return rb_io_size;
	}

	/**
	 * \brief The size of a matrix I/O stolen from another thread
	 * (the number of row blocks).
	 */
	int get_rb_steal_io_size() const {
		return rb_steal_io_size;
	}

	size_t get_cpu_cache_size() const {
		return cpu_cache_size;
	}

	bool use_hilbert_order() const {
		return hilbert_order;
	}

	void set_cpu_cache_size(size_t size) {
		cpu_cache_size = size;
	}

	void set_hilbert_order(bool hilbert) {
		hilbert_order = hilbert;
	}

	int get_num_nodes() const {
		return num_nodes;
	}
};

inline void matrix_config::print_help()
{
	printf("Configuration parameters in matrix operations.\n");
	printf("\tthreads: the number of threads processing the matrix\n");
	printf("\tprof_file: the output file containing CPU profiling\n");
	printf("\tin_mem_matrix: indicate whether to load the entire matrix to memory in advance\n");
	printf("\trow_block_size: the size of a row block (the number of rows)\n");
	printf("\trb_io_size: the size of a matrix I/O in 1D (the number of row blocks)\n");
	printf("\trb_steal_io_size: the size of a stolen matrix I/O(the number of row blocks)\n");
	printf("\tcpu_cache_size: the cpu cache size that can be used by a thread\n");
	printf("\thilbert_order: use the hilbert order\n");
	printf("\tnum_nodes: The number of NUMA nodes\n");
}

inline void matrix_config::print()
{
	BOOST_LOG_TRIVIAL(info) << "Configuration parameters in matrix operations.";
	BOOST_LOG_TRIVIAL(info) << "\tthreads: " << num_threads;
	BOOST_LOG_TRIVIAL(info) << "\tprof_file: " << prof_file;
	BOOST_LOG_TRIVIAL(info) << "\tin_mem_matrix: " << _in_mem_matrix;
	BOOST_LOG_TRIVIAL(info) << "\trow_block_size: " << row_block_size;
	BOOST_LOG_TRIVIAL(info) << "\trb_io_size" << rb_io_size;
	BOOST_LOG_TRIVIAL(info) << "\trb_steal_io_size" << rb_steal_io_size;
	BOOST_LOG_TRIVIAL(info) << "\tcpu_cache_size" << cpu_cache_size;
	BOOST_LOG_TRIVIAL(info) << "\thilbert_order" << hilbert_order;
	BOOST_LOG_TRIVIAL(info) << "\tnum_nodes" << num_nodes;
}

inline void matrix_config::init(config_map::ptr map)
{
	map->read_option_int("threads", num_threads);
	if (!power2(num_threads))
		throw fg::conf_exception("The number of worker threads has to be 2^n");
	map->read_option("prof_file", prof_file);
	map->read_option_bool("in_mem_matrix", _in_mem_matrix);
	map->read_option_int("row_block_size", row_block_size);
	map->read_option_int("rb_io_size", rb_io_size);
	map->read_option_int("rb_steal_io_size", rb_steal_io_size);
	map->read_option_int("cpu_cache_size", cpu_cache_size);
	map->read_option_bool("hilbert_order", hilbert_order);
	map->read_option_int("num_nodes", num_nodes);
}

extern matrix_config matrix_conf;

static const int PAGE_SIZE = 4096;

}

#endif
