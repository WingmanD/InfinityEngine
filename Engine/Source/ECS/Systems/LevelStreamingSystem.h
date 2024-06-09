#pragma once

#include "System.h"
#include "Level.h"
#include "ECS/Components/CLevelStreamingInvoker.h"
#include "ECS/Components/CTransform.h"
#include "LevelStreamingSystem.reflection.h"

REFLECTED()
class LevelStreamingSystem : public System<const CTransform, const CLevelStreamingInvoker>
{
    GENERATED()
    
public:
    LevelStreamingSystem() = default;
    LevelStreamingSystem(const LevelStreamingSystem& other);

    void SetLevel(const std::shared_ptr<Level>& level);
    std::shared_ptr<Level> GetLevel() const;
    
    // System
public:
    virtual void ProcessEntityList(EntityList& entityList, double deltaTime) override;

private:
    std::shared_ptr<Level> _level;
    bool _first = true;

private:
    void OnChunkLoaded(const Level::Chunk& chunk) const;
};
