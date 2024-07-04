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

#include <turbo/flags/declare.h>
//

// config_retry_times is the number of times to retry a config server operation.
TURBO_DECLARE_FLAG(int, config_retry_times);

// config_connect_timeout_ms is the timeout for connecting to the config server.
TURBO_DECLARE_FLAG(int, config_connect_timeout_ms);

// config_timeout_ms is the timeout for a config server operation.
TURBO_DECLARE_FLAG(int, config_timeout_ms);

// config_sleep_between_connect_error_ms is the time to sleep between connect errors.
TURBO_DECLARE_FLAG(int, config_sleep_between_connect_error_ms);

// config_verbose is a flag to enable verbose output for config server operations.
TURBO_DECLARE_FLAG(bool, config_verbose);

// config_retry_times is the number of times to retry a config server operation.
TURBO_DECLARE_FLAG(int, sns_retry_times);

// config_connect_timeout_ms is the timeout for connecting to the config server.
TURBO_DECLARE_FLAG(int, sns_connect_timeout_ms);

// config_timeout_ms is the timeout for a config server operation.
TURBO_DECLARE_FLAG(int, sns_timeout_ms);

// config_sleep_between_connect_error_ms is the time to sleep between connect errors.
TURBO_DECLARE_FLAG(int, sns_sleep_between_connect_error_ms);

// config_verbose is a flag to enable verbose output for config server operations.
TURBO_DECLARE_FLAG(bool, sns_verbose);
