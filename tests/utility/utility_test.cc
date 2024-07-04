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
// Created by jeff on 24-7-2.
//

#include <easdk/utility/utility.h>
#include <gtest/gtest.h>

TEST(UtilityTest, ServerList) {
    std::vector<std::string> servers = {
            "192.169.2.1:9090",
            "192.169.2.1:9091",
            "192.169.2.1:9092"
    };
    std::string server_list = easdk::make_list_naming(servers);
    EXPECT_EQ(server_list, "list://192.169.2.1:9090,192.169.2.1:9091,192.169.2.1:9092");
}