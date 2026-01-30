/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** Protocol - Network packet definitions and serialization
*/

#pragma once

#include "Types.hpp"

#include <array>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <vector>

namespace rtype::net
{
enum class PacketType : std::uint16_t
{
    Handshake = 1,
    PlayerInput = 2,
    PlayerState = 3,
    MonsterSpawn = 4,
    MonsterState = 5,
    MonsterDeath = 6,
    PlayerDeath = 7,
    BulletFired = 8,
    BulletState = 9,
    Disconnect = 10,
    PlayerAssignment = 11,
    PowerUpState = 12,
    LevelBegin = 13,
    CreateRoom = 14,
    JoinRoom = 15,
    LeaveRoom = 16,
    StartGame = 17,
    RoomList = 18,
    RoomCreated = 19,
    RoomJoined = 20,
    RoomLeft = 21,
    GameStarted = 22,
    RoomListResponse = 23,
    RoomError = 24,
    AllPlayersDead = 25,
    SpectatorMode = 26,
    HostChanged = 27,
    ShieldSpawn = 28,
    ShieldState = 29,
    ShieldDeath = 30
};

struct PacketHeader
{
    PacketType type{};
    std::uint16_t payloadSize{};
    SequenceNumber sequence{};
    Timestamp timestamp{};
};

struct PlayerInput
{
    PlayerId player{};
    bool up{};
    bool down{};
    bool left{};
    bool right{};
    bool fire{};
    bool swapWeapon{};
};

struct PlayerState
{
    PlayerId player{};
    float x{};
    float y{};
    std::uint8_t hp{};
    std::uint16_t score{};
    bool alive{true};
    PlayerPowerUpType powerUpType{PlayerPowerUpType::Nothing};
};

struct MonsterSpawn
{
    EntityId id{};
    float x{};
    float y{};
    std::uint8_t monsterType{};
};

struct MonsterState
{
    EntityId id{};
    std::uint8_t type{0};
    float x{};
    float y{};
    float vx{};  // Velocity for sprite flipping
    float vy{};
    bool alive{true};
};

struct MonsterDeath
{
    EntityId id{};
    PlayerId killer{};
};

struct PlayerDeath
{
    PlayerId player{};
};

struct BulletFired
{
    EntityId id{};
    PlayerId owner{};
    float x{};
    float y{};
    float vx{};
    float vy{};
    bool fromPlayer{true};
};

struct BulletState
{
    EntityId id{};
    float x{};
    float y{};
    std::uint8_t weaponType{0};
    bool fromPlayer{true};
    bool active{true};
};

struct DisconnectNotice
{
    PlayerId player{};
};

struct PlayerAssignment
{
    PlayerId playerId{};
};

struct PowerUpState
{
    EntityId id{};
    std::uint8_t type{};
    std::uint8_t value{};
    float x{};
    float y{};
    bool active{true};
};

struct ShieldSpawn
{
    EntityId id{};
    float x{};
    float y{};
    std::uint8_t shieldType{};  // Matches parent monster type for color
};

struct ShieldState
{
    EntityId id{};
    std::uint8_t type{0};
    float x{};
    float y{};
    float vx{};  // Velocity for sprite flipping
    float vy{};
    bool alive{true};
};

struct ShieldDeath
{
    EntityId id{};
};

struct LevelBegin
{
    std::uint8_t levelNumber{};
};

struct CreateRoom
{
    char roomName[32]{};
};

struct JoinRoom
{
    std::uint32_t roomId{};
};

struct LeaveRoom
{
    std::uint32_t roomId{};
};

struct StartGame
{
    std::uint32_t roomId{};
};

struct RoomCreated
{
    std::uint32_t roomId{};
    char roomName[32]{};
    PlayerId hostId{};
    PlayerId playerId{};  // The ID assigned to this client
};

struct RoomJoined
{
    std::uint32_t roomId{};
    char roomName[32]{};
    PlayerId hostId{};
    std::uint8_t playerCount{};
    PlayerId playerId{};  // The ID assigned to this client
};

struct RoomLeft
{
    std::uint32_t roomId{};
};

struct GameStarted
{
    std::uint32_t roomId{};
};

struct RoomListEntry
{
    std::uint32_t roomId{};
    char roomName[32]{};
    PlayerId hostId{};
    std::uint8_t playerCount{};
    std::uint8_t maxPlayers{};
    std::uint8_t state{};
};

struct RoomListResponse
{
    std::uint8_t roomCount{};
    RoomListEntry rooms[16]{};
};

struct RoomError
{
    std::uint8_t errorCode{};
    char message[64]{};
};

struct AllPlayersDead
{
    std::uint32_t roomId{};
};

struct SpectatorMode
{
    PlayerId playerId{};
    bool enabled{};
};

struct HostChanged
{
    std::uint32_t roomId{};
    PlayerId newHostId{};
};

class BinaryWriter
{
public:
    void writeU8(std::uint8_t value);
    void writeU16(std::uint16_t value);
    void writeU32(std::uint32_t value);
    void writeF32(float value);
    void writeBytes(const std::uint8_t* data, std::size_t size);

