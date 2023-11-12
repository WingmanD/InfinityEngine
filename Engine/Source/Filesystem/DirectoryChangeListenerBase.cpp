#include "DirectoryChangeListenerBase.h"

DirectoryChangeListenerBase::DirectoryChangeListenerBase(std::filesystem::path directory): _directory(std::move(directory))
{
}

const std::filesystem::path& DirectoryChangeListenerBase::GetDirectory() const
{
    return _directory;
}
