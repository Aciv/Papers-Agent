/**
 * 聊天WebSocket功能
 * 连接后端聊天WebSocket服务
 */

class ChatWebSocket {
    constructor() {
        this.ws = null;
        this.isConnected = false;
        this.reconnectAttempts = 0;
        this.maxReconnectAttempts = 5;
        this.reconnectDelay = 3000; // 3秒
    }

    /**
     * 连接到WebSocket服务器
     */
    connect() {
        if (this.isConnected) {
            console.log('WebSocket已经连接');
            return;
        }

        // 检查用户是否已登录
        const isLoggedIn = localStorage.getItem('isLoggedIn') === 'true';
        if (!isLoggedIn) {
            this.updateStatus('请先登录系统', 'disconnected');
            alert('请先登录系统以使用聊天功能');
            return;
        }

        // 构建WebSocket URL
        const baseURL = API.baseURL.replace('http://', 'ws://').replace('https://', 'wss://');
        const wsURL = `${baseURL}/api/chat`;
        
        console.log('正在连接到WebSocket:', wsURL);
        this.updateStatus('正在连接...', 'connecting');

        try {
            this.ws = new WebSocket(wsURL);
            
            this.ws.onopen = () => {
                console.log('WebSocket连接成功');
                this.isConnected = true;
                this.reconnectAttempts = 0;
                this.updateStatus('已连接到聊天服务器', 'connected');
                this.onConnected();
            };
            
            this.ws.onmessage = (event) => {
                console.log('收到消息:', event.data);
                this.onMessage(event.data);
            };
            
            this.ws.onerror = (error) => {
                console.error('WebSocket错误:', error);
                this.updateStatus('连接错误', 'disconnected');
            };
            
            this.ws.onclose = (event) => {
                console.log('WebSocket连接关闭:', event.code, event.reason);
                this.isConnected = false;
                this.onDisconnected();
                
                // 如果不是正常关闭，尝试重连
                if (event.code !== 1000 && this.reconnectAttempts < this.maxReconnectAttempts) {
                    this.attemptReconnect();
                }
            };
            
        } catch (error) {
            console.error('创建WebSocket连接失败:', error);
            this.updateStatus('连接失败', 'disconnected');
        }
    }

    /**
     * 断开WebSocket连接
     */
    disconnect() {
        if (this.ws) {
            this.ws.close(1000, '用户主动断开连接');
            this.ws = null;
        }
        this.isConnected = false;
        this.updateStatus('已断开连接', 'disconnected');
        this.onDisconnected();
    }

    /**
     * 发送消息
     * @param {string} message - 要发送的消息
     */
    send(message) {
        if (!this.isConnected || !this.ws) {
            console.error('WebSocket未连接，无法发送消息');
            this.updateStatus('未连接，无法发送消息', 'disconnected');
            return false;
        }

        if (this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(message);
            console.log('发送消息:', message);
            return true;
        } else {
            console.error('WebSocket未处于OPEN状态');
            this.updateStatus('连接异常，请重新连接', 'disconnected');
            return false;
        }
    }

    /**
     * 尝试重新连接
     */
    attemptReconnect() {
        this.reconnectAttempts++;
        console.log(`尝试重新连接 (${this.reconnectAttempts}/${this.maxReconnectAttempts})`);
        
        this.updateStatus(`正在重新连接... (${this.reconnectAttempts}/${this.maxReconnectAttempts})`, 'connecting');
        
        setTimeout(() => {
            if (!this.isConnected) {
                this.connect();
            }
        }, this.reconnectDelay);
    }

    /**
     * 更新连接状态显示
     * @param {string} message - 状态消息
     * @param {string} status - 状态类型: 'connected', 'disconnected', 'connecting'
     */
    updateStatus(message, status) {
        const statusElement = document.getElementById('connection-status');
        if (statusElement) {
            statusElement.textContent = this.getStatusEmoji(status) + ' ' + message;
            statusElement.className = 'connection-status ' + status;
        }
    }

    /**
     * 获取状态对应的表情符号
     * @param {string} status - 状态类型
     * @returns {string} 表情符号
     */
    getStatusEmoji(status) {
        switch(status) {
            case 'connected': return '🟢';
            case 'disconnected': return '🔴';
            case 'connecting': return '🟡';
            default: return '⚪';
        }
    }

