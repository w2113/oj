#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "../comm/log.hpp"
#include "../comm/util.hpp"

namespace ns_runner
{
    using namespace ns_util;
    using namespace ns_log;
    
    class Runner
    {
    public:
        Runner(){}
        ~Runner(){}
    public:
        //设置进程占用资源大小的接口
        static void setProcLimit(int _cpu_limit,int _mem_limit)
        {
            struct rlimit cpu_rlimit;
            cpu_rlimit.rlim_max = RLIM_INFINITY;
            cpu_rlimit.rlim_cur = _cpu_limit;
            setrlimit(RLIMIT_CPU,&cpu_rlimit);

            struct rlimit mem_rlimit;
            mem_rlimit.rlim_max = RLIM_INFINITY;
            mem_rlimit.rlim_cur = _mem_limit*1024;
            setrlimit(RLIMIT_AS,&mem_rlimit);
        }
        //指明文件名即可
        //返回值 > 0 :程序异常了
        //返回值 = 0 :正常运行完毕,结果不关心,保存到临时文件中
        //返回值 < 0 :内部错误
        //cpu_limit: 可以使用的最大cpu资源上限
        //mem_limit: 可以使用的最大内存大小(KB)
        static int Run(const std::string& file_name,int cpu_limit,int mem_limit)
        {
            //程序运行：
            //1、代码跑完结果正确
            //2、代码跑完结果不正确
            //3、代码没跑完,异常
            std::string _execute = PathUtil::Exe(file_name);
            std::string _stdin  = PathUtil::Stdin(file_name);
            std::string _stdout = PathUtil::Stdout(file_name);
            std::string _stderr = PathUtil::Stderr(file_name);
            
            umask(0);
            int _stdin_fd = open(_stdin.c_str(),O_CREAT | O_WRONLY,0644);
            int _stdout_fd = open(_stdout.c_str(),O_CREAT | O_WRONLY,0644);
            int _stderr_fd = open(_stderr.c_str(),O_CREAT | O_WRONLY,0644);

            if(_stdin_fd < 0 || _stdout_fd < 0 || _stderr_fd < 0)
            {
                LOG(ERROR)<<"运行时打开文件失败"<<"\n";
                return- 1;//代表打开文件失败
            }
            pid_t pid = fork();
            if(pid < 0)
            {
                LOG(ERROR)<<"运行时创建子进程失败"<<"\n";
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
                return -2; //代表创建子进程失败
            }
            else if(pid == 0)
            {
                dup2(_stdin_fd,0);
                dup2(_stdout_fd,1);
                dup2(_stderr_fd,2);

                setProcLimit(cpu_limit,mem_limit);
                execl(_execute.c_str()/*要执行谁*/,_execute.c_str()/*在命令行上如何执行*/,nullptr);
                exit(1);
            }
            else
            {
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
                int status = 0;
                waitpid(pid,&status,0);
                //程序运行异常,一定是因为收到了信号
                LOG(INFO)<<"运行完毕,INFO: "<< (status & 0x7F)<<"\n";
                return status & 0x7F;
            }
        } 
    };
}