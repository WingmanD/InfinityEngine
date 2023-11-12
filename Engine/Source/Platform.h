#pragma once

#ifdef _WIN32
#include "Filesystem/WindowsDirectoryChangeListener.h"
namespace Platform {
using DirectoryChangeListener = ::WindowsDirectoryChangeListener;
}
#else
#include "OtherDirectoryChangeListener.h"
namespace Platform {
using DirectoryChangeListener = ::OtherDirectoryChangeListener;
}
#endif
