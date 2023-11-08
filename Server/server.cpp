#include "server.hpp"

#include <thread>
#include <memory>

namespace server
{

TCPServer::TCPServer(const ServerConfig &config, request_handler_t rh,
    incomplete_message_handler_t imh, parse_handler_t ph)
    : _config(config)
    , _signals(_ios)
    , _acceptor(_ios)
    , _connection(nullptr)
    , _handler(std::move(rh), std::move(imh))
    , _parse_handler(std::move(ph))
{
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    _signals.add(SIGINT);
    _signals.add(SIGTERM);

#if defined(SIGQUIT)
    _signals.add(SIGQUIT);
#endif // defined(SIGQUIT)

    _signals.async_wait(std::bind(&TCPServer::handle_stop, this));

    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::tcp::resolver resolver(_ios);
    boost::asio::ip::tcp::resolver::query query(_config.ip, _config.port);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

    _acceptor.open(endpoint.protocol());
    _acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    _acceptor.bind(endpoint);
    _acceptor.listen();

    start_accept();
}

void TCPServer::Run()
{
    // Create a pool of threads to run all of the io_services.
    std::vector< std::shared_ptr<std::thread> > threads{};
    for (std::size_t i = 0; i < _config.jobs; ++i)
    {
        std::size_t (boost::asio::io_service::*run)() = &boost::asio::io_service::run;
        std::shared_ptr<std::thread> thread(
            new std::thread(std::bind(run, &_ios)));
        threads.push_back(thread);
    }

    // Wait for all threads in the pool to exit.
    for (std::size_t i = 0; i < threads.size(); ++i)
        threads[i]->join();
}

void TCPServer::start_accept()
{
    _connection.reset(new Connection(_ios, _handler, _parse_handler, _config.buffer_size));

    using namespace std::placeholders;
    _acceptor.async_accept(_connection->socket(),
        std::bind(&TCPServer::handle_accept, this, _1));
}

void TCPServer::handle_accept(const boost::system::error_code &e)
{
    if (!e)
        _connection->start();

    start_accept();
}

void TCPServer::handle_stop()
{
    _ios.stop();
}

}