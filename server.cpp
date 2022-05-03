#include <iostream>
#include "framework/Framework.h"
#include <string>

enum class MessageType : uint32_t {
    NEW_MESSAGE,
    PERSON_LEFT,
    PERSON_CONNECTED,
    SERVER_PING
};

class CustomServer : public MyFramework::ServerInterface<MessageType> {
    public:
        CustomServer(uint16_t port) : MyFramework::ServerInterface<MessageType>(port) {}
    protected:
        bool OnClientConnect(std::shared_ptr<MyFramework::Connection<MessageType>> client) {
            return true;
        }
        virtual void OnClientDisconnect(std::shared_ptr<MyFramework::Connection<MessageType>> client) {
            
        }

        virtual void OnMessage(std::shared_ptr<MyFramework::Connection<MessageType>> client, MyFramework::Message<MessageType>& message) {
            std::cout << "THE MESSAGE!" << std::endl;
            switch (message.header.id) {
                case MessageType::SERVER_PING:
                    std::cout << "[" << client->connection_id << "]: Server ping" << std::endl; 
                    client->Send(message);
                    break;
                default:
                    break;
            }
        }
};

int main(int argc, char* argv[]) {
    CustomServer server(6000);
    server.Start();
    while (1) {
        server.Update();
    }
    return 0;
}
