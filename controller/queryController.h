#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpRequest.h>
#include <json/json.h>
#include "../sql/sqlconnpool.h"

#include <faiss/index_io.h>
#include <memory>

class LoginController : public drogon::HttpController<LoginController> {
public:
    

    METHOD_LIST_BEGIN
    // 注册路由
    ADD_METHOD_TO(LoginController::queryVector, "/admin/queryVector", drogon::HttpMethod::Get);

    METHOD_LIST_END


   cSqlConnPool* conn_pool = cSqlConnPool::Instance();

    
    // 处理获取用户信息请求
    void queryVector(const drogon::HttpRequestPtr& req,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // 创建JSON响应
    drogon::HttpResponsePtr createJsonResponse(const Json::Value& json, drogon::HttpStatusCode code = drogon::k200OK);
    
    // 创建错误响应
    drogon::HttpResponsePtr createErrorResponse(const std::string& message, drogon::HttpStatusCode code = drogon::k400BadRequest);

};