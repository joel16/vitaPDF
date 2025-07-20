#include "db.h"
#include "fs.h"
#include "log.h"
#include "sqlite3.h"

namespace DB {
    static constexpr char dbPath[] = "ux0:data/vitaPDF/books.db";
    
    int Save(const BookEntry &entry) {
        sqlite3 *db = nullptr;
        char *error = nullptr;

        bool dbExists = FS::FileExists(dbPath);

        int ret = sqlite3_open_v2(dbPath, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
        if (ret != SQLITE_OK) {
            Log::Error("sqlite3_open_v2 failed to open %s\n", dbPath);
            return ret;
        }

        if (!dbExists) {
            const char *createTableSql =
                "CREATE TABLE IF NOT EXISTS books ("
                "    path TEXT PRIMARY KEY,"
                "    page INTEGER NOT NULL,"
                "    zoom REAL,"
                "    rotate REAL"
                ");";

            ret = sqlite3_exec(db, createTableSql, nullptr, nullptr, &error);
            if (ret != SQLITE_OK) {
                Log::Error("sqlite3_exec failed %s\n", dbPath);
                return ret;
            }
        }
        
        const char *insertSql =
            "INSERT INTO books (path, page, zoom, rotate) VALUES (?, ?, ?, ?) "
            "ON CONFLICT(path) DO UPDATE SET "
            "page=excluded.page, zoom=excluded.zoom, rotate=excluded.rotate;";
            
        sqlite3_stmt *stmt;
        ret = sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr);
        if (ret != SQLITE_OK) {
            Log::Error("sqlite3_prepare_v2(%s) failed %s\n", dbPath, sqlite3_errmsg(db));
            sqlite3_close(db);
            return -3;
        }
        
        sqlite3_bind_text(stmt, 1, entry.path, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, entry.page);
        sqlite3_bind_double(stmt, 3, entry.zoom);
        sqlite3_bind_double(stmt, 4, entry.rotate);
        
        ret = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        if (ret != SQLITE_DONE) {
            Log::Error("sqlite3_step failed\n");
            return ret;
        }
        
        return 0;
    }
    
    int GetBookEntry(const char *path, BookEntry &entry) {
        sqlite3 *db = nullptr;
        sqlite3_stmt *stmt = nullptr;
        int page = -1;
        
        if (!FS::FileExists(dbPath)) {
            return -1;
        }
        
        int ret = sqlite3_open_v2(dbPath, &db, SQLITE_OPEN_READONLY, nullptr);
        if (ret != SQLITE_OK) {
            Log::Error("sqlite3_open_v2 failed to open %s\n", dbPath);
            return -1;
        }
        
        const char *selectSql = "SELECT page, zoom, rotate FROM books WHERE path = ?;";
        ret = sqlite3_prepare_v2(db, selectSql, -1, &stmt, nullptr);
        if (ret != SQLITE_OK) {
            Log::Error("sqlite3_prepare_v2(%s) failed %s\n", dbPath, sqlite3_errmsg(db));
            sqlite3_close(db);
            return -1;
        }
        
        sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
        
        ret = sqlite3_step(stmt);
        if (ret == SQLITE_ROW) {
            entry.page = sqlite3_column_int(stmt, 0);
            entry.zoom = sqlite3_column_double(stmt, 1);
            entry.rotate = sqlite3_column_double(stmt, 2);
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return page;
    }
}
