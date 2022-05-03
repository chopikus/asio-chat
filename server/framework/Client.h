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
            ClientInterface(): socket_(context_) {}

            virtual ~ClientInterface() {
                Disconnect();
            }

            bool Connect(const std::string& host, const uint16_t port) {
                try {
                    connection_ = std::make_unique<Connection<T>>(); // TODO
                    tcp::resolver resolver(context_);
                    tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));
                    connection_ = std::make_unique<Connection<T>>(Connection<T>::Owner::CLIENT, context_, tcp::socket(context_), qMessagesIn);
                    connection_->ConnectToServer(endpoints);
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                }
                return false;
            }

            void Disconnect() {
                if (IsConnected()) {
                    connection_->Disconnect();
                }
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
        protected:
            asio::io_context context_;
            std::thread threadContext_;
            tcp::socket socket_;
            std::unique_ptr<Connection<T>> connection_;

        private:
            ThreadSafeDeque<OwnedMessage<T>> qMessagesIn;
    };
};

#endif
