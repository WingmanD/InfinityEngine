#pragma once

#include <filesystem>

class DirectoryChangeListenerBase
{
public:
    explicit DirectoryChangeListenerBase(std::filesystem::path directory);

    DirectoryChangeListenerBase(const DirectoryChangeListenerBase&) = delete;
    DirectoryChangeListenerBase(DirectoryChangeListenerBase&&) = delete;
    virtual ~DirectoryChangeListenerBase() = default;

    DirectoryChangeListenerBase& operator=(const DirectoryChangeListenerBase&) = delete;
    DirectoryChangeListenerBase& operator=(DirectoryChangeListenerBase&&) = delete;

    virtual void OnFileCreated(const std::filesystem::path& path) const = 0;
    virtual void OnFileModified(const std::filesystem::path& path) const = 0;
    virtual void OnFileDeleted(const std::filesystem::path& path) const = 0;

protected:
    const std::filesystem::path& GetDirectory() const;

private:
    std::filesystem::path _directory;
};
