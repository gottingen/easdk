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
#include <easdk/cli/sirius/zone_cmd.h>
#include <easdk/cli/sirius/option_context.h>
#include <turbo/log/logging.h>
#include <easdk/cli/sirius/flags.h>
#include <eapi/sirius/sirius.interface.pb.h>
#include <easdk/cli/sirius/show_help.h>
#include <collie/table/table.h>
#include <collie/strings/format.h>
#include <easdk/utility/utility.h>

namespace easdk::cli {
    /// Set up a subcommand and capture a shared_ptr to a struct that holds all its options.
    /// The variables of the struct are bound to the CLI options.
    /// We use a shared ptr so that the addresses of the variables remain for binding,
    /// You could return the shared pointer if you wanted to access the values in main.
    void setup_zone_cmd(turbo::cli::App &app) {
        // Create the option and subcommand objects.
        auto opt = ZoneOptionContext::get_instance();
        auto *ns = app.add_subcommand("zone", "zone operations");
        ns->callback([ns]() { run_zone_cmd(ns); });
        // Add options to sub, binding them to opt.
        //ns->require_subcommand();
        // add sub cmd
        auto cdb = ns->add_subcommand("create", " create zone");
        cdb->add_option("-n,--app", opt->app_name, "app name")->required();
        cdb->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        cdb->add_option("-q, --quota", opt->namespace_quota, "new app quota");
        cdb->callback([]() { run_zone_create_cmd(); });

        auto rdb = ns->add_subcommand("remove", " remove zone");
        rdb->add_option("-n,--app", opt->app_name, "app name")->required();
        rdb->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        rdb->callback([]() { run_zone_remove_cmd(); });

        auto mdb = ns->add_subcommand("modify", " modify zone");
        mdb->add_option("-n,--app", opt->app_name, "app name")->required();
        mdb->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        mdb->add_option("-q, --quota", opt->namespace_quota, "new app quota");
        mdb->callback([]() { run_zone_modify_cmd(); });

        auto lns = ns->add_subcommand("list", " list namespaces");
        lns->callback([]() { run_zone_list_cmd(); });

        auto idb = ns->add_subcommand("info", " get zone info");
        idb->add_option("-n,--app", opt->app_name, "app name")->required();
        idb->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        idb->callback([]() { run_zone_info_cmd(); });

    }

    /// The function that runs our code.
    /// This could also simply be in the callback lambda itself,
    /// but having a separate function is cleaner.
    void run_zone_cmd(turbo::cli::App* app) {
        // Do stuff...
        if(app->get_subcommands().empty()) {
            collie::println("{}", app->help());
        }
    }

    void run_zone_create_cmd() {
        collie::println(collie::Color::green, "start to create zone: {}", ZoneOptionContext::get_instance()->app_name);
        eapi::sirius::ZoneInfo request;
        ScopeShower ss;
        auto rs= make_zone_create(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, "create zone");
        easdk::Sns sns;
        rs = sns.initialize({turbo::get_flag(FLAGS_sirius_router)});
        RPC_INIT_ERROR_RETURN_OR_OK(ss, rs);
        rs = sns.create_zone(request);
        RPC_ERROR_RETURN_OR_OK(ss, rs);
    }
    void run_zone_remove_cmd() {
        collie::println(collie::Color::green, "start to remove zone: {}", ZoneOptionContext::get_instance()->app_name);
        eapi::sirius::ZoneInfo request;
        ScopeShower ss;
        auto rs = make_zone_remove(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, "remove zone");
        easdk::Sns sns;
        rs = sns.initialize({turbo::get_flag(FLAGS_sirius_router)});
        RPC_INIT_ERROR_RETURN_OR_OK(ss, rs);
        rs = sns.remove_zone(request);
        RPC_ERROR_RETURN_OR_OK(ss, rs);
    }
    void run_zone_modify_cmd() {
        collie::println(collie::Color::green, "start to modify zone: {}", ZoneOptionContext::get_instance()->app_name);
        ScopeShower ss;
        eapi::sirius::ZoneInfo request;
        auto rs = make_zone_modify(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, "modify zone");
        easdk::Sns sns;
        rs = sns.initialize({turbo::get_flag(FLAGS_sirius_router)});
        RPC_INIT_ERROR_RETURN_OR_OK(ss, rs);
        rs = sns.modify_zone(request);
        RPC_ERROR_RETURN_OR_OK(ss, rs);
    }

