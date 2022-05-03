#ifndef MESSAGE_HANDLER
#define MESSAGE_HANDLER
#include "Common.h"

namespace MyFramework {
    template <typename T>
    struct MessageHeader {
        T id{};
        uint32_t size = 0;
    };

    template<typename T>
    struct Message {
        MessageHeader<T> header{};
        std::vector<uint8_t> body{};

        size_t size() const {
            return sizeof(MessageHeader<T>) + body.size();
        }

        friend std::ostream& operator << (std::ostream& os, const Message<T>& msg) {
            os << "ID: " << int(msg.header.id) << " Size: " << msg.header.size;
            return os;
        }

        template<typename DataType>
        friend Message<T>& operator << (Message<T>& message, const DataType& data) {
            static_assert(std::is_standard_layout<DataType>::value, "The data structure is too complex to get size of it!");

            size_t previous_size = message.body.size();

            message.body.resize(message.body.size() + sizeof(DataType));

            std::memcpy(message.body.data() + previous_size, &data, sizeof(DataType));

            message.header.size = message.size();
            
            return message;
        }
        
        template<typename DataType>
        friend Message<T>& operator >> (Message<T>& message, DataType& output) {
            static_assert(std::is_standard_layout<DataType>::value, "The data structure is too complex to get size of it!");
            
            size_t new_size = message.body.size() - sizeof(DataType);
            
            std::memcpy(&output, message.body.data() + new_size, sizeof(DataType));
            
            message.body.resize(new_size);

            return message;
        }
    };

    template<typename T>
    class Connection;

    template<typename T>
    struct OwnedMessage {
        std::shared_ptr<Connection<T>> remote = nullptr;
        Message<T> message;

        friend std::ostream& operator<<(std::ostream& os, const OwnedMessage<T>& message) {
            os << message.message;
            return os;
        }
    };
};
#endif