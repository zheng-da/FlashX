#ifndef __COMP_IO_SCHEDULER_H__
#define __COMP_IO_SCHEDULER_H__

/**
 * Copyright 2014 Da Zheng
 *
 * This file is part of SAFSlib.
 *
 * SAFSlib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SAFSlib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SAFSlib.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * This is a I/O scheduler for requests generated by user tasks.
 * It is only used in global_cached_io.
 */
class comp_io_scheduler
{
	// If user computation has generated too many requests, we may not
	// complete a user computation (we can't fetch all requests generated
	// by it). We have to keep the incomplete computation here, and we
	// will try to fetch more requests from it later.
	fifo_queue<user_compute *> incomplete_computes;
	// The I/O instance where the I/O scheduler works on.
	io_interface *io;

protected:
	class compute_iterator {
		fifo_queue<user_compute *>::const_iterator it;
	public:
		compute_iterator(const fifo_queue<user_compute *> &computes,
				bool end): it(&computes) {
			if (end)
				it = computes.get_end();
		}

		user_compute *operator*() const {
			return *it;
		}

		compute_iterator &operator++() {
			++it;
			return *this;
		}

		bool operator==(const compute_iterator &it) const {
			return this->it == it.it;
		}

		bool operator!=(const compute_iterator &it) const {
			return this->it != it.it;
		}
	};

	/**
	 * TODO
	 * The iterator should only iterate on the user tasks with requests.
	 */

	compute_iterator get_begin() const {
		return compute_iterator(incomplete_computes, false);
	}

	compute_iterator get_end() const {
		return compute_iterator(incomplete_computes, true);
	}

public:
	comp_io_scheduler(int node_id);

	virtual ~comp_io_scheduler() {
		assert(incomplete_computes.is_empty());
	}

	virtual size_t get_requests(fifo_queue<io_request> &reqs) = 0;

	void set_io(io_interface *io) {
		this->io = io;
	}

	io_interface *get_io() const {
		return io;
	}

	void add_compute(user_compute *compute) {
		// We have to make sure the computation has requested new data
		// successfully, otherwise, it may not be executed again.
		if (!compute->test_flag(user_compute::IN_QUEUE)) {
			compute->inc_ref();
			incomplete_computes.push_back(compute);
			compute->set_flag(user_compute::IN_QUEUE, true);
		}
	}

	void delete_compute(user_compute *compute) {
		assert(compute->test_flag(user_compute::IN_QUEUE));
		compute->set_flag(user_compute::IN_QUEUE, false);
		compute->dec_ref();
		assert(compute->get_ref() == 0);
		compute_allocator *alloc = compute->get_allocator();
		alloc->free(compute);
	}

	size_t get_num_incomplete_computes() {
		return incomplete_computes.get_num_entries();
	}

	bool is_empty() {
		return incomplete_computes.is_empty();
	}

	/**
	 * Garbage collect user compute tasks.
	 */
	void gc_computes();
};

#endif
