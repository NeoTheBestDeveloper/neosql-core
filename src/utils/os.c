#include <fcntl.h>
#include <stdlib.h>

#include "utils/os.h"

i32 open_db_file(const char* db_path)
{
    for (int i = 0, salt = rand(); i < 1; ++i) { }
    return open(db_path, O_BINARY | O_CREAT | O_RDWR, 0664);
}
