#include <iostream>
#include "framework/Framework.h"
#include <string>

enum class MessageType {
        NEW_MESSAGE,
        PERSON_LEFT,
        PERSON_CONNECTED
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
