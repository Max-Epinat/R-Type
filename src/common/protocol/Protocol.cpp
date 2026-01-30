#include "rtype/common/Protocol.hpp"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include <cstring>
#include <cstddef>
#include <algorithm>

namespace rtype::net
{
namespace
{
constexpr std::size_t kHeaderSize = sizeof(std::uint16_t) * 2 + sizeof(std::uint32_t) * 2;

template <typename T>
void appendNetworkValue(std::vector<std::uint8_t> &buffer, T value)
{
    auto *raw = reinterpret_cast<const std::uint8_t *>(&value);
    buffer.insert(buffer.end(), raw, raw + sizeof(T));
}

inline std::uint32_t floatToU32(float value)
{
    std::uint32_t result;
    std::memcpy(&result, &value, sizeof(float));
    return result;
}

inline float u32ToFloat(std::uint32_t value)
{
    float result;
    std::memcpy(&result, &value, sizeof(std::uint32_t));
    return result;
}
}
void BinaryWriter::writeU8(std::uint8_t value)
{
    _buffer.push_back(value);
}

void BinaryWriter::writeU16(std::uint16_t value)
{
    const auto net = htons(value);
    appendNetworkValue(_buffer, net);
}

void BinaryWriter::writeU32(std::uint32_t value)
{
    const auto net = htonl(value);
    appendNetworkValue(_buffer, net);
}

void BinaryWriter::writeF32(float value)
{
    writeU32(floatToU32(value));
}

void BinaryWriter::writeBytes(const std::uint8_t* data, std::size_t size)
{
    _buffer.insert(_buffer.end(), data, data + size);
}

const std::vector<std::uint8_t> &BinaryWriter::data() const noexcept
{
    return _buffer;
}

std::vector<std::uint8_t> BinaryWriter::moveData() noexcept
{
    return std::move(_buffer);
}

BinaryReader::BinaryReader(const std::uint8_t* data, std::size_t size)
    : _data(data), _size(size)
{
}

bool BinaryReader::readU8(std::uint8_t &value)
{
    return read(value);
}

bool BinaryReader::readU16(std::uint16_t &value)
{
    if (!read(value))
        return false;
    value = ntohs(value);
    return true;
}

bool BinaryReader::readU32(std::uint32_t &value)
{
    if (!read(value))
        return false;
    value = ntohl(value);
    return true;
}

bool BinaryReader::readF32(float &value)
{
    std::uint32_t bits{};
    if (!readU32(bits))
        return false;
    value = u32ToFloat(bits);
    return true;
}

template <typename T>
bool BinaryReader::read(T &value)
{
    if (_offset + sizeof(T) > _size)
        return false;
    std::memcpy(&value, _data + _offset, sizeof(T));
    _offset += sizeof(T);
    return true;
}

PacketHeader deserializeHeader(const std::uint8_t* data, std::size_t size, bool &ok)
{
    ok = false;
    PacketHeader header{};
    BinaryReader reader(data, size);

    std::uint16_t type{};
    std::uint16_t payloadSize{};
    std::uint32_t sequence{};
    std::uint32_t timestamp{};

    if (!reader.readU16(type) || !reader.readU16(payloadSize) || !reader.readU32(sequence) || !reader.readU32(timestamp))
        return header;

    header.type = static_cast<PacketType>(type);
    header.payloadSize = payloadSize;
    header.sequence = sequence;
    header.timestamp = timestamp;
    ok = true;
    return header;
}

std::vector<std::uint8_t> serializePacket(PacketType type,
                                          SequenceNumber sequence,
                                          Timestamp timestamp,
                                          const std::uint8_t* payload,
                                          std::size_t payloadSize)
{
    BinaryWriter headerWriter;
    headerWriter.writeU16(static_cast<std::uint16_t>(type));
    headerWriter.writeU16(static_cast<std::uint16_t>(payloadSize));
    headerWriter.writeU32(sequence);
    headerWriter.writeU32(timestamp);

    auto packet = headerWriter.moveData();
    packet.insert(packet.end(), payload, payload + payloadSize);
    return packet;
}

bool deserializePayload(const std::uint8_t* packet, std::size_t packetSize, PacketHeader &header, std::vector<std::uint8_t> &payload)
{
    if (packetSize < kHeaderSize)
        return false;

    bool ok = false;
    header = deserializeHeader(packet, kHeaderSize, ok);
    if (!ok || packetSize < kHeaderSize + header.payloadSize)
        return false;

    payload.assign(packet + kHeaderSize, packet + kHeaderSize + header.payloadSize);
    return true;
}

std::vector<std::uint8_t> serializePlayerInput(const PlayerInput &input, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU8(input.player);
    writer.writeU8(input.up ? 1 : 0);
    writer.writeU8(input.down ? 1 : 0);
    writer.writeU8(input.left ? 1 : 0);
    writer.writeU8(input.right ? 1 : 0);
    writer.writeU8(input.fire ? 1 : 0);
    writer.writeU8(input.swapWeapon ? 1 : 0);
    return serializePacket(PacketType::PlayerInput, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializePlayerInput(const std::uint8_t* payload, std::size_t size, PlayerInput &out)
{
    BinaryReader reader(payload, size);
    std::uint8_t value{};

    return reader.readU8(out.player) &&
           reader.readU8(value) && (out.up = value != 0, true) &&
           reader.readU8(value) && (out.down = value != 0, true) &&
           reader.readU8(value) && (out.left = value != 0, true) &&
           reader.readU8(value) && (out.right = value != 0, true) &&
           reader.readU8(value) && (out.fire = value != 0, true) &&
           reader.readU8(value) && (out.swapWeapon = value != 0, true);
}

std::vector<std::uint8_t> serializePlayerState(const PlayerState &state, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU8(state.player);
    writer.writeF32(state.x);
    writer.writeF32(state.y);
    writer.writeU8(state.hp);
    writer.writeU16(state.score);
    writer.writeU8(state.alive ? 1 : 0);
    writer.writeU8(static_cast<std::uint8_t>(state.powerUpType));
    return serializePacket(PacketType::PlayerState, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializePlayerState(const std::uint8_t* payload, std::size_t size, PlayerState &out)
{
    BinaryReader reader(payload, size);
    std::uint8_t alive{};
    return reader.readU8(out.player) &&
           reader.readF32(out.x) &&
           reader.readF32(out.y) &&
           reader.readU8(out.hp) &&
           reader.readU16(out.score) &&
           reader.readU8(alive) &&
           reader.readU8(reinterpret_cast<uint8_t&>(out.powerUpType)) &&
           (out.alive = alive != 0, true);
}

std::vector<std::uint8_t> serializeMonsterSpawn(const MonsterSpawn &spawn, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(spawn.id);
    writer.writeF32(spawn.x);
    writer.writeF32(spawn.y);
    writer.writeU8(spawn.monsterType);
    return serializePacket(PacketType::MonsterSpawn, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeMonsterSpawn(const std::uint8_t* payload, std::size_t size, MonsterSpawn &out)
{
    BinaryReader reader(payload, size);
    return reader.readU32(out.id) &&
           reader.readF32(out.x) &&
           reader.readF32(out.y) &&
           reader.readU8(out.monsterType);
}

std::vector<std::uint8_t> serializeMonsterState(const MonsterState &state, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(state.id);
    writer.writeU8(state.type);
    writer.writeF32(state.x);
    writer.writeF32(state.y);
    writer.writeF32(state.vx);
    writer.writeF32(state.vy);
    writer.writeU8(state.alive ? 1 : 0);
    return serializePacket(PacketType::MonsterState, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeMonsterState(const std::uint8_t* payload, std::size_t size, MonsterState &out)
{
    BinaryReader reader(payload, size);
    std::uint8_t alive{};
    return reader.readU32(out.id) &&
           reader.readU8(out.type) &&
           reader.readF32(out.x) &&
           reader.readF32(out.y) &&
           reader.readF32(out.vx) &&
           reader.readF32(out.vy) &&
           reader.readU8(alive) &&
           (out.alive = alive != 0, true);
}

std::vector<std::uint8_t> serializeMonsterDeath(const MonsterDeath &death, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(death.id);
    writer.writeU8(death.killer);
    return serializePacket(PacketType::MonsterDeath, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeMonsterDeath(const std::uint8_t* payload, std::size_t size, MonsterDeath &out)
{
    BinaryReader reader(payload, size);
    return reader.readU32(out.id) && reader.readU8(out.killer);
}

std::vector<std::uint8_t> serializeShieldSpawn(const ShieldSpawn &spawn, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(spawn.id);
    writer.writeF32(spawn.x);
    writer.writeF32(spawn.y);
    writer.writeU8(spawn.shieldType);
    return serializePacket(PacketType::ShieldSpawn, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeShieldSpawn(const std::uint8_t* payload, std::size_t size, ShieldSpawn &out)
{
    BinaryReader reader(payload, size);
    return reader.readU32(out.id) &&
           reader.readF32(out.x) &&
           reader.readF32(out.y) &&
           reader.readU8(out.shieldType);
}

std::vector<std::uint8_t> serializeShieldState(const ShieldState &state, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(state.id);
    writer.writeU8(state.type);
    writer.writeF32(state.x);
    writer.writeF32(state.y);
    writer.writeF32(state.vx);
    writer.writeF32(state.vy);
    writer.writeU8(state.alive ? 1 : 0);
    return serializePacket(PacketType::ShieldState, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeShieldState(const std::uint8_t* payload, std::size_t size, ShieldState &out)
{
    BinaryReader reader(payload, size);
    std::uint8_t alive{};
    return reader.readU32(out.id) &&
           reader.readU8(out.type) &&
           reader.readF32(out.x) &&
           reader.readF32(out.y) &&
           reader.readF32(out.vx) &&
           reader.readF32(out.vy) &&
           reader.readU8(alive) &&
           (out.alive = alive != 0, true);
}

std::vector<std::uint8_t> serializeShieldDeath(const ShieldDeath &death, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(death.id);
    return serializePacket(PacketType::ShieldDeath, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeShieldDeath(const std::uint8_t* payload, std::size_t size, ShieldDeath &out)
{
    BinaryReader reader(payload, size);
    return reader.readU32(out.id);
}

std::vector<std::uint8_t> serializePlayerDeath(const PlayerDeath &death, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU8(death.player);
    return serializePacket(PacketType::PlayerDeath, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializePlayerDeath(const std::uint8_t* payload, std::size_t size, PlayerDeath &out)
{
    BinaryReader reader(payload, size);
    return reader.readU8(out.player);
}

std::vector<std::uint8_t> serializeBulletFired(const BulletFired &bullet, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(bullet.id);
    writer.writeU8(bullet.owner);
    writer.writeF32(bullet.x);
    writer.writeF32(bullet.y);
    writer.writeF32(bullet.vx);
    writer.writeF32(bullet.vy);
    writer.writeU8(bullet.fromPlayer ? 1 : 0);
    return serializePacket(PacketType::BulletFired, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeBulletFired(const std::uint8_t* payload, std::size_t size, BulletFired &out)
{
    BinaryReader reader(payload, size);
    std::uint8_t fromPlayer{};
    return reader.readU32(out.id) &&
           reader.readU8(out.owner) &&
           reader.readF32(out.x) &&
           reader.readF32(out.y) &&
           reader.readF32(out.vx) &&
           reader.readF32(out.vy) &&
           reader.readU8(fromPlayer) &&
           (out.fromPlayer = fromPlayer != 0, true);
}

std::vector<std::uint8_t> serializeBulletState(const BulletState &bullet, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(bullet.id);
    writer.writeF32(bullet.x);
    writer.writeF32(bullet.y);
    writer.writeU8(bullet.weaponType);
    writer.writeU8(bullet.fromPlayer ? 1 : 0);
    writer.writeU8(bullet.active ? 1 : 0);
    writer.writeU8(bullet.fromPlayer ? 1 : 0);
    return serializePacket(PacketType::BulletState, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeBulletState(const std::uint8_t* payload, std::size_t size, BulletState &out)
{
    BinaryReader reader(payload, size);
    std::uint8_t fromPlayer{};
    std::uint8_t active{};
    return reader.readU32(out.id) &&
           reader.readF32(out.x) &&
           reader.readF32(out.y) &&
           reader.readU8(out.weaponType) &&
           reader.readU8(fromPlayer) &&
           reader.readU8(active) &&
           reader.readU8(fromPlayer) &&
           (out.fromPlayer = fromPlayer != 0, out.active = active != 0, true) &&
           (out.fromPlayer = fromPlayer != 0, true);
}

std::vector<std::uint8_t> serializeDisconnect(const DisconnectNotice &notice, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU8(notice.player);
    return serializePacket(PacketType::Disconnect, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeDisconnect(const std::uint8_t* payload, std::size_t size, DisconnectNotice &out)
{
    BinaryReader reader(payload, size);
    return reader.readU8(out.player);
}

std::vector<std::uint8_t> serializePlayerAssignment(const PlayerAssignment &assignment, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU8(assignment.playerId);
    return serializePacket(PacketType::PlayerAssignment, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializePlayerAssignment(const std::uint8_t* payload, std::size_t size, PlayerAssignment &out)
{
    BinaryReader reader(payload, size);
    return reader.readU8(out.playerId);
}

std::vector<std::uint8_t> serializePowerUpState(const PowerUpState &state, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(state.id);
    writer.writeU8(state.type);
    writer.writeU8(state.value);
    writer.writeF32(state.x);
    writer.writeF32(state.y);
    writer.writeU8(state.active ? 1 : 0);
    return serializePacket(PacketType::PowerUpState, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializePowerUpState(const std::uint8_t* payload, std::size_t size, PowerUpState &out)
{
    BinaryReader reader(payload, size);
    std::uint8_t active{};
    return reader.readU32(out.id) &&
           reader.readU8(out.type) &&
           reader.readU8(out.value) &&
           reader.readF32(out.x) &&
           reader.readF32(out.y) &&
           reader.readU8(active) &&
           (out.active = (active != 0), true);
}

std::vector<std::uint8_t> serializeLevelBegin(const LevelBegin &level, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU8(level.levelNumber);
    return rtype::net::serializePacket(rtype::net::PacketType::LevelBegin, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeLevelBegin(const std::uint8_t* payload, std::size_t size, LevelBegin &out)
{
    BinaryReader reader(payload, size);
    return reader.readU8(out.levelNumber);
}

std::vector<std::uint8_t> serializeCreateRoom(const CreateRoom &room, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeBytes(reinterpret_cast<const std::uint8_t*>(room.roomName), 32);
    return serializePacket(PacketType::CreateRoom, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeCreateRoom(const std::uint8_t* payload, std::size_t size, CreateRoom &out)
{
    if (size < 32)
        return false;
    std::memcpy(out.roomName, payload, 32);
    out.roomName[31] = '\0';
    return true;
}

std::vector<std::uint8_t> serializeJoinRoom(const JoinRoom &join, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(join.roomId);
    return serializePacket(PacketType::JoinRoom, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeJoinRoom(const std::uint8_t* payload, std::size_t size, JoinRoom &out)
{
    BinaryReader reader(payload, size);
    return reader.readU32(out.roomId);
}

std::vector<std::uint8_t> serializeLeaveRoom(const LeaveRoom &leave, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(leave.roomId);
    return serializePacket(PacketType::LeaveRoom, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeLeaveRoom(const std::uint8_t* payload, std::size_t size, LeaveRoom &out)
{
    BinaryReader reader(payload, size);
    return reader.readU32(out.roomId);
}

std::vector<std::uint8_t> serializeStartGame(const StartGame &start, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(start.roomId);
    return serializePacket(PacketType::StartGame, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeStartGame(const std::uint8_t* payload, std::size_t size, StartGame &out)
{
    BinaryReader reader(payload, size);
    return reader.readU32(out.roomId);
}

std::vector<std::uint8_t> serializeRoomCreated(const RoomCreated &created, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(created.roomId);
    writer.writeBytes(reinterpret_cast<const std::uint8_t*>(created.roomName), 32);
    writer.writeU8(created.hostId);
    writer.writeU8(created.playerId);
    return serializePacket(PacketType::RoomCreated, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeRoomCreated(const std::uint8_t* payload, std::size_t size, RoomCreated &out)
{
    if (size < 38)
        return false;
    BinaryReader reader(payload, size);
    if (!reader.readU32(out.roomId))
        return false;
    std::memcpy(out.roomName, payload + 4, 32);
    out.roomName[31] = '\0';
    BinaryReader reader2(payload + 36, size - 36);
    return reader2.readU8(out.hostId) && reader2.readU8(out.playerId);
}

std::vector<std::uint8_t> serializeRoomJoined(const RoomJoined &joined, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(joined.roomId);
    writer.writeBytes(reinterpret_cast<const std::uint8_t*>(joined.roomName), 32);
    writer.writeU8(joined.hostId);
    writer.writeU8(joined.playerCount);
    writer.writeU8(joined.playerId);
    return serializePacket(PacketType::RoomJoined, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeRoomJoined(const std::uint8_t* payload, std::size_t size, RoomJoined &out)
{
    if (size < 39)
        return false;
    BinaryReader reader(payload, size);
    if (!reader.readU32(out.roomId))
        return false;
    std::memcpy(out.roomName, payload + 4, 32);
    out.roomName[31] = '\0';
    BinaryReader reader2(payload + 36, size - 36);
    return reader2.readU8(out.hostId) && reader2.readU8(out.playerCount) && reader2.readU8(out.playerId);
}

std::vector<std::uint8_t> serializeRoomLeft(const RoomLeft &left, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(left.roomId);
    return serializePacket(PacketType::RoomLeft, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeRoomLeft(const std::uint8_t* payload, std::size_t size, RoomLeft &out)
{
    BinaryReader reader(payload, size);
    return reader.readU32(out.roomId);
}

std::vector<std::uint8_t> serializeGameStarted(const GameStarted &started, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(started.roomId);
    return serializePacket(PacketType::GameStarted, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeGameStarted(const std::uint8_t* payload, std::size_t size, GameStarted &out)
{
    BinaryReader reader(payload, size);
    return reader.readU32(out.roomId);
}

std::vector<std::uint8_t> serializeRoomListResponse(const RoomListResponse &list, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU8(list.roomCount);
    for (std::uint8_t i = 0; i < list.roomCount && i < 16; ++i)
    {
        const auto& entry = list.rooms[i];
        writer.writeU32(entry.roomId);
        writer.writeBytes(reinterpret_cast<const std::uint8_t*>(entry.roomName), 32);
        writer.writeU8(entry.hostId);
        writer.writeU8(entry.playerCount);
        writer.writeU8(entry.maxPlayers);
        writer.writeU8(entry.state);
    }
    return serializePacket(PacketType::RoomListResponse, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeRoomListResponse(const std::uint8_t* payload, std::size_t size, RoomListResponse &out)
{
    BinaryReader reader(payload, size);
    if (!reader.readU8(out.roomCount))
        return false;
    
    std::size_t offset = 1;
    for (std::uint8_t i = 0; i < out.roomCount && i < 16; ++i)
    {
        if (offset + 40 > size)
            return false;
        
        auto& entry = out.rooms[i];
        BinaryReader entryReader(payload + offset, size - offset);
        if (!entryReader.readU32(entry.roomId))
            return false;
        
        std::memcpy(entry.roomName, payload + offset + 4, 32);
        entry.roomName[31] = '\0';
        
        BinaryReader tailReader(payload + offset + 36, size - offset - 36);
        if (!tailReader.readU8(entry.hostId) ||
            !tailReader.readU8(entry.playerCount) ||
            !tailReader.readU8(entry.maxPlayers) ||
            !tailReader.readU8(entry.state))
            return false;
        
        offset += 40;
    }
    return true;
}

std::vector<std::uint8_t> serializeRoomError(const RoomError &error, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU8(error.errorCode);
    writer.writeBytes(reinterpret_cast<const std::uint8_t*>(error.message), 64);
    return serializePacket(PacketType::RoomError, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeRoomError(const std::uint8_t* payload, std::size_t size, RoomError &out)
{
    if (size < 65)
        return false;
    BinaryReader reader(payload, size);
    if (!reader.readU8(out.errorCode))
        return false;
    std::memcpy(out.message, payload + 1, 64);
    out.message[63] = '\0';
    return true;
}

std::vector<std::uint8_t> serializeAllPlayersDead(const AllPlayersDead &msg, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(msg.roomId);
    return serializePacket(PacketType::AllPlayersDead, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeAllPlayersDead(const std::uint8_t* payload, std::size_t size, AllPlayersDead &out)
{
    if (size < 4)
        return false;
    BinaryReader reader(payload, size);
    return reader.readU32(out.roomId);
}

std::vector<std::uint8_t> serializeSpectatorMode(const SpectatorMode &spec, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU8(spec.playerId);
    writer.writeU8(spec.enabled ? 1 : 0);
    return serializePacket(PacketType::SpectatorMode, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeSpectatorMode(const std::uint8_t* payload, std::size_t size, SpectatorMode &out)
{
    if (size < 2)
        return false;
    BinaryReader reader(payload, size);
    if (!reader.readU8(out.playerId))
        return false;
    std::uint8_t enabled;
    if (!reader.readU8(enabled))
        return false;
    out.enabled = (enabled != 0);
    return true;
}

std::vector<std::uint8_t> serializeHostChanged(const HostChanged &msg, SequenceNumber sequence, Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU32(msg.roomId);
    writer.writeU8(msg.newHostId);
    return serializePacket(PacketType::HostChanged, sequence, timestamp, writer.data().data(), writer.data().size());
}

bool deserializeHostChanged(const std::uint8_t* payload, std::size_t size, HostChanged &out)
{
    if (size < 5)
        return false;
    BinaryReader reader(payload, size);
    if (!reader.readU32(out.roomId))
        return false;
    return reader.readU8(out.newHostId);
}

}
