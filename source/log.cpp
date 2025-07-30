#include <psp2/io/fcntl.h>
#include <psp2/kernel/clib.h>

#include "fs.h"
#include "utils.h"

namespace Log {
    static SceUID logHandle = 0;

    void Init(void) {
        constexpr char logPath[] = "ux0:data/vitaPDF/debug.log";

        if (!FS::FileExists(logPath)) {
            FS::CreateFile(logPath);
        }
            
        if (R_FAILED(logHandle = sceIoOpen(logPath, SCE_O_WRONLY | SCE_O_APPEND, 0))) {
            return;
        }
    }

    void Exit(void) {
        if (logHandle) {
            sceIoClose(logHandle);
        }
    }
    
    void Error(const char *data, ...) {
        char buf[512];
        va_list args;
        va_start(args, data);
        sceClibVsnprintf(buf, sizeof(buf), data, args);
        va_end(args);
        
        std::string error_string = "[ERROR] ";
        error_string.append(buf);
        
        sceClibPrintf("%s", error_string.c_str());
        if (R_FAILED(sceIoWrite(logHandle, error_string.data(), error_string.length()))) {
            return;
        }
    }
}