    /**
     * 连接成功时的回调
     */
    onConnected() {
        // 启用输入框和发送按钮
        const messageInput = document.getElementById('message-input');
        const sendButton = document.getElementById('send-button');
        const connectButton = document.getElementById('connect-button');
        const disconnectButton = document.getElementById('disconnect-button');
        
        if (messageInput) messageInput.disabled = false;
        if (sendButton) sendButton.disabled = false;
        if (connectButton) connectButton.style.display = 'none';
        if (disconnectButton) disconnectButton.style.display = 'inline-block';
        
        // 添加欢迎消息
        this.addMessage('bot', '✅ 连接成功！现在您可以开始提问了。');
    }

    /**
     * 断开连接时的回调
     */
    onDisconnected() {
        // 禁用输入框和发送按钮
        const messageInput = document.getElementById('message-input');
        const sendButton = document.getElementById('send-button');
        const connectButton = document.getElementById('connect-button');
        const disconnectButton = document.getElementById('disconnect-button');
        
        if (messageInput) messageInput.disabled = true;
        if (sendButton) sendButton.disabled = true;
        if (connectButton) connectButton.style.display = 'inline-block';
        if (disconnectButton) disconnectButton.style.display = 'none';
    }

    /**
     * 收到消息时的回调
     * @param {string} message - 收到的消息
     */
    onMessage(message) {
        this.addMessage('bot', message);
    }

    /**
     * 添加消息到聊天界面
     * @param {string} sender - 发送者: 'user' 或 'bot'
     * @param {string} text - 消息文本
     */
    addMessage(sender, text) {
        const chatMessages = document.getElementById('chat-messages');
        if (!chatMessages) return;

        const messageDiv = document.createElement('div');
        messageDiv.className = `message ${sender}-message`;
        
        const time = new Date().toLocaleTimeString('zh-CN', { 
            hour: '2-digit', 
            minute: '2-digit',
            second: '2-digit'
        });
        
        let displayText = this.escapeHtml(text).replace(/\n/g, '<br>');
        
        messageDiv.innerHTML = `
            <p style="margin:0; white-space: pre-wrap; word-wrap: break-word;">${displayText}</p>
            <div class="message-time">${sender === 'user' ? '您' : '助手'} • ${time}</div>
        `;
        
        chatMessages.appendChild(messageDiv);
        
        // 滚动到底部
        chatMessages.scrollTop = chatMessages.scrollHeight;
    }

    /**
     * 转义HTML特殊字符
     * @param {string} text - 原始文本
     * @returns {string} 转义后的文本
     */
    escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }
}

// 创建全局ChatWebSocket实例
const chatSocket = new ChatWebSocket();

/**
 * 连接WebSocket
 */
function connectWebSocket() {
    chatSocket.connect();
}

/**
 * 断开WebSocket连接
 */
function disconnectWebSocket() {
    chatSocket.disconnect();
}

/**
 * 发送消息
 */
function sendMessage() {
    const messageInput = document.getElementById('message-input');
    const message = messageInput.value.trim();
    
    if (!message) {
        alert('请输入消息');
        return;
    }
    
    if (chatSocket.send(message)) {
        // 添加用户消息到界面
        chatSocket.addMessage('user', message);
        // 清空输入框
        messageInput.value = '';
        // 聚焦到输入框
        messageInput.focus();
    }
}

/**
 * 处理回车键发送
 */
document.addEventListener('DOMContentLoaded', function() {
    const messageInput = document.getElementById('message-input');
    if (messageInput) {
        messageInput.addEventListener('keypress', function(event) {
            if (event.key === 'Enter' && !event.shiftKey) {
                event.preventDefault();
                sendMessage();
            }
        });
    }
});

/**
 * 检查用户登录状态并更新界面
 */
function updateChatInterface() {
    const isLoggedIn = localStorage.getItem('isLoggedIn') === 'true';
    const connectButton = document.getElementById('connect-button');
    
    if (connectButton) {
        if (isLoggedIn) {
            connectButton.disabled = false;
            connectButton.textContent = '连接聊天助手';
        } else {
            connectButton.disabled = true;
            connectButton.textContent = '请先登录';
        }
    }
}

// 监听登录状态变化
window.addEventListener('storage', function(event) {
    if (event.key === 'isLoggedIn') {
        updateChatInterface();
    }
});

// 初始化界面
document.addEventListener('DOMContentLoaded', updateChatInterface);