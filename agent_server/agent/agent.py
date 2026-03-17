
from dotenv import load_dotenv
import os
from langchain_openai import ChatOpenAI

load_dotenv(dotenv_path="./.env")


chat_model = ChatOpenAI(
    model=os.getenv("QWEN_MODEL"),
    base_url=os.getenv("QWEN_BASE_URL"),
    api_key=os.getenv("QWEN_API_KEY"),
    # extra_body={"thinking": {"type": "enabled"}}
)

middle_model = ChatOpenAI(
    model=os.getenv("QWEN_MODEL"),
    base_url=os.getenv("QWEN_BASE_URL"),
    api_key=os.getenv("QWEN_API_KEY")
)


from langchain.agents import create_agent
from langchain.agents.middleware import SummarizationMiddleware
from langgraph.checkpoint.memory import InMemorySaver
from langchain_core.runnables import RunnableConfig
from dataclasses import dataclass

import asyncio
from typing import Dict, List, Optional, Any, Callable, Union
from langchain_core.runnables import RunnableConfig
from dataclasses import dataclass, field
from datetime import datetime

@dataclass
class AgentTask:
    task_id: str
    user_id: str
    user_email: str
    message: str
    created_at: datetime = field(default_factory=datetime.now)
    future: asyncio.Future = field(default_factory=asyncio.Future)
    status: str = "pending"  # pending, processing, completed, failed
    result: Optional[str] = None
    error: Optional[str] = None
    completed_at: Optional[datetime] = None
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "task_id": self.task_id,
            "user_id": self.user_id,
            "user_email": self.user_email,
            "message": self.message,
            "created_at": self.created_at.isoformat(),
            "status": self.status,
            "result": self.result,
            "error": self.error,
            "completed_at": self.completed_at.isoformat() if self.completed_at else None
        }
    

from agent.tools import search_papers, pdf_read, getTime
@dataclass
class agentContext:
    user_email: str
    user_id: str
    current_time: str

checkpointer = InMemorySaver()

class AsyncAgentWorker:

    def __init__(self, worker_id: int):
        self.worker_id = worker_id
        self.is_running = False
        self.current_task: Optional[AgentTask] = None
        self.system_prompt = "你是一个论文管理大师，可以使用工具帮助检索论文并总结给用户"

        # 初始化Agent
        self.chat_model = ChatOpenAI(
            model=os.getenv("QWEN_MODEL"),
            base_url=os.getenv("QWEN_BASE_URL"),
            api_key=os.getenv("QWEN_API_KEY"),
        )
        
        self.middle_model = ChatOpenAI(
            model=os.getenv("QWEN_MODEL"),
            base_url=os.getenv("QWEN_BASE_URL"),
            api_key=os.getenv("QWEN_API_KEY")
        )
        

        
        self.agent = create_agent(
            model=self.chat_model,
            system_prompt=self.system_prompt ,
            tools=[search_papers, pdf_read, getTime],
            context_schema=agentContext,
            middleware=[
                SummarizationMiddleware(
                    model=self.middle_model,
                    keep=('tokens', 1000),
                    trigger=('tokens', 8000)
                )
            ],
            checkpointer=checkpointer,
        )
    
    async def process_task(self, task: AgentTask) -> AgentTask:

        self.current_task = task
        task.status = "processing"
        
        try:

            # 准备Agent配置
            config: RunnableConfig = {"configurable": {"thread_id": task.user_id}}
            from datetime import date
            # 调用Agent

            response = await asyncio.to_thread(
                self.agent.invoke,
                {"messages": task.message,
                 "context": agentContext(
                    user_email=task.user_email,
                    user_id=task.user_id,
                    current_time=str(date.today()))},
                config
            )
            
            # 提取响应
            if response and "messages" in response and response["messages"]:
                last_message = response["messages"][-1]
                response_text = str(last_message.content) if hasattr(last_message, 'content') else str(last_message)
            else:
                response_text = "抱歉，我没有理解你的意思。"
            
            # 更新任务结果
            task.result = response_text
            task.status = "completed"
            task.completed_at = datetime.now()
            
            
        except Exception as e:
            task.status = "failed"
            task.error = str(e)
            task.completed_at = datetime.now()
            print(f"Worker {self.worker_id} 处理任务失败: {e}")
        
        finally:
            self.current_task = None
        
        return task
    
    def get_status(self) -> Dict[str, Any]:
        return {
            "worker_id": self.worker_id,
            "is_running": self.is_running,
            "current_task": self.current_task.to_dict() if self.current_task else None
        }









