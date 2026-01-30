/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** CollisionSystem
*/

#include "rtype/server/systems/CollisionSystem.hpp"
#include <cmath>
#include <iostream>


namespace rtype::server
{

void CollisionSystem::update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroySet)
{
    registry.view<Hitbox, Transform>([&](EntityId hitboxId, Hitbox &hitbox, Transform &hitboxTransform) {
        registry.view<Hurtbox, Transform>([&](EntityId hurtboxId, Hurtbox &hurtbox, Transform &hurtboxTransform) {
            auto *hitboxTeam = registry.getComponent<TeamComponent>(hitboxId);
            auto *hurtboxTeam = registry.getComponent<TeamComponent>(hurtboxId);
            if (hitboxId == hurtboxId || (hitboxTeam && hurtboxTeam && (hitboxTeam->team == hurtboxTeam->team)))
                return;
            
            
            Collider *hitboxCircleCollider  = registry.getComponent<Collider>(hitboxId);
            Collider *hurtboxCircleCollider  = registry.getComponent<Collider>(hurtboxId);
            if (hitboxCircleCollider) {
                if (circleVsCircle(hitboxTransform, hitboxCircleCollider->radius, hurtboxTransform, hurtboxCircleCollider->radius)) {
                    hurtbox.collidedWith = hitboxId;
                    if (hitbox.destroyOnHit) {
                        toDestroySet.insert(hitboxId);
                    }
                }
            } else {
                BeamCollider *hitboxBeamCollider = registry.getComponent<BeamCollider>(hitboxId);
                if (hitboxBeamCollider && beamVsCircle(hitboxTransform, hitboxBeamCollider->length, hitboxBeamCollider->halfHeight, hurtboxTransform, hurtboxCircleCollider->radius)) {
                    hurtbox.collidedWith = hitboxId;
                    if (hitbox.destroyOnHit) {
                        toDestroySet.insert(hitboxId);
                    }
                }
            }
        });
    });
}



inline bool CollisionSystem::beamVsCircle(const Transform &projectileTransform, float beamLength, float beamHalfHeight, Transform &monsterTransform, float monsterRadius)
{
    float beamStartX = projectileTransform.x;
    float beamEndX = beamLength + beamStartX;

    bool collided = false;
    bool overlapsX = (monsterTransform.x - monsterRadius) <= beamEndX &&
                        (monsterTransform.x + monsterRadius) >= beamStartX;
    bool overlapsY = std::fabs(monsterTransform.y - projectileTransform.y) <= (beamHalfHeight + monsterRadius);
    return (overlapsX && overlapsY);
}

inline bool CollisionSystem::circleVsCircle(
const Transform& a, float ra,
const Transform& b, float rb)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy < (ra + rb) * (ra + rb);
}
}