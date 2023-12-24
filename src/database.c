#include "../include/database.h"


int create_db(char* file_name, struct database* db) {
    FILE* file = fopen(file_name, "wb+");
    if (file != NULL) {
        db->file = file;
        db->last_page_num = 0;
        db->first_free_space = NULL;
        return 0;
    }
    return 1;
}

int destroy_db(struct database* db) {
    fclose(db->file);

    struct free_space* fs = db->first_free_space;
    struct free_space* next_fs = fs->next_free_space;

    while (fs != NULL) {
        free(fs);
        fs = next_fs;
        next_fs = fs->next_free_space;
    }
        
    return 0;
}

void add_free_space_to_db(struct database* db, uint8_t content_type, uint8_t type_id) {
    struct free_space* fs = malloc(sizeof(struct free_space));
    fs->content_type = content_type;
    fs->type_id = type_id;
    fs->page_num = db->last_page_num;
    fs->next_free_space = NULL;

   
    if (db->first_free_space == NULL) {
        db->first_free_space = fs;
    } else {
        struct free_space* efs = db->first_free_space;
        while (efs->next_free_space != NULL) {
            efs = efs->next_free_space;
        }
        efs->next_free_space = fs;
    }
}

void remove_free_space_from_db(struct database* db, size_t page_num) {
    struct free_space* prev_fs = db->first_free_space;

    if (prev_fs->page_num == page_num) {
        db->first_free_space = NULL;
        return;
    }
    
    struct free_space* fs = prev_fs->next_free_space;

    while (prev_fs != NULL) {

        if (fs == NULL) {
            prev_fs->next_free_space = NULL;
            return;
        }

        if (fs->page_num == page_num) {
            prev_fs->next_free_space = fs->next_free_space;
            free(fs);
            return;
        }

        prev_fs = fs;
        fs = fs->next_free_space;
    }
}
 
size_t find_free_page_num(struct database* db, uint8_t content_type, uint8_t type_id) {
    struct free_space* fs = db->first_free_space;

    while (fs != NULL) {

        if (fs->content_type == content_type && fs->type_id == type_id) {
            return fs->page_num;
        }

        fs = fs->next_free_space;
    }

    return 0;
}

struct page create_new_node_page(struct database* db, struct node_type type) {
    db->last_page_num += 1;
    add_free_space_to_db(db, NODE_TYPE, type.id);

    struct page new_page = {

            .header = {
                    .is_occupied = true,
                    .page_num = db->last_page_num,
                    .next_same_page_num = 0,
                    .occupied_blocks_num = 0,
                    .occupied_blocks = {0}
            },

            .content_header = {
                    .content_type = NODE_TYPE,
                    .node_type = type
            },

            .blocks = {0}
    };

    return new_page;
}

struct page create_new_edge_page(struct database* db, struct edge_type type) {
    db->last_page_num += 1;
    add_free_space_to_db(db, EDGE_TYPE, type.id);

    struct page new_page = {

            .header = {
                    .is_occupied = true,
                    .page_num = db->last_page_num,
                    .next_same_page_num = 0,
                    .occupied_blocks_num = 0,
                    .occupied_blocks = {0}
            },

            .content_header = {
                    .content_type = EDGE_TYPE,
                    .edge_type = type
            },

            .blocks = {0}
    };

    return new_page;
}


struct page find_first_page(struct database* db, bool is_for_node, const uint8_t type_id) {
    struct page page;

    for (size_t page_num = 1; page_num <= db->last_page_num; page_num++) {

        read_page(db->file, page_num, &page);

        if (page.header.is_occupied) {

            bool is_node_type = page.content_header.content_type == NODE_TYPE;
            bool is_edge_type = page.content_header.content_type == EDGE_TYPE;
            bool id_is_equal = page.content_header.node_type.id == type_id;

            if (is_for_node && is_node_type && id_is_equal || !is_for_node && is_edge_type && id_is_equal) {
                return page;
            }
        }
    }

    page.header.is_occupied = 0;
    return page;
}


struct page read_node_page_with_free_space(struct database* db, struct node_type type) {
    size_t page_num = find_free_page_num(db, NODE_TYPE, type.id);

    if (page_num != 0) {
        struct page page;
        read_page(db->file, page_num, &page);
        return page;
    }

    return create_new_node_page(db, type); 
}


struct page read_edge_page_with_free_space(struct database* db, struct edge_type type) {
    size_t page_num = find_free_page_num(db, EDGE_TYPE, type.id);

    if (page_num != 0) {
        struct page page;
        read_page(db->file, page_num, &page);
        return page;
    }

    return create_new_edge_page(db, type); 
}


