#pragma once

#include "DirectoryChangeListenerBase.h"
#include <windows.h>

class WindowsDirectoryChangeListener : public DirectoryChangeListenerBase
{
public:
    explicit WindowsDirectoryChangeListener(const std::filesystem::path& directory);
    ~WindowsDirectoryChangeListener();

private:
    bool _exitRequested = false;
    std::thread _listenThread;

    HANDLE _directoryHandle;
    OVERLAPPED _overlapped;

private:
    void Listen();
};
