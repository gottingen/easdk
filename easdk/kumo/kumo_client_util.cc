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


#include <easdk/kumo/kumo_client_util.h>
#include <turbo/strings/str_split.h>
#include <map>

using std::string;
using std::vector;
using std::map;

namespace kumo {
namespace client {

int32_t divide_ceil(int32_t dividend, int32_t dividor, bool* is_legal) {
    if (is_legal != nullptr) {
        *is_legal = true;
    }
    if (dividor == 0) {
        if (is_legal == nullptr) {
            CLIENT_FATAL("dividor is 0");
            return dividend;
        }
        *is_legal = false;
        return dividend;
    }
    int ret = dividend / dividor;
    if (dividend % dividor == 0) {
        return ret;
    }
    return ret + 1;
}

int get_shard_id_by_bns_tag(std::string tag) {
    std::vector<std::string> split_vec = turbo::str_split(tag, ",", turbo::SkipEmpty());
    for (size_t i = 0; i < split_vec.size(); i++) {
        if (split_vec[i].compare(0, 6, "shard:") == 0) {
            int id = atoi(split_vec[i].substr(6).c_str());
            return id;
        }
    }
    return 0;
}

HeapSort::HeapSort(): _data(nullptr), _init_value(0) {}
int HeapSort::init_heap(int64_t count, const int64_t& init_value) {
    _init_value = init_value;
    _data =  new(std::nothrow) int64_t[count + 1];
    if (_data == nullptr) {
        CLIENT_FATAL("new heap_sort fail");
        return FILESYSTEM_ERROR;
    }
    _data[0] = count;
    for (int i = 1; i < count + 1; ++i) {
        _data[i] = init_value;
    }
    return SUCCESS;
}
int64_t HeapSort::get_min_value() {
    return _data[1];
}
int64_t HeapSort::heap_down(const int64_t& value) {
    int64_t min_value = _data[1];
    _data[1] = value;
    _min_heapify(1);
    return min_value;
}

void HeapSort::clear() {
    for (int i = 1; i < _data[0] + 1; ++i) {
        _data[i] = _init_value;
    }
}
void HeapSort::_min_heapify(int64_t index) {
    int64_t left = 2 * index;
    int64_t right = 2 * index + 1;
    int64_t min_index = index;
    if (left <= _data[0] && _data[left] < _data[index]) {
        min_index = left;
    }
    if (right <= _data[0] && _data[right] < _data[min_index]) {
        min_index = right;
    }
    if (min_index != index) {
        int64_t tmp = _data[index];
        _data[index] = _data[min_index];
        _data[min_index] = tmp;
        _min_heapify(min_index);
    }
    return;
}
HeapSort::~HeapSort() {
    if (_data != nullptr) {
        delete []_data;
    }
}
}
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
