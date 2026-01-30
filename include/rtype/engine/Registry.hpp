#pragma once

#include "rtype/common/Types.hpp"

#include <functional>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <optional>
#include <memory>

namespace rtype::engine
{

class Registry
{
public:
    // ========== Entity Management ==========

    EntityId createEntity();
    
    void destroyEntity(EntityId id);

    bool entityExists(EntityId id) const;

    // ========== Component Management ==========
    
    template <typename Component, typename... Args>
    Component &addComponent(EntityId id, Args &&...args);

    template <typename Component, typename... Args>
    Component &emplace(EntityId id, Args &&...args);

    template <typename Component>
    void removeComponent(EntityId id);
    
    /**
     * @brief Get a component from an entity (const version)
     * @tparam Component Type of component to get
     * @param id Entity to get component from
     * @return Pointer to component, or nullptr if entity doesn't have it
     */
    template <typename Component>
    const Component *getComponent(EntityId id) const;
    
    /**
     * @brief Get a component from an entity (mutable version)
     * @tparam Component Type of component to get
     * @param id Entity to get component from
     * @return Pointer to component, or nullptr if entity doesn't have it
     */
    template <typename Component>
    Component *getComponent(EntityId id);
    
    /**
     * @brief Old alias for getComponent (kept for compatibility)
     */
    template <typename Component>
    const Component *get(EntityId id) const;
    
    /**
     * @brief Old alias for getComponent (kept for compatibility)
     */
    template <typename Component>
    Component *get(EntityId id);
    
    /**
     * @brief Check if an entity has a specific component
     * @tparam Component Type of component to check
     * @param id Entity to check
     * @return true if entity has the component, false otherwise
     */
    template <typename Component>
    bool hasComponent(EntityId id) const;
    
    /**
     * @brief Old alias for hasComponent (kept for compatibility)
     */
    template <typename Component>
    bool has(EntityId id) const;

    // ========== Component Iteration ==========
    
    /**
     * @brief Iterate over all entities that have a specific component
     * @tparam Component Type of component to iterate
     * @tparam Func Function type (must accept EntityId and Component&)
     * @param func Callback function called for each entity
     */
    template <typename Component, typename Func>
    void forEach(Func &&func);
    
    /**
     * @brief Iterate over all entities that have a specific component (const version)
     */
    template <typename Component, typename Func>
    void forEach(Func &&func) const;
    
    /**
     * @brief Old alias for forEach (kept for compatibility)
     */
    template <typename Component, typename Func>
    void each(Func &&func);
    
    /**
     * @brief Old alias for forEach (kept for compatibility)
     */
    template <typename Component, typename Func>
    void each(Func &&func) const;
    
    /**
     * @brief Iterate over entities with MULTIPLE components (2 components)
     * @tparam C1 First component type
     * @tparam C2 Second component type
     * @tparam Func Function type (must accept EntityId, C1&, C2&)
     * @param func Callback function
     */
    template <typename C1, typename C2, typename Func>
    void view(Func &&func);
    
    /**
     * @brief Iterate over entities with MULTIPLE components (3 components)
     */
    template <typename C1, typename C2, typename C3, typename Func>
    void view(Func &&func);

    template <typename C1, typename C2, typename Func>
    void view(Func &&func) const;

    std::size_t entityCount() const;

    template <typename Component>
    std::size_t componentCount() const;

private:
    // ========== Internal Storage Structures ==========
    
    struct IComponentStorage
    {
        virtual ~IComponentStorage() = default;
        virtual void removeEntity(EntityId id) = 0;
        virtual bool hasEntity(EntityId id) const = 0;
        virtual std::size_t count() const = 0;
    };
    
    /**
     * @brief Typed storage for a specific component type
     * 
     * This is the INNER MAP : EntityId -> Component instance
     * All components of type T are stored together for cache efficiency.
     */
    template <typename Component>
    struct ComponentStorage : IComponentStorage
    {
        /// Map: Entity ID -> Component data
        std::unordered_map<EntityId, Component> components;
        
        void removeEntity(EntityId id) override 
        { 
            components.erase(id); 
        }
        
        bool hasEntity(EntityId id) const override
        {
            return components.find(id) != components.end();
        }
        
        std::size_t count() const override
        {
            return components.size();
        }
    };
    
    /**
     * @brief Get or create storage for a component type
     * @tparam Component Type of component
     * @return Pointer to the storage for this component type
     */
    template <typename Component>
    ComponentStorage<Component> *getOrCreateStorage();
    
    /**
     * @brief Get storage for a component type (const, doesn't create)
     * @tparam Component Type of component
     * @return Pointer to storage, or nullptr if it doesn't exist
     */
    template <typename Component>
    const ComponentStorage<Component> *getStorage() const;
    
    /**
     * @brief Get the type hash for a component type
     * Used as the key in the outer map to identify component types
     */
    template <typename Component>
    std::size_t getComponentTypeHash() const;
    
    // ========== Member Variables ==========
    
    /// OUTER MAP : Component type hash -> Storage for that type
    /// This allows us to store different component types dynamically
    std::unordered_map<std::size_t, std::unique_ptr<IComponentStorage>> _componentStorages;
    
    /// Pool of recycled entity IDs
    std::vector<EntityId> _freeEntityIds;
    
    EntityId _nextEntityId{1};
    
