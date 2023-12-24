#include "../include/query.h"


int create_node_type(struct database* db, const struct node_type type) {
    struct page new_page = create_new_node_page(db, type);
    return write_page(db->file, new_page.header.page_num, new_page);
}


int create_edge_type(struct database* db, const struct edge_type type) {
    struct page new_page = create_new_edge_page(db, type);
    return write_page(db->file, new_page.header.page_num, new_page);
}


int delete_node_type(struct database* db, uint8_t type_id) {
    struct page page = find_first_page(db, true, type_id);

    if (!page.header.is_occupied) {
        return 1;
    }

    page.header.is_occupied = false;

    if (write_page(db->file, page.header.page_num, page) != 0) {
        return 1;
    } 

    while (page.header.next_same_page_num != 0) {

        if (read_page(db->file, page.header.next_same_page_num, &page) != 0) {
            return 1;
        }

        page.header.is_occupied = false;

        if (write_page(db->file, page.header.page_num, page) != 0) {
            return 1;
        } 
    }

    return 0;
}


int delete_edge_type(struct database* db, uint8_t type_id) {
    struct page page = find_first_page(db, false, type_id);
    
    if (!page.header.is_occupied) {
        return 1;
    }

    page.header.is_occupied = false;

    if (write_page(db->file, page.header.page_num, page) != 0) {
        return 1;
    } 

    while (page.header.next_same_page_num != 0) {

        if (read_page(db->file, page.header.next_same_page_num, &page) != 0) {
            return 1;
        }

        page.header.is_occupied = false;

        if (write_page(db->file, page.header.page_num, page) != 0) {
            return 1;
        } 
    }

    return 0;
}


int create_node(struct database* db, const struct node node) {
    uint64_t data_size = calculate_node_data_size(node);

    size_t blocks_num = (data_size - 1) / BLOCK_DATA_SIZE + 1;
    struct block* blocks = malloc(sizeof(struct block) * blocks_num);

    if (!blocks) {
        return 1;
    } 

    fill_node_blocks(node, data_size, blocks_num, blocks);
    if (write_node_blocks(db, node.type, blocks_num, blocks) != 0) {
        return 1;
    }
    free(blocks);

    return 0;
}


int create_edge(struct database* db, struct edge edge) {

    struct block block = {
            .as_edge = {
                    .src_node_id = edge.src_node_id,
                    .dst_node_id = edge.dest_node_id,
                    .next_src_node_edge_addr = 0
            }
    };

    struct page page = read_edge_page_with_free_space(db, edge.type);
    uint64_t new_edge_addr = add_edge_block_to_page(db, edge.type, &page, block);

    if (new_edge_addr == 0) {
        return 1;
    }

    if (write_page(db->file, page.header.page_num, page) != 0) {
        return 1;
    } 

    uint64_t node_block_addr = find_node_block_by_id(db, edge.src_node_id);

    if (node_block_addr == 0) {
        return 1;
    }

    if (read_block(db->file, node_block_addr, &block) != 0) {
        return 1;
    }

    block.as_node.info.as_main.edges_num += 1;
    bool node_has_edge = block.as_node.info.as_main.first_edge_addr != 0;

    if (node_has_edge) {

        if (write_block(db->file, node_block_addr, block) != 0) {
            return 1;
        } 

        uint64_t cur_edge_addr = block.as_node.info.as_main.first_edge_addr;

        if (read_block(db->file, cur_edge_addr, &block) != 0) {
            return 1;
        }

        while (block.as_edge.next_src_node_edge_addr != 0) {
            cur_edge_addr = block.as_edge.next_src_node_edge_addr;

            if (read_block(db->file, cur_edge_addr, &block) != 0) {
                return 1;
            }
        }

        block.as_edge.next_src_node_edge_addr = new_edge_addr;

        if (write_block(db->file, cur_edge_addr, block) != 0) {
            return 1;
        } 

    } else {
        block.as_node.info.as_main.first_edge_addr = new_edge_addr;

        if (write_block(db->file, node_block_addr, block) != 0) {
            return 1;
        } 
    }
    return 0;
}


int match_node_by_attr(struct database* db, struct node_type node_type, uint8_t attr_num, struct attribute attr, struct node* node) {
    struct page page = find_first_page(db, true, node_type.id);

    if (!page.header.is_occupied) {
        return 1;
    }

    bool is_found = false;

    while (!is_found) {
        is_found = find_node_on_page_with_attr(db, page, node_type, attr_num, attr, node);

        if (page.header.next_same_page_num != 0) {
            if (read_page(db->file, page.header.next_same_page_num, &page) != 0) {
                return 1;
            }
        } else {
            break;
        }
    }
    return 0;
}


int match_node_by_id(struct database* db, struct node_type node_type, uint32_t node_id, struct node* node) {
    uint64_t block_addr = find_node_block_by_id(db, node_id);

    if (block_addr == 0) {
        return 1;
    }

    if (block_addr != 0) {
        struct block block = {0};
        if (read_block(db->file, block_addr, &block) != 0) {
            return 1;
        }
        fill_node_from_block(db, block, node_type, node);
    }
    return 0;
}


int match_node_by_edge(struct database* db, struct node_type node_type, uint32_t dst_node_id, struct node* node) {
    struct page page = find_first_page(db, true, node_type.id);

    if (!page.header.is_occupied) {
        return 1;
    }
    
    bool is_found = false;

    while (!is_found) {
        is_found = find_node_on_page_with_edge(db, page, node_type, dst_node_id, node);
        if (page.header.next_same_page_num != 0) {
            if (read_page(db->file, page.header.next_same_page_num, &page) != 0) {
                return 1;
            }
        } else {
            break;
        }
    }
    return 0;
}


int delete_node_by_id(struct database* db, uint32_t node_id) {

    struct block block;
    uint64_t block_addr = find_node_block_by_id(db, node_id);

    if (block_addr == 0) {
        return 1;
    }

    uint64_t edge_addr = 0;

    while (block_addr != 0) {

        if (delete_block_from_page(db, block_addr) != 0) {
            return 1;
        }

        if (read_block(db->file, block_addr, &block) != 0) {
            return 1;
        }
       
        bool is_additional = block.as_node.header.is_additional;

        if (!is_additional) {
            edge_addr = block.as_node.info.as_main.first_edge_addr;
        }

        block_addr = is_additional ? block.as_node.info.as_additional.next_additional_block_addr
                                   : block.as_node.info.as_main.additional_block_addr;
    }

    while (edge_addr != 0) {
        if (delete_block_from_page(db, edge_addr) != 0) {
            return 1;
        }
        if (read_block(db->file, edge_addr, &block) != 0) {
            return 1;
        }
        edge_addr = block.as_edge.next_src_node_edge_addr;
    }
    return 0;
}


int update_node_by_id(struct database* db, struct node_type node_type, uint32_t node_id, uint8_t attr_num, struct attribute attr) {
    struct node new_node = create_node_struct(node_type);

    if (match_node_by_id(db, node_type, node_id, &new_node) != 0) {
        destroy_node_struct(&new_node);
        return 1;
    }

    new_node.attributes[attr_num] = attr;

    if (delete_node_by_id(db, node_id) != 0) {
        destroy_node_struct(&new_node);
        return 1;
    }

    if (create_node(db, new_node) != 0) {
        destroy_node_struct(&new_node);
        return 1;
    }

    destroy_node_struct(&new_node);
    return 0;
}
