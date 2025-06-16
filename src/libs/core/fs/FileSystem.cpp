#include "FileSystem.hpp"

#include <core/cpp/String.hpp>
#include <core/cpp/Memory.hpp>

File* FileSystem::Open(const char* path, FileOpenMode mode) {
    char name[MaxPathSize];

    if (path[0] == '/') path++;

    File* root = this->RootDirectory();
    
    while (*path) {
        bool isLast = false;
        const char* delim = String::Find(path, '/');
        if (delim) {
            Memory::Copy(name, path, delim - path);
            path = delim + 1;
        } else {
            unsigned len = String::Length(path);
            Memory::Copy(name, path, len);
            name[len + 1] = '\0';
            path += len;
            isLast = true;
        }

        FileEntry* nextEntry = FindFile(root, name);
        if (nextEntry) {
            root->Release();

            // check if directory
            if (!isLast && nextEntry->Type() != FileType::Directory) return nullptr;

            root = nextEntry->Open(isLast ? mode : FileOpenMode::Read);
            nextEntry->Release();
        } else {
            root->Release();
            return nullptr;
        }
    }

    return root;
}

void GetFATName(const char* name, char shortName[12]) {
    memset(shortName, ' ', 12);
    shortName[11] = '\0';

    const char* ext = strchr(name, '.');
    if (ext == NULL)
        ext = name + 11;

    for (int i = 0; i < 8 && name[i] && name + i < ext; i++) 
        shortName[i] = toupper(name[i]);
    
    if (ext != name + 11)
        for (int i = 0; i < 3 && ext[i + 1]; i++)
            shortName[i + 8] = toupper(ext[i + 1]);
        
}

FileEntry* FileSystem::FindFile(File* dir, const char* name) {
    char shortName[12];        
    GetFATName(name, shortName);
    FileEntry* entry = dir->ReadFileEntry();
    while (entry) {
        const char* entryName = entry->Name();
        if (Memory::Compare(entryName, shortName, 11) == 0) {
            dir->Release();
            return entry;
        }
        entry->Release();
        entry = dir->ReadFileEntry();
    }
    return nullptr;
}