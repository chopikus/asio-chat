#include <iostream>
#include <boost/asio.hpp>
#include "framework/Framework.h"
#include <string>

enum class MessageType : uint32_t {
    NEW_CHAT_MESSAGE,
    PERSON_LEFT,
    PERSON_CONNECTED,
    SERVER_PING,
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

        void SendMessage() {
            MyFramework::Message<MessageType> message;
            message.header.id = MessageType::NEW_CHAT_MESSAGE;
            message << connection_->connection_id;
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
                    switch (buf[0]) {
                        case 'p':
                            std::cout << "OK, pinging the server" << std::endl;
                            client.PingServer();
                            break;
                        case 'q':
                            std::cout << "OK, client down" << std::endl;
                            closing = true;
                            break;
                        case 'a':
                            std::cout << "OK, messaging all clients" << std::endl;
                            client.SendMessage();
                            break;
                        default:
                            break;
                    }
                }
                boost::asio::async_read(stream, boost::asio::buffer(buf), read_handler);
            }
        };

    boost::asio::async_read(stream, boost::asio::buffer(buf), read_handler);

    std::thread t([&](){io_context.run();});

    while (!closing) {
        if (client.IsConnected()) {
            auto& q = client.IncomingQueue();
            while (!q.empty()) {
                MyFramework::Message<MessageType> message = q.pop_front().message;
                switch (message.header.id) {
                    case MessageType::SERVER_PING: {
                        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                        std::chrono::system_clock::time_point timeThen;
                        message >> timeThen;
                        // assuming the system clock hasn't changed while pinging
                        double elapsed_time_ms = std::chrono::duration<double, std::milli>(timeNow - timeThen).count();
                        std::cout << "Ping is " << elapsed_time_ms << " milliseconds" << std::endl;
                        break;
                    }
                    case MessageType::NEW_CHAT_MESSAGE: {
                        uint32_t client_id;

                        message >> client_id;
                        std::cout << "Messaging all clients from " << client_id << std::endl;
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
    std::cout << "ok" << std::endl;
    io_context.stop();
    if (t.joinable())
        t.join();
    return 0;
}
