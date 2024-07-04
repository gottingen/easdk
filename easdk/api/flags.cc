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
// Created by jeff on 24-7-3.
//
#include <turbo/flags/flag.h>
#include <easdk/api/flags.h>

TURBO_FLAG(int, config_retry_times, 3, "retry times for config server operation");
TURBO_FLAG(int, config_connect_timeout_ms, 500, "connect timeout for config server operation");
TURBO_FLAG(int, config_timeout_ms, 1000, "timeout for config server operation");
TURBO_FLAG(int, config_sleep_between_connect_error_ms, 1000, "sleep time between connect error for config server operation");
TURBO_FLAG(bool, config_verbose, false, "verbose for config server operation");

TURBO_FLAG(int, sns_retry_times, 3, "retry times for config server operation");
TURBO_FLAG(int, sns_connect_timeout_ms, 500, "connect timeout for config server operation");
TURBO_FLAG(int, sns_sleep_between_connect_error_ms, 1000, "sleep time between connect error for config server operation");
TURBO_FLAG(bool, sns_verbose, false, "verbose for config server operation");

