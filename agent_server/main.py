import asyncio
from aiohttp import web
import json
import logging
from typing import Dict, Any, Optional
import uuid
from datetime import datetime


from agent.agentPool import AsyncAgentPool
from agent.agent import AgentTask


logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

class AgentServer:

    def __init__(self, host: str = "0.0.0.0", port: int = 5000):
        self.host = host
        self.port = port
        self.app = web.Application()
        self.agent_pool = AsyncAgentPool(num_workers=3, max_queue_size=50)
        self.setup_routes()
        
    def setup_routes(self):

        self.app.router.add_post('/chat', self.handle_chat)
        self.app.router.add_get('/api/status', self.handle_status)
        self.app.router.add_get('/api/health', self.handle_health)
        self.app.router.add_post('/api/batch_chat', self.handle_batch_chat)
        
    async def handle_chat(self, request: web.Request) -> web.Response:

        try:
            # 解析请求体
            data = await request.json()
            logger.info(f"收到聊天请求: {data}")
            
            # 验证必需字段
            required_fields = ['user_id', 'message']
            for field in required_fields:
                if field not in data:
                    return web.json_response(
                        {'error': f'Missing required field: {field}'},
                        status=400
                    )
            
            user_id = data['user_id']
            message = data['message']
            user_email = data.get('user_email', f'{user_id}@example.com')
            
            # 提交请求到agent池
            result = await self.agent_pool.submit_request(user_id, user_email, message)
            
            if result is None:
                return web.json_response(
                    {'error': 'Failed to submit request to agent pool'},
                    status=503
                )
            
            # 返回响应
            response_data = {
                'task_id': str(uuid.uuid4()),
                'user_id': user_id,
                'reply': result,
                'timestamp': datetime.now().isoformat()
            }
            
            return web.json_response(response_data)
            
        except json.JSONDecodeError:
            return web.json_response(
                {'error': 'Invalid JSON in request body'},
                status=400
            )
        except Exception as e:
            logger.error(f"处理聊天请求时出错: {e}")
            return web.json_response(
                {'error': f'Internal server error: {str(e)}'},
                status=500
            )
    
   
    
    async def handle_status(self, request: web.Request) -> web.Response:
        """获取服务器和agent池状态"""
        try:
            # 获取agent池状态
            pool_status = await self.agent_pool.get_pool_status()
            
            # 获取服务器信息
            server_status = {
                'server': {
                    'host': self.host,
                    'port': self.port,
                    'status': 'running',
                    'uptime': 'N/A',  # 实际实现中可以跟踪启动时间
                    'timestamp': datetime.now().isoformat()
                },
                'agent_pool': pool_status,
                'system': {
                    'python_version': '3.x',  # 实际实现中可以获取真实版本
                    'platform': 'Windows/Linux'
                }
            }
            
            return web.json_response(server_status)
            
        except Exception as e:
            logger.error(f"获取状态时出错: {e}")
            return web.json_response(
                {'error': f'Failed to get status: {str(e)}'},
                status=500
            )
    
    async def handle_health(self, request: web.Request) -> web.Response:
        """健康检查端点"""
        try:
            # 检查agent池是否运行
            pool_status = await self.agent_pool.get_pool_status()
            
            health_status = {
                'status': 'healthy' if pool_status['is_running'] else 'unhealthy',
                'timestamp': datetime.now().isoformat(),
                'checks': {
                    'agent_pool': pool_status['is_running'],
                    'active_workers': pool_status['active_workers'] > 0,
                    'queue_healthy': pool_status['queue_size'] < pool_status['queue_max'] * 0.8
                }
            }
            
            status_code = 200 if health_status['status'] == 'healthy' else 503
            return web.json_response(health_status, status=status_code)
            
        except Exception as e:
            logger.error(f"健康检查时出错: {e}")
            return web.json_response(
                {'status': 'unhealthy', 'error': str(e)},
                status=503
            )
    
    async def start(self):
        """启动服务器"""

        await self.agent_pool.start()
        logger.info(f"Agent池已启动")

        runner = web.AppRunner(self.app)
        await runner.setup()
        

        site = web.TCPSite(runner, self.host, self.port)
        await site.start()
        
        logger.info(f"服务器已启动，监听地址: http://{self.host}:{self.port}")
        logger.info("可用端点:")
        logger.info("  POST /api/chat - 处理单个聊天请求")
        logger.info("  GET /api/status - 获取服务器状态")
        logger.info("  GET /api/health - 健康检查")
        
        return runner
    
    async def stop(self):

        logger.info("正在停止服务器...")
        await self.agent_pool.shutdown(wait=True)
        logger.info("服务器已停止")


async def main():

    # 创建服务器实例
    server = AgentServer(host="0.0.0.0", port=5000)
    
    try:
        # 启动服务器
        runner = await server.start()
        
        # 保持服务器运行
        print("\n" + "="*50)
        print("Agent服务器正在运行...")
        print("按 Ctrl+C 停止服务器")
        print("="*50 + "\n")
        
        # 等待中断信号
        await asyncio.Event().wait()
        
    except KeyboardInterrupt:
        print("\n收到中断信号，正在关闭服务器...")
    except Exception as e:
        logger.error(f"服务器运行时出错: {e}")
    finally:
        # 确保服务器被正确关闭
        await server.stop()


if __name__ == "__main__":

    asyncio.run(main())