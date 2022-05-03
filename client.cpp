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
                        std::cout << "Ping is " << (long long) elapsed_time_ms << std::endl;
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
