#include "connection.hpp"

#include <iostream>

namespace
{

void ignore(const boost::system::error_code&){}

}

namespace server
{

Connection::Connection(boost::asio::io_service& io_service, RequestHandler& handler,
    parse_handler_t ph, const std::size_t buffer_size)
    : _strand(io_service)
    , _socket(io_service)
    , _request_handler(handler)
    , _buffer(buffer_size)
    , _request_parser(ph)
{
}

boost::asio::ip::tcp::socket &Connection::socket()
{
    return _socket;
}

void Connection::start()
{
    using namespace std::placeholders;
    _socket.async_receive(boost::asio::buffer(_buffer),
        _strand.wrap(std::bind(&Connection::handle_read, shared_from_this(), _1, _2)));
}

void Connection::handle_read(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    if (ec && ec != boost::asio::error::eof)
    {
        std::cerr << "Error during async read. Code: " << ec << "\n";
        _request_parser.Reset();
        return;
    }

    std::string cpy(&_buffer.front(), bytes_transferred);
    auto result = _request_parser.Parse(cpy, _request);

    using namespace std::placeholders;
    if (result == boost::none)
    {
        auto reply = _request_handler.incomplete_message(bytes_transferred);
        boost::asio::async_write(_socket, reply.to_buffer_seq(), std::bind(&ignore, _1));

        _socket.async_receive(boost::asio::buffer(_buffer),
            _strand.wrap(std::bind(&Connection::handle_read, shared_from_this(), _1, _2)));
    }
    else
    {
        _request_handler.handle_request(result.value(), _request, _response);
        boost::asio::async_write(_socket, _response.to_buffer_seq(), _strand.wrap(
            std::bind(&Connection::handle_write, shared_from_this(), _1)));

        _request_parser.Reset();
    }
}

void Connection::handle_write(const boost::system::error_code &ec)
{
    if (ec && ec != boost::asio::error::eof)
    {
        std::cerr << "Async write error: " << ec.message() << "\n";
        return;
    }
}

}
