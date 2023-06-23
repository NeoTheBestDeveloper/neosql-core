#include <stdlib.h>
#include <unistd.h>

#include "db_header.h"
#include "page.h"

#include "db_driver.h"

DbDriverResult db_driver_open_db(i32 db_fd) {
    DbDriver db_driver = {
        .db_fd = db_fd,
    };

    DbHeaderResult header_res = db_header_read(db_driver.db_fd);
    DbDriverResult driver_res = {
        .status = DB_DRIVER_OK,
    };

    if (header_res.status == DB_HEADER_OK) {
        db_driver.header = header_res.header;
        driver_res.driver = db_driver;
        return driver_res;
    }

    driver_res.status = DB_DRIVER_INVALID_OR_CORRUPTED_HEADER;
    return driver_res;
}

DbDriverResult db_driver_create_db(i32 db_fd) {
    DbDriver db_driver = {
        .db_fd = db_fd,
        .header = db_header_new_default(),
    };

    db_header_write(&db_driver.header, db_fd);

    Page zeroed_page = page_new(0);
    page_write(&zeroed_page, db_fd);

    DbDriverResult res = {
        .driver = db_driver,
        .status = DB_DRIVER_OK,
    };

    page_free(&zeroed_page);

    return res;
}

void db_driver_free(DbDriver *driver) {
    //     for (u16 i = 0; i < driver->cached_pages_cnt; ++i) {
    //         page_free(driver->cached_pages->page + i);
    //     }
}
