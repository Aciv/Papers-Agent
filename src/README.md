# 论文管理系统 - 前端界面

基于Drogon后端控制器的简单前端交互界面。

## 功能特性

1. **用户认证**
   - 用户注册（用户名、密码、邮箱）
   - 用户登录（邮箱、密码）
   - 会话管理（基于localStorage）

2. **论文管理**
   - 添加arXiv论文收藏
   - 删除论文收藏
   - 查看收藏的论文列表

## 文件结构

```
src/
├── index.html          # 首页
├── login.html          # 登录页面
├── register.html       # 注册页面
├── papers.html         # 论文管理页面
├── css/
│   └── style.css      # 样式文件
├── js/
│   ├── api.js         # API接口封装
│   └── auth.js        # 认证功能
└── README.md          # 说明文档
```

## API接口

前端与以下后端API交互：

### LoginController
- `POST /api/register` - 用户注册
- `POST /api/login` - 用户登录
- `GET /api/user` - 获取用户信息

### PapersController
- `POST /api/paper/add` - 添加论文收藏
- `POST /api/paper/delete` - 删除论文收藏
- `GET /api/paper/show` - 获取论文列表

## 使用说明

### 1. 启动后端服务器
确保Drogon后端服务器正在运行（默认端口8848）：
```bash
# 假设后端已编译并运行
./your-drogon-app
```

### 2. 访问前端页面
可以直接在浏览器中打开HTML文件，或使用简单的HTTP服务器：

```bash
# 使用Python启动简单HTTP服务器
cd src
python3 -m http.server 8000
```

然后在浏览器中访问：http://localhost:8000

### 3. 使用流程
1. 访问首页（index.html）
2. 点击"注册"创建新账户
3. 使用注册的邮箱和密码登录
4. 登录后可以：
   - 在"我的论文"页面添加arXiv论文ID
   - 查看已收藏的论文列表
   - 删除不需要的论文收藏

## 技术实现

### 前端技术
- 纯HTML/CSS/JavaScript（无框架）
- 使用Fetch API进行HTTP请求
- 使用localStorage存储用户会话
- 响应式设计，支持移动端

### 主要JavaScript模块

#### api.js
封装所有与后端API的交互，提供简洁的接口：
```javascript
API.register(username, password, email)
API.login(user_email, password)
API.addPaper(arxiv_id)
API.deletePaper(arxiv_id)
API.getPapers()
```

#### auth.js
处理用户认证状态和导航：
- `checkLoginStatus()` - 检查并更新登录状态
- `logout()` - 用户登出
- `requireLogin()` - 页面访问控制
- `validateSession()` - 验证会话有效性

## 配置说明

### API基础URL
在`js/api.js`中修改`baseURL`变量以匹配后端服务器地址：
```javascript
const API = {
    baseURL: 'http://localhost:8848', // 修改为实际的后端地址
    // ...
};
```

### 会话管理
- 使用浏览器localStorage存储用户信息
- 会话状态在页面刷新后保持
- 支持跨页面导航的状态同步

## 注意事项

1. **CORS配置**：确保后端服务器配置了正确的CORS头，允许前端访问
2. **Session支持**：后端需要支持基于cookie的session管理
3. **arXiv ID格式**：添加论文时需要正确的arXiv ID格式（如：2401.12345）
4. **安全性**：前端仅做基本验证，主要安全验证应由后端完成

## 扩展建议

1. 添加论文搜索功能
2. 实现论文详情查看
3. 添加论文分类和标签
4. 实现用户个人资料编辑
5. 添加响应式通知系统
6. 集成更美观的UI组件库

## 故障排除

### 常见问题

1. **API请求失败**
   - 检查后端服务器是否运行
   - 确认API端口是否正确
   - 查看浏览器控制台错误信息

2. **登录状态不保持**
   - 检查浏览器是否禁用localStorage
   - 确认session cookie是否正确设置

3. **样式显示异常**
   - 检查CSS文件路径是否正确
   - 确认浏览器支持使用的CSS特性

### 调试方法
1. 打开浏览器开发者工具（F12）
2. 查看Console标签页的错误信息
3. 查看Network标签页的API请求响应
4. 检查Application标签页的localStorage内容

## 许可证

本项目基于MIT许可证开源。
