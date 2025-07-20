#pragma once

struct BookEntry {
    const char *path;
    int page;
    float zoom;
    float rotate;
};

namespace DB {
    int Save(const BookEntry &entry);
    int GetBookEntry(const char *path, BookEntry &entry);
}
