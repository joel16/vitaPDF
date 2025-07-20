#pragma once

#include <string>
#include <vector>
#include <SDL3_image/SDL_image.h>

typedef struct {
    SDL_Texture *ptr = nullptr;
    int width = 0;
    int height = 0;
} Tex;

extern std::vector<Tex> icons;
extern unsigned const FOLDER, BOOK;

namespace Textures {
    void Free(Tex &texture);
    void Init(void);
    void Exit(void);
}
