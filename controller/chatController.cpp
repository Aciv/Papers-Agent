// ChatWebSocket.cc
#include "chatController.h"
#include <drogon/HttpClient.h>
#include <json/json.h>
#include "../log_wrapper.h"

void ChatWebSocket::handleNewConnection(const HttpRequestPtr &req,
                                        const WebSocketConnectionPtr &wsConnPtr)
{
    // 新连接建立，初始化该连接的队列
    auto queue = std::make_shared<ConnectionQueue>();
    {
        auto session = req->session();
        queue->email = session->get<std::string>("user_email");
        queue->user_id = session->get<int64_t>("user_id");
        std::lock_guard<std::mutex> lock(globalMutex_);
        queues_[wsConnPtr] = queue;
    }
    APP_LOG_DEBUG("New WebSocket connection");

}

void ChatWebSocket::handleNewMessage(const WebSocketConnectionPtr &wsConnPtr,
                                      std::string &&message,
                                      const WebSocketMessageType &type)
{
    if (type != WebSocketMessageType::Text)
        return; 


    std::shared_ptr<ConnectionQueue> queue;
    {
        std::lock_guard<std::mutex> lock(globalMutex_);
        auto it = queues_.find(wsConnPtr);
        if (it == queues_.end())
            return;
        queue = it->second;
    }

    // 将消息加入队列
    {
        std::lock_guard<std::mutex> qlock(queue->mutex);
        queue->messageQueue.push(std::move(message));
    }

    // 如果没有正在处理的消息，则开始处理
    bool shouldProcess = false;
    {
        std::lock_guard<std::mutex> qlock(queue->mutex);
        if (!queue->isProcessing)
        {
            queue->isProcessing = true;
            shouldProcess = true;
        }
    }
    if (shouldProcess)
    {
        processNext(wsConnPtr);
    }
}

void ChatWebSocket::handleConnectionClosed(const WebSocketConnectionPtr &wsConnPtr)
{
    // 连接关闭，清理资源
    std::lock_guard<std::mutex> lock(globalMutex_);
    queues_.erase(wsConnPtr);
    APP_LOG_DEBUG("WebSocket connection closed" );

}

void ChatWebSocket::processNext(const WebSocketConnectionPtr &wsConnPtr)
{
    // 获取队列
    std::shared_ptr<ConnectionQueue> queue;
    {
        std::lock_guard<std::mutex> lock(globalMutex_);
        auto it = queues_.find(wsConnPtr);
        if (it == queues_.end())
            return;
        queue = it->second;
    }

    int64_t user_id = queue->user_id;
    std::string email = queue->email;

    // 取出下一个消息
    std::string nextMessage;
    {
        std::lock_guard<std::mutex> qlock(queue->mutex);
        if (queue->messageQueue.empty())
        {
            // 没有消息，重置处理标志
            queue->isProcessing = false;
            return;
        }
        nextMessage = queue->messageQueue.front();
        queue->messageQueue.pop();
    }


    // 调用 Python 微服务
    callPythonService(wsConnPtr, nextMessage, user_id, email);
}

void ChatWebSocket::callPythonService(const WebSocketConnectionPtr &wsConnPtr,
                                       const std::string &userMessage,
                                       int64_t user_id,
                                       const std::string &email)
{
    // Python 微服务地址
    auto client = HttpClient::newHttpClient("http://127.0.0.1:5000"); // 假设 Python 服务运行在 5000 端口

    // 创建 HTTP 请求
    auto req = HttpRequest::newHttpRequest();
    req->setMethod(Post);
    req->setPath("/chat"); // Python 服务 url
    req->setContentTypeCode(CT_APPLICATION_JSON);


    Json::Value json;
    json["user_id"] = user_id;
    json["user_email"] = email;
    json["message"] = userMessage;
 
    req->setBody(json.toStyledString());

    // 发送异步请求
    client->sendRequest(req,
        [this, wsConnPtr, user_id, email](ReqResult result, const HttpResponsePtr &resp) mutable
        {
            if (result != ReqResult::Ok || !resp)
            {
                // 请求失败，向用户返回错误信息
                wsConnPtr->send("Error: Failed to get response from service.");
            }
            else
            {
                // 解析 Python 服务返回的 JSON
                auto jsonResp = resp->getJsonObject();
                if (jsonResp)
                {
                    // Python 服务返回 {"reply": "..."}
                    std::string reply = (*jsonResp)["reply"].asString();
                    wsConnPtr->send(reply);
                }
                else
                {
                    wsConnPtr->send("Error: Invalid response from service.");
                }
            }

            // 处理完成后，继续处理队列中的下一个消息
            processNext(wsConnPtr);
        });
}