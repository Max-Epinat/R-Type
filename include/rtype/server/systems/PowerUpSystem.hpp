#pragma once

#include "rtype/engine/Registry.hpp"
#include "rtype/engine/ISystem.hpp"
#include "rtype/common/GameConfig.hpp"
#include "rtype/common/Components.hpp"
#include <random>

namespace rtype::server {
class PowerUpSystem : public engine::ISystem {
    public:
        explicit PowerUpSystem(const config::GameConfig &config)
        : ISystem(config) {}
        void update(float dt, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy);
    private:
        float _powerUpSpawnTimer{0.0f};
        std::mt19937 _rng{std::random_device{}()};
        void incrementPowerUpProgress(WeaponComponent &weapon);
};
}