
#include "FunctionalFunction.h"
#include <sys/socket.h>

void Login(json& res,int account,  std::string& pwd, mysqlconn& conn)
{
    int retCode=0 ;
    conn.callLoginFunc(account, pwd,retCode);
    switch (retCode) {
        case 0:
            res["type"]="login_response";
            res["code"] = 400;
            res["error"] = "Data abnormal crash";//数据异常崩溃
            res["success"]=false;
            break;
        case 1:
            res["type"]="login_response";
            res["code"] = 400;
            res["error"] = "Account does not exist";
            res["success"]=false;
            break;
        case 2:
            res["type"]="login_response";
            res["code"] = 400;
            res["error"] = "Incorrect password";
            res["success"]=false;
            break;
        case 3:
            res["type"]="login_response";
            res["code"] = 0;
            res["error"] = 0;
            res["success"]=true;
            break;
        default:
            throw std::out_of_range("System error, data anomaly");
            break;
    }

}

void Repost(json& res, json& message, SessionPtr target_session, mysqlconn& conn)
{
    // 目标用户不在线


    // 调用存储过程把消息写入数据库
    uint32_t outMessageId = 0;
    std::cout<<"正在调用数据库消息存储过程"<<std::endl;
    if (!conn.callMessage(message, outMessageId))
    {
        res["type"] = "repost_response";
        res["code"] = 500;
        res["error"] = "消息存储失败";
        res["success"] = false;
        return;
    }
    if(!outMessageId)
    {
        res["type"] = "repost_response";
        res["code"] = 500;
        res["error"] = "消息过程运行错误或者账号异常";
        res["success"] = false;
    }
    message["ID"]=outMessageId;

   // 目标用户在线，直接转发；协议要求每条JSON以\n结尾
    std::string forward = message.dump() + "\n";
     if (!(target_session == nullptr))
    {
         if (send(target_session->fd, forward.c_str(), forward.size(), 0) < 0)
         {
             res["type"] = "repost_response";
             res["code"] = 500;
             res["error"] = "消息转发失败";
             res["success"] = false;
             return;
         }
    }

    res["type"] = "repost_response";
    res["code"] = 0;
    res["error"] = 0;
    res["success"] = true;
    res["messageId"] = outMessageId;
}

void Pull(json& res, SessionPtr session, mysqlconn& conn)
{
    // 拉取人是自己，直接复用传入的会话对象，无需findSession
    res["type"] = "pull_response";
    if (!session->state)
    {
        res["code"] = 401;
        res["error"] = "当前登录状态异常请重新登录";
        res["success"] = false;
        return;
    }

    // 拉取的是发给自己的消息，targetID 取自己的账号
    uint32_t targetId = 0;
    try
    {
        targetId = static_cast<uint32_t>(std::stoul(session->account));
    }
    catch (const std::exception& e)
    {
        res["code"] = 400;
        res["error"] = "账号格式非法";
        res["success"] = false;
        return;
    }

    std::vector<MessageInfo> msgs;
    int retcode = 0;
    if (!conn.callLoadMessage(targetId, msgs, retcode))
    {
        res["code"] = 500;
        res["error"] = "消息加载失败, retcode=" + std::to_string(retcode);
        res["success"] = false;
        return;
    }

    // 大量消息直接在功能函数里逐条发给拉取人自己的socket
    for (const auto& m : msgs)
    {
        json one = {
            {"type", "repost"},
            {"ID", m.messageId},
            {"senderId", m.senderId},
            {"targetId", m.targetId},
            {"sendtime", m.sendtime},
            {"content", m.content},
        };
        std::string buf = one.dump() + "\n";
        if (send(session->fd, buf.c_str(), buf.size(), 0) < 0)
        {
            std::cerr << "消息推送失败, msgId=" << m.messageId << std::endl;
            break;
        }
    }

    if(retcode==1)
    {
      // 最后返回确认状态包（含条数）
      res["code"] = 0;
      res["error"] = 0;
      res["success"] = true;
      res["count"] = msgs.size();
    }
    else if(retcode==2)
    {
      // 最后返回确认状态包（含条数）
      res["code"] = 0;
      res["error"] = "未有离线消息";
      res["success"] = true;
      res["count"] = msgs.size();
    }
}

void DeleteCache(json& res,std::string serverId,mysqlconn& conn)
{
    auto messageID=serverId;
    const unsigned int parsedSendId = std::stoull(messageID);
    if(!conn.callDeleteMessage(parsedSendId))
    {
        res["type"]="receiveACK";
        res["code"]=500;
        res["error"]="，数据库调用异常";
        res["success"]="false";
        return;
    }
    res["type"]="receiveACK";
    res["code"]=0;
    res["error"]="";
    res["success"]=true;
}
