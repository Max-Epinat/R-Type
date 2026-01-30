#pragma once

#include <cstdint>
#include <cstddef>

namespace rtype
{
using EntityId = std::uint32_t;
using PlayerId = std::uint8_t;
using SequenceNumber = std::uint32_t;
using Timestamp = std::uint32_t;

enum class PlayerPowerUpType : std::uint8_t
{
    Nothing,
    Shield
};

enum class PowerUpTypes : std::uint8_t
{
    Nothing,
    WeaponUpgrade,
    Shield,
    MIN_VAL = WeaponUpgrade,
    MAX_VAL = Shield
};

struct Direction {
    float dx{1};
    float dy{1};
};


constexpr std::size_t kMaxPlayers = 4;
}
