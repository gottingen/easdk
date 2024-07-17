//
// Copyright (C) 2024 EA group inc.
// Author: Jeff.li lijippy@163.com
// All rights reserved.
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//


#pragma once

#include <easdk/kumo/kumo_client_define.h>
#include <string>
#include <vector>
#include <sys/time.h>


namespace kumo::client {
// @brief 通过bns提供的服务得到bns上的实例信息

extern int get_shard_id_by_bns_tag(std::string tag);

extern int32_t divide_ceil(int32_t dividend, int32_t dividor, bool* result);

class HeapSort {
public:
    HeapSort();
    int init_heap(int64_t count, const int64_t& init_value);
    int64_t get_min_value();
    int64_t heap_down(const int64_t& value);
    void clear();
    ~HeapSort();
private:
    void _min_heapify(int64_t index);

    int64_t* _data;
    int64_t _init_value;
};
}  // namespace kumo::client
