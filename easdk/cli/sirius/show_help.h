// Copyright 2023 The Turbo Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <turbo/utility/status.h>
#include <eapi/sirius/sirius.interface.pb.h>
#include <collie/table/table.h>
#include <easdk/cli/sirius/proto_help.h>

namespace easdk::cli {
    class ShowHelper {
    public:
        ~ShowHelper();

        static std::string json_format(const std::string &json_str);

        static collie::table::Table pre_send_error(const turbo::Status &s, const std::string &op) {
            collie::table::Table result;
            result.add_row(Row_t{"status", "op", "error code", "error message"});
            auto last = result.size() - 1;
            result[last].format().font_color(collie::Color::yellow).font_style({collie::FontStyle::bold});
            if (s.ok()) {
                result.add_row(
                        Row_t{"success", op,
                              std::to_string(s.raw_code()), s.message()});
                last = result.size() - 1;
                result[last][0].format().font_color(collie::Color::green);
                result[last][1].format().font_color(collie::Color::yellow);
                result[last][2].format().font_color(collie::Color::green);
                result[last][3].format().font_color(collie::Color::green);
            } else {
                result.add_row(
                        Row_t{"fail", op,
                              std::to_string(s.raw_code()), s.message()});
                last = result.size() - 1;
                result[last][0].format().font_color(collie::Color::red);
                result[last][1].format().font_color(collie::Color::yellow);
                result[last][2].format().font_color(collie::Color::red);
                result[last][3].format().font_color(collie::Color::red);
            }
            return result;
        }

    private:
        static collie::table::Table rpc_error_status_impl(const turbo::Status &s, int qt, const std::string &qts);

        static std::string get_level_str(int level);
    private:
        using Row_t = collie::table::Table::Row_t;
        collie::table::Table pre_send_result;
        collie::table::Table rpc_result;
        collie::table::Table result_table;

    };

    struct ScopeShower {
        ~ScopeShower();

        ScopeShower();

        explicit ScopeShower(const std::string &operation);

        void add_table(collie::table::Table &&table);

        void add_table(const std::string &stage, collie::table::Table &&table, bool ok = true);

        void add_table(const std::string &stage, const std::string &msg, bool ok);

        void prepare(const turbo::Status &status);

        std::vector<collie::table::Table> tables;
        collie::table::Table result_table;
    };
}  // namespace easdk::cli

#define PREPARE_ERROR_RETURN(show, rs, request) \
    do {                                            \
        if(!rs.ok()) {                        \
            show.add_table("prepare", std::move(ShowHelper::pre_send_error(rs, request))); \
            return;                                            \
        }                                               \
    }while(0)

#define PREPARE_ERROR_RETURN_OR_OK(show, rs, op) \
    do {                                            \
        if(!rs.ok()) {                        \
            show.add_table("prepare", ShowHelper::pre_send_error(rs, op), false); \
            return;                                            \
        } else {                                               \
            show.add_table("prepare", "ok", true);                   \
        }                                                      \
    }while(0)

#define RPC_INIT_ERROR_RETURN_OR_OK(show, rs) \
    do {                                            \
        if(!rs.ok()) {                        \
            show.add_table("rpc init", ShowHelper::pre_send_error(rs, "na"), false); \
            return;                                            \
        } else {                                               \
            show.add_table("rpc init", "ok", true);                   \
        }                                                      \
    }while(0)

#define RPC_ERROR_RETURN_OR_OK(show, rs) \
    do {                                            \
        if(!rs.ok()) {                               \
            show.add_table("rpc", ShowHelper::pre_send_error(rs, "na"), false); \
            return;                                    \
        } else {                                        \
            show.add_table("rpc","ok", true);                    \
        }                                             \
    }while(0)
