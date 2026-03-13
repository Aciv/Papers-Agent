#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpRequest.h>
#include <json/json.h>
#include "../query/FaissQuery.h"
#include "../sql/sql_wrapper.h"

#include <memory>

class QueryController : public drogon::HttpController<QueryController> {
public:
    
    METHOD_LIST_BEGIN
    // 注册路由
    // 单个向量查询
    ADD_METHOD_TO(QueryController::queryVector, "/admin/vector/query", drogon::HttpMethod::Post);
    // 批量向量查询
    ADD_METHOD_TO(QueryController::batchQueryVector, "/admin/vector/batch_query", drogon::HttpMethod::Post);
    // 初始化/重载索引
    ADD_METHOD_TO(QueryController::reloadIndex, "/admin/vector/reload", drogon::HttpMethod::Post);
    
    METHOD_LIST_END

    // 构造函数 - 初始化Faiss索引
    QueryController();

    // 单个向量查询
    // POST /admin/vector/query
    // Body: { "vector": [0.1, 0.2, ...], "categories": "", "author": "", "since_data":"", "k": 5, "nprobe": 10 }
    void queryVector(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // 批量向量查询
    // POST /admin/vector/batch_query
    // Body: { "vectors": [[0.1, 0.2, ...], [0.3, 0.4, ...]], "categories": "", "author": "", "since_data":"", "k": 5, "nprobe": 10 }
    void batchQueryVector(const drogon::HttpRequestPtr& req,
                          std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // 重新加载索引
    // POST /admin/vector/reload
    // Body: { "index_path": "/path/to/index.faiss" } (可选)
    void reloadIndex(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
private:

    std::vector<int64_t> queryMysql(std::string &categories, std::string &author, std::string &since_data);
    // 创建JSON响应
    drogon::HttpResponsePtr createJsonResponse(const Json::Value& json, 
                                                drogon::HttpStatusCode code = drogon::k200OK);
    
    // 创建错误响应
    drogon::HttpResponsePtr createErrorResponse(const std::string& message, 
                                                 drogon::HttpStatusCode code = drogon::k400BadRequest);

    // 默认索引路径
    static constexpr const char* DEFAULT_INDEX_PATH = "faiss_vector/index.faiss";

    WEB_ESSAY_LIBRARY::SqlWrapper sqlWrapper;
};
