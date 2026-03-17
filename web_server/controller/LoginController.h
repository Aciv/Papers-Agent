#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpRequest.h>
#include <json/json.h>
#include "../usersControl/UsersManage.h"
#include <memory>

class LoginController : public drogon::HttpController<LoginController> {
public:
    

    METHOD_LIST_BEGIN
    // 注册路由
    ADD_METHOD_TO(LoginController::getUser, "/api/user", drogon::HttpMethod::Get);
    ADD_METHOD_TO(LoginController::createUser, "/api/register", drogon::HttpMethod::Post);
    ADD_METHOD_TO(LoginController::loginUser, "/api/login", drogon::HttpMethod::Post);
    // ADD_METHOD_TO(LoginController::updateUser, "/user/{id}", drogon::HttpMethod::Put);
    // ADD_METHOD_TO(LoginController::deleteUser, "/user/{id}", drogon::HttpMethod::Delete);
    METHOD_LIST_END




    
    // 处理注册请求
    void createUser(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // 处理登录请求
    void loginUser(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // 处理获取用户信息请求
    void getUser(const drogon::HttpRequestPtr& req,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    
private:
    WEB_ESSAY_LIBRARY::UserDataController userDataController;
    // 创建JSON响应
    drogon::HttpResponsePtr createJsonResponse(const Json::Value& json, drogon::HttpStatusCode code = drogon::k200OK);
    
    // 创建错误响应
    drogon::HttpResponsePtr createErrorResponse(const std::string& message, drogon::HttpStatusCode code = drogon::k400BadRequest);
};