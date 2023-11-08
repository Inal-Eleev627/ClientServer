#include "argparser.hpp"

#include <format>
#include <iostream>
#include <tuple>
#include <utility>

#include "server.hpp"

namespace
{
    static const utils::args_t arg_desc = {
        {"-ip",     "Set server ip."},
        {"-port",   "Set server port"},
        {"-jobs",   "Set number of threads for server. Default 'std::thread::hardware_concurrency'"},
        {"-buffer", "Set max size of package for read/write in bytes."},
        {"-help",   "Display help information."}
    };

    static const std::string INTRO{ "Note: set value separator is '='. Example '-jobs=8'" };
    static const utils::HelperMessage HELP {arg_desc, INTRO};

    void custom_handle_request(const bool parse_result, const server::Request& req, server::Response& resp)
    {
        if (!parse_result)
        {
            resp.data = "Bad Request!";
            return;
        }

        resp.data += std::format("SERVER_COMPLETE_MASSAGE. TOTAL Recieved {} bytes.\n", std::to_string(req.data.size()));
        std::cerr << "WRITE: " << resp.data;
    }

    server::Response custom_incomplete_message(const std::size_t& bytes_transferred)
    {
        std::string data = std::format("INCOMPLETE_MESSAGE. Recieved {} bytes.\n", std::to_string(bytes_transferred));

        server::Response resp{};
        resp.data = data;
        std::cerr << "WRITE: " << resp.data;
        return resp;
    }

    boost::optional<bool> custom_parse_handler(const std::string& data, server::Request& req, std::string& complete_data)
    {
        if (data.empty())
            return boost::none;

        complete_data += data;
        complete_data = complete_data.erase(complete_data.size() - 1);

        std::string_view s(data.begin(), data.end());
        std::cerr << std::format("PARSE: Recieved -- {}.\n", s);

        if (data.find("CLIENT_COMPLETE_MASSAGE") == std::string::npos)
            return boost::none;

        req.data = complete_data;
        return true;
    }
} // anonymous namespace

int main(int argc, char* argv[])
{
    constexpr const int MIN_ARGS_REQUIRES = 2;
    utils::ArgParser argParser(MIN_ARGS_REQUIRES);

    if (!argParser.Parse(argc, argv))
    {
        std::cerr << std::format("Minimum arguments required: {}. Provided: {}\n", MIN_ARGS_REQUIRES, argc);
        return {};
    }

    if (argParser.Find("-help"))
    {
        HELP.PrintHelp();
        return {};
    }

    const auto ip = argParser.GetValue("-ip");
    const auto port = argParser.GetValue("-port");
    if (ip.empty() || port.empty())
    {
        std::cerr << std::format("Argument '{}' should not be nil.\n", ip.empty() ? "-ip" : "-port");
        return {};
    }

    try
    {
        // Configure the server.
        server::ServerConfig config{};
        config.ip = ip;
        config.port = port;
        config.jobs = std::thread::hardware_concurrency();

        const auto jobs = argParser.GetValue("-jobs");
        if (!jobs.empty())
            config.jobs = std::stoul(jobs);

        // Initialize the server.
        server::TCPServer srv(config,
            custom_handle_request,
            custom_incomplete_message,
            custom_parse_handler);

        std::cerr << "SERVER RUN!\n";
        // Run the server until stopped.
        srv.Run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return {};
}
