#include "../include/file.h"


int read_page(FILE* file, const size_t page_num, struct page* page) {
    if (!file) {
        return 1;
    }
    if (fseek(file, PAGE_SIZE * (page_num - 1), SEEK_SET) != 0) {
        return 1;
    };
    if (fread(page, sizeof(struct page), 1, file) != 1)  {
        return 1;
    };
    return 0;
}


int write_page(FILE* file, size_t page_num, const struct page page) {
    if (!file) {
        return 1;
    }
    if (fseek(file, PAGE_SIZE * (page_num - 1), SEEK_SET) != 0) {
        return 1;
    }
    if (fwrite(&page, PAGE_SIZE, 1, file) != 1) {
        return 1;
    }
    return 0;
    
}


int read_block(FILE* file, size_t block_addr, struct block* block) {
    if (!file) {
        return 1;
    }
    if (fseek(file, block_addr - 1, SEEK_SET) != 0) {
        return 1;
    };
    if (fread(block, sizeof(struct block), 1, file) != 1)  {
        return 1;
    };
    return 0;

}


int write_block(FILE* file, size_t block_addr, struct block block) {
    if (!file) {
        return 1;
    }
    if (fseek(file, block_addr - 1, SEEK_SET) != 0) {
        return 1;
    }
    if (fwrite(&block, sizeof(struct block), 1, file) != 1) {
        return 1;
    }
    return 0;
}


uint64_t find_block_addr(size_t page_num, size_t block_num) {
    uint64_t page_offset = (page_num - 1) * PAGE_SIZE;
    uint64_t block_offset = block_num * sizeof(struct block);

    return page_offset + sizeof(struct page_header) + sizeof(struct content_header) + block_offset + 1;
}


size_t find_page_num_from_addr(uint64_t addr) {
    return (addr - 1) / PAGE_SIZE + 1;
}


size_t find_block_num_from_addr(uint64_t block_addr) {
    size_t page_num = find_page_num_from_addr(block_addr);
    return (block_addr - sizeof(struct page_header) - sizeof(struct content_header) - (page_num - 1) * PAGE_SIZE) / sizeof(struct block);
}
