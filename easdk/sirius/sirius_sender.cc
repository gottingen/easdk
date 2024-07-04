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


#include <easdk/sirius/sirius_sender.h>
#include <easdk/utility/utility.h>
#include <collie/strings/str_split.h>

namespace easdk::sirius {


    turbo::Status SiriusSender::init(const std::vector<std::string> &servers) {
        _list_servers = easdk::make_list_naming(servers);
        melon::ChannelOptions channel_opt;
        channel_opt.timeout_ms = _request_timeout;
        channel_opt.connect_timeout_ms = _connect_timeout;
        if (_channel.Init(_list_servers.c_str(), "rr", &channel_opt) != 0) {
            return turbo::unavailable_error("init channel error");
        }
        return turbo::OkStatus();
    }

    SiriusSender &SiriusSender::set_verbose(bool verbose) {
        _verbose = verbose;
        return *this;
    }

    SiriusSender &SiriusSender::set_time_out(int time_ms) {
        _request_timeout = time_ms;
        return *this;
    }

    SiriusSender &SiriusSender::set_connect_time_out(int time_ms) {
        _connect_timeout = time_ms;
        return *this;
    }

    SiriusSender &SiriusSender::set_retry_time(int retry) {
        _retry_times = retry;
        return *this;
    }

}  // sirius::client

