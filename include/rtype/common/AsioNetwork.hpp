/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** AsioNetwork - ASIO implementation of network abstraction
*/

#pragma once

#include "rtype/common/INetwork.hpp"
#include <asio.hpp>
#include <memory>

namespace rtype::network
{

class AsioEndpoint : public IEndpoint
{
public:
    explicit AsioEndpoint(const asio::ip::udp::endpoint &endpoint)
        : _endpoint(endpoint) {}
    
    std::string toString() const override
    {
        return _endpoint.address().to_string() + ":" + std::to_string(_endpoint.port());
    }
    
    std::string getKey() const override
    {
        return _endpoint.address().to_string() + ":" + std::to_string(_endpoint.port());
    }
    
    std::unique_ptr<IEndpoint> clone() const override
    {
        return std::make_unique<AsioEndpoint>(_endpoint);
    }
    
    const asio::ip::udp::endpoint& getAsioEndpoint() const { return _endpoint; }
    asio::ip::udp::endpoint& getAsioEndpoint() { return _endpoint; }
    
private:
    asio::ip::udp::endpoint _endpoint;
};

class AsioUdpSocket : public ISocket
{
public:
    AsioUdpSocket(asio::io_context &ioContext, std::uint16_t port)
        : _socket(ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port))
    {
    }
    
    void sendTo(const std::vector<std::uint8_t> &data, const IEndpoint &target) override
    {
        const auto *asioEndpoint = dynamic_cast<const AsioEndpoint*>(&target);
        if (!asioEndpoint) {
            throw std::runtime_error("Invalid endpoint type");
        }
        
        _socket.async_send_to(
            asio::buffer(data),
            asioEndpoint->getAsioEndpoint(),
            [](std::error_code, std::size_t) {}
        );
    }
    
    void asyncReceive(
        std::function<void(const std::uint8_t*, std::size_t, std::unique_ptr<IEndpoint>)> callback
    ) override
    {
        _socket.async_receive_from(
            asio::buffer(_receiveBuffer),
            _remoteEndpoint,
            [this, callback](std::error_code ec, std::size_t bytesReceived) {
                if (!ec && bytesReceived > 0) {
                    auto endpoint = std::make_unique<AsioEndpoint>(_remoteEndpoint);
                    callback(_receiveBuffer.data(), bytesReceived, std::move(endpoint));
                }
                asyncReceive(callback);
            }
        );
    }
    
    std::uint16_t getLocalPort() const override
    {
        return _socket.local_endpoint().port();
    }
    
private:
    asio::ip::udp::socket _socket;
    asio::ip::udp::endpoint _remoteEndpoint;
    std::array<std::uint8_t, 65536> _receiveBuffer;
};

class AsioIOContext : public IIOContext
{
public:
    AsioIOContext() = default;
    
    void run() override
    {
        _ioContext.run();
    }
    
    void poll() override
    {
        _ioContext.poll();
    }
    
    void stop() override
    {
        _ioContext.stop();
    }
    
    std::unique_ptr<ISocket> createUdpSocket(std::uint16_t port) override
    {
        return std::make_unique<AsioUdpSocket>(_ioContext, port);
    }
    
    std::unique_ptr<IEndpoint> createEndpoint(const std::string &host, std::uint16_t port) override
    {
        asio::ip::udp::resolver resolver(_ioContext);
        auto endpoints = resolver.resolve(asio::ip::udp::v4(), host, std::to_string(port));
        return std::make_unique<AsioEndpoint>(*endpoints.begin());
    }
    
private:
    asio::io_context _ioContext;
};

inline std::unique_ptr<IIOContext> NetworkFactory::createIOContext()
{
    return std::make_unique<AsioIOContext>();
}

}
