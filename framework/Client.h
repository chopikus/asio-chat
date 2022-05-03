#ifndef CLIENT_H
#define CLIENT_H
#include "Common.h"
#include "Connection.h"
#include "MessageHandler.h"
#include "ThreadSafeDeque.h"

namespace MyFramework {
    using boost::asio::ip::tcp;
    template<typename T>
    class ClientInterface {
        public:
            ClientInterface() {}

            virtual ~ClientInterface() {
                Disconnect();
            }

            bool Connect(const std::string& host, const uint16_t port) {
                try {
                    tcp::resolver resolver(context_);
                    tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));
                    connection_ = std::make_unique<Connection<T>>(
                        Connection<T>::Owner::CLIENT,
                        context_,
                        tcp::socket(context_), 
                        qMessagesIn);
                    
                    connection_->ConnectToServer(endpoints);

                    threadContext_ = std::thread([this]() {context_.run();});
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
                return true;
            }

            void Disconnect() {
                if (IsConnected()) {
                    connection_->Disconnect();
                }
                context_.stop();
                if (threadContext_.joinable())
                    threadContext_.join();
                connection_.release();
            }

            bool IsConnected() {
                if (connection_) {
                    return connection_->IsConnected();
                }
                return false;
            }

            ThreadSafeDeque<OwnedMessage<T>>& IncomingQueue() {
                return qMessagesIn;
            }

            void Send(const Message<T>& message) {
                if (IsConnected()) {
                    connection_->Send(message);
                }
            }
        protected:
            boost::asio::io_context context_;
            std::thread threadContext_;
            std::unique_ptr<Connection<T>> connection_;

        private:
            ThreadSafeDeque<OwnedMessage<T>> qMessagesIn;
    };
};

#endif
