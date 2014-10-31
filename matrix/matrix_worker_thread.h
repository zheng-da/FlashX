#ifndef __MATRIX_WORKER_THREAD_H__
#define __MATRIX_WORKER_THREAD_H__

/*
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

#include "thread.h"
#include "matrix_io.h"
#include "io_interface.h"

class task_creator;

class matrix_worker_thread: public thread
{
	matrix_io_generator::ptr io_gen;
	std::shared_ptr<task_creator> tcreator;
	file_io_factory::shared_ptr factory;
	io_interface::ptr io;
	int worker_id;

	matrix_worker_thread(int worker_id, int node_id,
			file_io_factory::shared_ptr factory, matrix_io_generator::ptr gen,
			std::shared_ptr<task_creator> creator): thread("matrix-thread",
				node_id) {
		this->worker_id = worker_id;
		this->io_gen = gen;
		this->tcreator = creator;
		this->factory = factory;
	}
public:
	typedef std::shared_ptr<matrix_worker_thread> ptr;

	/*
	 * Create a worker thread.
	 * \param node_id It indicates which NUMA node this worker thread should run.
	 * \param factory The I/O factory for the file that stores the matrix.
	 * \param gen It defines how a matrix is accessed.
	 * \param creator It defines what computation is performed on the part of
	 * a matrix read from disks.
	 */
	static ptr create(int worker_id, int node_id,
			file_io_factory::shared_ptr factory, matrix_io_generator::ptr gen,
			std::shared_ptr<task_creator> creator) {
		return ptr(new matrix_worker_thread(worker_id, node_id, factory, gen,
					creator));
	}

	void init() {
		io = factory->create_io(this);
	}

	void run();
};

#endif
