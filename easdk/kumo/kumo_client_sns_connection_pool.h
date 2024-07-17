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
#include <map>
#include <string>
#include <vector>
#include <atomic>
#include <easdk/kumo/kumo_client_logic_db.h>
#include <easdk/kumo/kumo_client_instance.h>
#include <easdk/kumo/shard_operator_mgr.h>

namespace kumo::client {

    class SnsConnectionPool {
    public:
        SnsConnectionPool(
                const std::string &sns_name,
                const std::string &tag,
                int total_conn_num,
                int max_connection_per_instance,
                int id,
                ConnectionConf conn_conf,
                std::map<std::string, LogicDB *>::iterator begin,
                std::map<std::string, LogicDB *>::iterator end,
                BnsInfo *bns_info_ptr,
                std::string comment_format,
                SelectAlgo select_algo);

        ~SnsConnectionPool();

        // @brief 初始化
        // 从bns服务获得可用的实例，建立实例，并在实例上创建连接
        // 支持ip list，如果直接传进来instance的list，则忽略bns用该list来初始化
        int init(bool connect_all, std::vector<InstanceInfo> *list);

        // @brief 从连接池中取连接
        //根据ip_port选择对应的instance，如果不存在或者实例故障，则随机选一个
        //算法rand()%在线实例的个数得到n，找到第n个在线且有空闲连接的实例(加bns的读锁)
        //在实例上找到空闲连接（对实例加读锁）
        //释放实例的读锁
        //释放bns的读锁
        SmartConnection fetch_connection(const std::string &ip_port);

        // @brief 得到分表id, 该方法供connection的query使用
        // @logic_db_name: 逻辑库名
        // @table_name: 表名
        // @table_split: 是否分表，若为true则分表，分表id为table_id
        // @return 返回0表示计算正确
        int get_table_id(
                const std::string &logic_db_name,
                const std::string &table_name,
                uint32_t partition_key,
                bool *table_split,
                int *table_id);


        //对每个bns做健康检查
        int healthy_check(bool need_detect_dead);

        int hang_check();

        // @brief 将conn_num个连接分配到该连接池的可用实例上
        int alloc_connection(int conn_num);

        std::string get_name() const;

        int get_online_instance_num();

        int get_total_conn_num() const;

        std::string get_comment_format() const;

        bool is_split_table() const;

        void print_conn_info();

        void update_parent_weight(int64_t diff, size_t index);

        void total_fetch_add(int64_t diff) {
            _total_weight.fetch_add(diff);
        }

        int64_t get_total_weight() {
            return _total_weight.load();
        }

        int32_t get_connection_num_per_instance() const {
            return _conf_connection_per_instance;
        }

        void set_is_used(bool is_used) {
            _is_used = is_used;
        }

        // 凤脉专用, 更具ip_port获得一个instance, 没有直接返回NULL
        Instance *select_instance(const std::string &ip_port);

    private:
        //根据ip_port选择对应的instance，如果不存在或者实例故障，则随机选一个
        Instance *_select_instance(const std::string &ip_port);

        Instance *_select_instance_random();

        Instance *_select_instance_rolling(bool whether_lower);

        Instance *_select_instance_local_aware();

        // @brief 释放所以占用的资源
        void _clear();

    private:

        std::map<std::string, Instance *> _instances_map; // key为ip:port
        std::vector<std::string> _instances_index;
        //轮询访问中下次该访问实例id（ip+port）
        //std::string _rolling_instance;
        std::atomic<uint64_t> _rolling_num;
        std::shared_mutex _rw_lock;

        std::atomic<int64_t> _total_weight;

        //连接池上的空闲实例是否需要做重分配
        //bool _is_need_realloc;
        bool _is_used;
        //分库id
        int _id;
        std::string _bns_name;
        //_tag!=""时表示这个bns下的tag==_tag的实例属于这个pool
        std::string _tag;

        //该连接池上的总连接数，读完配置文件后不再改
        int _total_conn_num;

        //该值在init之后才有意义，初始化为2
        int32_t _conf_connection_per_instance;
        // 该bns连接池下每个实例的最大连接数
        int _max_connection_per_instance;

        ConnectionConf _conn_conf;

        // 计算分表id, key为逻辑库名
        std::map<std::string, LogicDB *> _name_logic_db_map;

        BnsInfo *_bns_info_ptr;//指向与该bns对应的实例信息

        std::string _comment_format;
        SelectAlgo _select_algo;

        SnsConnectionPool(const SnsConnectionPool &);

        SnsConnectionPool &operator=(const SnsConnectionPool &);
    };

}  // namespace kumo::client
