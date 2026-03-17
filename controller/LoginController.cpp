#include "LoginController.h"
#include "../log_wrapper.h"


void LoginController::createUser(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {

        // 解析JSON请求体
        auto jsonBody = req->getJsonObject();

        std::string body(req->body().data(), req->body().size()); // 正确转换string_view到string
        if (!jsonBody) {
            auto resp = createErrorResponse("无效的JSON格式: ");
            callback(resp);
            return;
        }

        // 获取请求参数
        std::string username = jsonBody->get("username", "").asString();
        std::string password = jsonBody->get("password", "").asString();
        std::string email = jsonBody->get("email", "").asString();
        

        // 验证参数
        if (username.empty() || password.empty() || email.empty()) {
            auto resp = createErrorResponse("用户名、密码和邮箱不能为空");
            callback(resp);
            return;
        }
        

        // 注册用户
        bool success = userDataController.registerUser(username, password, email);
        

        Json::Value response;
        if (success) {
            response["code"] = 200;
            response["message"] = "注册成功";
            response["data"]["username"] = username;
            response["data"]["email"] = email;
            
            auto resp = createJsonResponse(response);
            callback(resp);
        } else {
            auto resp = createErrorResponse("注册失败，用户名或邮箱已存在");
            callback(resp);
        }


        
    } catch (const std::exception& e) {
        APP_LOG_ERROR_FMT("注册异常: %s", e.what());
        auto resp = createErrorResponse("服务器内部错误");
        callback(resp);
    }
}



void LoginController::loginUser(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        // 解析JSON请求体
        auto jsonBody = req->getJsonObject();

        std::string body(req->body().data(), req->body().size()); // 正确转换string_view到string
        if (!jsonBody) {
            auto resp = createErrorResponse("无效的JSON格式: ");
            callback(resp);
            return;
        }
        
        // 获取请求参数
        std::string user_email = jsonBody->get("user_email", "").asString();
        std::string password = jsonBody->get("password", "").asString();
        
        APP_LOG_DEBUG_FMT("%s , %s", user_email.data(), password.data());

        // 验证参数
        if (user_email.empty() || password.empty()) {
            auto resp = createErrorResponse("邮箱和密码不能为空");
            callback(resp);
            return;
        }
        
        // 验证登录
        bool success = userDataController.loginUser(user_email, password);
        
        Json::Value response;
        if (success) {
            // 获取用户信息
            auto session = req->session();
            
            if (session->find("user_email")) {
                auto resp = createErrorResponse("用户已登录");
                callback(resp);
                return;
            }

            auto user = userDataController.getUserByEmail(user_email);
            
            session->insert("user_email", user_email);
            session->insert("is_logged_in", true);
            session->insert("user_id", user->getId());

            response["code"] = 200;
            response["message"] = "登录成功";
            response["data"]["id"] = user->getId();
            response["data"]["username"] = user->getUsername();
            response["data"]["email"] = user->getEmail();
            response["data"]["created_at"] = user->getCreatedAt();
            
            auto resp = createJsonResponse(response);
            callback(resp);
        } else {
            auto resp = createErrorResponse("用户名或密码错误");
            callback(resp);
        }
        
    } catch (const std::exception &e) {
        APP_LOG_ERROR_FMT("登录异常: %s", e.what());
        auto resp = createErrorResponse("服务器内部错误");
        callback(resp);
    }
}


void LoginController::getUser(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback) {


    try {
        if (!req->session()->find("is_logged_in") || 
            !req->session()->get<bool>("is_logged_in")) {
            auto resp = createErrorResponse("未登录");
            callback(resp);
            return;
        }
        // 从查询参数获取用户名
        if (!req->session()->find("user_email")) {
            auto resp = createErrorResponse("未找到用户邮箱");
            callback(resp);
            return;
        }
        
        std::string user_email = req->session()->get<std::string>("user_email");
        //std::string user_email = req->getParameter("user_email");
        
        if (user_email.empty()) {
            auto resp = createErrorResponse("邮箱参数不能为空");
            callback(resp);
            return;
        }
        
        // 获取用户信息
        auto user = userDataController.getUserByEmail(user_email);
        
        Json::Value response;
        if (user) {
            response["code"] = 200;
            response["message"] = "获取用户信息成功";
            response["data"]["id"] = user->getId();
            response["data"]["username"] = user->getUsername();
            response["data"]["email"] = user->getEmail();
            response["data"]["created_at"] = user->getCreatedAt();
            
            auto resp = createJsonResponse(response);
            callback(resp);
        } else {
            auto resp = createErrorResponse("用户不存在");
            callback(resp);
        }
        
    } catch (const std::exception& e) {
        APP_LOG_ERROR_FMT("获取用户信息异常: %s", e.what());
        auto resp = createErrorResponse("服务器内部错误");
        callback(resp);
    }
}

drogon::HttpResponsePtr LoginController::createJsonResponse(const Json::Value& json, drogon::HttpStatusCode code) {
    auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
    resp->setStatusCode(code);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
    return resp;
}

drogon::HttpResponsePtr LoginController::createErrorResponse(const std::string& message, drogon::HttpStatusCode code) {
    Json::Value json;
    json["code"] = static_cast<int>(code);
    json["message"] = message;
    json["data"] = Json::objectValue;
    
    return createJsonResponse(json, code);
}

