#include <pthread.h>
#include <mupdf/fitz.h>
#include "db.h"
#include "gui.h"
#include "log.h"
#include "reader.h"

namespace Reader {
    static fz_context *ctx = nullptr;
    static fz_document *doc = nullptr;
    static SDL_Rect viewport;
    static fz_rect pageBounds = fz_empty_rect;
    static fz_point pageCenter = fz_make_point(0.f, 0.f);
    static pthread_mutex_t mutex[FZ_LOCK_MAX];

    struct RenderData {
        fz_context *ctx;
        fz_display_list *list;
        fz_matrix ctm;
        fz_rect bounds;
        fz_pixmap *pix;
    };

    // Lock callbacks
    static void lock_mutex(void *user, int lock) {
        pthread_mutex_t *mutex = reinterpret_cast<pthread_mutex_t *>(user);
        pthread_mutex_lock(&mutex[lock]);
    }

    static void unlock_mutex(void *user, int lock) {
        pthread_mutex_t *mutex = reinterpret_cast<pthread_mutex_t *>(user);
        pthread_mutex_unlock(&mutex[lock]);
    }

    void Init(void) {
        for (int i = 0; i < FZ_LOCK_MAX; i++) {
            pthread_mutex_init(&mutex[i], nullptr);
        }

        fz_locks_context locks;
        locks.user = mutex;
        locks.lock = lock_mutex;
        locks.unlock = unlock_mutex;

        ctx = fz_new_context(nullptr, &locks, FZ_STORE_UNLIMITED);
        if (!ctx) {
            Log::Error("%s: Cannot create MuPDF context\n", __func__);
            return;
        }

        fz_register_document_handlers(ctx);
    }

    void Exit(void) {
        if (doc) {
            fz_drop_document(ctx, doc);
        }

        if (ctx) {
            fz_drop_context(ctx);
        }
        
        doc = nullptr;
        ctx = nullptr;
    }

    void OpenDocument(const std::string &path, Book &book) {
        if (!ctx) {
            return;
        }
        
        fz_try(ctx)
            doc = fz_open_document(ctx, path.c_str());
        fz_catch(ctx) {
            Log::Error("%s: Cannot open document: %s\n", __func__, fz_caught_message(ctx));
            return;
        }
        
        fz_try(ctx)
            book.pageCount = fz_count_pages(ctx, doc);
        fz_catch(ctx) {
            Log::Error("%s: Cannot count pages: %s\n", __func__, fz_caught_message(ctx));
            fz_drop_document(ctx, doc);
            doc = nullptr;
            return;
        }

        BookEntry entry;
        DB::GetBookEntry(path.c_str(), entry);
        
        if (entry.page >= 0) {
            book.pageNumber = entry.page;
        }

        if (book.pageNumber < 0 || book.pageNumber >= book.pageCount) {
            Log::Error("%s: Page number out of range: %d (of %d)\n", __func__, book.pageNumber + 1, book.pageCount);
            fz_drop_document(ctx, doc);
            doc = nullptr;
            return;
        }

        SDL_GetRenderViewport(GUI::GetRenderer(), &viewport);
        Reader::RenderPage(book);
    }

    static void *RenderThread(void *arg) {
        RenderData *data = reinterpret_cast<RenderData *>(arg);
        fz_context *tctx = fz_clone_context(data->ctx);

        fz_device *device = fz_new_draw_device(tctx, data->ctm, data->pix);
        fz_run_display_list(tctx, data->list, device, fz_identity, data->bounds, nullptr);
        fz_drop_device(tctx, device);

        fz_drop_context(tctx);
        return arg;
    }

