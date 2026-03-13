#include "queryController.h"
#include "../log_wrapper.h"

QueryController::QueryController() {
    // 在构造函数中初始化Faiss索引
    auto& faissQuery = VectorQuery::FaissQuery::getInstance();
    if (!faissQuery.isInitialized()) {
        bool success = faissQuery.initialize(DEFAULT_INDEX_PATH);
        if (success) {
            APP_LOG_INFO_FMT("QueryController: Faiss index loaded from %s", DEFAULT_INDEX_PATH);
        } else {
            APP_LOG_WARN_FMT("QueryController: Failed to load Faiss index from %s", DEFAULT_INDEX_PATH);
        }
    }
}

std::vector<int64_t> QueryController::queryMysql(std::string &categories, std::string &author, std::string &since_data) {
    std::vector<int64_t> vector_ids;

    std::string query = "SELECT faiss_id FROM papers WHERE";
    if(categories.size() > 0) {
        query += " categories LIKE '%" + categories + "%'";
    }
    if(author.size() > 0) {
        query += " AND author LIKE '%" + author + "%'";
    }
    if(since_data.size() > 0) {
        query += " AND since_data >= '" + since_data + "'";
    }

    
    MYSQL_RES* result = sqlWrapper.executeQueryWithResult(query);

    if (!result) {
        APP_LOG_ERROR("获取查询结果失败");
        return vector_ids;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        if (row[0]) {
            vector_ids.push_back(std::stoll(row[0]));
        }
    }

    return vector_ids;
}




void QueryController::queryVector(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        // 解析JSON请求体
        auto jsonBody = req->getJsonObject();
        if (!jsonBody) {
            callback(createErrorResponse("无效的JSON格式"));
            return;
        }

        // 获取查询向量
        if (!jsonBody->isMember("vector") || !(*jsonBody)["vector"].isArray()) {
            callback(createErrorResponse("缺少vector参数或格式错误"));
            return;
        }

        const Json::Value& vectorJson = (*jsonBody)["vector"];
        std::vector<float> queryVector;
        queryVector.reserve(vectorJson.size());
        
        for (const auto& val : vectorJson) {
            if (val.isNumeric()) {
                queryVector.push_back(val.asFloat());
            } else {
                callback(createErrorResponse("向量元素必须是数值类型"));
                return;
            }
        }

        std::string categories = jsonBody->get("categories", "").asString();
        std::string author = jsonBody->get("author", "").asString();
        std::string since_data = jsonBody->get("since_data", "").asString();

        std::vector<int64_t> vector_ids = queryMysql(categories, author, since_data);

        // 获取k值（默认5）
        int k = jsonBody->get("k", 5).asInt();
        if (k <= 0) {
            callback(createErrorResponse("k值必须为正整数"));
            return;
        }

        // 获取nprobe值（默认10）
        int nprobe = jsonBody->get("nprobe", 10).asInt();
        if (nprobe <= 0) {
            nprobe = 10;
        }
        
        // 执行向量查询
        auto& faissQuery = VectorQuery::FaissQuery::getInstance();
        if (!faissQuery.isInitialized()) {
            callback(createErrorResponse("Faiss索引未初始化", drogon::k500InternalServerError));
            return;
        }

        auto result = faissQuery.search(queryVector, vector_ids, k, nprobe);

        if (!result.success) {
            callback(createErrorResponse(result.errorMsg, drogon::k500InternalServerError));
            return;
        }

        // 构建响应
        Json::Value response;
        response["code"] = 200;
        response["message"] = "查询成功";
        response["data"]["dimension"] = result.dimension;
        response["data"]["k"] = result.k;
        
        Json::Value resultsJson(Json::arrayValue);
        for (size_t i = 0; i < result.ids.size(); ++i) {
            Json::Value item;
            item["id"] = static_cast<Json::Int64>(result.ids[i]);
            item["distance"] = result.distances[i];
            resultsJson.append(item);
        }
        response["data"]["results"] = resultsJson;

        callback(createJsonResponse(response));

    } catch (const std::exception& e) {
        APP_LOG_ERROR_FMT("向量查询异常: %s", e.what());
        callback(createErrorResponse("服务器内部错误", drogon::k500InternalServerError));
    }
}

