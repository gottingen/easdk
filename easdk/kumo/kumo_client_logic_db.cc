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


#include <easdk/kumo/kumo_client_logic_db.h>
#include <easdk/kumo/shard_operator_mgr.h>
#include <turbo/strings/str_split.h>
#include <turbo/strings/ascii.h>

using std::vector;
using std::string;

namespace kumo::client {


    LogicDB::LogicDB(const string &logic_db_name) :
            _name(logic_db_name) {}

    int LogicDB::get_table_id(
            const string &table_name,
            uint32_t partition_key,
            bool *split_table,
            int *table_id) {
        vector<TableSplit>::iterator iter = _table_infos.begin();
        for (; iter != _table_infos.end(); ++iter) {
            if (iter->table_name == table_name) {
                if (iter->table_split_function.empty()) {
                    *split_table = false;
                    *table_id = 0;
                    return SUCCESS;
                }
                ShardOperatorMgr *mgr = ShardOperatorMgr::get_s_instance();
                uint32_t id = 0;
                int ret = mgr->evaluate(iter->table_split_function, partition_key, &id);
                if (ret < 0) {
                    CLIENT_WARNING("table id compute fail, table_name:%s", table_name.c_str());
                    return ret;
                }
                *table_id = id;
                *split_table = true;
                return SUCCESS;
            }
        }
        //table_name表在配置文件中未配置，说明不分表
        *split_table = false;
        *table_id = 0;
        return SUCCESS;
    }

    string LogicDB::get_name() const {
        return _name;
    }

    int LogicDB::add_table_split(
            const string &table_name,
            const vector<string> &table_split_function,
            int table_split_count) {
        if (table_name.empty()
            || table_split_count <= 0
            || (table_split_count != 1 && table_split_function.empty())) {
            CLIENT_WARNING("table split configure is not right");
            return CONFPARAM_ERROR;
        }
        vector<string> names = turbo::str_split(table_name, "|", turbo::SkipEmpty());
        vector<string>::iterator it = names.begin();
        for (; it != names.end(); ++it) {
            string name(*it);
            turbo::trim_all(&name);
            TableSplit table_split(name, table_split_count, table_split_function);
            _table_infos.push_back(table_split);
        }
        return SUCCESS;
    }

    vector<TableSplit> LogicDB::get_table_infos() const {
        return _table_infos;
    }
}  // namespace kumo::client

