#include <iostream>
#include <format>
#include <utility>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <thread>

#include <boost/asio.hpp>
#include "server_helper.hpp"

using boost::asio::ip::tcp;

enum { max_length = 8024 };

void timeout()
{
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
}

void error_handler(boost::system::error_code, std::size_t)
{}

void foo()
{
    try
    {
        boost::asio::io_service io_service;
        tcp::socket s(io_service);

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(tcp::v4(), "127.0.0.1", "8585");
        tcp::resolver::iterator iterator = resolver.resolve(query);

        boost::asio::connect(s, iterator);

        using namespace std; // For strlen.
        for (int i = 0; i < 5; ++i)
        {
            std::ostringstream ss;
            ss << std::this_thread::get_id();
            std::string request = "request from thread " + ss.str();

            if (i == 4)
                request += " COMPLETE_MASSAGE";

            request.erase(std::remove(request.begin(), request.end(), '\n'), request.cend());
            std::this_thread::sleep_for(1s);
            //std::cin.getline(request, max_length);
            size_t request_length = strlen(request.c_str());
            boost::asio::async_write(s, boost::asio::buffer(request, request_length), std::bind(&timeout));

            char reply[max_length];
            using namespace std::placeholders;
            boost::asio::async_read(s, boost::asio::buffer(reply, max_length), std::bind(&error_handler, _1, _2));
            std::cout << "Reply is: ";
            std::cout.write(reply, strlen(reply));
            std::cout << "\n";
            
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

int main()
{
    auto f1 = std::async(&foo);
    auto f2 = std::async(&foo);
    auto f3 = std::async(&foo);
    auto f4 = std::async(&foo);

    f1.get();
    f2.get();
    f3.get();
    f4.get();
}
