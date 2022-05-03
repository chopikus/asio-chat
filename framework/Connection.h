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
                owner_ = parent;
            };
            
            virtual ~Connection() {
                
            }

            void ConnectToClient(uint32_t id = 0) {
                if (owner_ == Owner::SERVER) {
                    if (socket_.is_open()) {
                        connection_id = id;
                        ReadHeader();
                    }
                }
            }
            void ConnectToServer(const tcp::resolver::results_type& endpoints) {
                if (owner_ == Owner::CLIENT) {
                    boost::asio::async_connect(socket_, endpoints, 
                    [this](std::error_code ec, tcp::endpoint endpoint) {
                        if (!ec) {
                            ReadHeader();
                        }
                    });
                }
            }

            void Disconnect() {
                if (IsConnected()) {
                    boost::asio::post(context_, [this](){socket_.close();});
                }
            }

            bool IsConnected() const {
                return socket_.is_open();
            }

            void Send(const Message<T>& message) {
                boost::asio::post(context_, [this, message] () {
                    bool wasQueueEmpty = qMessagesOut_.empty();
                    qMessagesOut_.push_back(message);
                    std::cout << "trying to send message " << wasQueueEmpty << std::endl;
                    if (wasQueueEmpty) {
                        WriteHeader();
                    }
                });
            }
            
        protected:
            tcp::socket socket_;
            boost::asio::io_context& context_;
            ThreadSafeDeque<Message<T>> qMessagesOut_;
            ThreadSafeDeque<OwnedMessage<T>>& qMessagesIn_;    
            Message<T> tempMessage_;
            Owner owner_ = Owner::SERVER;

        private:
            //ASYNC
            void ReadHeader() {
                boost::asio::async_read(socket_, 
                boost::asio::buffer(&tempMessage_.header, sizeof(MessageHeader<T>)), 
                [this](std::error_code ec, std::size_t length) {
                    if (!ec) {
                        if (tempMessage_.header.size > 0) {
                            tempMessage_.body.resize(tempMessage_.header.size);
                            ReadBody();
                        } else {
                            AddToIncomingMessageQueue();
                        }
                    } else {
                        std::cout << " [" << connection_id << "] Read Header fail!" << std::endl;
                        std::cout << ec.value() << ' ' << ec.message() << std::endl;
                        socket_.close();
                    }
                });
            }

            //ASYNC
            void ReadBody() {
                boost::asio::async_read(socket_, 
                boost::asio::buffer(tempMessage_.body.data(), tempMessage_.body.size()),
                [this](std::error_code ec, std::size_t length) {
                    if (!ec) {
                        AddToIncomingMessageQueue();
                    } else {
                        std::cout << " [" << connection_id << "] Body Read Failed." << std::endl;
                        std::cout << ec.value() << ' ' << ec.message() << std::endl;
                        socket_.close();
                    }
                });
            }

            //ASYNC
            void WriteHeader() {
                boost::asio::async_write(socket_, 
                boost::asio::buffer(&qMessagesOut_.front().header, sizeof(MessageHeader<T>)), 
                [this] (std::error_code ec, std::size_t length) {
                    std::cout << "Written header" << std::endl;
                    if (!ec) {
                        if (qMessagesOut_.front().body.size() > 0) {
                            WriteBody();
                        } else {
                            qMessagesOut_.pop_front();
                            if (!qMessagesOut_.empty()) {
                                WriteHeader();
                            }
                        }
                    } else {
                        std::cout << " [" << connection_id << "] Header Write Failed." << std::endl;
                        socket_.close();
                    }
                });
            }

            //ASYNC
            void WriteBody() {
                boost::asio::async_write(socket_, 
                boost::asio::buffer(qMessagesOut_.front().body.data(), qMessagesOut_.front().body.size()), 
                [this] (std::error_code ec, std::size_t length) {
                    std::cout << "Written body" << std::endl;
                    if (!ec) {
                        qMessagesOut_.pop_front();
                        if (!qMessagesOut_.empty()) {
                            WriteHeader();
                        }
                    } else {
                        std::cout << " [" << connection_id << "] Body Write Failed." << std::endl;
                        socket_.close();
                    }
                });
            }
            
            void AddToIncomingMessageQueue() {
                if (owner_ == Owner::SERVER) {
                    qMessagesIn_.push_back({this->shared_from_this(), tempMessage_});
                } else {
                    qMessagesIn_.push_back({nullptr, tempMessage_});
                }
                ReadHeader();
            }
    };
};
#endif