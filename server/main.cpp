#include <boost/asio.hpp>
#include <ctime>
#include <iostream>
#include <string>

const int PORT = 13;
using boost::asio::ip::tcp;

std::string make_daytime_string() {
    using namespace std;
    time_t now = time(0);
    return ctime(&now);
}

int main(int argc, char* argv[]) {
    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 13));        
        for (;;) {
            std::cout << "Recived a request!" << std::endl;
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            std::string message = make_daytime_string();
            boost::system::error_code ignored_error;
            boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
        }
    } catch (std::exception& e) {
       std::cerr << e.what() << std::endl; 
    }
    return 0;
}
