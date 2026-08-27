#ifndef UNTITLED_FUNCTIONALFUNCTION_H
#define UNTITLED_FUNCTIONALFUNCTION_H
#include<iostream>
#include<string>
#include"mysql.h"
#include "json.hpp"
#include"OnlineSessionManager.h"
using json = nlohmann::json;
void Login(json& res, int account, std::string& pwd, mysqlconn& conn);
void Repost(json& res,json& message,SessionPtr target_session,mysqlconn& conn);
void Pull(json& res, SessionPtr session, mysqlconn& conn);
void DeleteCache(json& res,std::string serverId ,mysqlconn& conn);

#endif //UNTITLED_FUNCTIONALFUNCTION_H
