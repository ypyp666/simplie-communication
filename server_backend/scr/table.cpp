#include "table.h"
#include"FunctionalFunction.h"

CmdType JsonToCmdType(std::string type) {
    if (type=="login") return CmdType::Login;
    if (type=="logout") return CmdType::Logout;
    if (type=="modifyPwd") return CmdType::ModifyPwd;
    if (type=="modifyNam") return CmdType::ModifyNam;
    if (type=="repost") return CmdType::Repost;
    if (type=="pull_msg") return CmdType::Pull;

    return CmdType::Unknown;
}

std::string HandleLogin(const json& req, mysqlconn& conn, SessionPtr session)
{
    auto account = req.value<std::string>("account", "");
    auto pwd = req.value<std::string>("password", "");
    std::cout <<"登录函数当前处理账号:"<<account<<" "<<"密码："<<pwd<<std::endl;

    int my_account=0;
    json rsp;
    bool status=false;
    try
    {
        my_account = std::stoi(account);
        Login(rsp,my_account,pwd,conn);
        // 登录成功后登记在线会话并标记登录状态
        if (rsp.value<bool>("success", false)) {
            session->account = account;
            session->state = true;
            OnlineSessionManager::Instance().addSession(account, session);
        }

    }
    catch (const std::invalid_argument&)
    {
        // 没有合法数字
        rsp["type"]="login_response";
        rsp["code"] = 400;
        rsp["error"] = "illegal account";
        rsp["success"]=false;
        return rsp.dump();
    }
    catch (const std::out_of_range&)
    {
        // 数字太大/太小溢出
        rsp["type"]="login_response";
        rsp["code"] = 400;
        rsp["error"] = "illegal account";
        rsp["success"]=false;
        return rsp.dump();
    }
    std::cout<<rsp.dump();
    return rsp.dump();
}

std::string HandleRepost(const json& req, mysqlconn& conn,SessionPtr session) {
    json rsp,message;
    message=req;
    auto targetID=req.value<std::string>("targetId", "");
    SessionPtr fd_session=OnlineSessionManager::Instance().findSession(targetID);
    rsp["tempId"]=message["tempId"];
    if(!session->state)
    {
        rsp["type"] = "repost_response";
        rsp["code"] = 401;
        rsp["error"] = "当前登录状态异常请重新登录";
        rsp["success"] = false;
        return rsp.dump();
    }
    try
    {
        std::cout<<"当前正在运行消息转发"<<std::endl;
        Repost(rsp,message,fd_session,conn);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return rsp.dump();
}

std::string HandlePull(const json& req, mysqlconn& conn, SessionPtr session)
{
    json rsp;
    try
    {
        Pull(rsp, session, conn);
    }
    catch (const std::exception& e)
    {
        rsp["type"] = "pull_response";
        rsp["code"] = 500;
        rsp["error"] = "数据库调用失败";
        rsp["success"] = false;
        std::cerr << e.what() << '\n';
    }
    return rsp.dump();
}

std::string HandleReceiveACK(const json& req, mysqlconn& conn, SessionPtr session)
{
    json rsp;
    auto messageID=req.value<std::string>("serverId","");
     try
     {
        DeleteCache(rsp,messageID,conn);
     }
     catch(const std::exception& e)
     {
        rsp["type"]="receiveACK";
        rsp["code"]=500;
        rsp["error"]="删除失败";
        rsp["success"]="false";
        std::cerr<<e.what()<<'\n';
     }
    return rsp.dump();       
}

std::string HandleRegister(const json& req, mysqlconn& conn, SessionPtr session) {
    return "0";
}
std::string HandleGetInfo(const json& req, mysqlconn& conn, SessionPtr session) {
    return "0";
}
std::string HandleModifyPwd(const json& req, mysqlconn& conn, SessionPtr session) {
    return "0";
}
std::string HandleLogout(const json& req, mysqlconn& conn, SessionPtr session) {
    return "0";
}
std::string HandleUnknown(const json& req, mysqlconn& conn, SessionPtr session) {
    return "0";
}
