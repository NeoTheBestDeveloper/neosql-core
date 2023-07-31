#include "database_driver/addr.h"

bool addr_eq(Addr addr1, Addr addr2)
{
    return (addr1.page_id == addr2.page_id) && (addr1.offset == addr2.offset);
}

bool addr_is_null(Addr addr) { return addr_eq(addr, NULL_ADDR); }
