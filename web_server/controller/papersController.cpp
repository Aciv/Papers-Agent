#include "papersController.h"


    
void PapersController::addPaper(const drogon::HttpRequestPtr& req,
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

        auto jsonBody = req->getJsonObject();
        if (!jsonBody) {
            callback(createErrorResponse("无效的JSON格式"));
            return;
        }


        if (!jsonBody->isMember("arxiv_id")) {
            callback(createErrorResponse("缺少arxiv参数"));
            return;
        }
        std::string arxiv_id = jsonBody->get("arxiv_id", "").asString();   

        std::string query_exist = "select 1 from papers where arxiv_id = '" + arxiv_id + "' limit 1";
        auto sql_resp = sqlWrapper.executeQueryWithResult(query_exist);
        MYSQL_ROW row = mysql_fetch_row(sql_resp);
        if(!sql_resp || !row) {
            callback(createErrorResponse("未知arxiv 论文"));
            return;
        }
        
        std::string insert_paper = "INSERT INTO user_collections (user_id, arxiv_id) VALUES ( " +
                      std::to_string(req->session()->get<std::int64_t>("user_id"))
                       + ", '" +  arxiv_id + "')";
        
        if(sqlWrapper.executeQuery(insert_paper)){
            Json::Value response;
            response["code"] = 200;
            response["message"] = "插入成功";
            response["user email"] = user_email;
            response["arxiv id"] = arxiv_id;
            
            auto resp = createJsonResponse(response);
            callback(resp);
        }
        else{
            auto resp = createErrorResponse("插入，论文已存在");
            callback(resp);
        }
        
    } catch (const std::exception& e) {
        APP_LOG_ERROR_FMT("存入论文出错: %s", e.what());
        auto resp = createErrorResponse("服务器内部错误");
        callback(resp);
    }
}

void PapersController::deletePaper(const drogon::HttpRequestPtr& req,
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

        auto jsonBody = req->getJsonObject();
        if (!jsonBody) {
            callback(createErrorResponse("无效的JSON格式"));
            return;
        }


        if (!jsonBody->isMember("arxiv_id")) {
            callback(createErrorResponse("缺少arxiv参数"));
            return;
        }

        std::string arxiv_id = jsonBody->get("arxiv_id", "").asString();   

        std::string query_exist = "select 1 from user_collections where arxiv_id = '" + arxiv_id + "' limit 1";
        auto sql_resp = sqlWrapper.executeQueryWithResult(query_exist);
        MYSQL_ROW row = mysql_fetch_row(sql_resp);
        if(!sql_resp || !row) {
            callback(createErrorResponse("collections 中没有该论文"));
            return;
        }
        
        std::string delete_paper = "DELETE FROM user_collections where arxiv_id = '" +
                     arxiv_id + "'";

        if(sqlWrapper.executeQuery(delete_paper)){
            Json::Value response;
            response["code"] = 200;
            response["message"] = "删除成功";
            response["user email"] = user_email;
            response["arxiv id"] = arxiv_id;
            
            auto resp = createJsonResponse(response);
            callback(resp);
        }
        else{
            auto resp = createErrorResponse("删除论文失败");
            callback(resp);
        }
        
    } catch (const std::exception& e) {
        APP_LOG_ERROR_FMT("删除论文出错: %s", e.what());
        auto resp = createErrorResponse("服务器内部错误");
        callback(resp);
    }
}

void PapersController::getPapers(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        if (!req->session()->find("is_logged_in") || 
            !req->session()->get<bool>("is_logged_in")) {
            auto resp = createErrorResponse("未登录");
            callback(resp);
            return;
        }

        if (!req->session()->find("user_id")) {
            auto resp = createErrorResponse("未找到用户 id");
            callback(resp);
            return;
        }
        
        int64_t user_id = req->session()->get<std::int64_t>("user_id");

        

        std::string query_exist = "select u.arxiv_id, p.title from user_collections u "
                          "inner join papers p on u.arxiv_id = p.arxiv_id "
                          "where u.user_id = " + std::to_string(user_id);
        
        
        auto sql_resp = sqlWrapper.executeQueryWithResult(query_exist);

        if(!sql_resp) {
            callback(createErrorResponse("查询错误"));
            return;
        }
        MYSQL_ROW row;

        Json::Value response;
        

        response["code"] = 200;
        response["message"] = "查询成功";

        Json::Value resultsJson(Json::arrayValue);
        while ((row = mysql_fetch_row(sql_resp)) != NULL) {
            Json::Value item;
            item["arxiv id"] = row[0] ? row[0] : "";
            item["title"] = row[1] ? row[1] : "";
            resultsJson.append(item);
        }

        response["papers"] = resultsJson;

        callback(createJsonResponse(response));


    } catch (const std::exception& e) {
        APP_LOG_ERROR_FMT("获取论文出错: %s", e.what());
        auto resp = createErrorResponse("服务器内部错误");
        callback(resp);
    }
}

drogon::HttpResponsePtr PapersController::createJsonResponse(const Json::Value& json, 
                                                             drogon::HttpStatusCode code) {
    auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
    resp->setStatusCode(code);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
    return resp;
}

drogon::HttpResponsePtr PapersController::createErrorResponse(const std::string& message, 
                                                              drogon::HttpStatusCode code) {
    Json::Value json;
    json["code"] = static_cast<int>(code);
    json["message"] = message;
    json["data"] = Json::objectValue;
    
    return createJsonResponse(json, code);
}