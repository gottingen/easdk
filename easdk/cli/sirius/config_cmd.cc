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

#include <easdk/cli/sirius/config_cmd.h>
#include <easdk/cli/sirius/option_context.h>
#include <easdk/cli/sirius/show_help.h>
#include <easdk/cli/sirius/flags.h>
#include <easdk/api/config_watcher.h>
#include <easdk/api/config.h>
#include <collie/module/semver.h>
#include <alkaid/files/filesystem.h>
#include <turbo/times/time.h>
#include <melon/json2pb/pb_to_json.h>
#include <melon/json2pb/json_to_pb.h>
#include <easdk/utility/dumper.h>
#include <easdk/sirius/config_info_builder.h>
#include <collie/nlohmann/json.hpp>
#include <collie/strings/format.h>
#include <turbo/strings/substitute.h>

namespace easdk::cli {

    void ConfigCmd::setup_config_cmd(turbo::cli::App &app) {
        // Create the option and subcommand objects.
        auto opt = ConfigOptionContext::get_instance();
        auto *ns = app.add_subcommand("config", "config operations");
        ns->callback([ns]() { run_config_cmd(ns); });

        auto cc = ns->add_subcommand("create", " create config");
        auto *parameters_inputs = cc->add_option_group("parameters_inputs", "config input from parameters");
        auto *json_inputs = cc->add_option_group("json_inputs", "config input source from json format");
        parameters_inputs->add_option("-n,--name", opt->config_name, "config name")->required();
        auto *df_inputs = parameters_inputs->add_option_group("data_or_file", "config input source");
        df_inputs->add_option("-d,--data", opt->config_data, "config content");
        df_inputs->add_option("-f, --file", opt->config_file, "local config file");
        df_inputs->require_option(1);
        parameters_inputs->add_option("-v, --version", opt->config_version, "config version [1.2.3]");
        parameters_inputs->add_option("-t, --type", opt->config_type,
                                      "config type [json|toml|yaml|xml|gflags|text|ini]")->default_val("json");
        json_inputs->add_option("-j, --json", opt->config_json, "local config file form json format");
        cc->require_option(1);
        cc->callback([]() { run_config_create_cmd(); });

        auto cl = ns->add_subcommand("list", " list config");
        cl->add_option("-n,--name", opt->config_name, "config name");
        cl->callback([]() { run_config_list_cmd(); });

        auto cg = ns->add_subcommand("get", " get config");
        cg->add_option("-n,--name", opt->config_name, "config name")->required();
        cg->add_option("-v, --version", opt->config_version, "config version");
        cg->add_option("-o, --output", opt->config_file, "config save file");
        cg->callback([]() { run_config_get_cmd(); });

        auto cr = ns->add_subcommand("remove", " remove config");
        cr->add_option("-n,--name", opt->config_name, "config name")->required();
        cr->add_option("-v, --version", opt->config_version, "config version [1.2.3]");
        cr->callback([]() { run_config_remove_cmd(); });


        auto cd = ns->add_subcommand("dump", " dump config example to json file");
        auto *dump_parameters_inputs = cd->add_option_group("parameters_inputs", "config input from parameters");
        auto *dump_default = cd->add_option_group("default_example", "default config example");
        dump_parameters_inputs->add_option("-n,--name", opt->config_name, "config name")->required(true);
        dump_parameters_inputs->add_option("-v, --version", opt->config_version, "config version")->required(true);
        dump_parameters_inputs->add_option("-c, --content", opt->config_data, "config version")->required(true);
        dump_parameters_inputs->add_option("-t, --type", opt->config_type,
                                           "config type [json|toml|yaml|xml|gflags|text|ini]")->default_val("json");
        dump_parameters_inputs->add_option("-o, --output", opt->config_file, "config save file");
        dump_default->add_option("-e, --example", opt->config_example, "example output file");
        cd->require_option(1);
        cd->callback([]() { run_config_dump_cmd(); });


        auto ct = ns->add_subcommand("test", "test json config file");
        ct->add_option("-f, --file", opt->config_file, "local config file")->required(true);
        ct->callback([]() { run_config_test_cmd(); });

        auto cw = ns->add_subcommand("watch", "watch config");
        cw->add_option("-n, --name", opt->config_watch_list, "local config file")->required(true);
        cw->add_option("-d, --dir", opt->config_watch_dir, "local config file")->default_val("watch_config");
        cw->add_flag("-c, --clean", opt->clean_local, "clean cache")->default_val(false);
        cw->callback([]() { run_config_watch_cmd(); });

    }

