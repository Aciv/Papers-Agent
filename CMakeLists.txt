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

# 查找SimpleLog库
find_package(SimpleLog REQUIRED)

# Find OpenMP
find_package(OpenMP REQUIRED)

# Find BLAS/LAPACK
find_package(BLAS REQUIRED)
find_package(LAPACK REQUIRED)

# Find FAISS library
set(FAISS_ROOT "${CMAKE_SOURCE_DIR}/external/faiss")
set(FAISS_INCLUDE_DIR "${FAISS_ROOT}/include")
set(FAISS_LIBRARY "${FAISS_ROOT}/lib/libfaiss.a")
# Check if FAISS library exists
if(NOT EXISTS "${FAISS_LIBRARY}")
    message(FATAL_ERROR "FAISS library not found at: ${FAISS_LIBRARY}")
endif()

if(NOT EXISTS "${FAISS_INCLUDE_DIR}")
    message(FATAL_ERROR "FAISS include directory not found at: ${FAISS_INCLUDE_DIR}")
endif()


# 源文件列表
set(SOURCES
    main.cpp
    sql/sqlconnpool.cpp
    usersControl/UsersManage.cpp
    controller/LoginController.cpp
    controller/queryController.cpp
    query/FaissQuery.cpp
)

# 头文件列表
set(HEADERS
    sql/sqlconnpool.h
    sql/sql_wrapper.h
    usersControl/UsersManage.h
    usersControl/User.h
    controller/LoginController.h
    controller/queryController.h
    query/FaissQuery.h
)

# 添加可执行文件
add_executable(login_system ${SOURCES} ${HEADERS})

target_include_directories(login_system PRIVATE ${FAISS_INCLUDE_DIR})
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
    ${FAISS_LIBRARY}
    OpenMP::OpenMP_CXX
    ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES}
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
