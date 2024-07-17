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

#undef CR

#include <shared_mutex>
#include <vector>
#include <string>
#include <map>
#include "melon/fiber/butex.h"
#include <melon/fiber/fiber.h>
#include <turbo/log/logging.h>
#include <cinttypes>

namespace kumo::client {

    //从bns服务同步得到的每个实例信息
    struct InstanceInfo {
        std::string id; // id:port
        std::string ip;
        std::string tag;
        int port;
        bool enable; //干预状态
        int status; //非干预状态
        bool is_available; //干预状态、非干预状态同是为0时，is_avalibale才有效，为true
    };

    //每个bns信息
    struct BnsInfo {
        std::map<std::string, InstanceInfo> instance_infos; //key为ip:port
        std::string bns_name;
        std::mutex mutex_lock;//对instance_infos的修改操作需要加互斥锁
    };

    enum ConnectionType {
        NONE = 0,
        MYSQL_CONN = 1
    };
    //选择实例算法， 目前支持随机和轮询两种
    enum SelectAlgo {
        RANDOM = 1,
        ROLLING = 2,
        LOCAL_AWARE = 3
    };
    //从配置文件读取的连接的参数信息
    struct ConnectionConf {
        ConnectionType conn_type;
        int read_timeout;
        int write_timeout;
        int connect_timeout;
        std::string username;
        std::string password;
        std::string charset;
        int no_permission_wait_s;
        bool faulty_exit;
        bool async;
    };

    struct TableSplit {
        std::string table_name;
        int table_count; //分表的数量
        std::vector<std::string> table_split_function;

        TableSplit(const std::string &name, int count, const std::vector<std::string> &function) :
                table_name(name),
                table_count(count),
                table_split_function(function) {}
    };

    enum InstanceStatus {
        ON_LINE = 1,
        OFF_LINE = 2,
        FAULTY = 3,
        DELAY = 4
    };

    enum ConnectionStatus {
        IS_NOT_CONNECTED = 1,
        IS_NOT_USED = 2,
        IS_USING = 3,
        IS_CHECKING = 4,
        IS_BAD = 5
    };

    //数据类型
    enum ValueType {
        VT_INT32 = 1,
        VT_UINT32 = 2,
        VT_INT64 = 3,
        VT_UINT64 = 4,
        VT_FLOAT = 5,
        VT_DOUBLE = 6,
        VT_STRING = 7,
        ERROR_TYPE = -1
    };
    /**
     * 系统错误码
     * */
    enum ErrorCode {
        SUCCESS = 0,
        INPUTPARAM_ERROR = -1,
        FILESYSTEM_ERROR = -2,
        CONFPARAM_ERROR = -3,
        NEWOBJECT_ERROR = -4,
        GETSERVICE_ERROR = -5,
        COMPUTE_ERROR = -6, // 分库分表号计算失败
        SERVICE_NOT_INIT_ERROR = -7,
        INSTANCE_NOT_ENOUGH = -8,
        INSTANCE_ALREADY_DELETED = -9,
        GET_VALUE_ERROR = -10,
        VALUE_IS_NULL = -11,
        FETCH_CONNECT_FAIL = -12,
        GET_SERVICE_FAIL = -13,
        EXECUTE_FAIL = -14,
        SERVICE_INIT2_ERROR = -15,
        //sns syn eror -100~-200
        BNS_ERROR = -101,
        //BNS_INSTANCE_REPEAT = -102,
        BNS_NO_ONLINE_INSTANCE = -102,
        BNS_GET_INFO_ERROR = -103,
        //connection error -200~-300
        CONNECTION_CONNECT_FAIL = -201,
        CONNECTION_NOT_INIT = -202,
        CONNECTION_PING_FAIL = -203,
        CONNECTION_HANDLE_NULL = -204,
        INSTANCE_FAULTY_ERROR = -205,
        CONNECTION_ALREADY_DELETED = -206,
        NO_PARTITION_KEY_ERROR = -207,
        CONNECTION_QUERY_FAIL = -208,
        CONNECTION_RECONN_SUCCESS = -209,
        CONNECTION_IS_KILLED = -210,
        //thread error -300~-400
        THREAD_START_ERROR = -301,
        THREAD_START_REPEAT = -302,
        THREAD_FINAL_ERROR = -303
    };

