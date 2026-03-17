/**
 * API接口封装
 * 与后端Drogon控制器交互
 */

const API = {
    // API基础URL
    baseURL: 'http://localhost:8080', 
    
    /**
     * 发送HTTP请求
     * @param {string} method - HTTP方法
     * @param {string} endpoint - API端点
     * @param {Object} data - 请求数据
     * @returns {Promise<Object>} - 响应数据
     */
    async request(method, endpoint, data = null) {
        const url = `${this.baseURL}${endpoint}`;
        const options = {
            method,
            headers: { 'Content-Type': 'application/json' },
            credentials: 'include'
        };
        if (data) options.body = JSON.stringify(data);

        const response = await fetch(url, options);
        
        // 尝试解析响应体为 JSON（无论状态码如何）
        let result;
        try {
            result = await response.json();
        } catch (e) {
            // 响应不是 JSON（例如 HTML 错误页），则构造一个通用错误
            throw new Error(`HTTP ${response.status}: ${response.statusText}`);
        }

        // 如果状态码不是 2xx，将解析后的错误信息抛出
        if (!response.ok) {
            const error = new Error(result.message || `请求失败 (${response.status})`);
            error.status = response.status;
            error.data = result;   // 保留原始错误数据
            throw error;
        }

        return result;
    },
    
    /**
     * 用户注册
     * @param {string} username - 用户名
     * @param {string} password - 密码
     * @param {string} email - 邮箱
     * @returns {Promise<Object>} - 注册结果
     */
    async register(username, password, email) {
        return this.request('POST', '/api/register', {
            username,
            password,
            email
        });
    },
    
    /**
     * 用户登录
     * @param {string} user_email - 用户邮箱
     * @param {string} password - 密码
     * @returns {Promise<Object>} - 登录结果
     */
    async login(user_email, password) {
        return this.request('POST', '/api/login', {
            user_email,
            password
        });
    },
    
    /**
     * 获取用户信息
     * @returns {Promise<Object>} - 用户信息
     */
    async getUser() {
        return this.request('GET', '/api/user');
    },
    
    /**
     * 添加论文收藏
     * @param {string} arxiv_id - arXiv论文ID
     * @returns {Promise<Object>} - 添加结果
     */
    async addPaper(arxiv_id) {
        return this.request('POST', '/api/paper/add', {
            arxiv_id
        });
    },
    
    /**
     * 删除论文收藏
     * @param {string} arxiv_id - arXiv论文ID
     * @returns {Promise<Object>} - 删除结果
     */
    async deletePaper(arxiv_id) {
        return this.request('POST', '/api/paper/delete', {
            arxiv_id
        });
    },
    
    /**
     * 获取用户收藏的论文列表
     * @returns {Promise<Object>} - 论文列表
     */
    async getPapers() {
        return this.request('GET', '/api/paper/show');
    }
};

// 导出API对象
if (typeof module !== 'undefined' && module.exports) {
    module.exports = API;
}
