#ifndef SERVER_H
#define SERVER_H
#include "ThreadSafeDeque.h"
#include "MessageHandler.h"
#include "Connection.h"
#include "Common.h"

namespace MyFramework {
    using boost::asio::ip::tcp;
    template <typename T>
    class ServerInterface {
        public:
            ServerInterface(uint16_t port) : asioAcceptor(asioContext, tcp::endpoint(tcp::v4(), port)) {}
            virtual ~ServerInterface() {
                Stop();
            }

            bool Start() {
                std::cout << " I Started!" << std::endl;
                try {
                    WaitForClientConnection();
                    threadContext = std::thread([this]() { asioContext.run();});
                }
                catch (std::exception& e) {
                    std::cerr << "Exception: " << e.what() << std::endl;
                    return false;
                }
                return true;
            }

            void Stop() {
                asioContext.stop();
                if (threadContext.joinable()) threadContext.join();

                std::cout << " I stopped!" << std::endl;
            }

            // ASYNC
            void WaitForClientConnection() {
                asioAcceptor.async_accept([this](std::error_code ec, tcp::socket socket) {
                    if (!ec) {
                        std::cout << " New connection: " << socket.remote_endpoint() << std::endl;
                        std::shared_ptr<Connection<T>> new_connection = 
                        std::make_shared<Connection<T>>(Connection<T>::Owner::SERVER, asioContext, std::move(socket), qMessagesIn);
                        // check whether the user is appropriate
                        if (OnClientConnect(new_connection)) {
                            dequeConnections.push_back(std::move(new_connection));
                            dequeConnections.back()->ConnectToClient(++idCounter);
                            std::cout << " [" << idCounter << "] Connection approved!" << std::endl;
                        } else {
                            std::cout << "Connection denied!" << std::endl;
                        }
                        
                    } else {
                        std::cout << " New connection error: " << ec.message() << std::endl;
                    }
                    WaitForClientConnection();
                });
            }

            void MessageClient(std::shared_ptr<Connection<T>> client, const Message<T>& msg) {
                if (client && client->IsConnected()) {
                    client->Send(msg);
                    std::cout << "SENDING MESSAGE" << std::endl;
                } else {
                    OnClientDisconnect(client);
                    client.reset();
                    dequeConnections.erase(std::remove(dequeConnections.begin(), dequeConnections.end(), client), dequeConnections);
                }
            }

            void MessageAllClients(const Message<T>& message, std::shared_ptr<Connection<T>> ignore_client = nullptr) {
                bool invalidClientExists = false;
                for (auto& client : dequeConnections) {
                    if (client && client->IsConnected()){
                        if (client != ignore_client) {
                            client->Send(message);
                        }
                    } else {
                        OnClientDisconnect(client);
                        client.reset();
                        invalidClientExists = true;
                    }
                }
                if (invalidClientExists) {
                    dequeConnections.erase(
                        std::remove(dequeConnections.begin(), dequeConnections.end(), nullptr), dequeConnections.end());
                }
            }

            void Update(size_t maxMessages = std::numeric_limits<size_t>::max(), bool wait = true) {
                if (wait)
                    qMessagesIn.wait();
                size_t passedMessages = 0;
                while (passedMessages < maxMessages && !qMessagesIn.empty()) {
                    auto ownedMessage = qMessagesIn.pop_front();
                    OnMessage(ownedMessage.remote, ownedMessage.message);
                    passedMessages++;
                }
            }

        protected:
            // Called when client connects, all users are banned by default
            virtual bool OnClientConnect(std::shared_ptr<Connection<T>> client) {
                return false;
            }

            virtual void OnClientDisconnect(std::shared_ptr<Connection<T>> client) {

            }

            virtual void OnMessage(std::shared_ptr<Connection<T>> client, Message<T>& message) {

            }

            ThreadSafeDeque<OwnedMessage<T>> qMessagesIn;
            std::deque<std::shared_ptr<Connection<T>> > dequeConnections;
            boost::asio::io_context asioContext;
            std::thread threadContext;

            boost::asio::ip::tcp::acceptor asioAcceptor;
            uint32_t idCounter = 10000;
    };
}
#endif