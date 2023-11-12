#include "WindowsDirectoryChangeListener.h"
#include "Core.h"

WindowsDirectoryChangeListener::WindowsDirectoryChangeListener(const std::filesystem::path& directory) : DirectoryChangeListenerBase(directory)
{
    _directoryHandle = CreateFile(
        GetDirectory().c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        nullptr
    );

    _overlapped = {0};
    _overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    _listenThread = std::thread(&WindowsDirectoryChangeListener::Listen, this);
}

WindowsDirectoryChangeListener::~WindowsDirectoryChangeListener()
{
    _exitRequested = true;
    CancelIoEx(_directoryHandle, &_overlapped);
    if (_listenThread.joinable())
    {
        _listenThread.join();
    }
}

void WindowsDirectoryChangeListener::Listen()
{
    if (_directoryHandle == INVALID_HANDLE_VALUE)
    {
        LOG(L"Error in CreateFile");
        return;
    }

    char buffer[1024];
    DWORD bytesReturned;
    
    while (!_exitRequested)
    {
        const BOOL result = ReadDirectoryChangesW(
            _directoryHandle,
            &buffer,
            sizeof(buffer),
            TRUE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytesReturned,
            &_overlapped,
            nullptr
        );
        
        if (WaitForSingleObject(_overlapped.hEvent, INFINITE) == WAIT_OBJECT_0)
        {
            DWORD transferred;
            if (GetOverlappedResult(_directoryHandle, &_overlapped, &transferred, FALSE))
            {
                if (result == FALSE)
                {
                    continue;
                }

                FILE_NOTIFY_INFORMATION* notification;
                uint64 offset = 0;
                do
                {
                    notification = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(static_cast<char*>(buffer) + offset);
                    std::filesystem::path filePath = GetDirectory() / std::wstring(notification->FileName, notification->FileNameLength / sizeof(wchar_t));

                    OnFileModified(filePath);

                    offset += notification->NextEntryOffset;
                }
                while (notification->NextEntryOffset);
            }
            ResetEvent(_overlapped.hEvent);
        }
        else
        {
            break;
        }
    }

    CloseHandle(_directoryHandle);
    CloseHandle(_overlapped.hEvent);
}
