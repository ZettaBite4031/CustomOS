#include "vfs.h"
#include <arch/i686/vga_text.h>
#include <arch/i686/e9.h>

int VFS_Write(fd_t file, uint8_t* data, size_t size) {
    switch (file) {
        case VFS_FD_STDIN: 
            return 0; // doesn't make sense to write to input

        case VFS_FD_STDOUT:
        case VFS_FD_STDERR:
            for (size_t i = 0; i < size; i++) 
                VGA_putc(data[i]);
            return size;

        case VFS_FD_DEBUG:
            for (size_t i = 0; i < size; i++) 
                E9_putc(data[i]);
            return size;

        default: return -1; // Invalid file descriptor 
    }
}