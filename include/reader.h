#pragma once

#include <SDL3_image/SDL_image.h>
#include <string>

typedef struct {
    SDL_Texture *page = nullptr;
    int width = 0;
    int height = 0;
    int pageCount = 0;
    int pageNumber = 0;
    float rotate = 0.0f;
    float zoom = 1.f;
} Book;

namespace Reader {
    void Init(void);
    void Exit(void);
    void OpenDocument(const std::string &path, Book &book);
    void ResetPosition(const Book& book);
    void RenderPage(Book &book);
    void SetZoom(Book &book, float value);
}
