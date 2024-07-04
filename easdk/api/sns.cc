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

#include <easdk/api/sns.h>
#include <easdk/utility/utility.h>
#include <easdk/sirius/config_info_builder.h>
#include <melon/json2pb/json_to_pb.h>
#include <melon/json2pb/pb_to_json.h>
#include <alkaid/files/localfs.h>
#include <turbo/strings/substitute.h>
#include <easdk/api/flags.h>

namespace easdk {

    turbo::Status Sns::initialize(const std::vector<std::string> &servers) {
        return _sender.init(servers);
    }

    turbo::Status Sns::create_app(eapi::sirius::AppInfo &info, int retry_times) {
        eapi::CommonResponse response;
        STATUS_RETURN_IF_ERROR(_sender.send_request("create_app", info, response, retry_times));
        if(response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        return turbo::OkStatus();
    }

    turbo::Status Sns::remove_app(const std::string &ns, int retry_times) {
        eapi::CommonResponse response;
        eapi::sirius::AppInfo info;
        info.set_app_name(ns);
        STATUS_RETURN_IF_ERROR(_sender.send_request("delete_app", info, response, retry_times));
        if(response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        return turbo::OkStatus();
    }

    turbo::Status Sns::modify_app(eapi::sirius::AppInfo &ns_info, int retry_times) {
        eapi::CommonResponse response;
        STATUS_RETURN_IF_ERROR(_sender.send_request("update_app", ns_info, response, retry_times));
        if(response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        return turbo::OkStatus();
    }

    turbo::Status Sns::list_app(std::vector<eapi::sirius::AppInfo> &ns_list, int retry_times) {
        eapi::sirius::SnsQueryRequest request;
        eapi::sirius::AppListResponse response;
        STATUS_RETURN_IF_ERROR(_sender.send_request("list_app", request, response, retry_times));
        if(response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        for (auto &ns: response.app_info()) {
            ns_list.push_back(ns);
        }
        return turbo::OkStatus();
    }

    turbo::Status
    Sns::get_app(const std::string &ns_name, eapi::sirius::AppInfo &ns_pb, int retry_times) {
        eapi::sirius::SnsQueryRequest request;
        eapi::sirius::AppListResponse response;
        request.set_app_name(ns_name);
        STATUS_RETURN_IF_ERROR(_sender.send_request("get_app", request, response, retry_times));
        if(response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        if(response.app_info_size() != 1) {
            return turbo::invalid_argument_error("bad proto for app list size not 1");
        }
        ns_pb = response.app_info(0);
        return turbo::OkStatus();
    }

    turbo::Status Sns::create_zone(const eapi::sirius::ZoneInfo &zone_info, int retry_times) {
        eapi::CommonResponse response;
        STATUS_RETURN_IF_ERROR(_sender.send_request("create_zone", zone_info, response, retry_times));
        if(response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        return turbo::OkStatus();
    }

    turbo::Status Sns::remove_zone(const eapi::sirius::ZoneInfo &zone_info, int retry_times) {
        eapi::CommonResponse response;
        STATUS_RETURN_IF_ERROR(_sender.send_request("delete_zone", zone_info, response, retry_times));
        if(response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        return turbo::OkStatus();
    }

    turbo::Status Sns::remove_zone(const std::string &ns, const std::string &zone, int retry_times) {
        eapi::CommonResponse response;
        eapi::sirius::ZoneInfo info;
        info.set_app_name(ns);
        info.set_zone(zone);
        STATUS_RETURN_IF_ERROR(_sender.send_request("delete_zone", info, response, retry_times));
        if(response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        return turbo::OkStatus();
    }

    turbo::Status Sns::modify_zone(eapi::sirius::ZoneInfo &zone_info, int retry_times) {
        eapi::CommonResponse response;
        STATUS_RETURN_IF_ERROR(_sender.send_request("update_zone", zone_info, response, retry_times));
        if(response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        return turbo::OkStatus();
    }

    turbo::Status Sns::list_zone(std::vector<eapi::sirius::ZoneInfo> &zone_list, int retry_times) {
           eapi::sirius::SnsQueryRequest request;
        eapi::sirius::ZoneListResponse response;
        STATUS_RETURN_IF_ERROR(_sender.send_request("list_zone", request, response, retry_times));
        if(response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        for (auto &zone: response.zone_info()) {
            zone_list.push_back(zone);
        }
        return turbo::OkStatus();
    }

    turbo::Status
    Sns::list_zone(const std::string &ns, std::vector<eapi::sirius::ZoneInfo> &zone_list, int retry_times) {
           eapi::sirius::SnsQueryRequest request;
            eapi::sirius::ZoneListResponse response;
            request.set_app_name(ns);
            STATUS_RETURN_IF_ERROR(_sender.send_request("list_zone", request, response, retry_times));
            if(response.code() != eapi::kOk) {
                return turbo::unavailable_error(response.message());
            }
            for (auto &zone: response.zone_info()) {
                zone_list.push_back(zone);
            }
            return turbo::OkStatus();
    }

    turbo::Status
    Sns::get_zone(const std::string &ns_name, const std::string &zone_name, eapi::sirius::ZoneInfo &zone_pb,
                  int retry_times) {
        eapi::sirius::SnsQueryRequest request;
        eapi::sirius::ZoneListResponse response;
        request.set_app_name(ns_name);
        request.set_zone_name(zone_name);
        STATUS_RETURN_IF_ERROR(_sender.send_request("list_zone", request, response, retry_times));
        if(response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        if(response.zone_info_size() != 1) {
            return turbo::invalid_argument_error("bad proto for zone list size not 1");
        }
        zone_pb = response.zone_info(0);
        return turbo::OkStatus();
    }
}  // namespace easdk
