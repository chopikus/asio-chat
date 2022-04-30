#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>

using boost::asio::ip::tcp;

const int REPEAT_TIMER = 10;
const int START_TIME = 1; 

void get_time_info(std::shared_ptr<boost::asio::steady_timer> timer, const std::string& server_name) {
    try {
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve(server_name, "daytime");
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        std::vector<char> result;
        for (;;) {
            char buf[128];
            boost::system::error_code error;
            size_t len = socket.read_some(boost::asio::buffer(buf), error);
            for (size_t i = 0; i < len; i++) {
                result.push_back(buf[i]);
            }
            if (error == boost::asio::error::eof) {
                break;
            } else if (error) {
                throw boost::system::system_error(error);
            }
        }
        for (auto& ch : result) {
            std::cout << ch;
        }
        std::cout << std::endl;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    timer -> expires_at(timer->expiry() + boost::asio::chrono::seconds(REPEAT_TIMER));
    timer -> async_wait(boost::bind(::get_time_info, timer, server_name));
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: client <host>" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Starting..." << std::endl;
    std::string server_name(argv[1]);
    boost::asio::io_context timer_context;
    std::shared_ptr<boost::asio::steady_timer> timer_ptr = 
    std::make_shared<boost::asio::steady_timer>(timer_context, boost::asio::chrono::seconds(START_TIME)); 
    timer_ptr->async_wait(boost::bind(get_time_info, timer_ptr, server_name));
    timer_context.run();
    return 0;
}
