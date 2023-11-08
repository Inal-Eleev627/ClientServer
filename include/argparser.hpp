#pragma once

#include <algorithm>
#include <format>
#include <iostream>
#include <string>
#include <vector>

namespace utils
{

struct Argument
{
    std::string key{};
    std::string value{};
};

using args_t = std::vector<Argument>;

/// Simple application arguments parser with the ability to set their minimum required number and separator sign
/// @param {min_argc_required} set minimum required number of arguments. Default {0}.
/// @param {separator} set separator sign for argumet by rule KEY'sign'VALUE. Default {=}.
class ArgParser
{
    int     _min_argc_required;
    char    _separator;
    args_t  _args;

public:
    ArgParser(const int min_argc_required = 0, const char separator = '=')
        : _min_argc_required(min_argc_required)
        , _separator(separator)
    {}

    bool Parse(int argc, char* argv[]) noexcept
    {
        if (argc < _min_argc_required)
            return false;

        _args.clear();
        _args.reserve(argc);

        for (auto i = 0; i < argc; ++i)
        {
            std::string arg(argv[i]);
            const auto pos = arg.find(_separator);
            Argument a{};
            if (pos != std::string::npos)
            {
                a.key = arg.substr(0, pos);
                a.value = pos < arg.size() - 1 ? arg.substr(pos + 1) : std::string();
            }
            else
            {
                a.key = arg;
            }
            _args.push_back(a);
        }

        return true;
    }

    std::string GetValue(const std::string& key) const noexcept
    {
        auto found = std::find_if(_args.begin(), _args.end(),
            [&key](const args_t::value_type& a) { return a.key == key; });

        return found != _args.end() ? found->value : std::string();
    }

    bool Find(const std::string& key) const noexcept
    {
        return _args.end() != std::find_if(_args.begin(), _args.end(),
            [&key](const args_t::value_type& a) { return a.key == key; });
    }

    const args_t& GetArguments() const noexcept
    {
        return _args;
    }
};

/**
 * TODO: comments
*/
class HelperMessage
{
    const args_t&       _args;
    const std::string   _introduction;
    const std::string   _conclusion;

public:
    HelperMessage(const args_t& args,
        const std::string& intro = {},
        const std::string& concl = {})
        : _args(args)
        , _introduction(intro)
        , _conclusion(concl)
    {}

    void PrintHelp() const
    {
        if (!_introduction.empty())
            std::cerr << _introduction << "\n";

        if (!_args.empty())
            std::cerr << "Options:\n";

        for (const auto& kv : _args)
            std::cerr << std::format("\t{}\t\t{}\n", kv.key, kv.value);

        if (!_conclusion.empty())
            std::cerr << _conclusion << "\n";
    }
};

} // namespace utils
