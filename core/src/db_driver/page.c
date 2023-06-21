#include <malloc.h>
#include <unistd.h>

#include "core/src/utils/buf_writer.h"
#include "db_header.h"

#include "page.h"

Page page_new(PageSize page_size, u16 page_id) {
    u64 buffer_size = page_size_to_bytes(page_size - PAGE_HEADER_SIZE);
    u8 *buf = (u8 *)calloc(buffer_size, 1);

    Page page = {
        .first_free_byte = PAGE_HEADER_SIZE,
        .free_space = (u16)(buffer_size),
        .writer = buf_writer_new(buf, buffer_size),
        .page_id = page_id,
        .page_size = page_size,
    };

    return page;
}

const u8 *page_get_buf(const Page *page) {
    return buf_writer_get_buf(&page->writer);
}

Page page_read(PageSize page_size, u16 page_id, i32 fd) {
    Page page = {
        .page_id = page_id,
        .page_size = page_size,
    };

    u64 offset = page_id * page_size_to_bytes(page_size) + HEADER_SIZE;
    lseek(fd, (i64)offset, SEEK_SET);

    u8 page_header[PAGE_HEADER_SIZE] = {0};
    read(fd, page_header, PAGE_HEADER_SIZE);

    page.free_space = *(u16 *)page_header;
    page.first_free_byte = *(u16 *)(page_header + 2);

    u64 buffer_size = page_size_to_bytes(page_size) - PAGE_HEADER_SIZE;
    u8 *buf = (u8 *)malloc(buffer_size);

    read(fd, buf, buffer_size);

    page.writer = buf_writer_new(buf, buffer_size);

    return page;
}

void page_write(const Page *page, i32 fd) {
    u64 offset =
        page->page_id * page_size_to_bytes(page->page_size) + HEADER_SIZE;

    u8 page_header[PAGE_HEADER_SIZE] = {0};
    BufWriter writer = buf_writer_new(page_header, PAGE_HEADER_SIZE);

    buf_writer_write(&writer, &page->free_space, 2);
    buf_writer_write(&writer, &page->first_free_byte, 2);

    lseek(fd, (i64)offset, SEEK_SET);

    write(fd, buf_writer_get_buf(&writer), PAGE_HEADER_SIZE);
    write(fd, buf_writer_get_buf(&page->writer),
          page_size_to_bytes(page->page_size) - PAGE_HEADER_SIZE);
}

void page_free(Page *page) { free(page->writer.buf); }
