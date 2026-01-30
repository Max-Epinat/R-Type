/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** GameLogicHandler
*/

#ifndef GAMELOGICHANDLER_HPP_
#define GAMELOGICHANDLER_HPP_

#include "rtype/engine/Registry.hpp"
#include "rtype/engine/SystemPipeline.hpp"
#include "rtype/common/Types.hpp"
#include "rtype/common/GameConfig.hpp"
#include "rtype/common/Components.hpp"
#include "rtype/common/Protocol.hpp"
#include "rtype/server/EntityFactory.hpp"
#include <unordered_set>
#include <random>

namespace rtype::server
{
class GameLogicHandler {
    public:
        GameLogicHandler(config::GameConfig config);
        ~GameLogicHandler();
        EntityId spawnPlayer(PlayerId player_id);
        // void managePlayerMovement(const net::PlayerInput &input, const EntityId entity);
        void manageInputs(const PlayerInputComponent input, const EntityId entity);
        void updateGame(const float dt);
        void destroyEntity(rtype::EntityId id);
        const engine::Registry &getRegistry() const;
        engine::Registry &getRegistry();
        void markDestroy(EntityId id);
        void destroyEntityDestructionList();
        const std::unordered_set<rtype::EntityId> &getEntityDestructionSet();
        int getCurrentLevel() const;
        bool hasLevelChanged();

    protected:
    private:
        void initializeSystems();
        int _prevLevel{0};
        int _currentLevel{0};
        bool _levelChanged{false};
        config::GameConfig _config;
        engine::Registry _registry;
        EntityFactory _entityFactory;
        engine::SystemPipeline _systemPipeline;
        std::unordered_set<EntityId> toDestroySet;
};
}
#endif /* !GAMELOGICHANDLER_HPP_ */
