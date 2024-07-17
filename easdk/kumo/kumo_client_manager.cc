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


#include <easdk/kumo/kumo_client_manager.h>
#include <easdk/kumo/kumo_client_define.h>
#include <easdk/kumo/kumo_client_sns_connection_pool.h>


using std::string;
using std::map;
using std::vector;

namespace kumo::client {

    void *healthy_check_thread(void *pd) {
        CLIENT_WARNING("healthy check thread start");
        Manager *mg = static_cast<Manager *>(pd);
        if (mg == nullptr) {
            CLIENT_FATAL("Manager is nullptr");
            return nullptr;
        }
        int seconds = mg->get_healthy_check_seconds();
        uint64_t count = 0;
        int time = 0;
        bool need_detect_dead = false;
        while (mg->get_healthy_check_run()) {
            //进行健康检查操作
            if (time >= seconds * 1000 / 500) {
                need_detect_dead = true;
                CLIENT_WARNING("start healthy_check_thread, manager_name:%s, count:%llu",
                               mg->get_manager_name().c_str(), count + 1);
                time = 0;
                ++count;
            } else {
                need_detect_dead = false;
                ++time;
            }
            fiber_usleep(500000); //500毫秒
            mg->healthy_check_function(need_detect_dead);
        }
        return nullptr;
    }

    void *hang_check_thread(void *pd) {
        Manager *mg = static_cast<Manager *>(pd);
        if (mg == nullptr) {
            CLIENT_FATAL("Manager is nullptr");
            return nullptr;
        }
        int count = 0;
        while (mg->get_hang_check_run()) {
            fiber_usleep(1000000);
            TimeCost time_cost;
            mg->hang_check_function();
            CLIENT_WARNING("start hang_check_thread, manager_name:%s, count:%llu, time_cost:%lu",
                           mg->get_manager_name().c_str(), count, time_cost.get_time());
            ++count;
        }
        return nullptr;
    }

    void *bns_syn_thread(void *pd) {
        CLIENT_WARNING("sns syn thread start");
        Manager *mg = static_cast<Manager *>(pd);
        if (mg == nullptr) {
            CLIENT_FATAL("Manager is nullptr");
            return nullptr;
        }
        int seconds = mg->get_bns_syn_seconds();
        uint64_t count = 0;
        while (mg->get_bns_syn_run()) {
            int time = 0;
            while (time < seconds) {
                if (!mg->get_bns_syn_run()) {
                    break;
                }
                fiber_usleep(1000000);
                ++time;
            }
            CLIENT_WARNING("start sns_syn_thread, manager_name:%s, count:%llu",
                           mg->get_manager_name().c_str(), count);
            //进行健康检查操作，先等待一段时间后再检查
            mg->bns_syn_function();
            ++count;
        }
        return nullptr;
    }

    void *event_loop_thread(void *arg) {
        EpollServer *_epoll = static_cast<EpollServer *>(arg);
        _epoll->start_server();
        return nullptr;
    }

    Manager::Manager() :
            _hang_check(true),
            _manager_name("default"),
            _is_init_ok(false),
            _healthy_check_seconds(0),
            _single_healthy_check_thread(false),
            _healthy_check_run(false),
            _healthy_check_tid(0),
            _bns_syn_seconds(0),
            _single_bns_syn_thread(false),
            _bns_syn_run(false),
            _bns_syn_tid(0),
            _single_hang_check_thread(false),
            _hang_check_run(false),
            _hang_check_tid(0),
            _single_event_loop_thread(false),
            _event_loop_run(false),
            _event_loop_tid(0),
            _use_epoll(false),
            _no_permission_wait_s(0),
            _faulty_exit(true),
            _async(false) {}

    int Manager::init(const Option &option) {
        _healthy_check_seconds = option.healthy_check_secs;
        _use_epoll = option.use_epoll;
        _hang_check = option.hang_check;
        _faulty_exit = option.faulty_exit;
        _async = option.async;
        _no_permission_wait_s = option.no_permission_wait_s;

        _epoll = nullptr;
        if (_use_epoll) {
            CLIENT_WARNING("use epoll event loop");
            _epoll = EpollServer::get_instance();
            if (!_epoll->init()) {
                CLIENT_FATAL("create epoll server failed");
                return -1;
            }
            int ret = _start_event_loop_thread();
            if (ret < 0) {
                CLIENT_WARNING("start event loop thread failed");
                return ret;
            }
        } else {
            CLIENT_WARNING("use bthread default event loop");
        }
        if (mysql_library_init(0, nullptr, nullptr)) {
            CLIENT_WARNING("init mysql library fail");
            return CONFPARAM_ERROR;
        }
        // 初始化分库分表计算管理器
        ShardOperatorMgr::get_s_instance();
        if (option.services.empty()) {
            CLIENT_WARNING("this is a empty configure file ,no service");
            return CONFPARAM_ERROR;
        }

        // 加载每个service配置
        int idx = 0;
        for (auto &service: option.services) {
            int ret = _load_service_conf(service);
            if (ret < 0) {
                CLIENT_WARNING("load %dth service conf error", idx++);
                return ret;
            }
        }

        int ret = 0;
        if (_healthy_check_seconds > 0) {
            ret = _start_healthy_check_thread();
            if (ret < 0) {
                CLIENT_WARNING("start healthy check thread failed");
                return ret;
            }
        }

        if (_bns_syn_seconds > 0) {
            ret = _start_bns_syn_thread();
            if (ret < 0) {
                CLIENT_WARNING("start sns syn thread failed");
                return ret;
            }
        }
        if (_hang_check) {
            ret = _start_hang_check_thread();
            if (ret < 0) {
                CLIENT_WARNING("start hang check thread failed");
                return ret;
            }
        } else {
            CLIENT_WARNING("hang check is false");
        }
        //随机种子，全局初始化一次
        srand(time(0));
        _is_init_ok = true;
        CLIENT_NOTICE("manager init successfully, manager_name:%s", _manager_name.c_str());
        return SUCCESS;
    }

