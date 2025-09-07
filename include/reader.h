#pragma once

#include <mupdf/fitz.h>
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

struct RenderData {
    fz_context *ctx;
    fz_display_list *list;
    fz_matrix ctm;
    fz_rect bounds;
    fz_pixmap *pix;
};

namespace Reader {
    void Init(void);
    void Exit(void);
    void OpenDocument(const std::string &path, Book &book);
    void ResetPosition(const Book& book);
    void CreatePageTexture(Book &book, fz_pixmap *pix);
    void RenderPage(Book &book);
    void MovePage(Book &book, float x, float y);
    void SetOrientation(Book &book, float angle);
    void UpdateZoom(float old_zoom, float new_zoom);
}
