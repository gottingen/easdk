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

#include <easdk/kumo/kumo_client_row.h>
#include <stdlib.h>
#include <easdk/kumo/kumo_client_define.h>

using std::string;

namespace kumo::client {

    MysqlRow::MysqlRow(MYSQL_RES *res) :
            _res(res),
            _fetch_row(nullptr) {}

    MysqlRow::~MysqlRow() {
        _res = nullptr;
        _fetch_row = nullptr;
    }

    void MysqlRow::init_type() {
        uint32_t fields_count = mysql_num_fields(_res);
        for (uint32_t i = 0; i < fields_count; ++i) {
            MYSQL_FIELD *field = mysql_fetch_field_direct(_res, i);
            if (_is_int32_type(field->type) && !(field->flags & UNSIGNED_FLAG)) {
                _field_type.push_back(VT_INT32);
                continue;
            }
            if (_is_int32_type(field->type) && (field->flags & UNSIGNED_FLAG)) {
                _field_type.push_back(VT_UINT32);
                continue;
            }
            if (_is_int64_type(field->type) && !(field->flags & UNSIGNED_FLAG)) {
                _field_type.push_back(VT_INT64);
                continue;
            }
            if (_is_int64_type(field->type) && (field->flags & UNSIGNED_FLAG)) {
                _field_type.push_back(VT_UINT64);
                continue;
            }
            if (_is_float_type(field->type)) {
                _field_type.push_back(VT_FLOAT);
                continue;
            }
            if (_is_double_type(field->type)) {
                _field_type.push_back(VT_DOUBLE);
                continue;
            }
            _field_type.push_back(VT_STRING);
        }
    }

    void MysqlRow::set_row(MYSQL_ROW fetch_row) {
        _fetch_row = fetch_row;
    }

    int MysqlRow::get_string(uint32_t column_index, string *value) {
        if (_fetch_row[column_index] == nullptr) {
            return VALUE_IS_NULL;
        }
        *value = _fetch_row[column_index];
        return SUCCESS;
    }

    int MysqlRow::get_int32(uint32_t column_index, int32_t *value) {
        if (_fetch_row[column_index] == nullptr) {
            return VALUE_IS_NULL;
        }
        *value = atoi(_fetch_row[column_index]);
        return SUCCESS;
    }

    int MysqlRow::get_uint32(uint32_t column_index, uint32_t *value) {
        if (_fetch_row[column_index] == nullptr) {
            return VALUE_IS_NULL;
        }
        *value = strtoul(_fetch_row[column_index], nullptr, 0);
        return SUCCESS;
    }

    int MysqlRow::get_int64(uint32_t column_index, int64_t *value) {
        if (_fetch_row[column_index] == nullptr) {
            return VALUE_IS_NULL;
        }
        *value = strtol(_fetch_row[column_index], nullptr, 0);
        return SUCCESS;
    }

    int MysqlRow::get_uint64(uint32_t column_index, uint64_t *value) {
        if (_fetch_row[column_index] == nullptr) {
            return VALUE_IS_NULL;
        }
        *value = strtoul(_fetch_row[column_index], nullptr, 0);
        return SUCCESS;
    }

    int MysqlRow::get_float(uint32_t column_index, float *value) {
        if (_fetch_row[column_index] == nullptr) {
            return VALUE_IS_NULL;
        }
        *value = strtod(_fetch_row[column_index], nullptr);
        return SUCCESS;
    }

    int MysqlRow::get_double(uint32_t column_index, double *value) {
        if (_fetch_row[column_index] == nullptr) {
            return VALUE_IS_NULL;
        }
        *value = strtod(_fetch_row[column_index], nullptr);
        return SUCCESS;
    }

    const char *MysqlRow::get_value(uint32_t column_index) {
        return _fetch_row[column_index];
    }

    ValueType MysqlRow::get_type(uint32_t column_index) {
        return _field_type[column_index];
    }

    bool MysqlRow::_is_int32_type(enum_field_types source_type) {
        if (source_type == MYSQL_TYPE_TINY
            || source_type == MYSQL_TYPE_SHORT
            || source_type == MYSQL_TYPE_LONG
            || source_type == MYSQL_TYPE_INT24) {
            return true;
        }
        return false;
    }

    bool MysqlRow::_is_int64_type(enum_field_types source_type) {
        if (source_type == MYSQL_TYPE_LONGLONG) {
            return true;
        }
        return false;
    }

    bool MysqlRow::_is_float_type(enum_field_types source_type) {
        if (source_type == MYSQL_TYPE_FLOAT) {
            return true;
        }
        return false;
    }

    bool MysqlRow::_is_double_type(enum_field_types source_type) {
        if (source_type == MYSQL_TYPE_DOUBLE
            || source_type == MYSQL_TYPE_DECIMAL
            || source_type == MYSQL_TYPE_NEWDECIMAL) {
            return true;
        }
        return false;
    }

    // string类型目前不判断，直接返回true
    bool MysqlRow::_is_string_type(enum_field_types /*source_type*/) {
        return true;
    }

}  // namespace kumo::client