    int Manager::query(uint32_t partition_key, const std::string &sql, ResultSet *result) {
        if (_db_service_map.size() != 1) {
            CLIENT_WARNING("service num is not one, this method cann't be called, service num:%d",
                           _db_service_map.size());
            return GET_SERVICE_FAIL;
        }
        string name = _db_service_map.begin()->first;
        Service *service = get_service(name);
        if (service == nullptr) {
            CLIENT_WARNING("get service in query fail");
            return GET_SERVICE_FAIL;
        }
        return service->query(partition_key, sql, result);
    }

    std::string Manager::get_manager_name() {
        return _manager_name;
    }

    Service *Manager::get_service(const string &service_name) {
        if (!_is_init_ok) {
            CLIENT_WARNING("Manager is not inited");
            return nullptr;
        }
        map<string, Service *>::iterator iter = _db_service_map.find(service_name);
        if (iter == _db_service_map.end()) {
            CLIENT_WARNING("input service is not exist, service_name:%s", service_name.c_str());
            return nullptr;
        }
        return iter->second;
    }

    void Manager::bns_syn_function() {

    }

    void Manager::healthy_check_function(bool need_detect_dead) {
        //对每个servie做循环
        map<string, Service *>::iterator db_service_it = _db_service_map.begin();
        for (; db_service_it != _db_service_map.end(); ++db_service_it) {
            //每个service的bns 存储在map<unsigned, SnsConnectionPool*>
            Service *service = db_service_it->second;
            if (!service->is_inited()) {
                continue;
            }
            map<int, SnsConnectionPool *> id_bns_map = service->get_id_bns_map();
            map<int, SnsConnectionPool *>::iterator pool_it = id_bns_map.begin();
            //对sevice里的每个bns做循环
            for (; pool_it != id_bns_map.end(); ++pool_it) {
                int ret = pool_it->second->healthy_check(need_detect_dead);
                if (ret < 0) {
                    CLIENT_WARNING("sns pool healthy check fail, service_name: %s, bns_name: %s",
                                   service->get_name().c_str(), pool_it->second->get_name().c_str());
                }
            }
        }
    }

    void Manager::hang_check_function() {
        //对每个servie做循环
        map<string, Service *>::iterator db_service_it = _db_service_map.begin();
        for (; db_service_it != _db_service_map.end(); ++db_service_it) {
            //每个service的bns 存储在map<unsigned, SnsConnectionPool*>
            Service *service = db_service_it->second;
            if (!service->get_hang_check()) {
                continue;
            }
            map<int, SnsConnectionPool *> id_bns_map = service->get_id_bns_map();
            map<int, SnsConnectionPool *>::iterator pool_it = id_bns_map.begin();
            //对sevice里的每个bns做循环
            for (; pool_it != id_bns_map.end(); ++pool_it) {
                int ret = pool_it->second->hang_check();
                if (ret < 0) {
                    CLIENT_WARNING("sns pool hang check fail, service_name: %s, bns_name: %s",
                                   service->get_name().c_str(), pool_it->second->get_name().c_str());
                }
            }
        }
    }

    Manager::~Manager() {
        this->_clear();
    }

    int Manager::get_healthy_check_seconds() const {
        return _healthy_check_seconds;
    }

    int Manager::get_bns_syn_seconds() const {
        return _bns_syn_seconds;
    }

    bool Manager::get_healthy_check_run() const {
        return _healthy_check_run;
    }

    bool Manager::get_bns_syn_run() const {
        return _bns_syn_run;
    }

    bool Manager::get_hang_check_run() const {
        return _hang_check_run;
    }

    int Manager::_load_service_conf(const Service::Option &option) {
        const string &service_name = option.service_name;
        // service_name 不能重复
        if (_db_service_map.count(service_name) != 0) {
            CLIENT_WARNING("service configuration is repeat, service_name:%s",
                           service_name.c_str());
            return CONFPARAM_ERROR;
        }
        Service *service = new(std::nothrow) Service(_bns_infos,
                                                     _no_permission_wait_s, _faulty_exit, _async);
        if (nullptr == service) {
            CLIENT_WARNING("New ProxyService error");
            return NEWOBJECT_ERROR;
        }
        _db_service_map[service_name] = service;
        int ret = service->init(option);
        if (ret < 0) {
            CLIENT_WARNING("service init failed, service name: %s", service_name.c_str());
            return ret;
        }
        return SUCCESS;
    }

