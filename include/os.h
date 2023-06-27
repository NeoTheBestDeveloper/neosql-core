#pragma once

#ifdef _WIN64

#include <io.h>

#define O_BINARY _O_BINARY
#define F_OK 0
#define access _access
#define open _open
#define write _write
#define read _read

#else

#define O_BINARY 0

#endif // DEBUG
