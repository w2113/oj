#include "compile_run.hpp"
#include "../comm/httplib.h"
using namespace ns_compile_and_run;
using namespace httplib;

void Usage(std::string proc)
{
    std::cerr<<"Usage: "<<"\n\t"<<proc<<"port"<<std::endl;
}
//编译服务随时可能被多次请求,必须保证传递上来的code,文件名称唯一
int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        return 1;
    }
    Server svr;
    // svr.Get("/hello",[](const Request& req,Response &resp){
    //     resp.set_content("hello httplib,你好","text/plain;charset=utf-8");
    // });
    svr.Post("/compile_and_run",[](const Request& req,Response &resp){
        std::string in_json = req.body;
        std::string out_json; 
        if(!in_json.empty())
        {
            CompileAndRun::Start(in_json, &out_json);
            resp.set_content(out_json,"application/json;charset=utf-8");
        }
    });
    svr.listen("0.0.0.0",atoi(argv[1]));
    //svr.set_base_dir("./wwwroot");
    //提供的编译服务,打包形成一个网络服务
    //cpp-httplib

    //通过http让client给我们上传一个json string
    // in_json: {"code":"#include..","input":"","cpu_limit":1,"mem_limit":10240}
    // out_json:{"status":"0","reason":"","stdout":"","stderr":"",}
    // std::string in_json;
    // Json::Value in_value;
    // in_value["code"] = R"(#include<iostream>
    // int main(){
    //     std::cout << "你可以看见我了" << std::endl;
    //     return 0;
    // })";
    // in_value["input"] = "";
    // in_value["cpu_limit"] = 1;
    // in_value["mem_limit"] = 10240 * 3;

    // Json::FastWriter writer;
    // in_json = writer.write(in_value);

    // std::cout<<in_json<<std::endl;
    // std::string out_json;
    // CompileAndRun::Start(in_json,&out_json);
    // std::cout<<out_json<<std::endl;
    return 0;
}