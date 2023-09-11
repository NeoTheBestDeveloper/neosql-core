#include <unistd.h>

#include "nclib/stream.h"

#include "database_driver/database_defaults.h"
#include "database_driver/page.h"
#include "database_driver/pages.h"

Pages pages_new(i32 fd, u32 pages_count, u32 cached_pages_count)
{
    return (Pages) {
        .fd = fd,
        .pages_count = pages_count,
        .cached_pages_count = cached_pages_count,
        .cahed_pages_count_changed = false,
        .pages_count_changed = false,
    };
}
void pages_free(Pages* pages) { }

u16 pages_get_page_free_space(const Pages* pages, i32 page_id)
{
    seek_to_page(pages->fd, page_id);

    u8 buf[sizeof(u16)];
    read(pages->fd, buf, sizeof buf);

    Stream stream = stream_new(buf, sizeof buf, DEFAULT_DATABASE_ENDIAN);
    return stream_read_u16(&stream);
}
