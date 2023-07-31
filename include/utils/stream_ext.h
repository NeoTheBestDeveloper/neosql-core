#pragma once

#include "nclib/stream.h"

#include "database_driver/addr.h"

Addr stream_read_addr(Stream* stream)
{
    Addr res;
    stream->_stream_read_bytes_impl(stream, (u8*)&(res.page_id),
                                    sizeof res.page_id);
    stream->_stream_read_bytes_impl(stream, (u8*)&(res.offset),
                                    sizeof res.offset);
    return res;
}

void stream_write_addr(Stream* stream, Addr addr)
{
    stream->_stream_write_bytes_impl(stream, (u8*)&(addr.page_id),
                                     sizeof addr.page_id);
    stream->_stream_write_bytes_impl(stream, (u8*)&(addr.offset),
                                     sizeof addr.offset);
}
