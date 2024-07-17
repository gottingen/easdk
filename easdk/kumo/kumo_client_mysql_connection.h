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

#include <easdk/kumo/kumo_client_connection.h>
#include <mariadb/mysql.h>
#include <string>
#include <vector>
#include <atomic>
#include <easdk/kumo/kumo_client_result_set.h>
#include <easdk/kumo/kumo_client_epoll.h>

namespace kumo::client {

    class Instance;

    class SnsConnectionPool;

    class Connection;

    class MysqlConnection : public Connection {
        friend class ScopeProcWeight;

    public:

        // @brief 构造函数
        // instance: 指向连接所属的实例
        // pool: 指向连接所属的连接池
        MysqlConnection(Instance *instance, SnsConnectionPool *pool);

        virtual ~MysqlConnection() {};

        // @brief 连接数据库
        int connect(ConnectionConf &conn_conf, bool change_status);

        int reconnect(ConnectionConf &conn_conf, bool change_status);

        // @brief 在connection上执行sql语句，支持分表
        // vector<string> table_list : sql语句中的表名，由用户提供
        // table_name: 有两种格式
        //             1、db_name.table_name, 程序会分别读出db_name和table_name,
        //             然后根据该db下的分表逻辑去对table_name进行分表改写
        //             2、table_name，程序会认为该table_name所属的db_name是fetch_connection时
        //              传入的table_name
        int execute(const std::string &sql,
                    std::vector<std::string> &table_name_list,
                    ResultSet *result);

        // @brief 在connection上执行sql语句，支持分表
        // vector<string> table_list : sql语句中的表名，由用户提供
        // 是否保存结果由store决定
        int execute(const std::string &sql,
                    std::vector<std::string> &vector_list,
                    bool store,
                    ResultSet *result);

        // @brief 在connection上执行sql语句，保存存储结果
        // 从sql语句中解析表名
        int execute(const std::string &sql, ResultSet *result);

        // @brief 在connection上执行sql语句
        // 是否保存结果由store决定
        // 从sql语句中解析表名
        int execute(const std::string &sql, bool store, ResultSet *result);

        // @brief 在connection上执行sql语句，返回MYSQL_RES结果，需要用户释放资源
        int execute_raw(const std::string &sql, bool store, MYSQL_RES *&result);

        int execute_raw(const std::string &sql, MYSQL_RES *&result);

        // @brief 在connection上执行start transition操作
        int begin_transaction();

        // @brief 在connection上执行commit操作
        int commit();

        // @brief 在connection上执行rollback操作
        int rollback();

        // @breif 对连接做ping()操作
        int ping();

        int reset();

        // @brief 返回_sqlhandle上的错误码
        int get_error_code(int *error_code);

        // @breif 返回_sqlhandle上的错误描述
        std::string get_error_des();

        void set_read_timeout(const int32_t &read_time);

        virtual void close();

    private:

        // @brief 进行分表
        int _split_table(std::vector<std::string> &table_name_list, std::string &sql_rewrite);

        void _parse_db_name(const std::string &sql_rewrite, int pos, std::string &sql_db_name);

        //@brief 从sql语句中解析出表名
        void _parse_table_name_list(const std::string &sql_rewrite,
                                    std::vector<std::string> &table_name_list);

        //@brief 根据某个关键字解析出关键字后的表名
        void _parse_after_key_word(const std::string &key_word,
                                   const std::string &sql_rewrite,
                                   std::vector<std::string> &table_name_list);

        //@brief 从指定位置开始，解析出第一个字符串
        //返回字符串的下一个位置
        std::string::size_type _parse_first_string(
                const std::string &sql_rewrite,
                std::string::size_type pos,
                std::string &result);

        // @brief 改写sql语句中的table_name
        void _rewrite_table_name(std::string &sql_rewrite,
                                 const std::string &db_name,
                                 const std::string &table_name,
                                 const std::string table_id);

        // @brief 得到db_name和table_name
        int _get_db_table_name(std::vector<std::string>::iterator iter,
                               std::string &db_name,
                               std::string &table_name);

        // @brief 处理sql执行结果
        int _process_result(bool store, ResultSet *result);

        // @brief 处理sql返回值
        int _process_query(const std::string &sql_rewrite);

        // @breif 增加注释
        int _add_comment(SnsConnectionPool *pool, std::string &comment_format);

        // @brief 从num开始找到第一个有效的关键字
        // case_sensitive 是否区分大小写
        // 若找到，则返回关键字位置
        // 若没有，则返回string::npos
        std::string::size_type _find_key_word_after_num(
                const std::string &sql_rewrite,
                const std::string &key_word,
                std::string::size_type num,
                bool case_sensitive);

        std::string::size_type _findn_case_sensitive(
                const std::string &sql_rewrite,
                const std::string &key_word,
                std::string::size_type num);

        // @brief 判断指定位置的关键字是否是有效的关键字
        // 有效关键字是指该key_word不是某个字符串的子串
        bool _is_valid_key_word(std::string::size_type start_pos,
                                std::string::size_type end_pos,
                                const std::string &sql_rewrite);

        bool _is_valid_char(char c);

    private:
        MysqlConnection(const MysqlConnection &);

        MysqlConnection &operator=(const MysqlConnection &);

        bool _active_txn = false;
    };

}  // namespace kumo::client
