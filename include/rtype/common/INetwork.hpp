/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** INetwork - Network abstraction layer interfaces
*/

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace rtype::network
{

class IEndpoint
{
public:
    virtual ~IEndpoint() = default;

    virtual std::string toString() const = 0;

    virtual std::string getKey() const = 0;

    virtual std::unique_ptr<IEndpoint> clone() const = 0;
};

class ISocket
{
public:
    virtual ~ISocket() = default;

    virtual void sendTo(const std::vector<std::uint8_t> &data, const IEndpoint &target) = 0;

    virtual void asyncReceive(
        std::function<void(const std::uint8_t*, std::size_t, std::unique_ptr<IEndpoint>)> callback
    ) = 0;

    virtual std::uint16_t getLocalPort() const = 0;
};

class IIOContext
{
public:
    virtual ~IIOContext() = default;

    virtual void run() = 0;

    virtual void poll() = 0;

    virtual void stop() = 0;

    virtual std::unique_ptr<ISocket> createUdpSocket(std::uint16_t port) = 0;

    virtual std::unique_ptr<IEndpoint> createEndpoint(const std::string &host, std::uint16_t port) = 0;
};

class NetworkFactory
{
public:

    static std::unique_ptr<IIOContext> createIOContext();
};

}
