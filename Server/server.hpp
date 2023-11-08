#pragma once

#include "connection.hpp"
#include "server_helper.hpp"

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

namespace server
{

class TCPServer : private boost::noncopyable
{
    /// Server configuration params
    ServerConfig _config;

    /// The io_service used to perform asynchronous operations.
    boost::asio::io_service _ios;

    /// The signal_set is used to register for process termination notifications.
    boost::asio::signal_set _signals;

    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor _acceptor;

    /// The next connection to be accepted.
    ConnectionSP_t _connection;

    /// The handler for all incoming requests.
    RequestHandler _handler;

    parse_handler_t _parse_handler;

public:
    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
    explicit TCPServer(const ServerConfig& config,
        request_handler_t rh = {},
        incomplete_message_handler_t imh = {},
        parse_handler_t ph = {});

    /// Run the server's io_service loop.
    void Run();

private:
    /// Initiate an asynchronous accept operation.
    void start_accept();

    /// Handle completion of an asynchronous accept operation.
    void handle_accept(const boost::system::error_code& e);

    /// Handle a request to stop the server.
    void handle_stop();
};

} // namespace server
