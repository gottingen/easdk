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

#include <string>
#include <vector>
#include <easdk/kumo/kumo_client_define.h>

namespace kumo::client {

    class LogicDB {

    public:
        LogicDB(const std::string &logic_db_name);

        //split_table, true, 代表分表，分表id为table_id，false，代表不分表
        int get_table_id(const std::string &table_name,
                         uint32_t partition_key,
                         bool *split_table,
                         int *table_id);

        std::string get_name() const;

        int add_table_split(
                const std::string &table_name,
                const std::vector<std::string> &table_split_function,
                int table_split_count);

        std::vector<TableSplit> get_table_infos() const;

    private:
        std::string _name; // 逻辑库名
        std::vector<TableSplit> _table_infos; //该库的分表信息
    };

}  // namespace kumo::client
