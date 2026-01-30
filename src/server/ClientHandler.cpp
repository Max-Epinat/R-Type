/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** ClientHandler
*/

#include "rtype/server/ClientHandler.hpp"

namespace rtype::server
{

void ClientHandler::updateLastSeen(Timestamp now)
{
    this->_lastSeen = now;
}

const network::IEndpoint& ClientHandler::getEndpoint() const
{
    return *_endpoint;
}

EntityId ClientHandler::getEntityId() const
{
    return this->_entityId;
}

}