#include "UsersManage.h"
#include <regex>
#include <iostream>
#include "../log_wrapper.h"

namespace WEB_ESSAY_LIBRARY {

bool UserDataController::checkName(const std::string& username) {
    // 用户名必须是3-20个字符，可以包含字母、数字和下划线
    std::regex pattern("^[a-zA-Z0-9_]{3,20}$");
    return std::regex_match(username, pattern);
}
bool UserDataController::checkEmail(const std::string& email) {
    // 邮箱格式必须合法
    std::regex pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return std::regex_match(email, pattern);
}

bool UserDataController::registerUser(const std::string& username, const std::string& password, 
                          const std::string& email) {
    
    if (emailExists(email)) {
        APP_LOG_WARN_FMT("邮箱已存在: %s", email.c_str());
        return false;
    }
    
    if(!checkName(username) || !checkEmail(email)) {
        APP_LOG_WARN("用户名或邮箱格式不合法");
        return false;
    }
    

    // 插入新用户
    std::string sql = "INSERT INTO users (user_name, user_password, user_email) VALUES ('" +
                     username + "', '" + password + "', '" + email + "')";
    
    return executeQuery(sql);
}

bool UserDataController::loginUser(const std::string& email, const std::string& password) {
    std::string sql = "SELECT user_id FROM users WHERE user_email = '" + email + 
                     "' AND user_password = '" + password + "'";
    
    MYSQL_RES* result = executeQueryWithResult(sql);
    if (!result) {
        return false;
    }
    
    bool exists = (mysql_num_rows(result) > 0);
    mysql_free_result(result);
    
    return exists;
}

std::shared_ptr<User> UserDataController::getUserByEmail(const std::string& email) {
    std::string sql = "SELECT user_id, user_name, user_password, user_email, created_at, "
                       "last_active FROM users WHERE user_email = '" + email + "'";
    
    MYSQL_RES* result = executeQueryWithResult(sql);
    if (!result) {
        return nullptr;
    }
    
    auto user = extractUserFromResult(result);
    mysql_free_result(result);
    
    return user;
}

bool UserDataController::updataUserInfo(User &user){
    if(!user.isChanged()) {
        return true;
    }

    std::string sql = "UPDATE users SET user_name = '" + user.getUsername() + 
                      "', user_password = '" + user.getPassword() + 
                      "', user_email = '" + user.getEmail() + 
                      "', last_active = '" + user.getLastActive() + 
                      "' WHERE user_id = " + std::to_string(user.getId());
    
    return executeQuery(sql);
}


bool UserDataController::usernameExists(const std::string& username) {
    return dataExists("user_name", username);
}

bool UserDataController::emailExists(const std::string& email) {
    return dataExists("user_email", email);
}

bool UserDataController::dataExists(const std::string& column, const std::string value) {
    std::string sql = "SELECT user_id FROM users WHERE " + column + " = '" + value + "'";
    
    MYSQL_RES* result = executeQueryWithResult(sql);
    if (!result) {
        return false;
    }
    
    bool exists = (mysql_num_rows(result) > 0);
    mysql_free_result(result);
    
    return exists;

}


bool UserDataController::executeQuery(const std::string& sql) {
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

MYSQL_RES* UserDataController::executeQueryWithResult(const std::string& sql) {
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

std::shared_ptr<User> UserDataController::extractUserFromResult(MYSQL_RES* result) {
    if (!result || mysql_num_rows(result) == 0) {
        return nullptr;
    }
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        return nullptr;
    }
    
    auto user = std::make_shared<User>();
    
    // 注意：row[0]是id，row[1]是username，row[2]是password，row[3]是email，row[4]是created_at
    user->userInit(row[0] ? std::stoi(row[0]) : 0,
              row[1] ? row[1] : "",
              row[2] ? row[2] : "",
              row[3] ? row[3] : "",
              row[4] ? row[4] : "",
              row[5] ? row[5] : "");


    return user;
}

}