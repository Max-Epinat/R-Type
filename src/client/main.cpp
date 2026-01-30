#include "rtype/client/GameClient.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    const std::string host = (argc > 1) ? argv[1] : "127.0.0.1";
    const std::uint16_t port = (argc > 2) ? static_cast<std::uint16_t>(std::atoi(argv[2])) : 5000;

    try
    {
        rtype::client::GameClient client(host, port);
        client.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "[client] fatal error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
