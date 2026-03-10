#pragma once

#include "install/include/log.h"
#include "install/include/stdoutSender.h"
#include <memory>

namespace LogWrapper {
    

    inline void init_logger() {
        auto& logger = Aciv::utility::Log::get_Instance();
        logger.init(1024, 512);
        

        auto stdout_sender = std::make_unique<Aciv::utility::Stdout_sender>();
        logger.add_sender(std::move(stdout_sender));
        

        logger.set_pattern("[%T] [%l] %v \n");
    }
    
    // 日志记录宏 - 使用不同的前缀避免与Drogon宏冲突
    #define APP_LOG_INFO(msg) Aciv::utility::Log::get_Instance().record(Aciv::utility::level::info, msg)
    #define APP_LOG_WARN(msg) Aciv::utility::Log::get_Instance().record(Aciv::utility::level::warning, msg)
    #define APP_LOG_ERROR(msg) Aciv::utility::Log::get_Instance().record(Aciv::utility::level::error, msg)
    #define APP_LOG_CRITICAL(msg) Aciv::utility::Log::get_Instance().record(Aciv::utility::level::critical, msg)
    #define APP_LOG_DEBUG(msg) Aciv::utility::Log::get_Instance().record(Aciv::utility::level::debug, msg)
    

    #define APP_LOG_INFO_FMT(fmt, ...) do { \
        char buffer[512]; \
        snprintf(buffer, sizeof(buffer), fmt, ##__VA_ARGS__); \
        Aciv::utility::Log::get_Instance().record(Aciv::utility::level::info, buffer); \
    } while(0)
    
    #define APP_LOG_WARN_FMT(fmt, ...) do { \
        char buffer[512]; \
        snprintf(buffer, sizeof(buffer), fmt, ##__VA_ARGS__); \
        Aciv::utility::Log::get_Instance().record(Aciv::utility::level::warning, buffer); \
    } while(0)
    
    #define APP_LOG_ERROR_FMT(fmt, ...) do { \
        char buffer[512]; \
        snprintf(buffer, sizeof(buffer), fmt, ##__VA_ARGS__); \
        Aciv::utility::Log::get_Instance().record(Aciv::utility::level::error, buffer); \
    } while(0)
    
    #define APP_LOG_CRITICAL_FMT(fmt, ...) do { \
        char buffer[512]; \
        snprintf(buffer, sizeof(buffer), fmt, ##__VA_ARGS__); \
        Aciv::utility::Log::get_Instance().record(Aciv::utility::level::critical, buffer); \
    } while(0)
    
    #define APP_LOG_DEBUG_FMT(fmt, ...) do { \
        char buffer[512]; \
        snprintf(buffer, sizeof(buffer), fmt, ##__VA_ARGS__); \
        Aciv::utility::Log::get_Instance().record(Aciv::utility::level::debug, buffer); \
    } while(0)
    
}