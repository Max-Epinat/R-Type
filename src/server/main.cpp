#include "rtype/server/GameServer.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

int main(int argc, char **argv)
{
    const std::uint16_t port = (argc > 1) ? static_cast<std::uint16_t>(std::atoi(argv[1])) : 5000;

    try
    {
        rtype::server::GameServer server(port);
        server.start();
        std::cout << "[server] press Ctrl+C to exit\n";

        std::this_thread::sleep_for(std::chrono::hours(24));
    }
    catch (const std::exception &e)
    {
        std::cerr << "[server] fatal error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