uint64_t find_node_block_by_id(struct database* db, uint32_t id) {
    uint64_t block_addr = 0;
    struct page page;

    for (size_t page_num = 1; page_num <= db->last_page_num; page_num++) {

        if (read_page(db->file, page_num, &page) != 0) {
            return 0;
        }

        bool is_occupied = page.header.is_occupied;
        bool is_node_type = page.content_header.content_type == NODE_TYPE;

        if (is_occupied && is_node_type) {

            for (size_t i = 0; i < BLOCKS_NUM; i++) {

                if (page.header.occupied_blocks[i]) {

                    struct node_block node_block = page.blocks[i].as_node;

                    bool is_additional = node_block.header.is_additional;
                    bool id_is_equal = node_block.info.as_main.node_id == id;

                    if (!is_additional && id_is_equal) {
                        block_addr = find_block_addr(page.header.page_num, i);
                        return block_addr;
                    }
                }
            }
        }
    }
    return 0;
}


uint64_t add_node_block_to_page(struct database* db, struct node_type type, struct page* page, struct block block) {
    
    bool has_free_blocks = page->header.occupied_blocks_num < BLOCKS_NUM;

    if (!has_free_blocks)  {
        remove_free_space_from_db(db, page->header.page_num);
        if (write_page(db->file, page->header.page_num, *page) != 0) {
            return 0;
        }
        *page = read_node_page_with_free_space(db, type);
    }

    for (size_t i = 0; i < BLOCKS_NUM; i++) {
        if (!page->header.occupied_blocks[i]) {

            page->blocks[i] = block;
            page->header.occupied_blocks[i] = true;
            page->header.occupied_blocks_num += 1;

            return find_block_addr(page->header.page_num, i);
        }
    }
    return 0;
}


uint64_t add_edge_block_to_page(struct database* db, struct edge_type type, struct page* page, struct block block) {

    bool has_free_blocks = page->header.occupied_blocks_num < BLOCKS_NUM;

    if (!has_free_blocks) {
        remove_free_space_from_db(db, page->header.page_num);
        if (write_page(db->file, page->header.page_num, *page) != 0) {
            return 0;
        }
        *page = read_edge_page_with_free_space(db, type);
    }

    for (size_t i = 0; i < BLOCKS_NUM; i++) {
        if (!page->header.occupied_blocks[i]) {

            page->blocks[i] = block;
            page->header.occupied_blocks[i] = true;
            page->header.occupied_blocks_num += 1;

            return find_block_addr(page->header.page_num, i);
        }
    }
    return 0;
}


void fill_node_blocks(const struct node node, uint64_t data_size, size_t blocks_num, struct block* blocks) {

    size_t attr_num = 0;
    uint8_t *attr_ptr;
    size_t cur_attr_size = 0;
    size_t written_attr_size = 0;

    for (size_t i = 0; i < blocks_num; i++) {

        size_t block_data_size = data_size > BLOCK_DATA_SIZE? BLOCK_DATA_SIZE : data_size;
        data_size -= BLOCK_DATA_SIZE;

        struct node_block node_block = {0};
        if (i == 0) {
            node_block.header.is_additional = false;
            node_block.info.as_main.node_id = node.id;
            node_block.info.as_main.edges_num = 0;
            node_block.info.as_main.first_edge_addr = 0;
            node_block.info.as_main.data_size = data_size + BLOCK_DATA_SIZE;
        } else {
            node_block.header.is_additional = true;
            node_block.info.as_additional.data_size = block_data_size;
        }

        size_t filled_bytes_num = 0;

        while (attr_num < node.type.attribute_num && filled_bytes_num < block_data_size) {
            if (written_attr_size == 0) {
                switch (node.type.attribute_types[attr_num]) {
                    case STRING:
                        attr_ptr = (uint8_t *) node.attributes[attr_num].value.as_string;
                        cur_attr_size = strlen(node.attributes[attr_num].value.as_string) + 1;
                        break;
                    case BOOL:
                        attr_ptr = (uint8_t *) &node.attributes[attr_num];
                        cur_attr_size = sizeof(bool);
                        break;
                    default:
                        attr_ptr = (uint8_t *) &node.attributes[attr_num];
                        cur_attr_size = sizeof(int32_t);
                        break;
                }
            }

            while (written_attr_size < cur_attr_size && filled_bytes_num < block_data_size) {
                node_block.data[filled_bytes_num] = *attr_ptr;
                attr_ptr++;
                filled_bytes_num++;
                written_attr_size++;
            }

            if (written_attr_size == cur_attr_size) {
                attr_num++;
                written_attr_size = 0;
            }
        }
        blocks[i].as_node = node_block;
    }
}


int write_node_blocks(struct database* db, struct node_type type, size_t blocks_num, struct block* blocks) {
    struct page page = read_node_page_with_free_space(db, type);
    
    uint64_t prev_block_addr = 0;

    for (size_t i = 0; i < blocks_num - 1; i++) {
        size_t j = blocks_num - 1 - i;
        blocks[j].as_node.info.as_additional.next_additional_block_addr = prev_block_addr;

        prev_block_addr = add_node_block_to_page(db, type, &page, blocks[j]);
        if (prev_block_addr == 0) {
            return 1;
        }
    }

    blocks[0].as_node.info.as_main.additional_block_addr = prev_block_addr;
    if (add_node_block_to_page(db, type, &page, blocks[0]) == 0) {
        return 1;
    }

    return write_page(db->file, page.header.page_num, page);
}