    static void CreatePageTexture(Book &book, fz_pixmap *pix) {
        if (!pix || !pix->samples) {
            return;
        }

        SDL_Surface *surface = SDL_CreateSurfaceFrom(
            pix->w,
            pix->h,
            pix->n == 4 ? SDL_PIXELFORMAT_RGBA8888 : SDL_PIXELFORMAT_RGB24,
            pix->samples,
            pix->stride
        );

        if (!surface) {
            Log::Error("%s: SDL_CreateSurfaceFrom failed: %s\n", __func__, SDL_GetError());
            return;
        }
        
        if (book.page) {
            SDL_DestroyTexture(book.page);
            book.page = nullptr;
        }

        book.page = SDL_CreateTextureFromSurface(GUI::GetRenderer(), surface);
        if (!book.page) {
            Log::Error("%s: SDL_CreateTextureFromSurface failed: %s\n", __func__, SDL_GetError());
        }

        book.width = pix->w;
        book.height = pix->h;
        SDL_DestroySurface(surface);
    }
    
    void RenderPage(Book &book) {
        fz_page *page = fz_load_page(ctx, doc, book.pageNumber);
        
        // Get original page bounds
        fz_rect bounds = fz_bound_page(ctx, page);
        pageBounds = bounds;

        // Build display list
        fz_display_list *list = fz_new_display_list(ctx, bounds);
        fz_device *device = fz_new_list_device(ctx, list);
        fz_run_page(ctx, page, device, fz_identity, nullptr);
        fz_drop_device(ctx, device);
        fz_drop_page(ctx, page);
        
        // Apply zoom and rotation
        fz_matrix ctm = fz_scale(book.zoom, book.zoom);
        ctm = fz_pre_rotate(ctm, book.rotate);
        
        // Calculate bounding box for the zoomed page
        fz_rect transformed = fz_transform_rect(bounds, ctm);
        fz_irect bbox = fz_round_rect(transformed);
        
        // Create pixmap for the full page
        fz_pixmap *pix = fz_new_pixmap_with_bbox(ctx, fz_device_rgb(ctx), bbox, nullptr, 0);
        fz_clear_pixmap_with_value(ctx, pix, 0xFF);
        
        RenderData data;
        data.ctx = ctx;
        data.list = list;
        data.ctm = ctm;
        data.bounds = bounds;
        data.pix = pix;
        
        pthread_t tid;
        pthread_create(&tid, nullptr, RenderThread, &data);
        pthread_join(tid, (void **)&data);
        
        Reader::CreatePageTexture(book, pix);
        book.width = pix->w;
        book.height = pix->h;

        // Center position after rendering
        pageCenter.x = book.width / 2.0f;
        pageCenter.y = book.height / 2.0f;
        
        fz_drop_pixmap(ctx, pix);
        fz_drop_display_list(ctx, list);
    }

    static void MovePage(Book &book, float x, float y) {
        float halfViewportW = viewport.w / 2.0f;
        float halfViewportH = viewport.h / 2.0f;
        float halfPageW     = book.width  / 2.0f;
        float halfPageH     = book.height / 2.0f;
        
        // Move center by delta
        pageCenter.x += x;
        pageCenter.y += y;
        
        // Clamp horizontally
        if (book.width <= viewport.w) {
            pageCenter.x = halfPageW;
        }
        else {
            float minX = halfViewportW;
            float maxX = book.width - halfViewportW;
            
            if (pageCenter.x < minX) {
                pageCenter.x = minX;
            }

            if (pageCenter.x > maxX) {
                pageCenter.x = maxX;
            }
        }
        
        // Clamp vertically
        if (book.height <= viewport.h) {
            pageCenter.y = halfPageH;
        }
        else {
            float minY = halfViewportH;
            float maxY = book.height - halfViewportH;

            if (pageCenter.y < minY) {
                pageCenter.y = minY;
            }
            
            if (pageCenter.y > maxY) {
                pageCenter.y = maxY;
            }
        }
    }

    void SetZoom(Book &book, float value) {
        book.zoom = value;
        Reader::RenderPage(book);
        Reader::MovePage(book, 0.f, 0.f);
    }
    
    void SetOrientation(Book &book, float angle) {
        book.rotate = angle;
        Reader::RenderPage(book);
        Reader::ResetPosition(book);
    }
    
    void ResetPosition(const Book& book) {
        pageCenter = fz_make_point(
            ((pageBounds.x1 - pageBounds.x0) * book.zoom) / 2.f,
            ((pageBounds.y1 - pageBounds.y0) * book.zoom) / 2.f
        );
    }
}
