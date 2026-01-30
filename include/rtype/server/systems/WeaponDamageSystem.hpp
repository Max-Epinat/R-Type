/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** WeaponDamageSystem
*/

#ifndef WEAPONDAMAGESYSTEM_HPP_
#define WEAPONDAMAGESYSTEM_HPP_
#include "rtype/engine/Registry.hpp"
#include "rtype/engine/ISystem.hpp"
#include "rtype/common/Components.hpp"

namespace rtype::server {
class WeaponDamageSystem : public engine::ISystem {
    public:
        explicit WeaponDamageSystem(const config::GameConfig &config)
        : ISystem(config) {}
        void update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy);
        void dealDamage(std::uint8_t damage, Health &health);
    protected:
    private:
};
}

#endif /* !WEAPONDAMAGESYSTEM_HPP_ */
