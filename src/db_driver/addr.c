#include "addr.h"

Addr addr_new(u32 page_number, u16 offset, u64 page_size) {
    Addr addr = {
        .page_number = page_number,
        .offset = offset,
        .page_size = page_size,
    };

    return addr;
}

u64 addr_offset(Addr addr) {
    return addr.page_number * addr.page_size + addr.offset;
}

bool addr_cmp(Addr addr1, Addr addr2) {
    return (addr1.page_number == addr2.page_number) &&
           (addr1.offset == addr2.offset);
}
