#pragma once

#include "../comm/util.hpp"
#include "../comm/log.hpp"

#include <iostream>
#include <string>
#include <unordered_map>
#include <cassert>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>

namespace ns_model
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;
    struct Question
    {
        string number;//题号
        string title;//标题
        string star;//难度
        
        string desc;//描述
        string header;//预设代码
        string tail;//测试用例
        int cpu_limit;//时间要求
        int mem_limit;//空间要求
    };
    const std::string sqlquestions = "questions";
    const std::string host = "127.0.0.1";
    const std::string user = "oj_client";
    const std::string passwd = "123456";
    const std::string db = "oj";
    const int port = 3306;
    class Model
    {
        public:
        Model()
        {}
        bool QueryMySql(const std::string &sql,vector<Question>* out)
        {
            try{
                sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
                sql::Connection *con = driver->connect(host, user, passwd);
                con->setSchema(db);

                sql::Statement *stmt = con->createStatement();
                sql::ResultSet *res = stmt->executeQuery(sql);
                con->setClientOption("characterSetResults", "utf8");
                std::string t;
                while (res->next()) {
                    Question q;
                    q.number = res->getString("number");
                    q.title = res->getString("title");
                    q.star = res->getString("star");
                    q.desc = res->getString("_desc");
                    q.header = res->getString("header");                    
                    q.tail = res->getString("tail");
                    q.cpu_limit = res->getInt("cpu_limit");
                    q.mem_limit = res->getInt("mem_limit");
                    //std::cout << "After getting data: " << q.star << ", " << q.desc << ", " << q.header << std::endl;
                    out->push_back(q);
                }
                            
                delete res;
                delete stmt;
                delete con;
                return true; 
            }catch (sql::SQLException &e) {
                    LOG(WARNING) << "# ERR: SQLException in " << __FILE__;
                    LOG(WARNING) << "(" << __FUNCTION__ << ") on line " << __LINE__;
                    LOG(WARNING) << "# ERR: " << e.what();
                    LOG(WARNING) << " (MySQL error code: " << e.getErrorCode();
                    LOG(WARNING) << ", SQLState: " << e.getSQLState() << " )";
                    return false;
                }
            
        }
        bool GetAllQuestions(vector<Question>* out)
        {
            std::string sql = "SELECT * FROM " + sqlquestions;
            return QueryMySql(sql, out);
        }
        bool GetOneQuestion(const string& number,Question* q)
        {
            try {
                sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
                sql::Connection *con = driver->connect(host, user, passwd);
                con->setSchema(db);

                std::string sql = "SELECT * FROM " + sqlquestions + " WHERE number = ?";
                sql::PreparedStatement *pstmt = con->prepareStatement(sql);
                pstmt->setString(1, number);

                sql::ResultSet *res = pstmt->executeQuery();

                if (res->next()) {
                    q->number = res->getString("number");
                    q->title = res->getString("title");
                    q->star = res->getString("star");
                    q->desc = res->getString("_desc");
                    q->header = res->getString("header");
                    q->tail = res->getString("tail");
                    q->cpu_limit = res->getInt("cpu_limit");
                    q->mem_limit = res->getInt("mem_limit");
                    delete res;
                    delete pstmt;
                    delete con;
                    return true;
                }

                delete res;
                delete pstmt;
                delete con;
                return false;
            } catch (sql::SQLException &e) {
                LOG(WARNING) << "# ERR: SQLException in " << __FILE__;
                LOG(WARNING) << "(" << __FUNCTION__ << ") on line " << __LINE__;
                LOG(WARNING) << "# ERR: " << e.what();
                LOG(WARNING) << " (MySQL error code: " << e.getErrorCode();
                LOG(WARNING) << ", SQLState: " << e.getSQLState() << " )";
                return false;
            }
        }
        ~Model(){}
    };
}