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

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/select.h>
#include <thread>
#include <easdk/kumo/kumo_client_define.h>
#include <easdk/kumo/kumo_client_util.h>
#include <easdk/kumo/kumo_client_mysql_connection.h>

namespace kumo::client {

    class MysqlConnection;

    const int32_t EPOLL_MAX_SIZE = 65536;

    class EpollServer {
    public:
        static EpollServer *get_instance() {
            static EpollServer server;
            return &server;
        }

        void start_server();

        ~EpollServer();

        bool init();

        bool set_fd_mapping(int fd, MysqlEventInfo *conn);

        MysqlEventInfo *get_fd_mapping(int fd);

        void delete_fd_mapping(int fd);

        bool poll_events_mod(int fd, unsigned int events);

        bool poll_events_add(int fd, unsigned int events);

        bool poll_events_delete(int fd);

        void set_shutdown() {
            _shutdown = true;
        }

    private:
        EpollServer();

        MysqlEventInfo **_fd_mapping; // fd -> MysqlEventInfo.
        int _epfd;
        struct epoll_event *_events;
        size_t _event_size;
        bool _is_init;
        bool _shutdown;
    };

} // namespace kumo::client
