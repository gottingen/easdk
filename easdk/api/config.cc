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

#include <easdk/api/config.h>
#include <easdk/api/flags.h>
#include <easdk/utility/utility.h>
#include <easdk/sirius/config_info_builder.h>
#include <melon/json2pb/json_to_pb.h>
#include <melon/json2pb/pb_to_json.h>
#include <alkaid/files/localfs.h>

namespace easdk {

    turbo::Status Config::check_config(const std::string &json_content) {
        eapi::sirius::ConfigInfo config_pb;
        std::string errmsg;
        if (!json2pb::JsonToProtoMessage(json_content, &config_pb, &errmsg)) {
            return turbo::invalid_argument_error(errmsg);
        }
        return turbo::OkStatus();
    }

    turbo::Status Config::check_config_file(const std::string &config_path) {
        auto lfs = alkaid::Filesystem::localfs();
        std::string config_data;
        STATUS_RETURN_IF_ERROR(lfs->read_file(config_path, &config_data));
        return check_config(config_data);
    }

    turbo::Status Config::dump_config_json(const eapi::sirius::ConfigInfo &config, std::string &json) {
        std::string err;
        if (!json2pb::ProtoMessageToJson(config, &json, &err)) {
            return turbo::invalid_argument_error(err);
        }
        return turbo::OkStatus();
    }

    turbo::Status Config::dump_config_file(const std::string &config_path, const eapi::sirius::ConfigInfo &config) {
        std::string json;
        std::string err;
        if (!json2pb::ProtoMessageToJson(config, &json, &err)) {
            return turbo::invalid_argument_error(err);
        }
        auto lfs = alkaid::Filesystem::localfs();
        STATUS_RETURN_IF_ERROR(lfs->write_file(config_path, json));
        return turbo::OkStatus();
    }

    turbo::Status Config::initialize(const std::vector<std::string> &servers) {
        return _sender.init(servers);
    }

    turbo::Status
    Config::create_config(const std::string &config_name, const std::string &content,
                          const std::string &version, const std::string &config_type, int retry_times) {
        eapi::sirius::ConfigInfo config;
        sirius::ConfigInfoBuilder builder(&config);
        auto rs = builder.build_from_content(config_name, content, version, config_type);
        if (!rs.ok()) {
            return rs;
        }
        return create_config(config, retry_times);
    }

