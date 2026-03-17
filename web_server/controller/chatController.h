// ChatWebSocket.h
#pragma once

#include <drogon/WebSocketController.h>
#include <drogon/HttpClient.h>
#include <queue>
#include <mutex>


using namespace drogon;

class ChatWebSocket : public drogon::WebSocketController<ChatWebSocket>
{
public:
    // 注册 WebSocket 路径
    void handleNewMessage(const WebSocketConnectionPtr &wsConnPtr,
                          std::string &&message,
                          const WebSocketMessageType &type) override;

    void handleNewConnection(const HttpRequestPtr &req,
                             const WebSocketConnectionPtr &wsConnPtr) override;

    void handleConnectionClosed(const WebSocketConnectionPtr &wsConnPtr) override;

    WS_PATH_LIST_BEGIN
    WS_PATH_ADD("/api/chat", Get); 
    WS_PATH_LIST_END

private:

    struct ConnectionQueue
    {
        std::queue<std::string> messageQueue; 
        std::string email;
        int64_t user_id;
        bool isProcessing = false;            
        std::mutex mutex;                     
    };

   
    std::unordered_map<WebSocketConnectionPtr, std::shared_ptr<ConnectionQueue>> queues_;
    std::mutex globalMutex_;  

    void callPythonService(const WebSocketConnectionPtr &wsConnPtr,
                           const std::string &userMessage,
                           int64_t user_id,
                           const std::string &email); 


    void processNext(const WebSocketConnectionPtr &wsConnPtr);
};