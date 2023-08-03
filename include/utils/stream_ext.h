#pragma once

#include "nclib/stream.h"

#include "database_driver/addr.h"
#include "database_driver/block.h"
#include "database_driver/header.h"
#include "database_driver/page.h"

Addr stream_read_addr(Stream* stream);
void stream_write_addr(Stream* stream, Addr addr);

Header stream_read_header(Stream* stream);
void stream_write_header(Stream* stream, const Header* header);

Page stream_read_page(Stream* stream);
void stream_write_page(Stream* stream, const Page* page);

BlockHeader stream_read_block_header(Stream* stream);
void stream_write_block_header(Stream* stream, const BlockHeader* header);

BlockPartHeader stream_read_block_part_header(Stream* stream);
void stream_write_block_part_header(Stream* stream,
                                    const BlockPartHeader* header);
