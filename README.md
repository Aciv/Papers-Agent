# arXiv论文实时检索与智能问答系统

一个基于人工智能的学术论文检索与问答系统，支持实时arXiv论文搜索、语义检索、智能问答和个性化论文收藏管理。

## 🌟 系统特性

- **实时arXiv论文检索**：支持关键词搜索和语义相似度检索
- **智能问答助手**：基于大语言模型的论文内容问答系统
- **个性化论文收藏**：用户可收藏和管理感兴趣的论文
- **语义向量检索**：使用FAISS进行高效的向量相似度搜索
- **多用户系统**：完整的用户注册、登录和会话管理
- **响应式Web界面**：现代化的前端界面，支持实时聊天

## 🏗️ 系统架构

系统采用前后端分离架构，包含以下主要组件：

### 后端服务
1. **C++ Web服务器** (端口: 8080)
   - 基于Drogon框架的高性能HTTP服务器
   - 用户认证和会话管理
   - 论文收藏管理API
   - 数据库连接池管理

2. **Python Agent服务器** (端口: 5000)
   - 基于LangChain的智能问答系统
   - 异步任务处理池
   - arXiv论文搜索和PDF解析工具
   - 大语言模型集成（支持通义千问等）

### 前端界面
- 现代化的HTML/CSS/JavaScript界面
- 实时WebSocket聊天功能
- 响应式设计，支持移动端
- 用户友好的论文浏览和搜索界面

### 数据存储
- **MySQL数据库**：存储用户信息、论文元数据和收藏关系
- **FAISS向量索引**：存储论文语义向量，支持快速相似度检索
- **本地文件系统**：存储PDF论文文件和模型文件



## 🚀 快速开始

### 环境要求

- **操作系统**: Linux
- **Python**: 3.8+
- **C++编译器**: GCC 
- **MySQL**: 5.7+
- **CMake**: 3.16+

### 1. 数据库设置

```sql
-- 创建数据库
CREATE DATABASE web_essay_library CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 创建用户表
CREATE TABLE users (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    email VARCHAR(255) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 创建论文表
CREATE TABLE papers (
    arxiv_id VARCHAR(50) PRIMARY KEY,
    title TEXT NOT NULL,
    abstract TEXT,
    authors TEXT,
    categories TEXT,
    published_date DATE,
    pdf_url TEXT,
    vector_path TEXT
);

-- 创建用户收藏表
CREATE TABLE user_collections (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id BIGINT NOT NULL,
    arxiv_id VARCHAR(50) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (arxiv_id) REFERENCES papers(arxiv_id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_paper (user_id, arxiv_id)
);
```

### 2. 启动Python Agent服务器

```bash
cd agent_server

# 安装Python依赖
pip install -r requirements.txt

# 配置环境变量（编辑.env文件）
# QWEN_MODEL=qwen-max
# QWEN_BASE_URL=https://dashscope.aliyuncs.com/compatible-mode/v1
# QWEN_API_KEY=your_api_key_here

# 启动服务器
python main.py
```

Agent服务器将在 `http://localhost:5000` 启动，提供以下API端点：
- `POST /chat` - 处理聊天请求
- `GET /api/status` - 获取服务器状态
- `GET /api/health` - 健康检查

### 3. 构建和启动C++ Web服务器

```bash
cd web_server

# 创建构建目录
mkdir build && cd build

# 配置CMake（根据你的环境调整路径）
cmake -DCMAKE_PREFIX_PATH=/path/to/external/Log ..

# 编译
cmake --build . --config Release

# 启动服务器
./web_server
```

Web服务器将在 `http://localhost:8080` 启动，提供以下功能：
- 用户注册和登录
- 论文搜索和浏览
- 论文收藏管理
- 实时聊天界面

### 4. 访问Web界面

1. 打开浏览器访问 `http://localhost:8080`
2. 注册新账户或登录现有账户
3. 使用搜索功能查找arXiv论文
4. 点击"连接聊天助手"开始智能问答

## 🔧 配置说明

### Agent服务器配置 (agent_server/.env)

```env
# 通义千问API配置
QWEN_MODEL=qwen-max
QWEN_BASE_URL=https://dashscope.aliyuncs.com/compatible-mode/v1
QWEN_API_KEY=your_api_key_here

# 服务器配置
HOST=0.0.0.0
PORT=5000
WORKER_COUNT=3
MAX_QUEUE_SIZE=50
```

### Web服务器配置 (web_server/main.cpp)

主要配置项：
- MySQL连接参数（主机、端口、用户名、密码、数据库名）
- 服务器监听地址和端口
- 会话超时时间
- CORS配置

## 📊 API文档

### 用户认证API

| 方法 | 端点 | 描述 |
|------|------|------|
| POST | `/api/register` | 用户注册 |
| POST | `/api/login` | 用户登录 |
| GET | `/api/user` | 获取用户信息 |

### 论文管理API

| 方法 | 端点 | 描述 |
|------|------|------|
| POST | `/api/papers/add` | 添加论文到收藏 |
| POST | `/api/papers/delete` | 从收藏删除论文 |
| GET | `/api/papers/list` | 获取用户收藏的论文列表 |

### 聊天API

| 方法 | 端点 | 描述 |
|------|------|------|
| POST | `/api/chat` | 发送聊天消息（HTTP） |
| WS | `/api/chat` | WebSocket聊天连接 |

### Agent服务器API

| 方法 | 端点 | 描述 |
|------|------|------|
| POST | `/chat` | 处理聊天请求 |
| GET | `/api/status` | 获取服务器状态 |
| GET | `/api/health` | 健康检查 |

## 🛠️ 开发指南

### 添加新的Agent工具

1. 在 `agent_server/agent/tools.py` 中定义新工具函数
2. 使用 `@tool` 装饰器注册工具
3. 在 `agent_server/agent/agent.py` 的 `AsyncAgentWorker` 类中添加工具到工具列表

### 扩展论文搜索功能

1. 修改 `agent_server/agent/tools.py` 中的 `search_papers` 函数
2. 添加新的搜索参数或过滤条件
3. 更新前端搜索界面以支持新功能

### 自定义语义模型

1. 替换 `agent_server/model/` 目录中的模型文件
2. 确保模型与sentence-transformers兼容
3. 更新向量维度配置（当前为384维）
1. 修改 `agent_server/agent/tools.py` 中的 `search_papers` 函数
2. 添加新的搜索参数或过滤条件
3. 更新前端搜索界面以支持新功能

### 自定义语义模型

1. 替换 `agent_server/model/` 目录中的模型文件
2. 确保模型与sentence-transformers兼容
3. 更新向量维度配置（当前为384维）
