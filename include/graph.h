#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_SIZE 30
#define MAX_ATTR_NUM 255

#define INT32 0
#define FLOAT 1
#define BOOL 2
#define STRING 3

#define NODE_TYPE 0
#define EDGE_TYPE 1


struct node_type {
    uint8_t id;
    char name[MAX_NAME_SIZE];

    uint8_t attribute_num;
    uint8_t attribute_types[MAX_ATTR_NUM];
};


struct node {
    uint32_t id;
    struct node_type type;
    struct attribute* attributes;
    uint32_t edges_num;
    struct edge* edges;
};


struct attribute {
    union {
        int32_t as_int32;
        float as_float;
        bool as_bool;
        char* as_string;
    } value;
};


struct edge_type {
    uint8_t id;
    char name[MAX_NAME_SIZE];
};


struct edge {
    struct edge_type type;
    uint32_t src_node_id;
    uint32_t dest_node_id;
};


uint64_t calculate_node_data_size(struct node node);
struct node create_node_struct(struct node_type node_type);
void destroy_node_struct(struct node* node);
bool compare_attributes(uint8_t attr_type, struct attribute attr1, struct attribute attr2);

#endif