void QueryController::batchQueryVector(const drogon::HttpRequestPtr& req,
                                        std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        // 解析JSON请求体
        auto jsonBody = req->getJsonObject();
        if (!jsonBody) {
            callback(createErrorResponse("无效的JSON格式"));
            return;
        }

        // 获取查询向量数组
        if (!jsonBody->isMember("vectors") || !(*jsonBody)["vectors"].isArray()) {
            callback(createErrorResponse("缺少vectors参数或格式错误"));
            return;
        }

        const Json::Value& vectorsJson = (*jsonBody)["vectors"];
        if (vectorsJson.empty()) {
            callback(createErrorResponse("vectors数组不能为空"));
            return;
        }

        // 获取Faiss实例检查维度
        auto& faissQuery = VectorQuery::FaissQuery::getInstance();
        if (!faissQuery.isInitialized()) {
            callback(createErrorResponse("Faiss索引未初始化", drogon::k500InternalServerError));
            return;
        }

        int dimension = faissQuery.getDimension();
        int numQueries = static_cast<int>(vectorsJson.size());

        // 解析所有向量到一个展平的数组
        std::vector<float> queryVectors;
        queryVectors.reserve(numQueries * dimension);

        for (const auto& vecJson : vectorsJson) {
            if (!vecJson.isArray()) {
                callback(createErrorResponse("每个向量必须是数组"));
                return;
            }
            if (static_cast<int>(vecJson.size()) != dimension) {
                callback(createErrorResponse("向量维度不匹配，期望: " + std::to_string(dimension)));
                return;
            }
            for (const auto& val : vecJson) {
                if (val.isNumeric()) {
                    queryVectors.push_back(val.asFloat());
                } else {
                    callback(createErrorResponse("向量元素必须是数值类型"));
                    return;
                }
            }
        }

        std::string categories = jsonBody->get("categories", "").asString();
        std::string author = jsonBody->get("author", "").asString();
        std::string since_data = jsonBody->get("since_data", "").asString();
        std::vector<int64_t> vector_ids = queryMysql(categories, author, since_data);

        // 获取k值和nprobe
        int k = jsonBody->get("k", 5).asInt();
        if (k <= 0) k = 5;

        int nprobe = jsonBody->get("nprobe", 10).asInt();
        if (nprobe <= 0) nprobe = 10;

        // 执行批量查询
        auto result = faissQuery.batchSearch(queryVectors, vector_ids, numQueries, k, nprobe);

        if (!result.success) {
            callback(createErrorResponse(result.errorMsg, drogon::k500InternalServerError));
            return;
        }

        // 构建响应
        Json::Value response;
        response["code"] = 200;
        response["message"] = "批量查询成功";
        response["data"]["dimension"] = result.dimension;
        response["data"]["k"] = result.k;
        response["data"]["num_queries"] = numQueries;

        Json::Value allResults(Json::arrayValue);
        for (int q = 0; q < numQueries; ++q) {
            Json::Value queryResults(Json::arrayValue);
            for (int i = 0; i < result.k; ++i) {
                size_t idx = static_cast<size_t>(q * result.k + i);
                Json::Value item;
                item["id"] = static_cast<Json::Int64>(result.ids[idx]);
                item["distance"] = result.distances[idx];
                queryResults.append(item);
            }
            allResults.append(queryResults);
        }
        response["data"]["results"] = allResults;

        callback(createJsonResponse(response));

    } catch (const std::exception& e) {
        APP_LOG_ERROR_FMT("批量向量查询异常: %s", e.what());
        callback(createErrorResponse("服务器内部错误", drogon::k500InternalServerError));
    }
}



void QueryController::reloadIndex(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        std::string indexPath = DEFAULT_INDEX_PATH;

        // 检查是否提供了自定义路径
        auto jsonBody = req->getJsonObject();
        if (jsonBody && jsonBody->isMember("index_path")) {
            indexPath = (*jsonBody)["index_path"].asString();
        }

        auto& faissQuery = VectorQuery::FaissQuery::getInstance();
        bool success = faissQuery.initialize(indexPath);

        Json::Value response;
        if (success) {
            response["code"] = 200;
            response["message"] = "索引重新加载成功";
            response["data"]["index_path"] = indexPath;
            response["data"]["dimension"] = faissQuery.getDimension();
            response["data"]["index_size"] = static_cast<Json::UInt64>(faissQuery.getIndexSize());
            callback(createJsonResponse(response));
        } else {
            callback(createErrorResponse("索引加载失败，请检查路径: " + indexPath, 
                                         drogon::k500InternalServerError));
        }

    } catch (const std::exception& e) {
        APP_LOG_ERROR_FMT("重载索引异常: %s", e.what());
        callback(createErrorResponse("服务器内部错误", drogon::k500InternalServerError));
    }
}

drogon::HttpResponsePtr QueryController::createJsonResponse(const Json::Value& json, 
                                                             drogon::HttpStatusCode code) {
    auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
    resp->setStatusCode(code);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
    return resp;
}

drogon::HttpResponsePtr QueryController::createErrorResponse(const std::string& message, 
                                                              drogon::HttpStatusCode code) {
    Json::Value json;
    json["code"] = static_cast<int>(code);
    json["message"] = message;
    json["data"] = Json::objectValue;
    
    return createJsonResponse(json, code);
}
