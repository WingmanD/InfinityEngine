#pragma once

#include <filesystem>
#include "Platform.h"

class ShaderChangeListener : public Platform::DirectoryChangeListener
{
public:
    explicit ShaderChangeListener(const std::filesystem::path& directory);
    
    virtual void OnFileCreated(const std::filesystem::path& path) const override;
    virtual void OnFileModified(const std::filesystem::path& path) const override;
    virtual void OnFileDeleted(const std::filesystem::path& path) const override;
};