    const std::vector<std::uint8_t> &data() const noexcept;
    std::vector<std::uint8_t> moveData() noexcept;

private:
    std::vector<std::uint8_t> _buffer;
};

class BinaryReader
{
public:
    explicit BinaryReader(const std::uint8_t* data, std::size_t size);

    bool readU8(std::uint8_t &value);
    bool readU16(std::uint16_t &value);
    bool readU32(std::uint32_t &value);
    bool readF32(float &value);

private:
    template <typename T>
    bool read(T &value);

    const std::uint8_t* _data;
    std::size_t _size;
    std::size_t _offset{0};
};

PacketHeader deserializeHeader(const std::uint8_t* data, std::size_t size, bool &ok);

std::vector<std::uint8_t> serializePacket
(
    PacketType type,
    SequenceNumber sequence,
    Timestamp timestamp,
    const std::uint8_t* payload,
    std::size_t payloadSize
);

bool deserializePayload
(
    const std::uint8_t* packet,
    std::size_t packetSize,
    PacketHeader &header,
    std::vector<std::uint8_t> &payload
);

std::vector<std::uint8_t> serializePlayerInput
(
    const PlayerInput &input,
    SequenceNumber sequence,
    Timestamp timestamp
);

bool deserializePlayerInput
(
    const std::uint8_t* payload,
    std::size_t size,
    PlayerInput &out
);

std::vector<std::uint8_t> serializePlayerState
(
    const PlayerState &state,
    SequenceNumber sequence,
    Timestamp timestamp
);

bool deserializePlayerState
(
    const std::uint8_t* payload,
    std::size_t size,
    PlayerState &out
);

std::vector<std::uint8_t> serializeMonsterSpawn
(
    const MonsterSpawn &spawn,
    SequenceNumber sequence,
    Timestamp timestamp
);

bool deserializeMonsterSpawn
(
    const std::uint8_t* payload,
    std::size_t size,
    MonsterSpawn &out
);

std::vector<std::uint8_t> serializeMonsterState
(
    const MonsterState &state,
    SequenceNumber sequence,
    Timestamp timestamp
);

bool deserializeMonsterState
(
    const std::uint8_t* payload,
    std::size_t size,
    MonsterState &out
);

std::vector<std::uint8_t> serializeMonsterDeath
(
    const MonsterDeath &death,
    SequenceNumber sequence,
    Timestamp timestamp
);

bool deserializeMonsterDeath
(
    const std::uint8_t* payload,
    std::size_t size,
    MonsterDeath &out
);

std::vector<std::uint8_t> serializeShieldSpawn
(
    const ShieldSpawn &spawn,
    SequenceNumber sequence,
    Timestamp timestamp
);

bool deserializeShieldSpawn
(
    const std::uint8_t* payload,
    std::size_t size,
    ShieldSpawn &out
);

std::vector<std::uint8_t> serializeShieldState
(
    const ShieldState &state,
    SequenceNumber sequence,
    Timestamp timestamp
);

bool deserializeShieldState
(
    const std::uint8_t* payload,
    std::size_t size,
    ShieldState &out
);

std::vector<std::uint8_t> serializeShieldDeath
(
    const ShieldDeath &death,
    SequenceNumber sequence,
    Timestamp timestamp
);

bool deserializeShieldDeath
(
    const std::uint8_t* payload,
    std::size_t size,
    ShieldDeath &out
);

std::vector<std::uint8_t> serializePlayerDeath
(
    const PlayerDeath &death,
    SequenceNumber sequence,
    Timestamp timestamp
);

bool deserializePlayerDeath
(
    const std::uint8_t* payload,
    std::size_t size,
    PlayerDeath &out
);

std::vector<std::uint8_t> serializeBulletFired
(
    const BulletFired &bullet,
    SequenceNumber sequence,
    Timestamp timestamp
);

bool deserializeBulletFired
(
    const std::uint8_t* payload,
    std::size_t size,
    BulletFired &out
);

std::vector<std::uint8_t> serializeBulletState
(
    const BulletState &bullet,
    SequenceNumber sequence,
    Timestamp timestamp
);

bool deserializeBulletState
(
    const std::uint8_t* payload,
    std::size_t size,
    BulletState &out
);

std::vector<std::uint8_t> serializeDisconnect
(
    const DisconnectNotice &notice,
    SequenceNumber sequence,
    Timestamp timestamp
);

bool deserializeDisconnect
(
    const std::uint8_t* payload,
    std::size_t size,
    DisconnectNotice &out
);

std::vector<std::uint8_t> serializePlayerAssignment
(
    const PlayerAssignment &assignment,
    SequenceNumber sequence,
    Timestamp timestamp
);

bool deserializePlayerAssignment
(
    const std::uint8_t* payload,
    std::size_t size,
    PlayerAssignment &out
);

std::vector<std::uint8_t> serializePowerUpState
(
    const PowerUpState &state,
    SequenceNumber sequence,
    Timestamp timestamp
);

bool deserializePowerUpState
(
    const std::uint8_t* payload,
    std::size_t size,
    PowerUpState &out
);

std::vector<std::uint8_t> serializeLevelBegin(const LevelBegin &level, SequenceNumber sequence, Timestamp timestamp);
bool deserializeLevelBegin(const std::uint8_t* payload, std::size_t size, LevelBegin &out);

std::vector<std::uint8_t> serializeCreateRoom(const CreateRoom &room, SequenceNumber sequence, Timestamp timestamp);
bool deserializeCreateRoom(const std::uint8_t* payload, std::size_t size, CreateRoom &out);

std::vector<std::uint8_t> serializeJoinRoom(const JoinRoom &join, SequenceNumber sequence, Timestamp timestamp);
bool deserializeJoinRoom(const std::uint8_t* payload, std::size_t size, JoinRoom &out);

std::vector<std::uint8_t> serializeLeaveRoom(const LeaveRoom &leave, SequenceNumber sequence, Timestamp timestamp);
bool deserializeLeaveRoom(const std::uint8_t* payload, std::size_t size, LeaveRoom &out);

std::vector<std::uint8_t> serializeStartGame(const StartGame &start, SequenceNumber sequence, Timestamp timestamp);
bool deserializeStartGame(const std::uint8_t* payload, std::size_t size, StartGame &out);

std::vector<std::uint8_t> serializeRoomCreated(const RoomCreated &created, SequenceNumber sequence, Timestamp timestamp);
bool deserializeRoomCreated(const std::uint8_t* payload, std::size_t size, RoomCreated &out);

std::vector<std::uint8_t> serializeRoomJoined(const RoomJoined &joined, SequenceNumber sequence, Timestamp timestamp);
bool deserializeRoomJoined(const std::uint8_t* payload, std::size_t size, RoomJoined &out);

std::vector<std::uint8_t> serializeRoomLeft(const RoomLeft &left, SequenceNumber sequence, Timestamp timestamp);
bool deserializeRoomLeft(const std::uint8_t* payload, std::size_t size, RoomLeft &out);

std::vector<std::uint8_t> serializeGameStarted(const GameStarted &started, SequenceNumber sequence, Timestamp timestamp);
bool deserializeGameStarted(const std::uint8_t* payload, std::size_t size, GameStarted &out);

std::vector<std::uint8_t> serializeRoomListResponse(const RoomListResponse &list, SequenceNumber sequence, Timestamp timestamp);
bool deserializeRoomListResponse(const std::uint8_t* payload, std::size_t size, RoomListResponse &out);

std::vector<std::uint8_t> serializeRoomError(const RoomError &error, SequenceNumber sequence, Timestamp timestamp);
bool deserializeRoomError(const std::uint8_t* payload, std::size_t size, RoomError &out);

std::vector<std::uint8_t> serializeAllPlayersDead(const AllPlayersDead &msg, SequenceNumber sequence, Timestamp timestamp);
bool deserializeAllPlayersDead(const std::uint8_t* payload, std::size_t size, AllPlayersDead &out);

std::vector<std::uint8_t> serializeSpectatorMode(const SpectatorMode &spec, SequenceNumber sequence, Timestamp timestamp);
bool deserializeSpectatorMode(const std::uint8_t* payload, std::size_t size, SpectatorMode &out);

std::vector<std::uint8_t> serializeHostChanged(const HostChanged &msg, SequenceNumber sequence, Timestamp timestamp);
bool deserializeHostChanged(const std::uint8_t* payload, std::size_t size, HostChanged &out);

std::vector<std::uint8_t> serializeShieldSpawn(const ShieldSpawn &spawn, SequenceNumber sequence, Timestamp timestamp);
bool deserializeShieldSpawn(const std::uint8_t* payload, std::size_t size, ShieldSpawn &out);

std::vector<std::uint8_t> serializeShieldState(const ShieldState &state, SequenceNumber sequence, Timestamp timestamp);
bool deserializeShieldState(const std::uint8_t* payload, std::size_t size, ShieldState &out);

std::vector<std::uint8_t> serializeShieldDeath(const ShieldDeath &death, SequenceNumber sequence, Timestamp timestamp);
bool deserializeShieldDeath(const std::uint8_t* payload, std::size_t size, ShieldDeath &out);

}