    /// Set of all active entity IDs (for existence checks)
    std::unordered_set<EntityId> _activeEntities;
};


// ========== Template Implementations ==========

inline bool Registry::entityExists(EntityId id) const
{
    return _activeEntities.find(id) != _activeEntities.end();
}

inline std::size_t Registry::entityCount() const
{
    return _activeEntities.size();
}

template <typename Component>
std::size_t Registry::getComponentTypeHash() const
{
    return typeid(Component).hash_code();
}

template <typename Component>
std::size_t Registry::componentCount() const
{
    const auto *storage = getStorage<Component>();
    return storage ? storage->count() : 0;
}

template <typename Component>
Registry::ComponentStorage<Component> *Registry::getOrCreateStorage()
{
    const auto typeHash = getComponentTypeHash<Component>();
    auto it = _componentStorages.find(typeHash);
    
    if (it == _componentStorages.end())
    {
        auto storage = std::make_unique<ComponentStorage<Component>>();
        auto *rawPtr = storage.get();
        _componentStorages.emplace(typeHash, std::move(storage));
        return rawPtr;
    }
    
    return static_cast<ComponentStorage<Component> *>(it->second.get());
}

template <typename Component>
const Registry::ComponentStorage<Component> *Registry::getStorage() const
{
    const auto typeHash = getComponentTypeHash<Component>();
    auto it = _componentStorages.find(typeHash);
    
    if (it == _componentStorages.end())
        return nullptr;
    
    return static_cast<const ComponentStorage<Component> *>(it->second.get());
}

template <typename Component, typename... Args>
Component &Registry::addComponent(EntityId id, Args &&...args)
{
    auto *storage = getOrCreateStorage<Component>();
    auto &component = storage->components[id];
    component = Component{std::forward<Args>(args)...};
    return component;
}

template <typename Component, typename... Args>
Component &Registry::emplace(EntityId id, Args &&...args)
{
    return addComponent<Component>(id, std::forward<Args>(args)...);
}

template <typename Component>
void Registry::removeComponent(EntityId id)
{
    auto *storage = getOrCreateStorage<Component>();
    storage->removeEntity(id);
}

template <typename Component>
const Component *Registry::getComponent(EntityId id) const
{
    const auto *storage = getStorage<Component>();
    if (!storage)
        return nullptr;
    
    auto it = storage->components.find(id);
    if (it == storage->components.end())
        return nullptr;
    
    return &it->second;
}

template <typename Component>
Component *Registry::getComponent(EntityId id)
{
    auto *storage = getOrCreateStorage<Component>();
    auto it = storage->components.find(id);
    
    if (it == storage->components.end())
        return nullptr;
    
    return &it->second;
}

template <typename Component>
const Component *Registry::get(EntityId id) const
{
    return getComponent<Component>(id);
}

template <typename Component>
Component *Registry::get(EntityId id)
{
    return getComponent<Component>(id);
}

template <typename Component>
bool Registry::hasComponent(EntityId id) const
{
    const auto *storage = getStorage<Component>();
    if (!storage)
        return false;
    
    return storage->hasEntity(id);
}

template <typename Component>
bool Registry::has(EntityId id) const
{
    return hasComponent<Component>(id);
}

template <typename Component, typename Func>
void Registry::forEach(Func &&func)
{
    auto *storage = getOrCreateStorage<Component>();

    std::vector<EntityId> entityIds;
    entityIds.reserve(storage->components.size());
    
    for (const auto &[id, _] : storage->components)
        entityIds.push_back(id);

    for (EntityId id : entityIds)
    {
        auto it = storage->components.find(id);
        if (it != storage->components.end())
            func(id, it->second);
    }
}

template <typename Component, typename Func>
void Registry::forEach(Func &&func) const
{
    const auto *storage = getStorage<Component>();
    if (!storage)
        return;
    
    for (const auto &[id, component] : storage->components)
        func(id, component);
}

template <typename Component, typename Func>
void Registry::each(Func &&func)
{
    forEach<Component>(std::forward<Func>(func));
}

template <typename Component, typename Func>
void Registry::each(Func &&func) const
{
    forEach<Component>(std::forward<Func>(func));
}

template <typename C1, typename C2, typename Func>
void Registry::view(Func &&func)
{
    auto *storage1 = getOrCreateStorage<C1>();
    auto *storage2 = getOrCreateStorage<C2>();

    if (storage1->components.size() <= storage2->components.size()) {
        for (auto &[id, comp1] : storage1->components) {
            auto *comp2 = getComponent<C2>(id);
            if (comp2) {
                func(id, comp1, *comp2);
            }
        }
    } else {
        for (auto &[id, comp2] : storage2->components) {
            auto *comp1 = getComponent<C1>(id);
            if (comp1) {
                func(id, *comp1, comp2);
            }
        }
    }
}

template <typename C1, typename C2, typename C3, typename Func>
void Registry::view(Func &&func)
{
    forEach<C1>([&](EntityId id, C1 &comp1) {
        auto *comp2 = getComponent<C2>(id);
        auto *comp3 = getComponent<C3>(id);
        if (comp2 && comp3) {
            func(id, comp1, *comp2, *comp3);
        }
    });
}

template <typename C1, typename C2, typename Func>
void Registry::view(Func &&func) const
{
    forEach<C1>([&](EntityId id, const C1 &comp1) {
        const auto *comp2 = getComponent<C2>(id);
        if (comp2) {
            func(id, comp1, *comp2);
        }
    });
}

}


