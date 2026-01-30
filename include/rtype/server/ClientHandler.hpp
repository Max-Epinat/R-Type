/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** ClientHandler
*/

#ifndef CLIENTHANDLER_HPP_
#define CLIENTHANDLER_HPP_

#include "rtype/common/Components.hpp"
#include "rtype/common/Protocol.hpp"
#include "rtype/common/GameConfig.hpp"
#include "rtype/common/INetwork.hpp"
#include "rtype/engine/Registry.hpp"
#include <memory>

namespace rtype::server
{
class ClientHandler {
    public:
        ClientHandler(PlayerId id, std::unique_ptr<network::IEndpoint> endpoint, Timestamp time, EntityId entityId)
            : _id(id), _endpoint(std::move(endpoint)), _lastSeen(time), _entityId(entityId) {}
        
        void updateLastSeen(Timestamp now);
        Timestamp getLastSeen() const { return _lastSeen; }
        const network::IEndpoint& getEndpoint() const;
        EntityId getEntityId() const;
        void setEntityId(EntityId id) { _entityId = id; }
        
    private:
        PlayerId _id{};
        std::unique_ptr<network::IEndpoint> _endpoint;
        Timestamp _lastSeen{};
        EntityId _entityId;
};
}
#endif /* !CLIENTHANDLER_HPP_ */
