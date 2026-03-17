#include <drogon/drogon.h>
#include <drogon/HttpAppFramework.h>
#include "sql/sqlconnpool.h"
#include "usersControl/UsersManage.h"
#include "controller/LoginController.h"
#include "log_wrapper.h"

using namespace drogon;
const std::string BASE_SRC_PATH = "./src/";

int main() {
    // 初始化日志系统
    LogWrapper::init_logger();
    APP_LOG_INFO("启动登录系统服务器...");
    
    // 初始化MySQL连接池
    APP_LOG_INFO("初始化MySQL连接池...");
    cSqlConnPool* connPool = cSqlConnPool::Instance();
    
    // MySQL连接配置
    const char* host = "127.0.0.1";
    int port = 3306;
    const char* user = "root";
    const char* password = "dao181511";
    const char* dbName = "web_essay_library";
    int connSize = 10;
    
    APP_LOG_INFO_FMT("连接参数:");
    APP_LOG_INFO_FMT("  主机: %s", host);
    APP_LOG_INFO_FMT("  端口: %d", port);
    APP_LOG_INFO_FMT("  数据库: %s", dbName);
    APP_LOG_INFO_FMT("  用户名: %s", user);
    APP_LOG_INFO_FMT("  连接数: %d", connSize);
    
    try {
        connPool->Init(host, port, user, password, dbName, connSize);
        APP_LOG_INFO("MySQL连接池初始化成功");
        

        
    } catch (const std::exception& e) {
        APP_LOG_ERROR_FMT("MySQL连接池初始化失败: %s", e.what());
        APP_LOG_WARN("请确保MySQL服务正在运行，并且数据库'web_essay_library'存在");
        return 1;
    }


    app().setLogLevel(trantor::Logger::kWarn);


    app().enableSession(180);


    app().addListener("0.0.0.0", 8080);
    

     drogon::app().registerPreRoutingAdvice(
        [](const drogon::HttpRequestPtr &req,
           drogon::AdviceCallback &&stop,
           drogon::AdviceChainCallback &&pass) {
            
            if (req->method() == drogon::Options) {

                auto resp = drogon::HttpResponse::newHttpResponse();


                resp->addHeader("Access-Control-Allow-Origin", "http://152.136.99.95:8080");
                resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
                resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
                
                resp->addHeader("Access-Control-Allow-Credentials", "true");

                stop(resp);
                return;
            }

            pass();
        });


    // 添加默认路由
    app().setDocumentRoot(BASE_SRC_PATH);

    app().registerHandler("/",
        [](const HttpRequestPtr&,
           std::function<void(const HttpResponsePtr&)>&& callback) {
            std::string path = BASE_SRC_PATH + "index.html";
            auto resp = HttpResponse::newFileResponse(path);

            resp->setContentTypeCode(CT_TEXT_HTML);
            
            callback(resp);
        });
    
    // 添加健康检查端点
    app().registerHandler("/health",
        [](const HttpRequestPtr&,
           std::function<void(const HttpResponsePtr&)>&& callback) {
            Json::Value json;
            json["status"] = "healthy";
            json["service"] = "login-system";
            json["timestamp"] = (Json::Int64)trantor::Date::date().microSecondsSinceEpoch();
            
            auto resp = HttpResponse::newHttpJsonResponse(json);
            callback(resp);
        });
    
    APP_LOG_INFO("服务器运行信息:");
    APP_LOG_INFO_FMT("  地址: http://0.0.0.0:8080");
    APP_LOG_INFO_FMT("  本地访问: http://localhost:8080");
    APP_LOG_INFO_FMT("  健康检查: http://localhost:8080/health");
    APP_LOG_INFO("API端点:");
    APP_LOG_INFO("  POST /api/register - 用户注册");
    APP_LOG_INFO("  POST /api/login    - 用户登录");
    APP_LOG_INFO("  GET  /api/user     - 获取用户信息");
    APP_LOG_INFO("按 Ctrl+C 停止服务器");
    

    app().run();
    

    connPool->ClosePool();
    APP_LOG_INFO("服务器已停止，连接池已清理");
    
    return 0;
}