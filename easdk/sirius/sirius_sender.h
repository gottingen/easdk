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

#include <melon/utility/endpoint.h>
#include <melon/rpc/channel.h>
#include <melon/rpc/server.h>
#include <melon/rpc/controller.h>
#include <google/protobuf/descriptor.h>
#include <eapi/sirius/sirius.interface.pb.h>
#include <turbo/utility/status.h>

namespace easdk::sirius {

    /**
     * @ingroup ea_rpc
     * @brief SiriusSender is used to send messages to the meta server.
     *       It communicates with the meta server and sends messages to the meta server.
     *       It needs to be initialized before use. It need judge the leader of meta server.
     *       If the leader is not found, it will retry to send the request to the meta server.
     *       If the peer is not leader, it will redirect to the leader and retry to send the
     *       request to the meta server.
     * @code
     *      SiriusSender::get_instance()->init("127.0.0.1:8200");
     *      eapi::sirius::DiscoveryManagerRequest request;
     *      eapi::sirius::DiscoveryManagerResponse response;
     *      request.set_type(eapi::sirius::DiscoveryManagerRequest::ADD);
     *      request.set_name("test");
     *      request.set_version("1.0.0");
     *      request.set_content("test");
     *      auto rs = SiriusSender::get_instance()->discovery_manager(request, response);
     *      if(!rs.ok()) {
     *          TLOG_ERROR("meta manager error:{}", rs.message());
     *          return;
     *      }
     *      TLOG_INFO("meta manager success");
     *      return;
     *@endcode
     */
    class SiriusSender {
    public:
        static const int kRetryTimes = 5;

        /**
         * @brief get_instance is used to get the singleton instance of SiriusSender.
         * @return
         */
        static SiriusSender *get_instance() {
            static SiriusSender _instance;
            return &_instance;
        }

        SiriusSender() = default;

        /**
         * @brief is_inited is used to check if the SiriusSender is initialized.
         * @return
         */
        [[nodiscard]] bool is_inited() {
            return _is_inited;
        }

        /**
         * @brief init is used to initialize the SiriusSender. It must be called before using the SiriusSender.
         * @param raft_nodes [input] is the raft nodes of the meta server.
         * @return Status::OK if the SiriusSender was initialized successfully. Otherwise, an error status is returned.
         */
        turbo::Status init(const std::vector<std::string> &servers);

        /**
         * @brief init is used to initialize the SiriusSender. It can be called any time.
         * @param verbose [input] is the verbose flag.
         * @return SiriusSender itself.
         */
        SiriusSender &set_verbose(bool verbose);

        /**
         * @brief set_time_out is used to set the timeout for sending a request to the meta server.
         * @param time_ms [input] is the timeout in milliseconds for sending a request to the meta server.
         * @return SiriusSender itself.
         */
        SiriusSender &set_time_out(int time_ms);


        /**
         * @brief set_connect_time_out is used to set the timeout for connecting to the meta server.
         * @param time_ms [input] is the timeout in milliseconds for connecting to the meta server.
         * @return SiriusSender itself.
         */
        SiriusSender &set_connect_time_out(int time_ms);

        /**
         * @brief set_retry_time is used to set the number of times to retry sending a request to the meta server.
         * @param retry [input] is the number of times to retry sending a request to the meta server.
         * @return SiriusSender itself.
         */
        SiriusSender &set_retry_time(int retry);

        /**
         * @brief send_request is used to send a request to the meta server.
         * @param service_name [input] is the name of the service to send the request to.
         * @param request [input] is the request to send.
         * @param response [output] is the response received from the meta server.
         * @param retry_times [input] is the number of times to retry sending the request.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        template<typename Request, typename Response>
        turbo::Status send_request(const std::string &service_name,
                                   const Request &request,
                                   Response &response, int retry_times = -1);
    private:
        melon::Channel _channel;
        std::string _list_servers;
        std::vector<mutil::EndPoint> _servlet_nodes;
        int32_t _request_timeout = 30000;
        int32_t _connect_timeout = 5000;
        bool _is_inited{false};
        int _retry_times{kRetryTimes};
        bool _verbose{false};
    };

    template<typename Request, typename Response>
    inline turbo::Status SiriusSender::send_request(const std::string &service_name,
                                                  const Request &request,
                                                  Response &response, int retry_times) {
        const ::google::protobuf::ServiceDescriptor *service_desc = eapi::sirius::SiriusService::descriptor();
        const ::google::protobuf::MethodDescriptor *method =
                service_desc->FindMethodByName(service_name);
        if (method == nullptr) {
            LOG(ERROR)<< "service name not exist, service:"<<service_name;
            return turbo::unavailable_error("service name not exist, service:");
        }
        int retry_time = 0;
        uint64_t log_id = mutil::fast_rand();
        auto rtime = retry_times > 0 ? retry_times : _retry_times;
        do {
            melon::Controller cntl;
            cntl.set_log_id(log_id);
            _channel.CallMethod(method, &cntl, &request, &response, nullptr);
            LOG_IF(INFO, _verbose) << "meta_req[" << request.ShortDebugString() << "], meta_resp["
                                      << response.ShortDebugString() << "]";
            if (cntl.Failed()) {
                LOG(ERROR) << "connect with server fail. send request fail, error:" << cntl.ErrorText()
                                          << ", log_id:" << cntl.log_id();
                ++retry_time;
                continue;
            }
            return turbo::OkStatus();
        } while (retry_time < rtime);
        return turbo::unavailable_error("can not connect server after times try");
    }


}  // namespace easdk::sirius
