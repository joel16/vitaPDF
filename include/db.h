#pragma once

struct BookEntry {
    const char *path;
    int page = 0;
    float zoom = 1.0f;
    float rotate = 0.0f;
};

namespace DB {
    int Save(const BookEntry &entry);
    int GetBookEntry(const char *path, BookEntry &entry);
}