    void Manager::_clear() {
        CLIENT_WARNING("start clearn manager resource");
        _healthy_check_run = false;
        _bns_syn_run = false;
        _hang_check_run = false;
        _join_healthy_check_thread();
        _join_bns_syn_thread();
        _join_hang_check_thread();
        _join_event_loop_thread();
        _single_healthy_check_thread = false;
        _single_bns_syn_thread = false;
        _single_hang_check_thread = false;
        _single_event_loop_thread = false;
        map<string, Service *>::iterator iter = _db_service_map.begin();
        for (; iter != _db_service_map.end(); ++iter) {
            delete iter->second;
            iter->second = nullptr;
        }
        _db_service_map.clear();
        map<string, BnsInfo *>::iterator bns_iter = _bns_infos.begin();
        for (; bns_iter != _bns_infos.end(); ++bns_iter) {
            delete bns_iter->second;
            bns_iter->second = nullptr;
        }
        _bns_infos.clear();
        mysql_library_end();
        CLIENT_WARNING("clearn manager resource end");
    }

    int Manager::_start_healthy_check_thread() {
        if (!_single_healthy_check_thread) {
            _healthy_check_run = true;
            const fiber_attr_t attr = FIBER_ATTR_NORMAL;
            int ret = fiber_start_background(&_healthy_check_tid, &attr, healthy_check_thread, this);
            if (ret != 0) {
                CLIENT_WARNING("Start new healthy check thread failed!");
                _healthy_check_run = false;
                return THREAD_START_ERROR;
            }
            _single_healthy_check_thread = true;
            return SUCCESS;
        }
        CLIENT_WARNING("Another healthy check thread is already started");
        return THREAD_START_REPEAT;
    }

    int Manager::_start_bns_syn_thread() {
        if (!_single_bns_syn_thread) {
            _bns_syn_run = true;
            const fiber_attr_t attr = FIBER_ATTR_NORMAL;
            int ret = fiber_start_background(&_bns_syn_tid, &attr, bns_syn_thread, this);
            if (ret != 0) {
                CLIENT_WARNING("Start new sns syn thread failed!");
                _bns_syn_run = false;
                return THREAD_START_ERROR;
            }
            _single_bns_syn_thread = true;
            return SUCCESS;
        }
        CLIENT_WARNING("Another sns syn thread is already started");
        return THREAD_START_REPEAT;
    }

    int Manager::_start_hang_check_thread() {
        if (!_single_hang_check_thread) {
            _hang_check_run = true;
            const fiber_attr_t attr = FIBER_ATTR_NORMAL;
            int ret = fiber_start_background(&_hang_check_tid, &attr, hang_check_thread, this);
            if (ret != 0) {
                CLIENT_WARNING("Start new hang check thread failed!");
                _hang_check_run = false;
                return THREAD_START_ERROR;
            }
            _single_hang_check_thread = true;
            return SUCCESS;
        }
        CLIENT_WARNING("Another hang check thread is already started");
        return THREAD_START_REPEAT;
    }

    int Manager::_start_event_loop_thread() {
        if (!_single_event_loop_thread) {
            _event_loop_run = true;
            const fiber_attr_t attr = FIBER_ATTR_NORMAL;
            int ret = fiber_start_background(&_event_loop_tid, &attr, event_loop_thread, _epoll);
            if (ret != 0) {
                CLIENT_WARNING("Start new event loop thread failed!");
                _event_loop_run = false;
                return THREAD_START_ERROR;
            }
            _single_event_loop_thread = true;
            return SUCCESS;
        }
        CLIENT_WARNING("Another event loop thread is already started");
        return THREAD_START_REPEAT;
    }

    void Manager::_join_healthy_check_thread() {
        if (_single_healthy_check_thread) {
            fiber_join(_healthy_check_tid, nullptr);
            _single_healthy_check_thread = false;
            CLIENT_WARNING("join healthy check thread successfully");
        }
    }

    void Manager::_join_bns_syn_thread() {
        if (_single_bns_syn_thread) {
            fiber_join(_bns_syn_tid, nullptr);
            _single_bns_syn_thread = false;
            CLIENT_WARNING("join sns syn thread successfully");
        }
    }

    void Manager::_join_hang_check_thread() {
        if (_single_hang_check_thread) {
            fiber_join(_hang_check_tid, nullptr);
            _single_hang_check_thread = false;
            CLIENT_WARNING("join hang check thread successfully");
        }
    }

    void Manager::_join_event_loop_thread() {
        if (_single_event_loop_thread) {
            _epoll->set_shutdown();
            fiber_join(_event_loop_tid, nullptr);
            _single_event_loop_thread = false;
            CLIENT_WARNING("join event loop thread successfully");
        }
    }
}  // namespace kumo::client