    void run_zone_list_cmd() {
        collie::println(collie::Color::green, "start to get zone list");

        ScopeShower ss;
        PREPARE_ERROR_RETURN_OR_OK(ss, turbo::OkStatus(), "list zone");
        std::vector<eapi::sirius::ZoneInfo> zone_list;
        easdk::Sns sns;
        auto rs = sns.initialize({turbo::get_flag(FLAGS_sirius_router)});
        RPC_INIT_ERROR_RETURN_OR_OK(ss, rs);
        if(ZoneOptionContext::get_instance()->app_name.empty()) {
            rs = sns.list_zone(zone_list);
        } else {
            rs = sns.list_zone(ZoneOptionContext::get_instance()->app_name, zone_list);
        }
        RPC_ERROR_RETURN_OR_OK(ss, rs);
        auto table = show_discovery_query_zone_response(zone_list);
        ss.add_table("result", std::move(table));
    }

    void run_zone_info_cmd() {
        collie::println(collie::Color::green, "start to get zone list");
        ScopeShower ss;
        PREPARE_ERROR_RETURN_OR_OK(ss, turbo::OkStatus(), "get zone");
        easdk::Sns sns;
        auto rs = sns.initialize({turbo::get_flag(FLAGS_sirius_router)});
        RPC_INIT_ERROR_RETURN_OR_OK(ss, rs);
        eapi::sirius::ZoneInfo zone;
        rs = sns.get_zone(ZoneOptionContext::get_instance()->app_name, ZoneOptionContext::get_instance()->zone_name, zone);
        RPC_ERROR_RETURN_OR_OK(ss, rs);
        auto table = show_discovery_query_zone_response({zone});
        ss.add_table("result", std::move(table));
    }

    collie::table::Table show_discovery_query_zone_response(const std::vector<eapi::sirius::ZoneInfo> &zones) {
        collie::table::Table summary;
        summary.add_row({"app", "zone", "id", "version", "quota"});
        for (auto &zone: zones) {
            summary.add_row(
                    collie::table::Table::Row_t{zone.app_name(), zone.zone(), collie::format("{}", zone.zone_id()),
                                        collie::format("{}", zone.version()),
                                        collie::format("{}", zone.quota())});
            auto last = summary.size() - 1;
            summary[last].format().font_color(collie::Color::green);
        }
        return summary;
    }

    turbo::Status make_zone_create(eapi::sirius::ZoneInfo *zone_req) {
        auto rs = check_valid_name_type(ZoneOptionContext::get_instance()->app_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(ZoneOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        zone_req->set_app_name(ZoneOptionContext::get_instance()->app_name);
        zone_req->set_zone(ZoneOptionContext::get_instance()->zone_name);
        return turbo::OkStatus();
    }

    turbo::Status make_zone_remove(eapi::sirius::ZoneInfo *zone_req) {
        auto rs = check_valid_name_type(ZoneOptionContext::get_instance()->app_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(ZoneOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        zone_req->set_app_name(ZoneOptionContext::get_instance()->app_name);
        zone_req->set_zone(ZoneOptionContext::get_instance()->zone_name);
        return turbo::OkStatus();
    }

    turbo::Status make_zone_modify(eapi::sirius::ZoneInfo *zone_req) {
        auto rs = check_valid_name_type(ZoneOptionContext::get_instance()->app_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(ZoneOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        zone_req->set_app_name(ZoneOptionContext::get_instance()->app_name);
        zone_req->set_zone(ZoneOptionContext::get_instance()->zone_name);
        return turbo::OkStatus();
    }

}  // namespace easdk::cli
