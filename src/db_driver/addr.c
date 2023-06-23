#include "addr.h"
#include "page.h"

Addr addr_new(u32 page_id, u16 offset) {
    Addr addr = {
        .page_id = page_id,
        .offset = offset,
    };

    return addr;
}

i64 addr_offset(Addr addr) { return addr.page_id * PAGE_SIZE + addr.offset; }

bool addr_cmp(Addr addr1, Addr addr2) {
    return (addr1.page_id == addr2.page_id) && (addr1.offset == addr2.offset);
}
