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

#include <time.h>
#include <mariadb/mysql.h>
#include <sys/epoll.h>
#include <sstream>
#include <melon/fiber/fiber.h>
#include <melon/fiber/unstable.h>
#include <mariadb/errmsg.h>

namespace kumo::client {

    class Connection;

    // non-blocking implementation
    extern MYSQL *mysql_async_real_connect(Connection *connection, const char *host,
                                           const char *user, const char *passwd,
                                           const char *db, unsigned int port,
                                           const char *unix_socket,
                                           unsigned long client_flag);

    // non-blocking implementation
    int mysql_async_real_query(Connection *connection,
                               const char *stmt_str,
                               unsigned long length,
                               std::ostringstream *os = nullptr);

    // non-blocking implementation
    int mysql_async_ping(Connection *connection);

    //void mysql_async_free_result(MYSQL_RES *res, Connection *connection);

    MYSQL_RES *mysql_async_store_result(Connection *connection);

    int mysql_async_set_character_set(Connection *connection, const char *char_set);

}  // namespace kumo::client
