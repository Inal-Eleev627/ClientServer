#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

namespace server
{

constexpr const std::size_t DEFAULT_BUFFER_SIZE = 8192;

struct ServerConfig
{
    std::string ip{};
    std::string port{};
    std::size_t jobs{};
    std::size_t buffer_size{ DEFAULT_BUFFER_SIZE };
};

struct Request
{
    std::string data{};
};

struct Response
{
    std::string data{};

    std::vector<boost::asio::const_buffer> to_buffer_seq()
    {
        std::vector<boost::asio::const_buffer> buffer{ boost::asio::buffer(data) };
        return buffer;
    }
};

using request_handler_t = std::function< void(const bool, const Request&, Response&) >;
using incomplete_message_handler_t = std::function< Response(const std::size_t&) >;

/// The common handler for all incoming requests.
class RequestHandler : private boost::noncopyable
{
    request_handler_t _handler;
    incomplete_message_handler_t _incomplete_mh;

public:
    /// Construct with a directory containing files to be served.
    RequestHandler(request_handler_t h, incomplete_message_handler_t imh)
        : _handler(h)
        , _incomplete_mh(imh)
    {}

    RequestHandler(RequestHandler&& other) noexcept
    {
        this->_handler = std::move(other._handler);
    }

    /// Handle a request and produce a response.
    void handle_request(const bool parse_result, const Request& req, Response& resp)
    {
        if (_handler)
            return _handler(parse_result, req, resp);

        if (!parse_result)
        {
            resp.data = "Bad Request!";
            return;
        }

        resp.data = req.data + " response from server!\n";
    }

    Response incomplete_message(const std::size_t bytes_transferred)
    {
        if (_incomplete_mh)
            return _incomplete_mh(bytes_transferred);

        Response resp{};
        resp.data = "INCOMPLETE_MESSAGE";
        return resp;
    }
};

using parse_handler_t = std::function< boost::optional<bool>(const std::string&, Request&, std::string&) >;

class RequestParser
{
    std::string complete_data;
    parse_handler_t _handler;

public:
    RequestParser(parse_handler_t h)
        : _handler(h)
    {}

    void Reset() { complete_data.clear(); }

    /// Parse some data. The 'boost::optional<bool>' return value is true when a complete request
    /// has been parsed, false if the data is invalid, 'boost::none' when more data is required.
    boost::optional<bool> Parse(const std::string& data, Request& req)
    {
        if (_handler)
            return _handler(data, req, complete_data);

        if (data.empty())
            return boost::none;

        complete_data.append(data);
        req.data = complete_data;
        return true;
    }
};

}
