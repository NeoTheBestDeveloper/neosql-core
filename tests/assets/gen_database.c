#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#define DB_PATH "./test_database.db"

#define PAGES_COUNT (4)

#define DEFAULT_PAGE_SIZE (4096)
#define PAGE_HEADER_SIZE (32)
#define PAGE_HEADER_RESERVED_SIZE (PAGE_HEADER_SIZE - 4)
#define PAGE_PAYLOAD_SIZE (DEFAULT_PAGE_SIZE - PAGE_HEADER_SIZE)

#define LIST_BLOCK_HEADER_SIZE (16)

uint8_t page_zeroes[PAGE_PAYLOAD_SIZE] = { 0 };
uint8_t reserved_page[PAGE_HEADER_RESERVED_SIZE] = { 0 };
uint8_t null_addr[6] = { 0 };
uint8_t big_payload[10000] = { 0 };

typedef struct {
    int32_t page_id; // Starts from zero.
    int16_t offset; // Offset inside page payload, not inside all page.
} Addr;

void write_header(int32_t fd)
{
    uint32_t pages_count = PAGES_COUNT;
    int8_t storage_type = 0;

    write(fd, "NEOSQL", 6);
    write(fd, &pages_count, 4);
    write(fd, &storage_type, 1);
    write(fd, null_addr, 6);
    write(fd, null_addr, 6);

    uint8_t reserved[77] = { 0 };
    write(fd, reserved, 77);
}

void append_zero_page(int32_t fd)
{
    int16_t free_space = PAGE_PAYLOAD_SIZE;
    int16_t first_free_byte = 0;

    write(fd, &free_space, 2);
    write(fd, &first_free_byte, 2);
    write(fd, reserved_page, PAGE_HEADER_RESERVED_SIZE);
    write(fd, page_zeroes, PAGE_PAYLOAD_SIZE);
}

int main(int argc, char* argv[])
{
    int32_t fd = open(DB_PATH, O_CREAT | O_TRUNC | O_WRONLY, 0777);

    write_header(fd);

    for (int32_t i = 0; i < PAGES_COUNT; ++i) {
        append_zero_page(fd);
    }

    int8_t block_type = 0;
    int8_t is_owerflow = 0;
    char const first_payload[] = "Hi there!";
    char const second_payload[] = "Hi abuses, I'm kind of sort of your king!";

    int16_t first_block_size = 1 + 1 + 6 + 8 + sizeof first_payload;
    int16_t second_block_size = 1 + 1 + 6 + 8 + sizeof second_payload;

    uint64_t first_block_payload_size = sizeof first_payload;
    uint64_t second_block_payload_size = sizeof second_payload;

    lseek(fd, 100, SEEK_SET);
    write(fd, &(uint16_t) { 0 }, 2);
    write(fd, &(uint16_t) { PAGE_PAYLOAD_SIZE }, 2);
    lseek(fd, PAGE_HEADER_RESERVED_SIZE, SEEK_CUR);

    write(fd, &block_type, 1);
    write(fd, &is_owerflow, 1);
    write(fd, null_addr, sizeof null_addr);
    write(fd, &first_block_payload_size, 8);
    write(fd, first_payload, (sizeof first_payload));

    lseek(fd, 100 + DEFAULT_PAGE_SIZE, SEEK_SET);

    write(fd, &(uint16_t) { 0 }, 2);
    write(fd, &(uint16_t) { PAGE_PAYLOAD_SIZE }, 2);

    lseek(fd, PAGE_HEADER_RESERVED_SIZE, SEEK_CUR);

    write(fd, &block_type, 1);
    write(fd, &is_owerflow, 1);
    write(fd, null_addr, sizeof null_addr);
    write(fd, &second_block_payload_size, 8);
    write(fd, second_payload, (sizeof second_payload));

    uint64_t first_payload_size
        = PAGE_PAYLOAD_SIZE - first_block_size - LIST_BLOCK_HEADER_SIZE;
    for (uint64_t i = 0; i < 10000; ++i) {
        big_payload[i] = i % 255;
    }

    lseek(fd, 100 + first_block_size + PAGE_HEADER_SIZE, SEEK_SET);

    is_owerflow = 1;
    write(fd, &block_type, 1);
    write(fd, &is_owerflow, 1);
    write(fd, &(Addr) { .page_id = 1, .offset = second_block_size }, 6);
    write(fd, &first_payload_size, 8);
    write(fd, big_payload, first_payload_size);

    uint64_t second_payload_size
        = PAGE_PAYLOAD_SIZE - second_block_size - LIST_BLOCK_HEADER_SIZE;
    lseek(fd, 100 + DEFAULT_PAGE_SIZE + PAGE_HEADER_SIZE + second_block_size,
          SEEK_SET);
    is_owerflow = 1;
    write(fd, &block_type, 1);
    write(fd, &is_owerflow, 1);
    write(fd, &(Addr) { .page_id = 2, .offset = 0 }, 6);
    write(fd, &second_payload_size, 8);
    write(fd, big_payload + first_payload_size, second_payload_size);

    is_owerflow = 0;

    uint64_t third_payload_size
        = 10000 - first_payload_size - second_payload_size;

    lseek(fd, 100 + 2 * DEFAULT_PAGE_SIZE, SEEK_SET);

    write(fd, &(uint16_t) { PAGE_PAYLOAD_SIZE - third_payload_size }, 2);
    write(fd, &(uint16_t) { third_payload_size }, 2);

    lseek(fd, PAGE_HEADER_RESERVED_SIZE, SEEK_CUR);

    is_owerflow = 0;
    write(fd, &block_type, 1);
    write(fd, &is_owerflow, 1);
    write(fd, &(Addr) { .page_id = 0, .offset = 0 }, 6);
    write(fd, &third_payload_size, 8);
    write(fd, big_payload + first_payload_size + second_payload_size,
          third_payload_size);

    close(fd);
    return 0;
}
