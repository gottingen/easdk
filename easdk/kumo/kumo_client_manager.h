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

#include <pthread.h>
#include <vector>
#include <string>
#include <cstddef>
#include <easdk/kumo/kumo_client_service.h>
#include <easdk/kumo/kumo_client_epoll.h>
#include <easdk/kumo/shard_operator_mgr.h>
#include <easdk/kumo/kumo_client_util.h>

namespace kumo::client {

    // @brief 健康检查线程
    void *healthy_check_thread(void *);

    // @brief bns同步线程
    void *bns_syn_thread(void *);

    // @brief 时间循环探测线程
    void *event_loop_thread(void *);

    void *hang_check_thread(void *);

    // @brief 控制类，单例模式
    class Manager {
    public:
        // @breif 构造函数
        Manager();

        struct Option {
            int healthy_check_secs = 0;
            bool use_epoll = false;
            bool hang_check = true;
            bool faulty_exit = true;
            bool async = false;
            int no_permission_wait_s = 0;

            std::vector<Service::Option> services;
        };

        int init(const Option &option);

        // @brief 若client只配置了一个service，可直接调用该接口执行sql语句
        int query(uint32_t partition_key, const std::string &sql, ResultSet *result);

        std::string get_manager_name();

        // @brief 根据客户端输入的db_name,得到相应的service
        // @return : - 非NULL：service_name对应的service
        //           - nullptr：service_name不存在或输入有错误
        Service *get_service(const std::string &service_name);

        // @brief 从bns服务同步得到每个实例的信息
        void bns_syn_function();

        // @brief 健康检查
        void healthy_check_function(bool need_detect_dead);

        void hang_check_function();

        // @brief join 健康检查线程和bns同步线程，在析构函数前调用
        ~Manager();

        int get_healthy_check_seconds() const;

        int get_bns_syn_seconds() const;

        int get_hang_check_seconds() const;

        bool get_healthy_check_run() const;

        bool get_bns_syn_run() const;

        bool get_hang_check_run() const;

    private:

        // @brief 从配置文件加载每个service的配置
        int _load_service_conf(const Service::Option &option);

        void _clear();

        int _start_healthy_check_thread();

        int _start_bns_syn_thread();

        int _start_event_loop_thread();

        int _start_hang_check_thread();

        void _join_healthy_check_thread();

        void _join_bns_syn_thread();

        void _join_event_loop_thread();

        void _join_hang_check_thread();

    private:
        bool _hang_check;
        std::string _manager_name;

        // service_name 与service的对应关系
        std::map<std::string, Service *> _db_service_map;

        // bns同步线程得到的数据, key值为bns_name
        std::map<std::string, BnsInfo *> _bns_infos;

        bool _is_init_ok;

        // 健康检查的时间间隔，单位为s
        int _healthy_check_seconds;
        bool _single_healthy_check_thread;
        bool _healthy_check_run;
        fiber_t _healthy_check_tid;

        // bns同步的时间间隔，单位为s
        int _bns_syn_seconds;
        bool _single_bns_syn_thread;
        bool _bns_syn_run;
        fiber_t _bns_syn_tid;

        //hang_check时间间隔
        bool _single_hang_check_thread;
        bool _hang_check_run;
        fiber_t _hang_check_tid;

        bool _single_event_loop_thread;
        bool _event_loop_run;
        fiber_t _event_loop_tid;
        bool _use_epoll;
        int _no_permission_wait_s;
        bool _faulty_exit;
        bool _async;

        EpollServer *_epoll;

        Manager(const Manager &);

        Manager &operator=(const Manager &);
    };
}  // namespace kumo::client
