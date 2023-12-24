#ifndef FILE_H
#define FILE_H

#include <stdio.h>

#include "graph.h"

#define PAGE_SIZE 4096

#define CONTENT_SIZE (PAGE_SIZE - sizeof(struct page_header) - sizeof(struct content_header))
#define BLOCKS_NUM 11
#define BLOCK_SIZE (CONTENT_SIZE / BLOCKS_NUM)
#define BLOCK_DATA_SIZE (BLOCK_SIZE - sizeof(struct main_node_info) - sizeof(struct node_header))


#pragma pack(push, 1)
struct page_header {
    uint8_t is_occupied;

    uint64_t page_num;
    uint64_t next_same_page_num;

    uint64_t occupied_blocks_num;
    uint8_t occupied_blocks[BLOCKS_NUM];
};

struct content_header {
    uint8_t content_type;
    union {
        struct node_type node_type;
        struct edge_type edge_type;
    };
};

struct main_node_info {
    uint32_t node_id;
    uint64_t first_edge_addr;
    uint64_t edges_num;
    uint64_t additional_block_addr;
    uint64_t data_size;
};

struct additional_node_info {
    uint64_t next_additional_block_addr;
    uint64_t data_size;
};

struct node_header {
    uint8_t is_additional;
};

struct node_block {
    struct node_header header;

    union {
        struct main_node_info as_main;
        struct additional_node_info as_additional;
    } info;

    uint8_t data[BLOCK_DATA_SIZE];
};

struct edge_block {
    uint32_t src_node_id;
    uint32_t dst_node_id;
    uint64_t next_src_node_edge_addr;
};

struct block {
    union {
        struct node_block as_node;
        struct edge_block as_edge;
    };
};

struct page {
    struct page_header header;
    struct content_header content_header;
    struct block blocks[BLOCKS_NUM];
};
#pragma pack(pop)


int read_page(FILE* file, size_t page_num, struct page* page);
int write_page(FILE* file, size_t page_num, const struct page page);

int read_block(FILE* file, size_t block_addr, struct block* block);
int write_block(FILE* file, size_t block_addr, struct block block);

uint64_t find_block_addr(size_t page_num, size_t block_num);
size_t find_page_num_from_addr(uint64_t addr);
size_t find_block_num_from_addr(uint64_t block_addr);

#endif