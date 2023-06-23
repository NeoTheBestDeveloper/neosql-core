#ifdef _WIN64
#include <io.h>
#define O_BINARY _O_BINARY
#else
#define O_BINARY 0
#endif // DEBUG
