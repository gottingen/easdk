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

#include <turbo/flags/app.h>
#include <eapi/sirius/sirius.interface.pb.h>
#include <turbo/utility/status.h>
#include <collie/table/table.h>
#include <easdk/api/config_watcher.h>
#include <string>

namespace easdk::cli {

    struct ConfigOptionContext {
        static ConfigOptionContext *get_instance() {
            static ConfigOptionContext ins;
            return &ins;
        }
        // for config
        std::string config_name;
        std::string config_data;
        std::string config_file;
        std::string config_version;
        std::string config_type;
        std::string config_json;
        std::string config_example;
        std::vector<std::string> config_watch_list;
        std::string config_watch_dir;
        bool clean_local;
        eapi::sirius::ConfigInfo config_request;
    };

    struct ConfigCmd {
        static void setup_config_cmd(turbo::cli::App &app);

        static void run_config_cmd(turbo::cli::App *app);

        static void run_config_create_cmd();

        static void run_config_list_cmd();

        static void run_config_dump_cmd();

        static void run_config_test_cmd();

        static void run_config_version_list_cmd();

        static void run_config_get_cmd();

        static void run_config_remove_cmd();

        static void run_config_watch_cmd();

        [[nodiscard]] static turbo::Status
        make_example_config_dump(eapi::sirius::ConfigInfo *req);


        static collie::table::Table show_query_ops_config_list_response(const std::vector<eapi::sirius::ConfigInfo> &res);

        static turbo::Status save_config_to_file(const std::string &basedir, const easdk::ConfigCallbackData &data);
    };

}  // namespace easdk::cli
