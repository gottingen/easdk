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
//

#pragma once

#include <turbo/utility/status.h>
#include <google/protobuf/descriptor.h>
#include <eapi/sirius/sirius.interface.pb.h>
#include <easdk/sirius/sirius_sender.h>
#include <collie/module/semver.h>

namespace easdk {

    class Config {
    public:
        Config() = default;

        ~Config() = default;


        static turbo::Status check_config(const std::string &json_content);

        static turbo::Status check_config_file(const std::string &config_path);

        static turbo::Status dump_config_json(const eapi::sirius::ConfigInfo &config, std::string &json);

        static turbo::Status dump_config_file(const std::string &config_path, const eapi::sirius::ConfigInfo &config);

        turbo::Status initialize(const std::vector<std::string> &servers);

        turbo::Status create_config(const std::string &config_name,
                                    const std::string &content,
                                    const std::string &version,
                                    const std::string &config_type = "json",
                                    int retry_time = 0);

        turbo::Status create_config(const eapi::sirius::ConfigInfo &request, int retry_times = 0);

        turbo::Status create_config_content_in_file(const std::string &config_name,
                                                    const std::string &path,
                                                    const std::string &version,
                                                    const std::string &config_type = "json",
                                                    int retry_time = 0);

        turbo::Status create_config_by_json_file(const std::string &json_path, int retry_times = 0);

        turbo::Status create_config_by_json(const std::string &json_path, int retry_times = 0);

        turbo::Status list_config(std::vector<eapi::sirius::ConfigInfo> &configs, int retry_times = 0);

        turbo::Status list_config(const std::string &config_name, std::vector<eapi::sirius::ConfigInfo> &versions,
                                  int retry_times = 0);

        turbo::Status
        get_config(const std::string &config_name, const std::string &version, eapi::sirius::ConfigInfo &config,
                   int retry_times = 0);

        turbo::Status
        get_config_latest(const std::string &config_name, eapi::sirius::ConfigInfo &config, int retry_times = 0);

        turbo::Status
        remove_config(const std::string &config_name, const std::string &version, int retry_times = 0);

        turbo::Status
        remove_config(const std::string &config_name, const collie::ModuleVersion &version, int retry_times = 0);

        turbo::Status
        remove_config_all_version(const std::string &config_name, int retry_times = 0);

    private:
        easdk::sirius::SiriusSender _sender;
    };

}  // namespace easdk