    turbo::Status
    Config::create_config(const eapi::sirius::ConfigInfo &request,
                          int retry_times) {
        eapi::CommonResponse response;
        STATUS_RETURN_IF_ERROR(_sender.send_request("create_config", request, response, retry_times));
        if (response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        return turbo::OkStatus();
    }

    turbo::Status Config::create_config_content_in_file(const std::string &config_name,
                                                        const std::string &path,
                                                        const std::string &version,
                                                        const std::string &config_type,
                                                        int retry_times) {
        eapi::sirius::ConfigInfo request;
        easdk::sirius::ConfigInfoBuilder builder(&request);
        auto rs = builder.build_from_file(config_name, path, version, config_type);
        if (!rs.ok()) {
            return rs;
        }
        return create_config(request, retry_times);
    }

    turbo::Status Config::create_config_by_json_file(const std::string &json_path, int retry_times) {
        eapi::sirius::ConfigInfo request;
        easdk::sirius::ConfigInfoBuilder builder(&request);
        auto rs = builder.build_from_json_file(json_path);
        if(!rs.ok()) {
            return rs;
        }
        return create_config(request, retry_times);
    }

    turbo::Status Config::create_config_by_json(const std::string &json_path, int retry_times) {
        eapi::sirius::ConfigInfo request;
        easdk::sirius::ConfigInfoBuilder builder(&request);
        auto rs = builder.build_from_json(json_path);
        if(!rs.ok()) {
            return rs;
        }
        return create_config(request, retry_times);
    }

    turbo::Status Config::list_config(std::vector<eapi::sirius::ConfigInfo> &configs, int retry_times) {
        eapi::sirius::ConfigQueryRequest req;
        eapi::sirius::ConfigListResponse response;
        STATUS_RETURN_IF_ERROR(_sender.send_request("list_config", req, response, retry_times));
        if (response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        auto &res_configs = response.configs();
        for (auto config: res_configs) {
            configs.push_back(config);
        }
        return turbo::OkStatus();
    }

    turbo::Status
    Config::list_config(const std::string &config_name, std::vector<eapi::sirius::ConfigInfo> &versions,
                                         int retry_times) {
        eapi::sirius::ConfigQueryRequest req;
        eapi::sirius::ConfigListResponse response;
        req.set_name(config_name);
        STATUS_RETURN_IF_ERROR(_sender.send_request("list_config", req, response, retry_times));
        if (response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        auto &res_configs = response.configs();
        for (auto config: res_configs) {
            versions.push_back(config);
        }
        return turbo::OkStatus();
    }

    turbo::Status
    Config::get_config(const std::string &config_name, const std::string &version, eapi::sirius::ConfigInfo &config,
                                int retry_times) {
        eapi::sirius::ConfigInfo request;
        eapi::sirius::ConfigListResponse response;
        request.set_name(config_name);
        auto rs = string_to_version(version, request.mutable_version());
        if (!rs.ok()) {
            return rs;
        }
        STATUS_RETURN_IF_ERROR(_sender.send_request("get_config", request, response, retry_times));
        if (response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        if (response.configs_size() != 1) {
            return turbo::invalid_argument_error("bad proto for config list size not 1");
        }
        config = response.configs(0);
        return turbo::OkStatus();
    }

    turbo::Status
    Config::get_config_latest(const std::string &config_name, eapi::sirius::ConfigInfo &config,
                                       int retry_times) {
        eapi::sirius::ConfigInfo request;
        eapi::sirius::ConfigListResponse response;
        request.set_name(config_name);
        STATUS_RETURN_IF_ERROR(_sender.send_request("get_config", request, response, retry_times));
        if (response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        if (response.configs_size() != 1) {
            return turbo::invalid_argument_error("bad proto for config list size not 1");
        }
        config = response.configs(0);
        return turbo::OkStatus();
    }

    turbo::Status
    Config::remove_config(const std::string &config_name, const std::string &version, int retry_times) {
        eapi::sirius::ConfigInfo request;
        eapi::CommonResponse response;
        request.set_name(config_name);
        auto rs = string_to_version(version, request.mutable_version());
        if (!rs.ok()) {
            return rs;
        }
        STATUS_RETURN_IF_ERROR(_sender.send_request("delete_config", request, response, retry_times));
        if (response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        return turbo::OkStatus();
    }

    turbo::Status
    Config::remove_config(const std::string &config_name, const collie::ModuleVersion &version, int retry_times) {
        eapi::sirius::ConfigInfo request;
        eapi::CommonResponse response;
        request.set_name(config_name);
        request.mutable_version()->set_major(static_cast<int32_t>(version.major));
        request.mutable_version()->set_minor(static_cast<int32_t>(version.minor));
        request.mutable_version()->set_patch(static_cast<int32_t>(version.patch));
        STATUS_RETURN_IF_ERROR(_sender.send_request("delete_config", request, response, retry_times));
        if (response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        return turbo::OkStatus();
    }

    turbo::Status
    Config::remove_config_all_version(const std::string &config_name, int retry_times) {
        eapi::sirius::ConfigInfo request;
        eapi::CommonResponse response;
        request.set_name(config_name);
        STATUS_RETURN_IF_ERROR(_sender.send_request("delete_config", request, response, retry_times));
        if (response.code() != eapi::kOk) {
            return turbo::unavailable_error(response.message());
        }
        return turbo::OkStatus();
    }

}  // namespace easdk