    /// The function that runs our code.
    /// This could also simply be in the callback lambda itself,
    /// but having a separate function is cleaner.
    void ConfigCmd::run_config_cmd(turbo::cli::App *app) {
        // Do stuff...
        if (app->get_subcommands().empty()) {
            collie::println("{}", app->help());
        }
    }

    void ConfigCmd::run_config_create_cmd() {
        eapi::sirius::ConfigInfo config_info;
        ScopeShower ss;
        auto opt = ConfigOptionContext::get_instance();
        easdk::sirius::ConfigInfoBuilder builder(&config_info);
        /// json builder
        turbo::Status rs;
        if (!opt->config_json.empty()) {
            rs = builder.build_from_json_file(opt->config_json);
        } else {
            /// parameter
            if (!opt->config_file.empty()) {
                rs = builder.build_from_file(opt->config_name,
                                             opt->config_file,
                                             opt->config_version,
                                             opt->config_type);
            } else {
                rs = builder.build_from_content(opt->config_name,
                                                opt->config_data,
                                                opt->config_version,
                                                opt->config_type);
            }
        }
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, "create config");
        easdk::Config client;
        rs = client.initialize({turbo::get_flag(FLAGS_sirius_router)});
        RPC_INIT_ERROR_RETURN_OR_OK(ss, rs);
        rs = client.create_config(config_info);
        RPC_ERROR_RETURN_OR_OK(ss, rs);
    }

    void ConfigCmd::run_config_dump_cmd() {
        eapi::sirius::ConfigInfo request;

        ScopeShower ss;
        auto opt = ConfigOptionContext::get_instance();
        turbo::Status rs;
        std::string file_path;
        if (!opt->config_example.empty()) {
            rs = make_example_config_dump(&request);
            file_path = opt->config_example;
        } else {
            file_path = opt->config_file;
            easdk::sirius::ConfigInfoBuilder builder(&request);
            rs = builder.build_from_content(opt->config_name, opt->config_data, opt->config_version, opt->config_type);
        }

        if (!rs.ok()) {
            ss.add_table("prepare", rs.to_string(), false);
            return;
        } else {
            ss.add_table("prepare", "ok", true);
        }
        if (!rs.ok()) {
            ss.add_table("prepare file", rs.to_string(), false);
            return;
        } else {
            ss.add_table("prepare file", "ok", true);
        }
        std::string json;
        std::string err;
        rs = easdk::Dumper::dump_proto(request, json);
        if (!rs.ok()) {
            ss.add_table("convert", rs.to_string(), false);
            return;
        } else {
            ss.add_table("convert", "ok", true);
        }

        auto lfs = alkaid::Filesystem::localfs();
        auto file_rs = lfs->create_sequential_write_file();
        if (!file_rs.ok()) {
            ss.add_table("create file", file_rs.status().to_string(), false);
            return;
        } else {
            ss.add_table("create file", "ok", true);
        }
        auto file = file_rs.value();
        rs = file->append(json);
        if (!rs.ok()) {
            ss.add_table("write", rs.to_string(), false);
            return;
        } else {
            ss.add_table("write", "ok", true);
        }
        (void) file->close();
        ss.add_table("summary", collie::format("success write to  file: {}", file_path), true);
    }

    void ConfigCmd::run_config_test_cmd() {
        eapi::sirius::ConfigInfo request;
        ScopeShower ss;
        if (ConfigOptionContext::get_instance()->config_file.empty()) {
            ss.add_table("prepare", "no input file", false);
            return;
        }
        std::string content;
        auto lfs = alkaid::Filesystem::localfs();
        ss.add_table("open file", "ok", true);
        auto r = lfs->read_file(ConfigOptionContext::get_instance()->config_file, &content);
        if (!r.ok()) {
            ss.add_table("read file", r.to_string(), false);
            return;
        }
        ss.add_table("read file", "ok", true);
        easdk::sirius::ConfigInfoBuilder builder(&request);
        auto rs = builder.build_from_json(content);
        if (!rs.ok()) {
            ss.add_table("convert", rs.to_string(), false);
            return;
        }
        ss.add_table("convert", "ok", true);
        collie::println("name size:{}", request.name().size());
        collie::table::Table result_table;
        result_table.add_row(collie::table::Table::Row_t{"name", request.name()});
        result_table.add_row(
                collie::table::Table::Row_t{"version", collie::format("{}.{}.{}", request.version().major(),
                                                                      request.version().minor(),
                                                                      request.version().patch())});
        result_table.add_row(collie::table::Table::Row_t{"type", config_type_to_string(request.type())});
        result_table.add_row(collie::table::Table::Row_t{"size", collie::to_str(request.content().size())});
        turbo::Time cs = turbo::Time::from_time_t(request.time());
        result_table.add_row(collie::table::Table::Row_t{"time", turbo::Time::format(cs)});
        result_table.add_row(collie::table::Table::Row_t{"content", request.content()});
        ss.add_table("result", std::move(result_table), true);
    }

    void ConfigCmd::run_config_list_cmd() {
        if (!ConfigOptionContext::get_instance()->config_name.empty()) {
            run_config_version_list_cmd();
            return;
        }

        ScopeShower ss;
        easdk::Config client;
        auto rs = client.initialize({turbo::get_flag(FLAGS_sirius_router)});
        RPC_INIT_ERROR_RETURN_OR_OK(ss, rs);
        std::vector<eapi::sirius::ConfigInfo> response;
        rs = client.list_config(response);
        RPC_ERROR_RETURN_OR_OK(ss, rs);
        auto table = show_query_ops_config_list_response(response);
        ss.add_table("summary", std::move(table), true);
    }

    void ConfigCmd::run_config_version_list_cmd() {

        ScopeShower ss;
        easdk::Config client;
        auto rs = client.initialize({turbo::get_flag(FLAGS_sirius_router)});
        RPC_INIT_ERROR_RETURN_OR_OK(ss, rs);
        std::vector<eapi::sirius::ConfigInfo> response;
        rs = client.list_config(ConfigOptionContext::get_instance()->config_name, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs);
        auto table = show_query_ops_config_list_response(response);
        ss.add_table("summary", std::move(table), true);
    }

    void ConfigCmd::run_config_get_cmd() {
        if (ConfigOptionContext::get_instance()->config_version.empty()) {
            run_config_version_list_cmd();
            return;
        }

        ScopeShower ss("get config info");
        easdk::Config client;
        auto rs = client.initialize({turbo::get_flag(FLAGS_sirius_router)});
        RPC_INIT_ERROR_RETURN_OR_OK(ss, rs);
        eapi::sirius::ConfigInfo response;
        rs = client.get_config(ConfigOptionContext::get_instance()->config_name,
                               ConfigOptionContext::get_instance()->config_version, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs);
        auto table = show_query_ops_config_list_response(std::vector<eapi::sirius::ConfigInfo>{response});
        ss.add_table("summary", std::move(table), true);
    }

    void ConfigCmd::run_config_remove_cmd() {
        ScopeShower ss;
        easdk::Config client;
        auto rs = client.initialize({turbo::get_flag(FLAGS_sirius_router)});
        RPC_INIT_ERROR_RETURN_OR_OK(ss, rs);
        if(ConfigOptionContext::get_instance()->config_version.empty()){
            rs = client.remove_config_all_version(ConfigOptionContext::get_instance()->config_name);
        } else {
            rs = client.remove_config(ConfigOptionContext::get_instance()->config_name,
                                           ConfigOptionContext::get_instance()->config_version);
        }
        RPC_ERROR_RETURN_OR_OK(ss, rs);
    }

    [[nodiscard]] turbo::Status
    ConfigCmd::make_example_config_dump(eapi::sirius::ConfigInfo *req) {
        req->set_name("example");
        req->set_time(static_cast<int>(turbo::Time::current_seconds()));
        req->set_type(eapi::sirius::CF_JSON);
        auto v = req->mutable_version();
        v->set_major(1);
        v->set_minor(2);
        v->set_patch(3);

        nlohmann::json json_content;
        json_content["servlet"] = "sug";
        json_content["zone"]["name"] = "ea_search";
        json_content["zone"]["user"] = "jeff";
        json_content["zone"]["instance"] = {"192.168.1.2", "192.168.1.3", "192.168.1.3"};
        nlohmann::to_string(json_content);
        req->set_content(nlohmann::to_string(json_content));
        std::cout << json_content << std::endl;
        return turbo::OkStatus();
    }

    collie::table::Table
    ConfigCmd::show_query_ops_config_list_response(const std::vector<eapi::sirius::ConfigInfo> &res) {
        collie::table::Table result;
        auto &config_list = res;
        result.add_row(collie::table::Table::Row_t{"config size", collie::to_str(config_list.size())});
        auto last = result.size() - 1;
        result[last].format().font_color(collie::Color::green);
        result.add_row(collie::table::Table::Row_t{"number", "config"});
        last = result.size() - 1;
        result[last].format().font_color(collie::Color::green);
        int i = 0;
        std::vector<eapi::sirius::ConfigInfo> sorted_list;
        for (auto &ns: config_list) {
            sorted_list.push_back(ns);
        }
        auto less_fun = [](const eapi::sirius::ConfigInfo &lhs, const eapi::sirius::ConfigInfo &rhs) -> bool {
            return std::less()(lhs.name(), rhs.name());
        };
        std::sort(sorted_list.begin(), sorted_list.end(), less_fun);
        for (auto &ns: sorted_list) {
            result.add_row(collie::table::Table::Row_t{collie::to_str(i++), ns.name()});
            last = result.size() - 1;
            result[last].format().font_color(collie::Color::yellow);

        }
        return result;
    }

    void ConfigCmd::run_config_watch_cmd() {
        auto opt = ConfigOptionContext::get_instance();
        easdk::ConfigWatcher watcher;
        auto rs = watcher.initialize({turbo::get_flag(FLAGS_sirius_router)});
        if(!rs.ok()){
            collie::println("watch error:{}", rs.to_string());
            return;
        }
        if (opt->clean_local) {
            collie::println("remove local config cache dir:{}", opt->config_watch_dir);
            (void)alkaid::remove_all(opt->config_watch_dir);
        }

        if (!alkaid::exists(opt->config_watch_dir).value()) {
            (void)alkaid::create_directories(opt->config_watch_dir);
        }
        if (!rs.ok()) {
            collie::println("watch error:{}", rs.to_string());
        }

        auto new_config_func = [&watcher](const easdk::ConfigCallbackData &data) -> void {
            auto opt = ConfigOptionContext::get_instance();
            auto rs = save_config_to_file(opt->config_watch_dir, data);
            if (rs.ok()) {
                collie::println(collie::Color::green, "on new config:{} version:{}.{}.{} type:{}", data.config_name,
                                data.new_version.major,
                                data.new_version.minor, data.new_version.patch, data.type);
            } else {
                collie::println("{}", rs.to_string());
            }
            rs = watcher.apply(data.config_name, data.new_version);
            if (rs.ok()) {
                collie::println(collie::Color::green, "apply new config:{} version:{}.{}.{} type:{}", data.config_name,
                                data.new_version.major,
                                data.new_version.minor, data.new_version.patch, data.type);
            } else {
                collie::println("{}", rs.to_string());
            }
        };
        auto new_version_func = [&watcher](const easdk::ConfigCallbackData &data) -> void {
            auto opt = ConfigOptionContext::get_instance();
            auto rs = save_config_to_file(opt->config_watch_dir, data);
            if (rs.ok()) {
                collie::println(collie::Color::green, "on new config version:{} version:{}.{}.{} type:{}",
                                data.config_name, data.new_version.major,
                                data.new_version.minor, data.new_version.patch, data.type);
            } else {
                collie::println("{}", rs.to_string());
            }
            rs = watcher.apply(data.config_name, data.new_version);
            if (rs.ok()) {
                collie::println(collie::Color::green, "apply new config version:{} version:{}.{}.{} type:{}",
                                data.config_name, data.new_version.major,
                                data.new_version.minor, data.new_version.patch, data.type);
            } else {
                collie::println("{}", rs.to_string());
            }
        };
        easdk::ConfigEventListener listener{new_config_func, new_version_func};
        for (auto &it: opt->config_watch_list) {
            rs = watcher.watch_config(it, listener);
            if (!rs.ok()) {
                collie::println(collie::Color::red, "{}", rs.to_string());
            }
        }
        while (1) {
            sleep(1);
        }
    }

    turbo::Status
    ConfigCmd::save_config_to_file(const std::string &basedir, const easdk::ConfigCallbackData &data) {
        std::string file_name = collie::format("{}/{}-{}.{}.{}.{}", basedir, data.config_name, data.new_version.major,
                                               data.new_version.minor, data.new_version.patch, data.type);
        if (alkaid::exists(file_name).value()) {
            return turbo::already_exists_error(turbo::substitute("write file [$0] already exists", file_name));
        }
        auto lfs = alkaid::Filesystem::localfs();
        auto rs = lfs->write_file(file_name, data.new_content);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }
}  // namespace easdk::cli
