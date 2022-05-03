#ifndef CONNECTION_H
#define CONNECTION_H

#include <boost/asio.hpp>
#include "Common.h"
#include "MessageHandler.h"
#include "ThreadSafeDeque.h"

namespace MyFramework {
    using boost::asio::ip::tcp;
    template <typename T>
    class Connection : public std::enable_shared_from_this<Connection<T>> {
        public:
            enum class Owner {
                SERVER,
                CLIENT
            };
            uint32_t connection_id;

            Connection(Owner parent, boost::asio::io_context& asioContext,
                tcp::socket socket, ThreadSafeDeque<OwnedMessage<T>>& qMessagesIn) :
                context_(asioContext), socket_(std::move(socket)),
                qMessagesIn_(qMessagesIn) 
            {
                ownerType = parent;
            };
            
            virtual ~Connection() {
                
            }

            void ConnectToClient(uint32_t id) {
                if (ownerType == Owner::SERVER) {
                    if (socket_.is_open()) {
                        connection_id = id;
                    }
                }
            }
            void ConnectToServer() {
                
            }

            bool Disconnect() {
                return false;
            }

            bool isConnected() const {
                return socket_.is_open();
            }

            bool Send(const Message<T>& message);
            
        protected:
            tcp::socket socket_;
            boost::asio::io_context& context_;
            ThreadSafeDeque<Message<T>> qMessagesOut_;
            ThreadSafeDeque<OwnedMessage<T>>& qMessagesIn_;    
            Message<T> tempMessage_;
            Owner ownerType = Owner::SERVER;

        private:
            //ASYNC
            void ReadHeader() {
                boost::asio::async_read(socket_, tempMessage_.header, sizeof(MessageHeader<T>), 
                [this](std::error_code ec, size_t length) {
                    if (!ec) {

                    } else {
                        std::cout << "[SERVER] [" << connection_id << "] Read Header fail!" << std::endl;
                    }
                });
            }

            void ReadBody() {

            }

            void WriteHeader() {

            }

            void WriteBody() {

            }
    };
};
#endif