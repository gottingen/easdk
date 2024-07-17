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

#pragma once

#include <easdk/kumo/kumo_client_define.h>
#include <stack>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace kumo::client {

    class BaseOp {
    public:
        virtual uint32_t operator()(const uint32_t &left, const uint32_t &right) = 0;
    };

    class BAndOp : public BaseOp {
    public:
        virtual uint32_t operator()(const uint32_t &left, const uint32_t &right) {
            return left & right;
        }
    };

    class BOrOp : public BaseOp {
    public:
        virtual uint32_t operator()(const uint32_t &left, const uint32_t &right) {
            return left | right;
        }
    };

    class RShiftOp : public BaseOp {
    public:
        virtual uint32_t operator()(const uint32_t &left, const uint32_t &right) {
            return left >> right;
        }
    };

    class LShiftOp : public BaseOp {
    public:
        virtual uint32_t operator()(const uint32_t &left, const uint32_t &right) {
            return left << right;
        }
    };

    class ModOp : public BaseOp {
    public:
        virtual uint32_t operator()(const uint32_t &left, const uint32_t &right) {
            return left % right;
        }
    };

    class PlusOp : public BaseOp {
    public:
        virtual uint32_t operator()(const uint32_t &left, const uint32_t &right) {
            return left + right;
        }
    };

    class MinusOp : public BaseOp {
    public:
        virtual uint32_t operator()(const uint32_t &left, const uint32_t &right) {
            return left - right;
        }
    };

    class MultiplyOp : public BaseOp {
    public:
        virtual uint32_t operator()(const uint32_t &left, const uint32_t &right) {
            return left * right;
        }
    };

    class DivOp : public BaseOp {
    public:
        virtual uint32_t operator()(const uint32_t &left, const uint32_t &right) {
            return left / right;
        }
    };

    typedef std::shared_ptr<BaseOp> SmartOp;

    // 单例模式
    class ShardOperatorMgr {
    public:

        static ShardOperatorMgr *get_s_instance();

        static void destory_s_instance();

        int evaluate(const std::vector<std::string> &expression, uint32_t arg, uint32_t *result);

        int split(const std::string &expression, std::vector<std::string> &res);

    private:

        ShardOperatorMgr() {}

        void _init();

        SmartOp _get_operator(const std::string &token);

        int _get_priority(const std::string &op);

        int _compute(std::stack<uint32_t> &data, std::stack<std::string> &op);

    private:

        // 创建单例类
        static ShardOperatorMgr *_s_instance;
        static std::mutex _s_singleton_lock;
        std::unordered_map<std::string, SmartOp> _str_op_map;
        std::unordered_map<std::string, int> _str_priority_map;
    };

}  // namespace kumo::client

