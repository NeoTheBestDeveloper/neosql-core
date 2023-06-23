#include <stdlib.h>
#include <unistd.h>

#include "db_header.h"
#include "page.h"

#include "db_driver.h"

static void db_driver_append_cached_page(DbDriver *driver, Page *page);

DbDriverResult db_driver_open_db(i32 db_fd) {
  DbDriver db_driver = {
      .db_fd = db_fd,
      .cached_pages_cnt = 0,
      .cached_pages = NULL,
  };

  DbHeaderResult header_res = db_header_read(db_driver.db_fd);
  DbDriverResult driver_res;

  switch (header_res.status) {
  case DB_HEADER_OK: {
    db_driver.header = header_res.header;
    driver_res.status = DB_DRIVER_OK;
    driver_res.driver = db_driver;
    return driver_res;
  }

  default: {
    driver_res.status = DB_DRIVER_INVALID_OR_CORRUPTED_HEADER;
    return driver_res;
  }
  }
}

DbDriverResult db_driver_create_db(i32 db_fd) {
  DbDriver db_driver = {
      .db_fd = db_fd,
      .header = db_header_new_default(),
      .cached_pages_cnt = 0,
      .cached_pages = NULL,
  };

  db_header_write(&db_driver.header, db_fd);

  Page zeroed_page = page_new(FOUR_KB, 0);
  page_write(&zeroed_page, db_fd);

  DbDriverResult res = {
      .driver = db_driver,
      .status = DB_DRIVER_OK,
  };

  return res;
}

static void db_driver_append_cached_page(DbDriver *driver, Page *page) {
  driver->cached_pages = (CachedPage *)realloc(driver->cached_pages,
                                               (driver->cached_pages_cnt + 1) *
                                                   (sizeof(CachedPage)));

  CachedPage cached_page = {
      .page = page,
      .is_changed = false,
  };

  driver->cached_pages[driver->cached_pages_cnt] = cached_page;
  driver->cached_pages_cnt += 1;
}

void db_driver_free(DbDriver *driver) {
  for (u16 i = 0; i < driver->cached_pages_cnt; ++i) {
    page_free(driver->cached_pages->page + i);
  }
}
