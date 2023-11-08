#pragma once

#include "server_helper.hpp"

#include <memory>
#include <vector>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

namespace server
{

/// Represents a single connection from a client.
class Connection 
    : public std::enable_shared_from_this<Connection>
    , private boost::noncopyable
{
    /// Strand to ensure the connection's handlers are not called concurrently.
    boost::asio::io_service::strand _strand;

    /// Socket for the connection.
    boost::asio::ip::tcp::socket _socket;

    /// The handler used to process the incoming request.
    RequestHandler& _request_handler;

    /// The parser for the incoming request.
    RequestParser _request_parser;

    /// Buffer for incoming data.
    std::vector<char> _buffer;

    /// The incoming request.
    Request _request;

    /// The reply to be sent back to the client.
    Response _response;

public:
    /// Construct a connection with the given io_service.
    explicit Connection(boost::asio::io_service& io_service, RequestHandler& handler,
        parse_handler_t ph, const std::size_t buffer_size);

    /// Get the socket associated with the connection.
    boost::asio::ip::tcp::socket& socket();

    /// Start the first asynchronous operation for the connection.
    void start();

private:
    /// Handle completion of a read operation.
    void handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred);

    /// Handle completion of a write operation.
    void handle_write(const boost::system::error_code& e);
};

using ConnectionSP_t = std::shared_ptr<Connection>;

}



