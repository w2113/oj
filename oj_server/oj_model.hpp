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

namespace ns_model
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;
    struct Question
    {
        std::string number;//题号
        std::string title;//标题
        std::string star;//难度
        int cpu_limit;//时间要求
        int mem_limit;//空间要求
        string desc;//描述
        string header;//预设代码
        string tail;//测试用例
    };
    const string questins_list = "./questions/questions.list";
    const string questins_path = "./questions/";

    class Model
    {
        private:
        //题号:题目细节
        unordered_map<string,Question> questions;
        public:
        Model()
        {
            assert(LoadQuestionList(questins_list));
        }
        bool LoadQuestionList(const string& question_list)
        {
            //加载配置文件
            ifstream in(question_list);
            if(!in.is_open())
            {
                LOG(FATAL)<<"加载题库失败"<<"\n";
                return false;
            }
            string line;
            while(getline(in,line))
            {
                vector<string> tokens;
                StringUtil::SplitString(line,&tokens," ");
                if(tokens.size() != 5)
                {
                    LOG(WARNING)<<"加载部分题目失败"<<"\n";
                    continue;
                }
                Question q;
                q.number = tokens[0];
                q.title = tokens[1];
                q.star = tokens[2];
                q.cpu_limit = atoi(tokens[3].c_str());
                q.mem_limit = atoi(tokens[4].c_str());

                string path = questins_path;
                path += q.number;
                path += "/";

                FIleUtil::ReadFile(path + "desc.txt",&(q.desc),true);
                FIleUtil::ReadFile(path + "header.cpp",&(q.header),true);
                FIleUtil::ReadFile(path + "tail.cpp",&(q.tail),true);
                std::cout << "After getting data: " << q.star << ", " << q.desc << ", " << q.header << std::endl;
                questions.insert({q.number,q});
            }
            LOG(INFO)<<"加载题库...成功"<<"\n";
            in.close();
            return true;
        }
        bool GetAllQuestions(vector<Question>* out)
        {
            if(questions.size() == 0)
            {
                LOG(ERROR)<<"用户获取题库失败"<<"\n";
                return false;
            }
            for(const auto&q : questions)
            {
                out->push_back(q.second);
            }
            return true;
        }
        bool GetOneQuestion(const string& number,Question* q)
        {
            if(questions.size() == 0)
            {
                return false;
            }
            const auto& iter = questions.find(number);
            if(iter == questions.end())
            {
                LOG(ERROR)<<"用户获取题目失败,编号:"<<number<<"\n";
                return false;
            }
            (*q) = iter->second;
            return true;
        }
        ~Model(){}
    };
}