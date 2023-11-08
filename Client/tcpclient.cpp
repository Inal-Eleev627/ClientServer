#include "tcpclient.h"

#include <ctime>
#include <iostream>

TcpClient::TcpClient(const std::string& ip, const std::string& port, data_handler_t dh)
    : _active(false)
    , _work(new boost::asio::io_service::work(_ios))
    , _thread(boost::bind(&boost::asio::io_service::run, &_ios))
    , _socket(_ios)
    , _cancelation_timer(_ios)
    , _timeout_timer(_ios)
    , _write_timer(_ios)
    , _data_handler(dh)
{
    boost::asio::ip::tcp::resolver resolver(_ios);
    auto iter = resolver.resolve(ip, port);

    boost::asio::ip::tcp::resolver::iterator end; // End marker.
    if (iter == end)
        throw std::runtime_error(std::move(std::format("Resolving server endpoint {}:{} failed.", ip, port)));

    _endpoint = *iter;

    emit StatusChanged(static_cast<int>(TcpClientStatus::disconnected));
}

TcpClient::~TcpClient()
{
    _active = false;
    _cancelation_timer.cancel();
    _write_timer.cancel();
    _timeout_timer.cancel();

    {
        std::lock_guard<std::recursive_mutex> lg(_socket_mutex);
        _socket.close();
    }

    _work.reset();
    _thread.join();
    emit StatusChanged(static_cast<int>(TcpClientStatus::disconnected));
}

void TcpClient::Start(const boost::system::error_code& ec)
{
    if (ec == boost::asio::error::operation_aborted)
        return;

    using namespace std::placeholders;
    schedule_cancelation_timer(std::bind(&TcpClient::handle_deadline, this, _1));

    start_connect();
}

void TcpClient::Stop()
{
    _active = false;
    {
        std::lock_guard<std::recursive_mutex> lg(_socket_mutex);
        _socket.close();
    }

    _write_timer.cancel();
    _timeout_timer.cancel();
    emit StatusChanged(static_cast<int>(TcpClientStatus::idle));
}

void TcpClient::start_connect()
{
    _active = true;

    using namespace std::placeholders;

    // Connect operation timeout
    _timeout_timer.expires_from_now(std::chrono::seconds(10));
    _timeout_timer.async_wait(std::bind(&TcpClient::check_timeout, this, _1));

    std::lock_guard<std::recursive_mutex> lg(_socket_mutex);
    _socket.async_connect(_endpoint, std::bind(&TcpClient::handle_connect, this, _1));
}

void TcpClient::handle_connect(const boost::system::error_code& ec)
{
    if (!_active)
        return;

    if (ec)
    {
        std::cerr << "Connection error: " << ec.message() <<"\n";
        return;
    }

    emit StatusChanged(static_cast<int>(TcpClientStatus::connected));

    start_async_read();
    start_async_write();
}

void TcpClient::start_async_read()
{
    if (!_active)
        return;

    // Read operation timeout
    _timeout_timer.expires_from_now(std::chrono::seconds(5));

    using namespace std::placeholders;
    boost::asio::async_read_until(_socket, _input_buffer, "\n",
        std::bind(&TcpClient::handle_read, this, _1, _2));
}

void TcpClient::handle_read(const boost::system::error_code& ec, std::size_t bytesTransferred)
{
    if (bytesTransferred == 0)
        return;

    if (ec)
    {
        std::cerr << "Error while reading data: " << ec.message() << "\n";
        return;
    }

    std::string data;
    std::istream is(&_input_buffer);
    std::getline(is, data);

    if (!data.empty())
    {
        emit ReadData(data.c_str());
        if (data.find("SERVER_COMPLETE_MASSAGE") != std::string::npos)
        {
            Stop();

            using namespace std::placeholders;
            schedule_cancelation_timer(std::bind(&TcpClient::Start, this, _1));
            return;
        }
    }
    start_async_read();
}

void TcpClient::start_async_write()
{
    if (!_active)
        return;

    std::string data{};
    // Function provided by TcpClient user to customize data generation
    if (_data_handler)
        _data_handler(data);

    {
        std::lock_guard<std::recursive_mutex> lg(_socket_mutex);
        using namespace std::placeholders;
        // starting an asynchronous write operation
        boost::asio::async_write(_socket, boost::asio::buffer(data, data.size()),
            std::bind(&TcpClient::handle_write, this, _1));
    }

    emit WroteData(data.c_str());
}

void TcpClient::handle_write(const boost::system::error_code& ec)
{
    if (!_active)
        return;

    if (ec)
    {
        std::cout << "Error on heartbeat: " << ec.message() << "\n";
        return;
    }

    using namespace std::placeholders;
    boost::asio::chrono::milliseconds TIMER(500);

    // shcedule timer 500 milliseconds before the next write operation
    _write_timer.expires_from_now(TIMER);
    _write_timer.async_wait(std::bind(&TcpClient::start_async_write, this));
}

void TcpClient::handle_deadline(const boost::system::error_code& ec)
{
    if (ec == boost::asio::error::operation_aborted)
        return;

    std::string complete_msg("CLIENT_COMPLETE_MASSAGE");

    {
        // after cancelation timer expires, send complete message token to server
        std::lock_guard<std::recursive_mutex> lg(_socket_mutex);
        boost::asio::async_write(_socket, boost::asio::buffer(complete_msg),
            [](const boost::system::error_code&, std::size_t){});
    }

    emit WroteData(complete_msg.c_str());
}

void TcpClient::check_timeout(const boost::system::error_code& ec)
{
    if (!_active)
        return;

    auto pos_infin_t = std::chrono::steady_clock::time_point::max;
    auto now_t = std::chrono::steady_clock::now;

    if (_timeout_timer.expiry() <= now_t())
    {
        {   // closing socket of reason operation timeout reached
            std::lock_guard<std::recursive_mutex> lg(_socket_mutex);
            _socket.close();
        }
        _timeout_timer.expires_at(pos_infin_t());
    }

    using namespace std::placeholders;
    // re-registrate timeout handler
    _timeout_timer.async_wait(std::bind(&TcpClient::check_timeout, this, _1));
}

// Cancel timer for two operations on a random schedule in the range of 10 seconds.
// First operation: initiate a new connection and schedule second operation.
// Second operation: send the termination token, set client to idle and schedule first operation.
void TcpClient::schedule_cancelation_timer(cancel_handler_t task)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srand((time_t)ts.tv_nsec);

    auto random = (rand() % 9) + 1;
    boost::asio::chrono::seconds RESTART_TIMER(random);
    _cancelation_timer.expires_from_now(RESTART_TIMER);
    _cancelation_timer.async_wait(task);
}
