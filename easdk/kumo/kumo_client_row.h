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

#include <mariadb/mysql.h>
#include <string>
#include <easdk/kumo/kumo_client_define.h>

namespace kumo::client {

    class Row {
    public:
        virtual ~Row() {}

        virtual void set_row(MYSQL_ROW /*fetch_row*/) {}

        virtual void init_type() = 0;

        virtual int get_string(uint32_t column_index, std::string *value) = 0;

        virtual int get_int32(uint32_t column_index, int32_t *value) = 0;

        virtual int get_uint32(uint32_t column_index, uint32_t *value) = 0;

        virtual int get_int64(uint32_t column_index, int64_t *value) = 0;

        virtual int get_uint64(uint32_t column_index, uint64_t *value) = 0;

        virtual int get_float(uint32_t column_index, float *value) = 0;

        virtual int get_double(uint32_t column_index, double *value) = 0;

        virtual const char *get_value(uint32_t column_index) = 0;

        virtual ValueType get_type(uint32_t column_index) = 0;
    };

    class MysqlRow : public Row {

    public:
        explicit MysqlRow(MYSQL_RES *res);

        ~MysqlRow();

        void set_row(MYSQL_ROW fetch_row);

        void init_type();

        int get_string(uint32_t column_index, std::string *value);

        int get_int32(uint32_t column_index, int32_t *value);

        int get_uint32(uint32_t column_index, uint32_t *value);

        int get_int64(uint32_t column_index, int64_t *value);

        int get_uint64(uint32_t column_index, uint64_t *value);

        int get_float(uint32_t column_index, float *value);

        int get_double(uint32_t column_index, double *value);

        const char *get_value(uint32_t column_index);

        ValueType get_type(uint32_t column_index);

    private:
        // @breif 判断是否是给定的类型
        bool _is_int32_type(enum_field_types source_type);

        bool _is_int64_type(enum_field_types source_type);

        bool _is_float_type(enum_field_types source_type);

        bool _is_double_type(enum_field_types source_type);

        bool _is_string_type(enum_field_types source_type);

    private:
        MYSQL_RES *_res;
        std::vector<ValueType> _field_type;
        MYSQL_ROW _fetch_row;
    };
}  // namespace kumo::client
