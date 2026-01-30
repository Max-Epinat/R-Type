/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** ShootingSystem
*/

#ifndef SHOOTINGSYSTEM_HPP_
#define SHOOTINGSYSTEM_HPP_
#include "rtype/engine/Registry.hpp"
#include "rtype/engine/ISystem.hpp"
#include "rtype/common/Components.hpp"
#include "rtype/common/GameConfig.hpp"
#include "rtype/server/EntityFactory.hpp"

namespace rtype::server {
class ShootingSystem : public engine::ISystem {
    public:
        explicit ShootingSystem(const config::GameConfig &config)
        : ISystem(config) {}
        void update(float dt, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy);

    protected:
    private:
        void handleLaserFireInput(EntityId entity, WeaponComponent &weapon, PlayerComponent &player, FireCooldown &cooldown, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroySet);
        void shootProjectile(WeaponComponent &weapon, const EntityId entity, engine::Registry &registry);
        void startLaserBeam(EntityId entity, WeaponComponent &weapon, PlayerComponent &playerComp, FireCooldown &cooldown, engine::Registry &registry);
        void stopActiveLaser(WeaponComponent &weapon, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroySet);
        void cycleEquippedWeapon(WeaponComponent &weapon);
        std::uint8_t computeLaserDamage(std::uint8_t weaponLevel) const;
        std::uint8_t computeRocketDamage(std::uint8_t weaponLevel) const;
};
}
#endif /* !SHOOTINGSYSTEM_HPP_ */
