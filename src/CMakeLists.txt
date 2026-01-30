find_package(asio REQUIRED CONFIG)
find_package(SFML REQUIRED CONFIG)
find_package(nlohmann_json REQUIRED CONFIG)
find_package(dylib REQUIRED CONFIG)

set(COMMON_SOURCES
  common/protocol/Protocol.cpp
  common/config/GameConfig.cpp
)

set(PLATFORM_NETWORK_LIBS)
if(WIN32)
  set(PLATFORM_NETWORK_LIBS ws2_32 iphlpapi)
endif()

add_library(rtype_common STATIC ${COMMON_SOURCES})
target_include_directories(rtype_common PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(rtype_common PUBLIC asio::asio ${PLATFORM_NETWORK_LIBS})
target_compile_definitions(rtype_common PUBLIC ASIO_STANDALONE)

set(ENGINE_SOURCES
  engine/Registry.cpp
  engine/SystemPipeline.cpp
)

add_library(rtype_engine STATIC ${ENGINE_SOURCES})


target_include_directories(rtype_engine PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(rtype_engine PUBLIC rtype_common)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

if(RTYPE_BUILD_SERVER)
  add_executable(rtype_server
    server/main.cpp
    server/GameServer.cpp
    server/ClientHandler.cpp
    server/GameLogicHandler.cpp
    server/EntityFactory.cpp
    server/systems/PlayerInputSystem.cpp
    server/systems/ShootingSystem.cpp
    server/systems/WeaponDamageSystem.cpp
    server/systems/MovementSystem.cpp
    server/systems/LaserBeamSystem.cpp
    server/systems/ProjectileLifetimeSystem.cpp
    server/systems/FireCooldownSystem.cpp
    server/systems/CleanupSystem.cpp
    server/systems/LevelSystem.cpp
    server/systems/BoundarySystem.cpp
    server/systems/CollisionSystem.cpp
    server/systems/Boss2BehaviorSystem.cpp
    server/systems/PowerUpSystem.cpp
    server/Room.cpp
    server/RoomManager.cpp
    server/systems/WeaponDamageSystem.cpp
    server/systems/ShieldFollowSystem.cpp
  )
  target_include_directories(rtype_server PRIVATE ${CMAKE_SOURCE_DIR}/include)
  target_link_libraries(rtype_server PRIVATE rtype_engine rtype_common asio::asio ${PLATFORM_NETWORK_LIBS} dylib::dylib)
endif()

if(RTYPE_BUILD_CLIENT)
  add_executable(rtype_client
    client/main.cpp
    client/GameClient.cpp
    client/SFMLRenderer.cpp
    client/SpriteManager.cpp
  )
  target_include_directories(rtype_client PRIVATE ${CMAKE_SOURCE_DIR}/include)

  target_link_libraries(rtype_client PRIVATE rtype_engine rtype_common asio::asio ${PLATFORM_NETWORK_LIBS} sfml::sfml nlohmann_json::nlohmann_json)
endif()
