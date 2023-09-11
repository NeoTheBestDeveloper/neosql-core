#pragma once

#include "database_driver/page.h"
#include "nclib/typedefs.h"

typedef struct {
    i32 fd;
    u32 pages_count;
    u32 cached_pages_count;
    bool pages_count_changed;
    bool cahed_pages_count_changed;
} Pages;

Pages pages_new(i32 fd, u32 pages_count, u32 cached_pages_count);
void pages_free(Pages* pages);

u16 pages_get_page_free_space(const Pages* pages, i32 page_id);
const u8* pages_read_page_payload(const Pages* pages, i32 page_id, i16 offset);

// Can change file inly after commit.
void pages_append_payload_to_page(Pages* pages, i32 page_id, const u8* payload,
                                  u64 payload_size);

// Can change file inly after commit.
i32 pages_allocate_new_page(Pages* pages);

// Dump all changes in memory onto file.
void pages_commit(Pages* pages);
