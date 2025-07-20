#include <memory>
#include <psp2/json.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>

#include "config.h"
#include "db.h"
#include "fs.h"
#include "log.h"
#include "utils.h"

#define CONFIG_VERSION 1

config_t cfg;

namespace Config {
    static constexpr char configPath[] = "ux0:data/vitaPDF/config.json";

    static const char *configFile =
        "{\n"
        "\t\"version\": %d,\n"
        "\t\"cwd\": \"%s\",\n"
        "\t\"device\": \"%s\",\n"
        "\t\"filename\": %s,\n"
        "\t\"sort\": %d,\n"
        "\t\"dark_theme\": %s\n"
        "}";

    static int configVersion = 0;
    
    class Allocator : public sce::Json::MemAllocator {
        public:
            Allocator() {
            }
            
            virtual void *allocateMemory(size_t size, void *unk) override {
                return std::malloc(size);
            }
            
            virtual void freeMemory(void *ptr, void *unk) override {
                std::free(ptr);
            }
    };
    
    int Save(config_t &config) {
        int ret = 0;
        char buf[512];
        SceSize len = std::snprintf(buf, 512, configFile, CONFIG_VERSION,  config.cwd.c_str(), config.device.c_str(),
            config.filename? "true" : "false", config.sort, config.darkTheme? "true" : "false");
        
        if (R_FAILED(ret = FS::WriteFile(configPath, buf, len))) {
            return ret;
        }
        
        return 0;
    }
    
    static void SetDefault(config_t &config) {
        config.device = "ux0:";
        config.cwd = "/";
    }

    int Load(void) {
        int ret = 0;
            
        if (!FS::FileExists(configPath)) {
            Config::SetDefault(cfg);
            return Config::Save(cfg);
        }
            
        SceUID file = 0;
        if (R_FAILED(ret = file = sceIoOpen(configPath, SCE_O_RDONLY, 0))) {
            Log::Error("sceIoOpen(%s) failed: 0x%lx\n", configPath, ret);
            return ret;
        }
            
        SceSize size = sceIoLseek(file, 0, SCE_SEEK_END);
        std::unique_ptr<char[]> buffer(new char[size]);
        
        if (R_FAILED(ret = sceIoPread(file, buffer.get(), size, SCE_SEEK_SET))) {
            Log::Error("sceIoRead(%s) failed: 0x%lx\n", configPath, ret);
            sceIoClose(file);
            return ret;
        }
        
        if (R_FAILED(ret = sceIoClose(file))) {
            Log::Error("sceIoClose(%s) failed: 0x%lx\n", configPath, ret);
            return ret;
        }

        Allocator *alloc = new Allocator();

        sce::Json::InitParameter params;
        params.allocator = alloc;
        params.bufSize = size;

        sce::Json::Initializer init = sce::Json::Initializer();
        if (R_FAILED(ret = init.initialize(&params))) {
            Log::Error("sce::Json::Initializer::initialize failed  0x%lx\n", ret);
            init.terminate();
            delete alloc;
            return ret;
        }

        sce::Json::Value value = sce::Json::Value();
        if (R_FAILED(ret = sce::Json::Parser::parse(value, buffer.get(), params.bufSize))) {
            Log::Error("sce::Json::Parser::parse failed  0x%lx\n", ret);
            init.terminate();
            delete alloc;
            return ret;
        }

        // We know sceJson API loops through the child values in root alphabetically.
        cfg.cwd = value.getValue(0).getString().c_str();
        cfg.darkTheme = value.getValue(1).getBoolean();
        cfg.device = value.getValue(2).getString().c_str();
        cfg.filename = value.getValue(3).getBoolean();
        cfg.sort = value.getValue(4).getInteger();
        configVersion = value.getValue(5).getInteger();

        // Build path with device + cwd
        const std::string path = cfg.device + cfg.cwd;

        init.terminate();
        delete alloc;
        
        if (!FS::DirExists(path)) {
            cfg.device = "ux0:";
            cfg.cwd = "/";
        }
            
        // Delete config file if config file is updated. This will rarely happen.
        if (configVersion < CONFIG_VERSION) {
            sceIoRemove(configPath);
            Config::SetDefault(cfg);
            return Config::Save(cfg);
        }
        
        return 0;
    }
}
