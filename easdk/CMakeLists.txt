#
# Copyright 2023 The titan-search Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
###########################################################################
# below are examples, after initialization, modify it to your own.
###########################################################################
###carbin_example

file(GLOB_RECURSE EASDK_UTILITY_SRCS "utility/*.cc")

carbin_cc_object(
        NAMESPACE easdk
        NAME utility_obj
        SOURCES
        ${EASDK_UTILITY_SRCS}
        CXXOPTS
        ${CARBIN_CXX_OPTIONS}
)

file(GLOB_RECURSE SIRIUS_SDK_SRCS "sirius/*.cc")
carbin_cc_object(
        NAMESPACE easdk
        NAME sirius_obj
        SOURCES
        ${SIRIUS_SDK_SRCS}
        CXXOPTS
        ${CARBIN_CXX_OPTIONS}
)

file(GLOB_RECURSE SIRIUS_CLI_SRCS "api/*.cc")

carbin_cc_object(
        NAMESPACE easdk
        NAME api_obj
        SOURCES
        ${SIRIUS_CLI_SRCS}
        CXXOPTS
        ${CARBIN_CXX_OPTIONS}
)

carbin_cc_library(
        NAMESPACE easdk
        NAME easdk
        OBJECTS utility_obj sirius_obj api_obj
        CXXOPTS
        ${CARBIN_CXX_OPTIONS}
        PLINKS
        ${CARBIN_DEPS_LINK}
        PUBLIC
)

file(GLOB_RECURSE EASDK_CLI_SRCS "cli/*.cc")
carbin_cc_binary(
        NAMESPACE easdk
        NAME eacli
        SOURCES
        ${EASDK_CLI_SRCS}
        CXXOPTS
        ${CARBIN_CXX_OPTIONS}
        LINKS
        easdk::easdk
        ${CARBIN_DEPS_LINK}
        PUBLIC
)


###carbin_example