#pragma once
#include "compiler.hpp"
#include "runner.hpp"
#include "../comm/log.hpp"
#include "../comm/util.hpp"

#include <signal.h>
#include <unistd.h>
#include <jsoncpp/json/json.h>

namespace ns_compile_and_run
{
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_compiler;
    using namespace ns_runner;
    class CompileAndRun
    {
        public:
        static void RemoveTempFile(const std::string& file_name)
        {
            //清理文件的个数是不确定的
            std::string _src = PathUtil::Src(file_name);
            if(FIleUtil::IsFileExists(_src)) unlink(_src.c_str());

            std::string _comilpe_error = PathUtil::CompilerError(file_name);
            if(FIleUtil::IsFileExists(_comilpe_error)) unlink(_comilpe_error.c_str());
            
            std::string _execute = PathUtil::Exe(file_name);
            if(FIleUtil::IsFileExists(_execute)) unlink(_execute.c_str()); 

            std::string _stdin = PathUtil::Stdin(file_name);
            if(FIleUtil::IsFileExists(_stdin)) unlink(_stdin.c_str());

            std::string _stdout = PathUtil::Stdout(file_name);
            if(FIleUtil::IsFileExists(_stdout)) unlink(_stdout.c_str());
            
            std::string _stderr = PathUtil::Stderr(file_name);
            if(FIleUtil::IsFileExists(_stderr)) unlink(_stderr.c_str());
        }
            //code > 0 :进程收到了信号导致异常崩溃
            //code < 0 :整个过程非运行报错(代码为空,编译报错等)
            //code = 0 :整个过程全部完成
            static std::string CodeToDesc(int code,const std::string& file_name)
            {
                std::string desc;
                switch(code)
                {
                    case 0:
                        desc = "编译运行成功";
                        break;
                    case -1:
                        desc = "提交的代码为空";
                        break;
                    case -2:
                        desc = "未知错误";
                        break;
                    case -3:
                        //desc = "代码编译时发生了错误";
                        FIleUtil::ReadFile(PathUtil::CompilerError(file_name),&desc,true);
                        break;
                    case SIGABRT://6
                        desc = "内存超过范围";
                        break;
                    case SIGXCPU://24
                        desc = "CPU使用超时";
                        break;
                    case SIGFPE://8
                        desc = "浮点数溢出";
                        break;
                    default:
                        desc = "未知:" + std::to_string(code);
                        break;
                }
                return desc;
            }
            
            //输入:
            //code:用户提交的代码
            //input:用户给自己提交的代码对应的输入,不做处理
            //cpu_limit
            //mem_limit
            //输出:
            //status: 状态码
            //reason: 请求结果
            //选填:
            //stdout: 我的程序运行完的结果
            //stderr: 运行完的错误结果
            //参数:
            //in_json: {"code":"#include..","input":"","cpu_limit":1,"mem_limit":10240}
            //out_json:{"status":"0","reason":"","stdout":"","stderr":"",}
            static void Start(const std::string& in_json,std::string* out_json)
            {
                Json::Value in_value;
                Json::Reader reader;   
                reader.parse(in_json,in_value);//差错处理
                
                std::string code = in_value["code"].asString();
                std::string input = in_value["input"].asString();
                int cpu_limit = in_value["cpu_limit"].asInt();
                int mem_limit = in_value["mem_limit"].asInt();

                int status_code = 0;
                Json::Value out_value;
                int run_result = 0;
                std::string file_name;
                if(code.size() == 0)
                {                    
                    status_code = -1;//代码为空
                    goto END;

                }
                //形成的文件名具有唯一性,没有目录没有后缀
                //毫秒级时间戳+原子性递增的唯一值:来保证唯一性
                file_name = FIleUtil::UniqFileName();
                if(!FIleUtil::WriteFile(PathUtil::Src(file_name),code))
                {//形成临时src文件                    
                    status_code = -2;
                    goto END;
                }
                if(!Compiler::Compile(file_name))
                {   
                    status_code = -3;
                    goto END;
                }
                run_result = Runner::Run(file_name,cpu_limit,mem_limit);
                if(run_result < 0)
                {                   
                    status_code = -2;
                    goto END;
                }
                else if(run_result > 0)
                {
                    status_code = run_result;
                }
                else
                {
                    //运行成功
                    status_code = 0;
                }
                END:
                out_value["status"] = status_code;
                out_value["reason"] = CodeToDesc(status_code,file_name);
                if(status_code == 0)
                {
                    //整个过程全部成功
                    std::string _stdout;
                    FIleUtil::ReadFile(PathUtil::Stdout(file_name),&_stdout,true);
                    out_value["stdout"] = _stdout;

                    std::string _stderr;
                    FIleUtil::ReadFile(PathUtil::Stderr(file_name),&_stderr,true);
                    out_value["stderr"] = _stderr;
                }
                //Json::StyledWriter writer;
                Json::StreamWriterBuilder builder;
                // 设置不转义非ASCII字符
                builder.settings_["unescaped_unicode"] = true;
                // *out_json = writer.write(out_value);
                std::ostringstream oss;
                std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
                writer->write(out_value, &oss);
                *out_json = oss.str();

                RemoveTempFile(file_name);
            }
    };
}