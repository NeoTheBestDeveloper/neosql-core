#include "driver/driver.h"
#include "driver/header.h"
#include "driver/page.h"

DriverResult driver_open_db(int32_t fd) {
    DriverResult driver_res = {
        .status = DRIVER_ok,
    };

    Driver driver = {
        .fd = fd,
    };

    HeaderResult header_res = header_read(driver.fd);

    if (header_res.status == HEADER_ok) {
        driver.header = header_res.header;
        driver_res.driver = driver;
        return driver_res;
    }

    driver_res.status = DRIVER_invalid_or_corrupted_header;
    return driver_res;
}

DriverResult driver_create_db(int32_t fd) {
    Driver driver = {
        .fd = fd,
        .header = header_default(),
    };

    header_write(driver.header, fd);

    Page zeroed_page = page_new(0);
    for (int16_t i = 0; i < DEFAULT_EMPTY_DB_ZERO_PAGES_COUNT; ++i) {
        zeroed_page.page_id = i;
        page_write(zeroed_page, fd);
    }
    page_free(&zeroed_page);

    DriverResult res = {
        .driver = driver,
        .status = DRIVER_ok,
    };

    return res;
}

void driver_free(Driver *driver) {}
