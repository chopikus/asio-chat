#include <iostream>
#include <boost/asio.hpp>
#include "framework/Framework.h"
#include <string>

enum class MessageType {
    NEW_MESSAGE,
    PERSON_LEFT,
    PERSON_CONNECTED,
    SERVER_PING
};

class CustomClient : public MyFramework::ClientInterface<MessageType> {
    public:
        void PingServer() {
            MyFramework::Message<MessageType> message;
            message.header.id = MessageType::SERVER_PING;
            std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
            message << timeNow;
            Send(message);
        }
};

int main(int argc, char* argv[]) {
    CustomClient client;
    client.Connect("127.0.0.1", 6000);
    /* following is some strange stuff to async'ly read keys in Linux */

    boost::asio::io_context io_context; 
    boost::asio::posix::stream_descriptor stream(io_context, STDIN_FILENO);

    char buf[1] = {};

    std::function<void(boost::system::error_code, size_t)> read_handler;
    bool closing = false;
    read_handler = [&](boost::system::error_code ec, size_t len) {   
            auto& q = client.IncomingQueue();
            if (ec) {
                std::cerr << "exit with " << ec.message() << std::endl;
            } else {
                if (len == 1) {
                    if (buf[0] == 'p') {
                        std::cout << "OK, pinging the server" << std::endl;
                        client.PingServer();
                    } else if (buf[0] == 'q') {
                        std::cout << "OK, Client down" << std::endl;
                        closing = true;
                    }
                }
                boost::asio::async_read(stream, boost::asio::buffer(buf), read_handler);
            }
        };

    boost::asio::async_read(stream, boost::asio::buffer(buf), read_handler);

    io_context.run();
    return 0;
}
