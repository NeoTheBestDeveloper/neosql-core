#pragma once

#include <stdlib.h>
#include <time.h>

#include "nclib/typedefs.h"

#include "utils/os.h"

#define TEST_DB_PATH "../tests/assets/database.db"
#define TMP_FILE_PATH_LEN (100)

static inline void gen_rand_str(u64 size, char* buf)
{
    srand((u32)clock());

    int min = 97;
    int max = 122;
    int range = (max - min) + 1;

    for (u64 i = 0; i < size; ++i) {
        buf[i] = (char)(min + (rand() % range));
    }
}

#define WITH_TMP_FILE(_desciptor_name_)                                       \
    char _tmp_file_[TMP_FILE_PATH_LEN + 1] = { 0 };                           \
    gen_rand_str(TMP_FILE_PATH_LEN, _tmp_file_);                              \
    i32 _desciptor_name_ = open_db_file(_tmp_file_);                          \
                                                                              \
    for (int i = 0; i < 1; ++i, close(_desciptor_name_), unlink(_tmp_file_))\
