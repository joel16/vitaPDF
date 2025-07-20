#include "gui.h"
#include "imgui.h"
#include "log.h"
#include "textures.h"

std::vector<Tex> icons;
unsigned const FOLDER = 0, BOOK = 1;

namespace Textures {
    static bool Create(unsigned char *data, Tex &texture) {
        texture.ptr = SDL_CreateTexture(
            GUI::GetRenderer(),
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STREAMING,
            texture.width,
            texture.height
        );
        
        if (!texture.ptr) {
            Log::Error("Failed to create SDL texture: %s\n", SDL_GetError());
            return false;
        }
        
        if (SDL_UpdateTexture(texture.ptr, nullptr, data, texture.width * 4) != 0) {
            Log::Error("Failed to update texture: %s\n", SDL_GetError());
            SDL_DestroyTexture(texture.ptr);
            return false;
        }
        
        return true;
    }

    static bool LoadImage(const std::string &path, Tex &texture) {
        texture.ptr = IMG_LoadTexture(GUI::GetRenderer(), path.c_str());

        if (!texture.ptr) {
            Log::Error("Couldn't load %s: %s\n", path.c_str(), SDL_GetError());
            return false;
        }

        SDL_PropertiesID properties = SDL_GetTextureProperties(texture.ptr);
        texture.width = SDL_GetNumberProperty(properties, SDL_PROP_TEXTURE_WIDTH_NUMBER, -1);
        texture.height = SDL_GetNumberProperty(properties, SDL_PROP_TEXTURE_HEIGHT_NUMBER, -1);
        return true;
    }

    void Free(Tex &texture) {
        if (texture.ptr) {
            SDL_DestroyTexture(texture.ptr);
            texture.ptr = nullptr;
        }
    }
    
    void Init(void) {
        const int numIcons = 2;
        std::string filenames[numIcons] = {
            "app0:res/folder.png",
            "app0:res/book.png"
        };

        for (int i = 0; i < numIcons; i++) {
            Tex texture;
            bool ret = Textures::LoadImage(filenames[i], texture);
            IM_ASSERT(ret);

            icons.push_back(texture);
        }
    }

    void Exit(void) {
        for (unsigned int i = 0; i < icons.size(); i++) {
            Textures::Free(icons[i]);
        }
    }
}
