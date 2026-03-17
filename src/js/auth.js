/**
 * 认证相关功能
 * 处理用户登录状态和导航
 */

/**
 * 检查用户登录状态并更新导航栏
 */
function checkLoginStatus() {
    const isLoggedIn = localStorage.getItem('isLoggedIn') === 'true';
    const loginLink = document.getElementById('login-link');
    const registerLink = document.getElementById('register-link');
    const papersLink = document.getElementById('papers-link');
    const chatLink = document.getElementById('chat-link');
    const logoutLink = document.getElementById('logout-link');
    const userStatus = document.getElementById('user-status');
    
    if (isLoggedIn) {
        // 用户已登录
        if (loginLink) loginLink.style.display = 'none';
        if (registerLink) registerLink.style.display = 'none';
        if (papersLink) papersLink.style.display = 'inline-block';
        if (chatLink) chatLink.style.display = 'inline-block';
        if (logoutLink) logoutLink.style.display = 'inline-block';
        
        // 显示用户信息
        if (userStatus) {
            const user = JSON.parse(localStorage.getItem('user') || '{}');
            userStatus.innerHTML = `
                <h3>👋 欢迎回来，${user.username || '用户'}！</h3>
                <p>邮箱：${user.email || '未设置'}</p>
                <p>注册时间：${user.created_at || '未知'}</p>
                <p>
                    <a href="papers.html" class="btn btn-primary">管理我的论文</a>
                    <a href="chat.html" class="btn btn-success">智能聊天助手</a>
                </p>
            `;
        }
    } else {
        // 用户未登录
        if (loginLink) loginLink.style.display = 'inline-block';
        if (registerLink) registerLink.style.display = 'inline-block';
        if (papersLink) papersLink.style.display = 'none';
        if (chatLink) chatLink.style.display = 'none';
        if (logoutLink) logoutLink.style.display = 'none';
        
        // 显示登录提示
        if (userStatus) {
            userStatus.innerHTML = `
                <h3>🔐 请先登录</h3>
                <p>登录后可以收藏和管理您的论文，并使用智能聊天助手</p>
                <p>
                    <a href="login.html" class="btn btn-primary">登录</a>
                    <a href="register.html" class="btn">注册</a>
                </p>
            `;
        }
    }
}

/**
 * 用户登出
 */
function logout() {
    if (confirm('确定要退出登录吗？')) {
        // 清除本地存储
        localStorage.removeItem('user');
        localStorage.removeItem('isLoggedIn');
        
        // 发送登出请求到服务器（如果需要）
        fetch(`${API.baseURL}/api/logout`, {
            method: 'POST',
            credentials: 'include'
        }).catch(error => {
            console.error('Logout request failed:', error);
        });
        
        // 更新页面状态
        checkLoginStatus();
        
        // 跳转到首页
        window.location.href = 'index.html';
    }
}

/**
 * 验证用户是否已登录
 * 如果未登录，重定向到登录页面
 * @param {string} redirectTo - 重定向页面（默认为login.html）
 */
function requireLogin(redirectTo = 'login.html') {
    const isLoggedIn = localStorage.getItem('isLoggedIn') === 'true';
    if (!isLoggedIn) {
        window.location.href = redirectTo;
        return false;
    }
    return true;
}

/**
 * 验证用户是否未登录
 * 如果已登录，重定向到指定页面
 * @param {string} redirectTo - 重定向页面（默认为index.html）
 */
function requireGuest(redirectTo = 'index.html') {
    const isLoggedIn = localStorage.getItem('isLoggedIn') === 'true';
    if (isLoggedIn) {
        window.location.href = redirectTo;
        return false;
    }
    return true;
}

/**
 * 获取当前用户信息
 * @returns {Object|null} 用户信息或null
 */
function getCurrentUser() {
    const userStr = localStorage.getItem('user');
    if (userStr) {
        try {
            return JSON.parse(userStr);
        } catch (error) {
            console.error('Failed to parse user data:', error);
            return null;
        }
    }
    return null;
}

/**
 * 更新用户信息
 * @param {Object} userData - 新的用户数据
 */
function updateUserInfo(userData) {
    const currentUser = getCurrentUser() || {};
    const updatedUser = { ...currentUser, ...userData };
    localStorage.setItem('user', JSON.stringify(updatedUser));
}

/**
 * 检查用户会话是否有效
 * 通过API验证用户登录状态
 * @returns {Promise<boolean>} 会话是否有效
 */
async function validateSession() {
    try {
        const result = await API.getUser();
        return result.code === 200;
    } catch (error) {
        console.error('Session validation failed:', error);
        return false;
    }
}

/**
 * 自动检查会话有效性
 * 如果会话无效，清除登录状态
 */
async function autoValidateSession() {
    const isLoggedIn = localStorage.getItem('isLoggedIn') === 'true';
    if (isLoggedIn) {
        const isValid = await validateSession();
        if (!isValid) {
            // 会话无效，清除登录状态
            localStorage.removeItem('user');
            localStorage.removeItem('isLoggedIn');
            checkLoginStatus();
            
            // 如果当前页面需要登录，重定向到登录页面
            const protectedPages = ['papers.html'];
            const currentPage = window.location.pathname.split('/').pop();
            if (protectedPages.includes(currentPage)) {
                window.location.href = 'login.html';
            }
        }
    }
}

