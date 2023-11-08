#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QString>

#include <memory>
#include <functional>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

enum TcpClientStatus
{
    disconnected = 0,
    connected,
    idle
};

class TcpClient : public QObject
{
    Q_OBJECT;

    std::atomic_bool _active;
    boost::asio::io_service _ios;

    using work_t = boost::asio::io_service::work;
    std::unique_ptr<work_t> _work;

    std::thread _thread;

    std::recursive_mutex _socket_mutex;
    boost::asio::ip::tcp::socket _socket;
    boost::asio::ip::tcp::endpoint _endpoint;

    boost::asio::streambuf _input_buffer;

    boost::asio::steady_timer _cancelation_timer;
    boost::asio::steady_timer _timeout_timer;
    boost::asio::steady_timer _write_timer;

    using data_handler_t = std::function<void(std::string&)>;
    data_handler_t _data_handler;

    using cancel_handler_t = std::function<void(const boost::system::error_code&)>;

public:
    explicit TcpClient(const std::string& ip,
                       const std::string& port,
                       data_handler_t dh);
    ~TcpClient();

    void Start(const boost::system::error_code& ec = {});
    void Stop();

signals:
    void WroteData(const QString);
    void ReadData(const QString);
    void StatusChanged(const int);

private:
    void start_connect();
    void handle_connect(const boost::system::error_code& ec);
    void start_async_read();
    void handle_read(const boost::system::error_code& ec, std::size_t bytesTransferred);
    void start_async_write();
    void handle_write(const boost::system::error_code& ec);
    void handle_deadline(const boost::system::error_code& ec);
    void check_timeout(const boost::system::error_code& ec);
    void schedule_cancelation_timer(cancel_handler_t task);
};

#endif // TCPCLIENT_H
