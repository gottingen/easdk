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
// Created by jeff on 23-11-30.
//
#include <easdk/cli/sirius/mock.h>
#include <easdk/cli/sirius/option_context.h>
#include <easdk/cli/sirius/show_help.h>
#include <collie/nlohmann/json.hpp>
#include <easdk/utility/utility.h>
#include <easdk/sirius/sirius_sender.h>
#include <easdk/utility/dumper.h>
#include <easdk/utility/loader.h>
#include <easdk/sirius/servlet_instance_builder.h>
#include <easdk/cli/sirius/app_cmd.h>
#include <easdk/cli/sirius/zone_cmd.h>
#include <easdk/cli/sirius/flags.h>
#include <easdk/cli/sirius/config_cmd.h>
#include <turbo/strings/numbers.h>
#include <melon/naming/sns_naming_service.h>
#include <turbo/flags/flag.h>
#include <turbo/flags/declare.h>
#include <collie/strings/format.h>

TURBO_DECLARE_FLAG(std::string, sns_server);
namespace easdk::cli {


    static int seq_id() {
        static std::atomic<int> seq{0};
        return seq++;
    }

    void DiscoveryCmd::setup_discovery_cmd(turbo::cli::App &app) {
        // Create the option and subcommand objects.
        auto opt = DiscoveryOptionContext::get_instance();
        auto *discovery_cmd = app.add_subcommand("mock", "discovery operations");
        discovery_cmd->callback([discovery_cmd]() { run_discovery_cmd(discovery_cmd); });

        auto dai = discovery_cmd->add_subcommand("mock", " mock servlet serving");
        auto *add_parameters_inputs = dai->add_option_group("parameters_inputs", "config input from parameters");
        auto *add_json_inputs = dai->add_option_group("json_inputs", "config input source from json format");
        add_parameters_inputs->add_option("-n,--app", opt->app_name, "app name")->required();
        add_parameters_inputs->add_option("-z,--zone", opt->zones, "zone name")->required();
        add_parameters_inputs->add_option("-s,--servlet", opt->servlet_name, "servlet name")->default_val("sorter");
        add_parameters_inputs->add_option("-a, --address", opt->address, "instance address")->default_val("192.168.8.6");
        add_parameters_inputs->add_option("-e, --env", opt->envs, "instance env");
        add_parameters_inputs->add_option("-c, --color", opt->colors, "instance color");
        add_parameters_inputs->add_option("-t, --status", opt->status, "instance color")->default_val("NORMAL");
        add_parameters_inputs->add_option("-f, --fiber", opt->fibers, "instance color")->default_val(50);
        add_json_inputs->add_option("-j, --json", opt->json_file, "json input file")->required(true);
        dai->require_option(1);
        dai->callback([]() { run_mock_instance_cmd(); });


        auto dd = discovery_cmd->add_subcommand("dump", " dump instance example to json file");
        dd->add_option("-o,--output", opt->dump_file, "dump file path")->default_val("example_discovery.json");
        dd->add_flag("-q,--quiet", opt->quiet, "quiet or print")->default_val(false);
        dd->callback([]() { run_discovery_dump_cmd(); });
        /// run after cmd line parse complete
    }

    /// The function that runs our code.
    /// This could also simply be in the callback lambda itself,
    /// but having a separate function is cleaner.
    void DiscoveryCmd::run_discovery_cmd(turbo::cli::App *app) {
        // Do stuff...
        if (app->get_subcommands().empty()) {
            collie::println("{}", app->help());
        }
    }


