#pragma once

#include <string>
#include <ctime>
namespace WEB_ESSAY_LIBRARY {

class User {
public:
    User() = default;
    
    User(int64_t id, const std::string& username, const std::string& password, 
         const std::string& email, const std::string& createdAt)
        : id(id), username(username), password(password), 
          email(email), createdAt(createdAt) {}
    
    // Getters
    int64_t getId() const { return id; }
    const std::string& getUsername() const { return username; }
    const std::string& getPassword() const { return password; }
    const std::string& getEmail() const { return email; }
    const std::string& getCreatedAt() const { return createdAt; }
    const std::string& getLastActive() const { return lastActive; }
    bool isChanged() const { return ischanged; }

    void userInit(int64_t id, const std::string& username, const std::string& password, 
         const std::string& email, const std::string& createdAt, const std::string& lastActive) {
        this->id = id;
        this->username = username;
        this->password = password;
        this->email = email;
        this->createdAt = createdAt;
        this->lastActive = lastActive;
    }

    // Setters
    void setUsername(const std::string& username) { ischanged = true; this->username = username; }
    void setPassword(const std::string& password) { ischanged = true; this->password = password; }
    void setEmail(const std::string& email) { ischanged = true; this->email = email; }
    void setCreatedAt(const std::string& createdAt) { ischanged = true; this->createdAt = createdAt; }
    void setLastActive(const std::string& lastActive) { ischanged = true; this->lastActive = lastActive; }
    
private:
    int64_t id = 0;
    std::string username;
    std::string password;
    std::string email;
    std::string createdAt;
    std::string lastActive;
    bool ischanged = false;
};

}