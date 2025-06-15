#include "vfs.h"
#include <core/arch/i686/VGATextDevice.hpp>
#include <core/arch/i686/E9Device.hpp>

arch::i686::E9Device* GetGlobalE9Device();
arch::i686::VGATextDevice* GetGlobalVGADevice();

int VFS_Write(fd_t file, uint8_t* data, size_t size) {
    switch (file) {
        case VFS_FD_STDIN: 
            return 0; // doesn't make sense to write to input

        case VFS_FD_STDOUT:
        case VFS_FD_STDERR:
            GetGlobalVGADevice()->Write(data, size);
            return size;

        case VFS_FD_DEBUG:
            GetGlobalE9Device()->Write(data, size);
            return size;

        default: return -1; // Invalid file descriptor 
    }
}