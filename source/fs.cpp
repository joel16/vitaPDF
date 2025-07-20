#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/clib.h>
#include <cstdio>
#include <cstring>
#include <sstream>

#include "config.h"
#include "fs.h"
#include "log.h"
#include "utils.h"

namespace FS {
    bool FileExists(const std::string &path) {
        SceUID file = 0;
        
        if (R_SUCCEEDED(file = sceIoOpen(path.c_str(), SCE_O_RDONLY, 0777))) {
            sceIoClose(file);
            return true;
        }
        
        return false;
    }

    bool DirExists(const std::string &path) {
        SceUID dir = 0;
        
        if (R_SUCCEEDED(dir = sceIoDopen(path.c_str()))) {
            sceIoDclose(dir);
            return true;
        }
        
        return false;
    }

    // Recursive mkdir based on -> https://newbedev.com/mkdir-c-function
    int MakeDir(const std::string &path) {
        std::string currentLevel = "";
        std::string level;
        std::stringstream ss(path);
        
        // split path using slash as a separator
        while (std::getline(ss, level, '/')) {
            currentLevel += level; // append folder to the current level
            
            // create current level
            if (!FS::DirExists(currentLevel) && sceIoMkdir(currentLevel.c_str(), 0777) != 0) {
                return -1;
            }
                
            currentLevel += "/"; // don't forget to append a slash
        }
        
        return 0;
    }

    int GetFileSize(const std::string &path, SceOff &size) {
        int ret = 0;
        SceIoStat stat;

        if (R_FAILED(ret = sceIoGetstat(path.c_str(), &stat))) {
            Log::Error("sceIoGetstat(%s) failed: 0x%lx\n", path.c_str(), ret);
            return ret;
        }
        
        size = stat.st_size;
        return 0;
    }
    
    const char *GetFileExt(const char *filename) {
        const char *dot = std::strrchr(filename, '.');
        if (!dot || dot == filename) {
            return "";
        }
        
        static char ext[10];
        std::strncpy(ext, dot, sizeof(ext) - 1);
        ext[sizeof(ext) - 1] = '\0'; // Ensure null termination
        return ext;
    }

    static bool MatchExtension(const char* ext, const char* const* extList, std::size_t count) {
        for (std::size_t i = 0; i < count; ++i) {
            if (strncasecmp(ext, extList[i], strlen(extList[i])) == 0) {
                return true;
            }
        }
        
        return false;
    }
    
    static bool IsBookExtension(const char* ext) {
        static const char* bookExts[] = {
            ".CBT", ".CBZ", ".EPUB", ".FB2", ".MOBI", ".PDF", ".XPS"
        };
        
        return FS::MatchExtension(ext, bookExts, sizeof(bookExts) / sizeof(bookExts[0]));
    }
    
    bool IsBookType(const char* filename) {
        return FS::IsBookExtension(FS::GetFileExt(filename));
    }   

    int GetDirList(const std::string &path, std::vector<SceIoDirent> &entries) {
        int ret = 0;
        SceUID dir = 0;
        entries.clear();

        // Create ".." entry
        SceIoDirent entry;
        std::strncpy(entry.d_name, "..", 3);
        entry.d_stat.st_mode = SCE_S_IFDIR;
        entry.d_stat.st_size = 0;
        entries.push_back(entry);
        
        if (R_FAILED(ret = dir = sceIoDopen(path.c_str()))) {
            Log::Error("sceIoDopen(%s) failed: %08x\n", path.c_str(), ret);
            return ret;
        }

        while (true) {
            SceIoDirent entry;
            sceClibMemset(&entry, 0, sizeof(entry));

            if (R_FAILED(ret = sceIoDread(dir, &entry))) {
                Log::Error("sceIoDread(%s) failed: %08x\n", path.c_str(), ret);
                sceIoDclose(dir);
                return ret;
            }

            if (ret) {
                if ((!FS::IsBookType(entry.d_name)) && (!SCE_S_ISDIR(entry.d_stat.st_mode))) {
                    continue;
                }
            }
            else {
                break;
            }

            entries.push_back(entry);
        }
        
        sceIoDclose(dir);
        return 0;
    }

    //TODO: Clean up change directory impl.
    static int ChangeDir(const std::string &path, std::vector<SceIoDirent> &entries) {
        int ret = 0;
        std::vector<SceIoDirent> newEntries;
        const std::string newPath = cfg.device + path;
        
        if (R_FAILED(ret = FS::GetDirList(newPath, newEntries))) {
            return ret;
        }
            
        // Free entries and change the current working directory.
        entries.clear();
        cfg.cwd = path;
        Config::Save(cfg);
        entries = newEntries;
        return 0;
    }

    static int GetPrevPath(char path[256]) {
        if (cfg.cwd == "") {
            return -1;
        }
            
        // Remove upmost directory
        bool copy = false;
        int len = 0;
        for (ssize_t i = cfg.cwd.length(); i >= 0; i--) {
            if (cfg.cwd.c_str()[i] == '/') {
                copy = true;
            }
            if (copy) {
                path[i] = cfg.cwd.c_str()[i];
                len++;
            }
        }
        
        // remove trailing slash
        if (len > 1 && path[len - 1] == '/') {
            len--;
        }
            
        path[len] = '\0';
        return 0;
    }
    
    int ChangeDirNext(const std::string &path, std::vector<SceIoDirent> &entries) {
        std::string newPath = cfg.cwd;
        
        if (newPath != "/") {
            newPath.append("/");
        }
            
        newPath.append(path);
        return FS::ChangeDir(newPath, entries);
    }
    
    int ChangeDirPrev(std::vector<SceIoDirent> &entries) {
        char newPath[256];
        if (FS::GetPrevPath(newPath) < 0) {
            return -1;
        }
        
        return FS::ChangeDir(std::string(newPath), entries);
    }
    
    const std::string BuildPath(SceIoDirent &entry) {
        std::string path = cfg.device + cfg.cwd;
        path.append("/");
        path.append(entry.d_name);
        return path;
    }

    int CreateFile(const std::string &path) {
        int ret = 0;
        SceUID file = 0;
        
        if (R_FAILED(ret = file = sceIoOpen(path.c_str(), SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777))) {
            Log::Error("sceIoOpen(%s) failed: 0x%lx\n", path.c_str(), ret);
            return ret;
        }
            
        if (R_FAILED(ret = sceIoClose(file))) {
            Log::Error("sceIoClose(%s) failed: 0x%lx\n", path.c_str(), ret);
            return ret;
        }
            
        return 0;
    }

    int WriteFile(const std::string &path, const void *data, SceSize size) {
        int ret = 0, bytesWritten = 0;
        SceUID file = 0;

        if (R_FAILED(ret = file = sceIoOpen(path.c_str(), SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777))) {
            Log::Error("sceIoOpen(%s) failed: 0x%lx\n", path.c_str(), ret);
            return ret;
        }

        if (R_FAILED(ret = bytesWritten = sceIoWrite(file, data, size))) {
            Log::Error("sceIoWrite(%s) failed: 0x%lx\n", path.c_str(), ret);
            sceIoClose(file);
            return ret;
        }
        
        if (R_FAILED(ret = sceIoClose(file))) {
            Log::Error("sceIoClose(%s) failed: 0x%lx\n", path.c_str(), ret);
            return ret;
        }

        return bytesWritten;
    }
}
