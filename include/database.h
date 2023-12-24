#ifndef DATABASE_H
#define DATABASE_H

#include "file.h"


struct database {
    FILE* file;
    size_t last_page_num;
    struct free_space* first_free_space;
};

struct free_space {
    uint8_t content_type;
    uint8_t type_id;
    size_t page_num;
    struct free_space* next_free_space;
};


int create_db(char* file_name, struct database* db);
int destroy_db(struct database* db);

struct page create_new_node_page(struct database* db, struct node_type type);
struct page create_new_edge_page(struct database* db, struct edge_type type);

struct page find_first_page(struct database* db, bool is_for_node, const uint8_t type_id);

uint64_t find_node_block_by_id(struct database* db, uint32_t id);

uint64_t add_edge_block_to_page(struct database* db, struct edge_type type, struct page* page, struct block block);
struct page read_edge_page_with_free_space(struct database* db, struct edge_type type);

void fill_node_blocks(struct node node, uint64_t data_size, size_t blocks_num, struct block* blocks);
int write_node_blocks(struct database* db, struct node_type type, size_t blocks_num, struct block* blocks);

void fill_node_from_block(struct database* db, struct block block, struct node_type node_type, struct node* node);
bool find_node_on_page_with_attr(struct database* db, struct page page, struct node_type node_type, uint8_t attr_num, struct attribute attr, struct node* node);
bool find_node_on_page_with_edge(struct database* db, struct page page, struct node_type node_type, uint32_t dst_node_id, struct node* node);

int delete_block_from_page(struct database* db, uint64_t block_addr);


#endif

