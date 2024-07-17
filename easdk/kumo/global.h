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

#include <string>

namespace kumo::client {

    extern char *INSTANCE_STATUS_CSTR[5];
    extern int DEFAULT_READ_TIMEOUT;
    extern int DEFAULT_WRITE_TIMEOUT;
    extern int DEFAULT_CONNECT_TIMEOUT;
    extern std::string DEFAULT_CHARSET;
    extern char *CONN_TYPE_CSTR[3];

}  // namespace kumo::client
