#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "driver/driver.h"
#include "driver/header.h"
#include "driver/list_block.h"
#include "driver/page.h"
#include "utils/buf_reader.h"
#include "utils/buf_writer.h"

#include "utils/pseudo_static.h" // For testing static functions.

static void driver_append_page(Driver* driver, const Page* page)
{
    driver->header.pages_count += 1;
    header_write(&driver->header, driver->fd);

    page_write(page, driver->fd);
}

static ListBlockHeader read_list_block_header(Page const* page, i16 offset)
{
    ListBlockHeader header = { 0 };

    BufReader reader
        = buf_reader_new(page->payload + offset, LIST_BLOCK_HEADER_SIZE);

    buf_reader_read(&reader, &(header.type),
                    1); // at file 1 byte signed number.

    buf_reader_read(&reader, &(header.is_overflow),
                    1); // at file 1 byte signed number.

    buf_reader_read(&reader, &(header.next), 6); // Addr size is 6.

    buf_reader_read(&reader, &(header.payload_size),
                    8); // at file 8 byte unsigned number.

    return header;
}

static ListBlock driver_read_block_part(const Page* page, Addr addr)
{

    ListBlockHeader header = read_list_block_header(page, addr.offset);

    ListBlock block = (ListBlock) {
        .header = header,
        .payload = malloc(header.payload_size),
    };

    i32 payload_offset = addr.offset + LIST_BLOCK_HEADER_SIZE;
    memcpy(block.payload, page->payload + payload_offset, header.payload_size);

    return block;
}

static ListBlock driver_block_from_parts(const ListBlock* parts, u64 parts_cnt)
{
    u64 first_payload_size = parts[0].header.payload_size;
    u64 full_payload_size = first_payload_size;

    for (u64 i = 1; i < parts_cnt; ++i) {
        full_payload_size += parts[i].header.payload_size;
    }

    ListBlock res = {
        .header = {
            .is_overflow = false,  
            .payload_size = full_payload_size,
            .type = parts[0].header.type, 
            .next = parts[parts_cnt - 1].header.next,
        },
        .payload = malloc(full_payload_size),
    };

    memcpy(res.payload, parts[0].payload, first_payload_size);

    BufWriter writer = buf_writer_new(res.payload + first_payload_size,
                                      full_payload_size - first_payload_size);

    for (u64 i = 1; i < parts_cnt; ++i) {
        buf_writer_write(&writer, parts[i].payload,
                         parts[i].header.payload_size);
    }

    return res;
}

// TODO: Optimize memory usage. Not neccery new allocation for new page. Read
// pages from cache.
static ListBlock driver_read_list_block(const Driver* driver, Addr addr)
{
    ListBlock* parts = malloc(sizeof *parts);
    u64 parts_readen = 0;
    bool is_block_readen = false;

    while (!is_block_readen) {
        Page curr_page = page_read(addr.page_id, driver->fd);

        ListBlock part = driver_read_block_part(&curr_page, addr);

        // Save readen part.
        parts[parts_readen] = part;
        parts_readen += 1;

        // Check is_overflow.
        if (part.header.is_overflow) {
            parts = realloc(parts, (sizeof *parts) * (parts_readen + 1));
            addr = part.header.next;
        }
        else {
            is_block_readen = true;
        }

        page_free(&curr_page);
    }

    ListBlock block = driver_block_from_parts(parts, parts_readen);

    for (u64 i = 0; i < parts_readen; ++i) {
        list_block_free(parts + i);
    }
    free(parts);

    return block;
}

static void driver_write_block_part(const ListBlock* block, Page* page,
                                    i16 page_offset, u64 part_payload_size)
{
    BufWriter writer
        = buf_writer_new(page->payload + page_offset,
                         LIST_BLOCK_HEADER_SIZE + part_payload_size);

    buf_writer_write(&writer, &block->header.type, 1);
    buf_writer_write(&writer, &block->header.is_overflow, 1);
    buf_writer_write(&writer, &block->header.next, 6);
    buf_writer_write(&writer, &block->header.payload_size, 8);
    buf_writer_write(&writer, block->payload, part_payload_size);

    page->free_space -= part_payload_size;
    page->first_free_byte += part_payload_size;
}

// Write block into page without partitioning.
static bool driver_try_write_at_exists_page(const Driver* driver,
                                            const ListBlock* block, Addr addr)
{
    bool block_written = false;
    for (i32 i = 0; i < driver->header.pages_count && !block_written; ++i) {
        // TODO: Optimize by reading only page headers.
        Page page = page_read(i, driver->fd);

        if (page.free_space
            >= block->header.payload_size + LIST_BLOCK_HEADER_SIZE) {
            driver_write_block_part(block, &page, addr.offset,
                                    block->header.payload_size);
            page_write(&page, driver->fd);
            block_written = true;
        }

        page_free(&page);
    }

    return block_written;
}

// TODO: Write to cache
static void driver_write_list_block(Driver* driver, const ListBlock* block,
                                    Addr addr)
{
    if (driver_try_write_at_exists_page(driver, block, addr)) {
        return;
    }

    i64 payload_written = 0;
    i64 left_to_write = block->header.payload_size - payload_written;

    Page* pages = malloc(sizeof *pages);
    u64 pages_written = 0;
    pages[0] = page_new(driver->header.pages_count);

    while (true) {
        i64 can_write = 0;

        if (left_to_write >= pages[pages_written].free_space) {
            can_write = pages[pages_written].free_space;
        }
        else {
            can_write = left_to_write;
        }

        driver_write_block_part(block, pages + pages_written, 0, can_write);

        left_to_write -= can_write;

        pages_written += 1;

        if (left_to_write == 0) {
            break;
        }

        pages = realloc(pages, pages_written + sizeof *pages);
    }

    for (u64 i = 0; i < pages_written; ++i) {
        driver_append_page(driver, pages + i);
        page_free(pages + i);
    }
    free(pages);
}

DriverResult driver_open_db(i32 fd)
{
    DriverResult driver_res = {
        .status = DRIVER_OK,
    };

    Driver driver = {
        .fd = fd,
    };

    HeaderResult header_res = header_read(driver.fd);

    if (header_res.status == HEADER_OK) {
        driver.header = header_res.header;
        driver_res.driver = driver;
        return driver_res;
    }

    driver_res.status = DRIVER_INVALID_OR_CORRUPTED_HEADER;
    return driver_res;
}

Driver driver_create_db(i32 fd)
{
    Driver driver = {
        .fd = fd,
        .header = header_default(),
    };

    header_write(&(driver.header), fd);

    Page zeroed_page = page_new(0);
    for (i16 i = 0; i < DEFAULT_EMPTY_DB_ZERO_PAGES_COUNT; ++i) {
        zeroed_page.page_id = i;
        page_write(&zeroed_page, fd);
    }
    page_free(&zeroed_page);

    return driver;
}

void driver_free(Driver* driver) { }