    static void* run_mock(bool& stop) {
        auto ctx = DiscoveryOptionContext::get_instance();

        auto opt = DiscoveryOptionContext::get_instance();
        melon::SnsPeer instance;
        instance.set_app_name(ctx->app_name);
        auto index = turbo::Uniform(opt->gen, 0u, opt->zones.size());

        instance.set_zone(ctx->zones[index]);
        auto i = seq_id();
        instance.set_servlet_name(ctx->servlet_name + collie::to_str(i));
        index = turbo::Uniform(opt->gen, 0u, opt->envs.size());
        instance.set_env(ctx->envs[index]);
        index = turbo::Uniform(opt->gen, 0u, ctx->colors.size());
        instance.set_color(ctx->colors[index]);
        instance.set_status(melon::PeerStatus::NORMAL);
        instance.set_address(ctx->address);
        melon::naming::SnsNamingClient sns_naming;
        auto rs = sns_naming.register_peer(instance);
        if (rs != 0) {
            collie::println("register server error");
            return nullptr;
        }

        collie::println("register server:{}.{}.{}#{} {} {} {} {}",
                        instance.app_name(), instance.zone(),
                        instance.servlet_name(), instance.address(),
                        instance.env(), instance.color(),
                        instance.mtime(), static_cast<int>(instance.status()));
        while (!stop) {
            fiber_usleep(3 *1000 * 1000);
        }
        return nullptr;
    };

    void DiscoveryCmd::run_mock_instance_cmd() {
        bool stop = false;
        auto ctx = DiscoveryOptionContext::get_instance();
        std::string sns_server =  easdk::make_list_naming({turbo::get_flag(FLAGS_sirius_router)});
        turbo::set_flag(&FLAGS_sns_server, sns_server);
        std::vector< melon::SnsPeer> instances(ctx->fibers);
        std::vector<std::unique_ptr<melon::naming::SnsNamingClient>> clients(ctx->fibers);
        for (size_t i = 0; i < ctx->fibers; i++) {
            auto &instance = instances[i];
            instance.set_app_name(ctx->app_name);
            auto index = turbo::Uniform(ctx->gen, 0u, ctx->zones.size());
            instance.set_zone(ctx->zones[index]);
            instance.set_servlet_name(ctx->servlet_name + collie::to_str(i));
            index = turbo::Uniform(ctx->gen, 0u, ctx->envs.size());
            instance.set_env(ctx->envs[index]);
            index = turbo::Uniform(ctx->gen, 0u, ctx->colors.size());
            instance.set_color(ctx->colors[index]);
            instance.set_status(melon::PeerStatus::NORMAL);
            instance.set_address(ctx->address);
            LOG(INFO)<< instance.ShortDebugString();
            clients[i] = std::make_unique<melon::naming::SnsNamingClient>();
            auto rs = clients[i]->register_peer(instance);
            if (rs != 0) {
                collie::println("register server error");
                return;
            }
        }
        size_t n = 1000;
        while (n > 0) {
            fiber_usleep(5* 1000 * 1000);
            n--;
        }
        stop = true;
        for (auto &client: clients) {
            client.reset();
        }
    }
    void DiscoveryCmd::run_discovery_dump_cmd() {
        eapi::sirius::ServletInfo instance;
        easdk::sirius::ServletInstanceBuilder builder(&instance);
        builder.set_namespace("ex_namespace")
                .set_zone("ex_zone")
                .set_servlet("ex_servlet")
                .set_env("ex_env")
                .set_color("green")
                .set_status("NORMAL")
                .set_address("127.0.0.1:12345");
        auto rs = easdk::Dumper::dump_proto_to_file(DiscoveryOptionContext::get_instance()->dump_file, instance);
        if (!rs.ok()) {
            collie::println("dump example discovery instance error:{}", rs.message());
            return;
        }

        if (!DiscoveryOptionContext::get_instance()->quiet) {
            std::string json_str;
            rs = easdk::Dumper::dump_proto(instance, json_str);
            if (!rs.ok()) {
                collie::println("dump example discovery instance error:{}", rs.message());
                return;
            }
            std::cout << ShowHelper::json_format(json_str) << std::endl;
        }

    }

    [[nodiscard]] turbo::Status
    DiscoveryCmd::make_discovery_info_instance(melon::SnsRequest *req) {
        auto opt = DiscoveryOptionContext::get_instance();
        req->set_app_name(opt->app_name);
        for(auto &zone: opt->zones) {
            req->add_zones(zone);
        }
        for(auto &env: opt->envs) {
            req->add_env(env);
        }
        for(auto &color: opt->colors) {
            req->add_color(color);
        }
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Result<int> DiscoveryCmd::string_to_status(const std::string &status) {
        int s;
        if(!turbo::simple_atoi(status, &s) ){
            return turbo::invalid_argument_error("unknown status");
        }
        return s;
    }
}  // namespace easdk::cli
