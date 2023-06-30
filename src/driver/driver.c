#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "driver/driver.h"
#include "driver/header.h"
#include "driver/list_block.h"
#include "driver/page.h"
#include "utils/buf_reader.h"

#include "pseudo_static.h" // For testing static functions.
#include "utils/buf_writer.h"

static ListBlockHeader _read_list_block_header(Page const* page,
                                               int16_t offset)
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

// TODO: Optimize memory usage. Not neccery new allocation for new page. Read
// pages from cache.
static ListBlock _driver_read_list_block(Driver* const driver, Addr addr)
{
    ListBlock* blocks = malloc(sizeof *blocks);
    uint64_t blocks_readen = 0;
    bool is_block_readen = false;

    while (!is_block_readen) {
        Page curr_page = page_read(addr.page_id, driver->fd);

        // Reading block header.
        ListBlockHeader header
            = _read_list_block_header(&curr_page, addr.offset);

        ListBlock block = (ListBlock) {
            .header = header,
            .payload = malloc(header.payload_size),
        };

        // Copying block payload.
        int32_t payload_offset = addr.offset + LIST_BLOCK_HEADER_SIZE;

        memcpy(block.payload, curr_page.payload + payload_offset,
               header.payload_size);

        // Save readen block.
        blocks[blocks_readen] = block;
        blocks_readen += 1;

        // FIX: Unneccary allocation inside last cycle.
        blocks = realloc(blocks, (sizeof *blocks) * (blocks_readen + 1));

        // Check is_overflow.
        if (header.is_overflow) {
            addr = header.next;
        }
        else {
            is_block_readen = true;
        }

        page_free(&curr_page);
    }

    // We read overflowed block and it must to be created for several blocks.
    if (blocks_readen != 1) {
        uint64_t first_payload_size = blocks[0].header.payload_size;
        uint64_t full_payload_size = first_payload_size;

        for (uint64_t i = 1; i < blocks_readen; ++i) {
            full_payload_size += blocks[i].header.payload_size;
        }

        blocks[0].payload = realloc(blocks[0].payload, full_payload_size);

        BufWriter writer
            = buf_writer_new(blocks[0].payload + first_payload_size,
                             full_payload_size - first_payload_size);

        for (uint64_t i = 1; i < blocks_readen; ++i) {
            buf_writer_write(&writer, blocks[i].payload,
                             blocks[i].header.payload_size);
        }

        blocks[0].header.payload_size = full_payload_size;
    }

    ListBlock res = blocks[0];
    res.header.is_overflow = false;
    res.header.next = blocks[blocks_readen - 1].header.next;

    for (uint64_t i = 1; i < blocks_readen; ++i) {
        list_block_free(blocks + i);
    }
    free(blocks);

    return res;
}

DriverResult driver_open_db(int32_t fd)
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

Driver driver_create_db(int32_t fd)
{
    Driver driver = {
        .fd = fd,
        .header = header_default(),
    };

    header_write(&(driver.header), fd);

    Page zeroed_page = page_new(0);
    for (int16_t i = 0; i < DEFAULT_EMPTY_DB_ZERO_PAGES_COUNT; ++i) {
        zeroed_page.page_id = i;
        page_write(&zeroed_page, fd);
    }
    page_free(&zeroed_page);

    return driver;
}

void driver_free(Driver* driver) { }
