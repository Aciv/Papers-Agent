import asyncio
from typing import Dict, List, Optional, Any
from langchain_core.runnables import RunnableConfig
from dataclasses import dataclass, field


from agent.agent import AsyncAgentWorker, AgentTask

import uuid

class AsyncAgentPool:
    def __init__(
        self,
        num_workers: int = 5,
        max_queue_size: int = 100,
    ):

        self.num_workers = num_workers
        self.max_queue_size = max_queue_size

        self.task_queue: asyncio.Queue = asyncio.Queue(maxsize=max_queue_size)

        self.workers: List[AsyncAgentWorker] = []
        
        self.history_dict: Dict[str, RunnableConfig] = {}

        self.is_running = False
        self.worker_tasks: List[asyncio.Task] = []
    
    async def start(self):

        if self.is_running:
            return
        

        self.workers = [AsyncAgentWorker(i) for i in range(self.num_workers)]
        

        self.is_running = True
        for worker in self.workers:
            task = asyncio.create_task(self._worker_loop(worker))
            self.worker_tasks.append(task)
        
        print(f"AsyncAgentPool 已启动，{self.num_workers} 个工作器运行中")
    
    async def _worker_loop(self, worker: AsyncAgentWorker):
        """工作器主循环"""
        worker.is_running = True
        
        while self.is_running:
            try:
                # 从队列获取任务
                task = await asyncio.wait_for(
                    self.task_queue.get(),
                    timeout=1.0
                )
                
                # 处理任务
                processed_task = await worker.process_task(task)
                try:
 
                # 根据结果设置 Future
                    if processed_task.status == "completed":
                        processed_task.future.set_result(processed_task.result)
                    else:
                        # 失败时设置异常
                        processed_task.future.set_exception(
                            Exception(processed_task.error or "Unknown error")
                        )
                except Exception as e:
                    # 处理过程中出现未捕获异常
                    task.future.set_exception(e)
                finally:

                    self.task_queue.task_done()
                
            except asyncio.TimeoutError:
                # 队列为空，继续等待
                continue
            except asyncio.CancelledError:
                # 任务被取消
                break
            except Exception as e:
                print(f"工作器 {worker.worker_id} 发生错误: {e}")
                continue
        
        worker.is_running = False
    
    async def submit_request(self, user_id: str, user_email: str, message: str) -> Optional[str]:
        """
        提交用户请求到Agent池
        
        Args:
            user_id: 用户ID
            user_email: 用户邮箱
            message: 用户消息
            
        Returns:
            任务结果或None（如果提交失败）
        """
        if not self.is_running:
            await self.start()
        
        # 检查队列是否已满
        if self.task_queue.qsize() >= self.max_queue_size:
            print(f"任务队列已满，无法处理用户 {user_id} 的请求")
            return None

        # 创建任务
        task_id = str(uuid.uuid4())
        task = AgentTask(
            task_id=task_id,
            user_id=user_id,
            user_email=user_email,
            message=message
        )
        
        try:
            # 提交到队列
            await self.task_queue.put(task)
            # 等待任务完成并获取结果
            result = await task.future
            return result
            
        except Exception as e:
            print(f"提交请求失败: {e}")
            return None
    
    

    
    async def get_pool_status(self) -> Dict[str, Any]:
        """
        获取Agent池状态
        
        Returns:
            状态字典
        """
        # 获取工作器状态
        worker_statuses = [worker.get_status() for worker in self.workers]
        

        return {
            "is_running": self.is_running,
            "num_workers": self.num_workers,
            "active_workers": sum(1 for w in self.workers if w.is_running),
            "queue_size": self.task_queue.qsize(),
            "queue_max": self.max_queue_size,
            "worker_statuses": worker_statuses
        }
    
    async def shutdown(self, wait: bool = True):
        """
        关闭Agent池
        
        Args:
            wait: 是否等待任务完成
        """
        if not self.is_running:
            return
        
        self.is_running = False
        
        # 取消所有工作器任务
        for task in self.worker_tasks:
            task.cancel()
        
        # 等待工作器停止
        if wait:
            try:
                await asyncio.gather(*self.worker_tasks, return_exceptions=True)
            except asyncio.CancelledError:
                pass
        
        print("AsyncAgentPool 已关闭")
