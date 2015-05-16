#ifndef __ONE_VAL_MATRIX_STORE_H__
#define __ONE_VAL_MATRIX_STORE_H__

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

#include "virtual_matrix_store.h"

namespace fm
{

namespace detail
{

class one_val_matrix_store: public virtual_matrix_store
{
	scalar_variable::ptr val;
	matrix_layout_t layout;
	std::unique_ptr<char[]> portion_buf;
	int num_nodes;
public:
	one_val_matrix_store(scalar_variable::ptr val, size_t nrow, size_t ncol,
			matrix_layout_t layout, int num_nodes);

	virtual matrix_store::ptr materialize() const;

	virtual const char *get(size_t row, size_t col) const {
		return val->get_raw();
	}

	virtual matrix_store::const_ptr get_cols(const std::vector<off_t> &idxs) const;
	virtual matrix_store::const_ptr get_rows(const std::vector<off_t> &idxs) const;

	virtual std::shared_ptr<const local_matrix_store> get_portion(
			size_t start_row, size_t start_col, size_t num_rows,
			size_t num_cols) const;
	virtual std::shared_ptr<const local_matrix_store> get_portion(
			size_t id) const;

	virtual matrix_store::const_ptr transpose() const;

	virtual matrix_layout_t store_layout() const {
		return layout;
	}
	virtual int get_num_nodes() const {
		return num_nodes;
	}
};

}

}

#endif