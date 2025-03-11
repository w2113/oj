#include <iostream>
#include <signal.h>
#include "../comm/httplib.h"
#include "oj_control.hpp"

using namespace httplib;
using namespace ns_control;
static Control *ctrl_ptr = nullptr;
void Recovery(int signo)
{
    ctrl_ptr->RecoveryMachine();
}
int main()
{
    std::signal(SIGQUIT,Recovery);
    //用户请求的服务路由功能
    Server svr;
    Control ctrl;
    ctrl_ptr = &ctrl;
    //获取所有的题目列表
    svr.Get("/all_questions",[&ctrl](const Request& req,Response &resp){
        std::string html;
        ctrl.AllQuestions(&html);
        resp.set_content(html,"text/html;charset=utf-8");
    });
    //根据题目编号,获取题目的内容
    //question/100->正则匹配
    //R"()",原始字符串,保持原貌,不需强转
    svr.Get(R"(/question/(\d+))",[&ctrl](const Request& req,Response &resp){
        std::string number = req.matches[1];
        std::string html;
        ctrl.Question(number,&html);
        resp.set_content(html,"text/html;charset=utf-8");
    });
    //用户提交代码,使用判题功能
    svr.Post(R"(/judge/(\d+))",[&ctrl](const Request& req,Response &resp){
        std::string number = req.matches[1];
        std::string result_json;
        ctrl.Judge(number,req.body,&result_json);
        resp.set_content(result_json,"application/json;charset=utf-8");
        // resp.set_content("指定题目的判题: "+number,"text/plain;charset=utf-8");
    });
    svr.set_base_dir("./wwwroot");
    svr.listen("0.0.0.0",8080);
    return 0;
}