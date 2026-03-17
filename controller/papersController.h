#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpRequest.h>
#include <json/json.h>
#include <memory>
#include "../sql/sql_wrapper.h"

class PapersController : public drogon::HttpController<PapersController> {
public:
    

    METHOD_LIST_BEGIN
    // 注册路由
    ADD_METHOD_TO(PapersController::addPaper, "/api/paper/add", drogon::HttpMethod::Post);
    ADD_METHOD_TO(PapersController::deletePaper, "/api/paper/delete", drogon::HttpMethod::Post);
    ADD_METHOD_TO(PapersController::getPapers, "/api/paper/show", drogon::HttpMethod::Get);

    METHOD_LIST_END



    
    void addPaper(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback);

                       
    void deletePaper(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void getPapers(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback);


private:
    
    WEB_ESSAY_LIBRARY::SqlWrapper sqlWrapper;
    // 创建JSON响应
    drogon::HttpResponsePtr createJsonResponse(const Json::Value& json, drogon::HttpStatusCode code = drogon::k200OK);
    
    // 创建错误响应
    drogon::HttpResponsePtr createErrorResponse(const std::string& message, drogon::HttpStatusCode code = drogon::k400BadRequest);
};