    class BthreadCond {
    public:
        BthreadCond() {
            fiber_cond_init(&_cond, nullptr);
            fiber_mutex_init(&_mutex, nullptr);
            _count = 1;
        }

        ~BthreadCond() {
            fiber_mutex_destroy(&_mutex);
            fiber_cond_destroy(&_cond);
        }

        void init(int count = 1) {
            _count = count;
        }

        void reset() {
            _count = 1;
            fiber_mutex_destroy(&_mutex);
            fiber_cond_destroy(&_cond);

            fiber_cond_init(&_cond, nullptr);
            fiber_mutex_init(&_mutex, nullptr);
        }

        int signal() {
            int ret = 0;
            fiber_mutex_lock(&_mutex);
            _count--;
            fiber_cond_signal(&_cond);
            fiber_mutex_unlock(&_mutex);
            return ret;
        }

        int wait() {
            int ret = 0;
            fiber_mutex_lock(&_mutex);
            while (_count > 0) {
                ret = fiber_cond_wait(&_cond, &_mutex);
                //CWARNING_LOG("bthread signal wait return: %d", ret);
            }
            fiber_mutex_unlock(&_mutex);
            return ret;
        }

    private:
        int _count;
        fiber_cond_t _cond;
        fiber_mutex_t _mutex;
    };

    class TimeCost {
    public:
        TimeCost() {
            gettimeofday(&_start, nullptr);
        }

        ~TimeCost() {}

        void reset() {
            gettimeofday(&_start, nullptr);
        }

        int64_t get_time() {
            gettimeofday(&_end, nullptr);
            return (_end.tv_sec - _start.tv_sec) * 1000000
                   + (_end.tv_usec - _start.tv_usec); //macro second
        }

    private:
        timeval _start;
        timeval _end;
    };

    struct MysqlEventInfo {
        BthreadCond cond;
        TimeCost cost;
        int event_in;
        int event_out;
        int timeout;
    };

#define CLIENT_DEBUG(_fmt_, args...)


#define CLIENT_TRACE(_fmt_, args...) \
    do {\
        char buf[2048]; \
        snprintf(buf, sizeof(buf) - 1, "[%s][%d][%s][%" PRIu64 "]" _fmt_, \
                strrchr(__FILE__, '/') + 1, __LINE__, __FUNCTION__, fiber_self(), ##args);\
        LOG(INFO) << buf; \
    }while (0);


#define CLIENT_NOTICE(_fmt_, args...) \
    do {\
        char buf[2048]; \
        snprintf(buf, sizeof(buf) - 1, "[%s][%d][%s][%" PRIu64 "]" _fmt_, \
                strrchr(__FILE__, '/') + 1, __LINE__, __FUNCTION__, fiber_self(), ##args);\
        LOG(INFO) << buf; \
    }while (0);


#define CLIENT_WARNING(_fmt_, args...) \
    do {\
        char buf[2048]; \
        snprintf(buf, sizeof(buf) - 1, "[%s][%d][%s][%" PRIu64 "]" _fmt_, \
                strrchr(__FILE__, '/') + 1, __LINE__, __FUNCTION__, fiber_self(), ##args);\
        LOG(WARNING) << buf; \
    }while (0);


#define CLIENT_FATAL(_fmt_, args...) \
    do {\
        char buf[2048]; \
        snprintf(buf, sizeof(buf) - 1, "[%s][%d][%s][%" PRIu64 "]" _fmt_, \
                strrchr(__FILE__, '/') + 1, __LINE__, __FUNCTION__, fiber_self(), ##args);\
        LOG(ERROR) << buf; \
    }while (0);


}  // namespace kumo::client
