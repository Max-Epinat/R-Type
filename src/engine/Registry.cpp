#include "rtype/engine/Registry.hpp"

namespace rtype::engine
{

// ========== Entity Management ==========

EntityId Registry::createEntity()
{
    EntityId id;
    
    if (!_freeEntityIds.empty())
    {
        id = _freeEntityIds.back();
        _freeEntityIds.pop_back();
    }
    else
    {
        id = _nextEntityId++;
    }
    
    _activeEntities.insert(id);
    
    return id;
}

void Registry::destroyEntity(EntityId id)
{
    _activeEntities.erase(id);
    
    for (auto &[typeHash, storage] : _componentStorages)
    {
        if (storage)
            storage->removeEntity(id);
    }
    
    _freeEntityIds.push_back(id);
}



}
