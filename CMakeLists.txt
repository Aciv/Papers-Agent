cmake_minimum_required(VERSION 3.5)
project(LoginSystem)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 查找Drogon库
find_package(Drogon REQUIRED)

# 查找JSON库
find_package(jsoncpp REQUIRED)

find_package(SimpleLog REQUIRED)

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MYSQL_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/install/include
)

# 源文件列表
set(SOURCES
    main.cpp
    sql/sqlconnpool.cpp
    usersControl/UsersManage.cpp
    controller/LoginController.cpp
)

# 头文件列表
set(HEADERS
    sql/sqlconnpool.h
    usersControl/UsersManage.h
    usersControl/User.h
    controller/LoginController.h
)

# 添加可执行文件
add_executable(login_system ${SOURCES} ${HEADERS})

# 链接库
target_link_libraries(login_system PRIVATE
    Drogon::Drogon
    SimpleLog::Log 
    ${MYSQL_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    pthread
    dl
    z
    ssl
    crypto
    mysqlclient
)

# 设置目标属性
set_target_properties(login_system PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# 添加编译定义
target_compile_definitions(login_system PRIVATE
    _GLIBCXX_USE_CXX11_ABI=1
)

# 添加编译选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(login_system PRIVATE
        -Wall
        -Wextra
        -Werror
        -O2
        -finput-charset=UTF-8
        -fexec-charset=UTF-8
        -lmysqlclient
    )
endif()

# 安装目标（可选）
# install(TARGETS login_system DESTINATION bin)

# 打印配置信息
message(STATUS "==========================================")
message(STATUS "Login System Build Configuration")
message(STATUS "==========================================")
message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Source Directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Binary Directory: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "MySQL Include: ${MYSQL_INCLUDE_DIRS}")
message(STATUS "MySQL Libraries: ${MYSQL_LIBRARIES}")
message(STATUS "JSONCPP Include: ${JSONCPP_INCLUDE_DIR}")
message(STATUS "JSONCPP Libraries: ${JSONCPP_LIBRARIES}")
message(STATUS "==========================================")