void fill_node_attributes_from_buffer(struct node_type node_type, const uint8_t* buffer, struct node* node) {
    size_t string_len;

    for (size_t attr_num = 0; attr_num < node_type.attribute_num; attr_num++) {

        switch (node_type.attribute_types[attr_num]) {

            case INT32:
                node->attributes[attr_num].value.as_int32 = (int32_t) *buffer;
                buffer += sizeof(int32_t);
                break;

            case FLOAT:
                node->attributes[attr_num].value.as_float = (float) *buffer;
                buffer += sizeof(float);
                break;

            case BOOL:
                node->attributes[attr_num].value.as_bool = (bool) *buffer;
                buffer += sizeof(bool);
                break;

            case STRING:
                string_len = strlen((char *) buffer) + 1;
                node->attributes[attr_num].value.as_string = malloc(string_len);
                if (node->attributes[attr_num].value.as_string) {
                    for (size_t i = 0; i < string_len; i++) {
                        node->attributes[attr_num].value.as_string[i] = ((char *) buffer)[i];
                    }
                }
                buffer += string_len;
                break;
        }
    }
}


void read_attributes_to_buffer(struct database* db, struct block block, uint64_t buffer_size, uint8_t buffer[]) {
    size_t read_bytes_num = 0;

    while (read_bytes_num < buffer_size) {

        bool is_additional = block.as_node.header.is_additional;
        uint64_t block_data_size = is_additional? block.as_node.info.as_additional.data_size : (buffer_size > BLOCK_DATA_SIZE? BLOCK_DATA_SIZE : buffer_size);

        for (size_t i = 0; i < block_data_size; i++) {
            buffer[read_bytes_num] = block.as_node.data[i];
            read_bytes_num++;
        }

        uint64_t next_block_addr = is_additional? block.as_node.info.as_additional.next_additional_block_addr : block.as_node.info.as_main.additional_block_addr;
        bool has_next_block = next_block_addr != 0;

        if (has_next_block) {
            read_block(db->file,next_block_addr, &block);
        }
    }
}


void fill_node_from_block(struct database* db, struct block block, struct node_type node_type, struct node* node) {
    node->id = block.as_node.info.as_main.node_id;
    node->type = node_type;

    uint64_t edge_addr = block.as_node.info.as_main.first_edge_addr;
    uint64_t edges_num = block.as_node.info.as_main.edges_num;
    if (edges_num > 0) {
        node->edges = malloc(sizeof(struct edge) * edges_num);
    }

    uint64_t all_data_size = block.as_node.info.as_main.data_size;
    uint8_t buffer[all_data_size];

    read_attributes_to_buffer(db, block, all_data_size, buffer);
    fill_node_attributes_from_buffer(node_type, buffer, node);

    while (edge_addr != 0) {

        struct page page;
        read_page(db->file, find_page_num_from_addr(edge_addr), &page);
        read_block(db->file, edge_addr, &block);

        struct edge edge = {
                .type = page.content_header.edge_type,
                .src_node_id = block.as_edge.src_node_id,
                .dest_node_id = block.as_edge.dst_node_id
        };

        node->edges[node->edges_num] = edge;
        node->edges_num++;
        edge_addr = block.as_edge.next_src_node_edge_addr;
    }
}



bool find_node_on_page_with_attr(struct database* db, struct page page, struct node_type node_type, uint8_t attr_num, struct attribute attr, struct node* node) {
    for (size_t i = 0; i < BLOCKS_NUM; i++) {

        if (page.header.occupied_blocks[i]) {

            struct block block = page.blocks[i];

            if (!block.as_node.header.is_additional) {

                fill_node_from_block(db, block, node_type, node);
                bool is_equal = compare_attributes(node_type.attribute_types[attr_num - 1], node->attributes[attr_num - 1], attr);
                if (is_equal) {
                    return is_equal;
                }
            }
        }
    }
    return false;
}


bool find_node_on_page_with_edge(struct database* db, struct page page, struct node_type node_type, uint32_t dst_node_id, struct node* node) {
    for (size_t i = 0; i < BLOCKS_NUM; i++) {

        if (page.header.occupied_blocks[i]) {

            struct block block = page.blocks[i];

            if (!block.as_node.header.is_additional) {

                fill_node_from_block(db, block, node_type, node);
                for (size_t j = 0; j < node->edges_num; j++) {
                    if (node->edges[j].dest_node_id == dst_node_id) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


int delete_block_from_page(struct database* db, uint64_t block_addr) {
    struct page page;
   
    if (read_page(db->file, find_page_num_from_addr(block_addr), &page) != 0) {
        return 1;
    }

    page.header.occupied_blocks[find_block_num_from_addr(block_addr)] = false;
    page.header.occupied_blocks_num--;

    if (write_page(db->file, page.header.page_num, page) != 0) {
        return 1;
    }
    
    add_free_space_to_db(db, page.content_header.content_type, page.content_header.node_type.id);
    return 0;
}
