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


#include <easdk/kumo/kumo_client_epoll.h>

namespace kumo::client {

    EpollServer::EpollServer() :
            _epfd(-1),
            _event_size(0),
            _is_init(false),
            _shutdown(false) {
        _fd_mapping = new MysqlEventInfo *[EPOLL_MAX_SIZE];
        _events = new struct epoll_event[EPOLL_MAX_SIZE];
    }

    EpollServer::~EpollServer() {
        if (_epfd > 0) {
            close(_epfd);
        }
        delete[]_fd_mapping;
        delete[]_events;
    }

    bool EpollServer::init() {
        if (_is_init) {
            return true;
        }
        _event_size = EPOLL_MAX_SIZE;
        _epfd = epoll_create(EPOLL_MAX_SIZE);
        if (_epfd < 0) {
            CLIENT_FATAL("epoll_create() failed.");
            return false;
        }
        _is_init = true;
        return true;
    }

    bool EpollServer::set_fd_mapping(int fd, MysqlEventInfo *conn) {
        if (fd < 0 || fd >= EPOLL_MAX_SIZE) {
            CLIENT_FATAL("Wrong fd[%d]", fd);
            return false;
        }
        _fd_mapping[fd] = conn;
        return true;
    }

    MysqlEventInfo *EpollServer::get_fd_mapping(int fd) {
        if (fd < 0 || fd >= EPOLL_MAX_SIZE) {
            CLIENT_FATAL("Wrong fd[%d]", fd);
            return nullptr;
        }
        return _fd_mapping[fd];
    }

    void EpollServer::delete_fd_mapping(int fd) {
        if (fd < 0 || fd >= EPOLL_MAX_SIZE) {
            return;
        }
        _fd_mapping[fd] = nullptr;
        return;
    }

    bool EpollServer::poll_events_mod(int fd, unsigned int events) {
        struct epoll_event ev;
        ev.events = 0;
        ev.data.ptr = nullptr;
        ev.data.fd = 0;
        ev.data.u32 = 0;
        ev.data.u64 = 0;
        if (events & EPOLLIN) {
            ev.events |= EPOLLIN;
        }
        if (events & EPOLLOUT) {
            ev.events |= EPOLLOUT;
        }
        ev.events |= EPOLLERR | EPOLLHUP;
        ev.data.fd = fd;

        if (0 > epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev)) {
            CLIENT_FATAL("poll_events_mod() epoll_ctl error:%m , epfd=%d, fd=%d, event=%d\n",
                         _epfd, fd, events);
            return false;
        }
        return true;
    }

    bool EpollServer::poll_events_add(int fd, unsigned int events) {
        struct epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        if (events & EPOLLIN) {
            ev.events |= EPOLLIN;
        }
        if (events & EPOLLOUT) {
            ev.events |= EPOLLOUT;
        }
        ev.events |= EPOLLERR | EPOLLHUP;
        ev.data.fd = fd;

        if (0 > epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev)) {
            CLIENT_FATAL("poll_events_add_socket() epoll_ctl error:%m , epfd=%d, fd=%d\n",
                         _epfd, fd);
            return false;
        }
        return true;
    }

    bool EpollServer::poll_events_delete(int fd) {
        struct epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.data.fd = fd;

        if (epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, &ev) < 0) {
            CLIENT_FATAL("poll_events_delelte_socket() epoll_ctl error:%m, epfd=%d, fd=%d\n",
                         _epfd, fd);
            return false;
        }
        return true;
    }

    void EpollServer::start_server() {

        // Initail epoll info.
        if (!init()) {
            CLIENT_FATAL("initial epoll info failed.");
            return;
        }
        // Process epoll events.
        while (!_shutdown) {
            int fd_cnt = epoll_wait(_epfd, _events, EPOLL_MAX_SIZE, 2000);

            for (int idx = 0; idx < fd_cnt; ++idx) {
                int fd = _events[idx].data.fd;
                int event = _events[idx].events;

                // Check if socket in fd_mapping or not.
                MysqlEventInfo *info = _fd_mapping[fd];
                if (info == nullptr) {
                    CLIENT_FATAL("Can't find event_info in fd_mapping, fd:[%d], fd_cnt:[%d]",
                                 fd, idx);
                    continue;
                }

                // Check socket event.
                // EPOLLHUP: closed by client. because of protocol of sending package is wrong.
                if (event & EPOLLIN) {
                    info->event_out |= MYSQL_WAIT_READ;
                }
                if (event & EPOLLOUT) {
                    info->event_out |= MYSQL_WAIT_WRITE;
                }
                if (event & EPOLLHUP || event & EPOLLERR) {
                    info->event_out |= MYSQL_WAIT_EXCEPT;
                }
                poll_events_delete(fd);
                delete_fd_mapping(fd);
                info->cond.signal();
            }
        }
        return;
    }

}  // namespace kumo::client
