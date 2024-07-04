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

#include <easdk/cli/sirius/proto_help.h>
#include <collie/strings/str_split.h>
#include <collie/strings/case_conv.h>
#include <collie/meta/reflect.h>
#include <turbo/strings/numbers.h>
#include <turbo/strings/substitute.h>

namespace easdk::cli {

    std::string config_type_to_string(eapi::sirius::ConfigType type) {
        switch (type) {
            case eapi::sirius::CF_JSON:
                return "json";
            case eapi::sirius::CF_TEXT:
                return "text";
            case eapi::sirius::CF_INI:
                return "ini";
            case eapi::sirius::CF_YAML:
                return "yaml";
            case eapi::sirius::CF_XML:
                return "xml";
            case eapi::sirius::CF_GFLAGS:
                return "gflags";
            case eapi::sirius::CF_TOML:
                return "toml";
            default:
                return "unknown format";
        }
    }

    turbo::Result<eapi::sirius::ConfigType> string_to_config_type(const std::string &str) {
        auto lc = collie::str_to_lower(str);
        if (lc == "json") {
            return eapi::sirius::CF_JSON;
        } else if (lc == "text") {
            return eapi::sirius::CF_TEXT;
        } else if (lc == "ini") {
            return eapi::sirius::CF_INI;
        } else if (lc == "yaml") {
            return eapi::sirius::CF_YAML;
        } else if (lc == "xml") {
            return eapi::sirius::CF_XML;
        } else if (lc == "gflags") {
            return eapi::sirius::CF_GFLAGS;
        } else if (lc == "toml") {
            return eapi::sirius::CF_TOML;
        }
        return turbo::invalid_argument_error("unknown format " + str);
    }

    turbo::Status string_to_version(const std::string &str, eapi::sirius::Version *v) {
        std::vector<std::string> vs = collie::str_split(str, ".");
        if (vs.size() != 3) {
            return turbo::invalid_argument_error(turbo::substitute("version $0 error, should be like 1.2.3", str));
        }
        int64_t m;
        if (!turbo::simple_atoi(vs[0], &m)) {
            return turbo::invalid_argument_error(turbo::substitute("version $0 error, should be like 1.2.3", str));
        }
        v->set_major(m);
        if (!turbo::simple_atoi(vs[1], &m)) {
            return turbo::invalid_argument_error(turbo::substitute("version $0 error, should be like 1.2.3", str));
        }
        v->set_minor(m);
        if (!turbo::simple_atoi(vs[2], &m)) {
            return turbo::invalid_argument_error(turbo::substitute("version $0 error, should be like 1.2.3", str));
        }
        v->set_patch(m);
        return turbo::OkStatus();
    }

    std::string version_to_string(const eapi::sirius::Version &v) {
        return turbo::substitute("$0.$1.$2", v.major(), v.minor(), v.patch());
    }


}  // namespace easdk::cli
