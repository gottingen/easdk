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
#include <easdk/cli/sirius/app_cmd.h>
#include <easdk/cli/sirius/option_context.h>
#include <easdk/cli/sirius/show_help.h>
#include <easdk/utility/utility.h>
#include <easdk/api/sns.h>
#include <collie/strings/format.h>
#include <easdk/cli/sirius/flags.h>
#include <turbo/flags/flag.h>

namespace easdk::cli {
    /// Set up a subcommand and capture a shared_ptr to a struct that holds all its options.
    /// The variables of the struct are bound to the CLI options.
    /// We use a shared ptr so that the addresses of the variables remain for binding,
    /// You could return the shared pointer if you wanted to access the values in main.
    void setup_app_cmd(turbo::cli::App &app) {
        // Create the option and subcommand objects.
        auto opt = AppOptionContext::get_instance();
        auto *ns = app.add_subcommand("app", "app operations");
        ns->callback([ns]() { run_namespace_cmd(ns); });
        // Add options to sub, binding them to opt.
        //ns->require_subcommand();
        // add sub cmd
        auto cns = ns->add_subcommand("create", " create app");
        cns->add_option("-n,--name", opt->app_name, "app name")->required();
        cns->add_option("-q, --quota", opt->app_quota, "new app quota");
        cns->callback([]() { run_ns_create_cmd(); });

        auto rns = ns->add_subcommand("remove", " remove app");
        rns->add_option("-n,--name", opt->app_name, "app name")->required();
        rns->add_option("-q, --quota", opt->app_quota, "new app quota");
        rns->callback([]() { run_ns_remove_cmd(); });

        auto mns = ns->add_subcommand("modify", " modify app");
        mns->add_option("-n,--name", opt->app_name, "app name")->required();
        mns->add_option("-q, --quota", opt->app_quota, "new app quota");
        mns->callback([]() { run_ns_modify_cmd(); });

        auto lns = ns->add_subcommand("list", " list namespaces");
        lns->callback([]() { run_ns_list_cmd(); });

        auto ins = ns->add_subcommand("info", " get app info");
        ins->add_option("-n,--name", opt->app_name, "app name")->required();
        ins->callback([]() { run_ns_info_cmd(); });

    }

    /// The function that runs our code.
    /// This could also simply be in the callback lambda itself,
    /// but having a separate function is cleaner.
    void run_namespace_cmd(turbo::cli::App *app) {
        // Do stuff...
        if (app->get_subcommands().empty()) {
            collie::println("{}", app->help());
        }
    }

    void run_ns_create_cmd() {
        collie::println(collie::Color::green, "start to create app: {}",
                       AppOptionContext::get_instance()->app_name);
        ScopeShower ss;
        eapi::sirius::AppInfo app_info;
        auto rs = make_app_create(&app_info);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, "create app");
        easdk::Sns sns;
        rs = sns.initialize({turbo::get_flag(FLAGS_sirius_router)});
        RPC_INIT_ERROR_RETURN_OR_OK(ss, rs);
        rs = sns.create_app(app_info);
        RPC_ERROR_RETURN_OR_OK(ss, rs);
    }

    void run_ns_remove_cmd() {
        collie::println(collie::Color::green, "start to remove app: {}",
                       AppOptionContext::get_instance()->app_name);
        eapi::sirius::AppInfo request;
        ScopeShower ss;
        auto rs = make_app_remove(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, "remove app");
        easdk::Sns sns;
        rs = sns.initialize({turbo::get_flag(FLAGS_sirius_router)});
        RPC_INIT_ERROR_RETURN_OR_OK(ss, rs);
        rs = sns.remove_app(request.app_name());
        RPC_ERROR_RETURN_OR_OK(ss, rs);
    }

    void run_ns_modify_cmd() {
        collie::println(collie::Color::green, "start to modify app: {}",
                       AppOptionContext::get_instance()->app_name);
        eapi::sirius::AppInfo request;
        ScopeShower ss;
        auto rs = make_app_modify(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, "modify app");
        easdk::Sns sns;
        rs = sns.initialize({turbo::get_flag(FLAGS_sirius_router)});
        RPC_INIT_ERROR_RETURN_OR_OK(ss, rs);
        rs = sns.modify_app(request);
        RPC_ERROR_RETURN_OR_OK(ss, rs);
    }

    void run_ns_list_cmd() {
        collie::println(collie::Color::green, "start to get app list");
        ScopeShower ss;
        easdk::Sns sns;
        auto rs = sns.initialize({turbo::get_flag(FLAGS_sirius_router)});
        RPC_INIT_ERROR_RETURN_OR_OK(ss, rs);
        std::vector<eapi::sirius::AppInfo> res;
        rs = sns.list_app(res);
        RPC_ERROR_RETURN_OR_OK(ss, rs);
        auto table = show_discovery_query_app_response(res.data(), res.size());
        ss.add_table("result", std::move(table));
    }

    void run_ns_info_cmd() {
        collie::println(collie::Color::green, "start to get app info");
        ScopeShower ss;
        easdk::Sns sns;
        auto rs = sns.initialize({turbo::get_flag(FLAGS_sirius_router)});
        RPC_INIT_ERROR_RETURN_OR_OK(ss, rs);
        eapi::sirius::AppInfo response;
        rs = sns.get_app(AppOptionContext::get_instance()->app_name, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs);
        auto table = show_discovery_query_app_response(&response, 1);
        ss.add_table("result", std::move(table));
    }

    collie::table::Table show_discovery_query_app_response(const eapi::sirius::AppInfo *res, size_t size) {
        collie::table::Table result;
        result.add_row(
                collie::table::Table::Row_t{"app", "id", "version", "quota"});
        auto last = result.size() - 1;
        result[last].format().font_color(collie::Color::green);


        for (size_t i = 0; i < size; ++i) {
            result.add_row(
                    collie::table::Table::Row_t{res[i].app_name(),
                          collie::to_str(res[i].app_id()),
                          collie::to_str(res[i].version()),
                          collie::to_str(res[i].quota())});
            last = result.size() - 1;
            result[last].format().font_color(collie::Color::green);
        }
        return result;
    }

    turbo::Status
    make_app_create(eapi::sirius::AppInfo *req) {
        auto rs = check_valid_name_type(AppOptionContext::get_instance()->app_name);
        if (!rs.ok()) {
            return rs;
        }
        req->set_app_name(AppOptionContext::get_instance()->app_name);
        req->set_quota(AppOptionContext::get_instance()->app_quota);
        return turbo::OkStatus();
    }

    turbo::Status
    make_app_remove(eapi::sirius::AppInfo *req) {
        auto rs = check_valid_name_type(AppOptionContext::get_instance()->app_name);
        if (!rs.ok()) {
            return rs;
        }
        req->set_app_name(AppOptionContext::get_instance()->app_name);
        return turbo::OkStatus();
    }

    turbo::Status
    make_app_modify(eapi::sirius::AppInfo *req) {
        auto rs = check_valid_name_type(AppOptionContext::get_instance()->app_name);
        if (!rs.ok()) {
            return rs;
        }
        req->set_app_name(AppOptionContext::get_instance()->app_name);
        req->set_quota(AppOptionContext::get_instance()->app_quota);
        return turbo::OkStatus();
    }


}  // namespace easdk::cli
