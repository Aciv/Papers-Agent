#include "sqlconnpool.h"
#include "../log_wrapper.h"

namespace WEB_ESSAY_LIBRARY {

class SqlWrapper {
public:
    SqlWrapper(){ conn_pool = cSqlConnPool::Instance(); }
    ~SqlWrapper() = default;



    bool executeQuery(const std::string& sql){
        MYSQL* conn = conn_pool->GetConn();
        if (!conn) {
            APP_LOG_ERROR("获取数据库连接失败");
            return false;
        }
        
        if (mysql_query(conn, sql.c_str())) {
            APP_LOG_ERROR_FMT("SQL执行错误: %s", mysql_error(conn));
            conn_pool->FreeConn(conn);
            return false;
        }
        
        conn_pool->FreeConn(conn);
        return true;
    }
    

    // with result
    MYSQL_RES* executeQueryWithResult(const std::string& sql){
        MYSQL* conn = conn_pool->GetConn();
        if (!conn) {
            APP_LOG_ERROR("获取数据库连接失败");
            return nullptr;
        }
        
        if (mysql_query(conn, sql.c_str())) {
            APP_LOG_ERROR_FMT("SQL执行错误: %s", mysql_error(conn));
            conn_pool->FreeConn(conn);
            return nullptr;
        }
        
        MYSQL_RES* result = mysql_store_result(conn);
        conn_pool->FreeConn(conn);
        
        return result;
    }


private:
    cSqlConnPool* conn_pool;

};

}