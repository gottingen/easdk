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

#include <turbo/flags/app.h>
#include <easdk/cli/sirius/option_context.h>
#include <easdk/sirius/sirius_sender.h>
#include <collie/strings/format.h>
#include <easdk/cli/sirius/config_cmd.h>
#include <easdk/cli/sirius/sns_cmd.h>
#include <easdk/cli/sirius/flags.h>

int main(int argc, char **argv) {
    turbo::cli::App app{"EA sirius client"};
    auto opt = easdk::cli::OptionContext::get_instance();
    app.add_flag("-V, --verbose", opt->verbose, "verbose detail message default(false)")->default_val(false);
    app.add_option("-d,--discovery_server", FLAGS_sirius_router, "server address default(\"127.0.0.1:8010\")")->default_val(
            "127.0.0.1:8010");
    app.add_flag("-r,--router", opt->router, "server address default(false)")->default_val(false);
    app.add_option("-T,--timeout", opt->timeout_ms, "timeout ms default(2000)");
    app.add_option("-C,--connect", opt->connect_timeout_ms, "connect timeout ms default(100)");
    app.add_option("-R,--retry", opt->max_retry, "max try time default(3)");
    app.add_option("-I,--interval", opt->time_between_discovery_connect_error_ms,
                   "time between discovery server connect error ms default(1000)");
    app.callback([&app] {
        if (app.get_subcommands().empty()) {
            collie::println("{}", app.help());
        }
    });

    // Call the setup functions for the subcommands.
    // They are kept alive by a shared pointer in the
    // lambda function
    easdk::cli::ConfigCmd::setup_config_cmd(app);
    easdk::cli::SnsCmd::setup_sns_cmd(app);
    // More setup if needed, i.e., other subcommands etc.

    TURBO_APP_PARSE(app, argc, argv);

    return 0;
}
