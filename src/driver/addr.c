#include "driver/addr.h"
#include "driver/page.h"

i64 addr_offset(Addr addr)
{
    return addr.page_id * DEFAULT_PAGE_SIZE + addr.offset;
}

bool addr_cmp(Addr addr1, Addr addr2)
{
    return (addr1.page_id == addr2.page_id) && (addr1.offset == addr2.offset);
}

bool is_null(Addr addr) { return addr_cmp(addr, NULL_ADDR); }
