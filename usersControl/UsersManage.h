#pragma once

#include "User.h"
#include "../sql/sql_wrapper.h"
#include <mysql/mysql.h>
#include <string>
#include <memory>

namespace WEB_ESSAY_LIBRARY {

class UserDataController {
public:
    UserDataController() = default;
    
    
    // 用户注册
    bool registerUser(const std::string& username, const std::string& password, 
                     const std::string& email);
    
    // 用户登录验证
    bool loginUser(const std::string& username, const std::string& password);
    
    // 根据用户名获取用户信息
    std::shared_ptr<User> getUserByEmail(const std::string& username);
    bool updataUserInfo(User &user);
    

    
    // check if username or email already exists
    bool usernameExists(const std::string& username);
    bool emailExists(const std::string& email);
    
    bool dataExists(const std::string& column, const std::string value);
    
private:
    WEB_ESSAY_LIBRARY::SqlWrapper sql_wrapper;

    bool checkName(const std::string& username);
    bool checkEmail(const std::string& email);

    // 从结果集中提取用户信息
    std::shared_ptr<User> extractUserFromResult(MYSQL_RES* result);
};

}

