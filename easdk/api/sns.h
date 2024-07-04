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

namespace easdk {

    class Sns {
    public:
        Sns() = default;
        ~Sns() = default;

        turbo::Status initialize(const std::vector<std::string> &servers);

        turbo::Status create_app(eapi::sirius::AppInfo &info, int retry_times = 0);

        turbo::Status remove_app(const std::string &ns, int retry_times = 0);

        turbo::Status modify_app(eapi::sirius::AppInfo &ns_info, int retry_times = 0);

        turbo::Status list_app(std::vector<eapi::sirius::AppInfo> &ns_list, int retry_times = 0);

        turbo::Status get_app(const std::string &ns_name, eapi::sirius::AppInfo &ns_pb, int retry_times = 0);

        turbo::Status create_zone(const eapi::sirius::ZoneInfo &info, int retry_times = 0);

        turbo::Status remove_zone(const eapi::sirius::ZoneInfo &info, int retry_times = 0);

        turbo::Status remove_zone(const std::string &ns, const std::string &zone, int retry_times = 0);

        turbo::Status modify_zone(eapi::sirius::ZoneInfo &zone_info, int retry_times = 0);

        turbo::Status list_zone(std::vector<eapi::sirius::ZoneInfo> &zone_list, int retry_times = 0);

        turbo::Status
        list_zone(const std::string &ns, std::vector<eapi::sirius::ZoneInfo> &zone_list, int retry_times = 0);
        turbo::Status get_zone(const std::string &ns_name, const std::string &zone_name, eapi::sirius::ZoneInfo &zone_pb,
                               int retry_times = 0);

    private:
        easdk::sirius::SiriusSender _sender;
    };

}  // namespace easdk
