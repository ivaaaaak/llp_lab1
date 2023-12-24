#include "../include/graph.h"


uint64_t calculate_node_data_size(const struct node node) {
    uint64_t data_size = 0;

    for (uint8_t i = 0; i < node.type.attribute_num; i++) {
        switch (node.type.attribute_types[i]) {
            
            case INT32:
                data_size += sizeof(int32_t);
                break;

            case FLOAT:
                data_size += sizeof(float);
                break;

            case BOOL:
                data_size += sizeof(bool);
                break;

            case STRING:
                data_size += strlen(node.attributes[i].value.as_string) + 1;
                break;    
        }
    }
    return data_size;
}


struct node create_node_struct(struct node_type node_type) {
    struct node node = {
        .type = node_type,
        .attributes = malloc(sizeof(struct attribute) * node_type.attribute_num)
    };
    return node;
}


void destroy_node_struct(struct node* node) {
    
    for (size_t i = 0; i < node->type.attribute_num; i++) {

        if (node->type.attribute_types[i] == STRING) {

            free(node->attributes[i].value.as_string);
        }
    }
    
    free(node->attributes);

    if (node->edges_num > 0) {

        free(node->edges);
    }
}


bool compare_attributes(uint8_t attr_type, struct attribute attr1, struct attribute attr2) {

    switch (attr_type) {

        case INT32:
            return attr1.value.as_int32 == attr2.value.as_int32;

        case FLOAT:
            return attr1.value.as_float == attr2.value.as_float;

        case BOOL:
            return attr1.value.as_bool == attr2.value.as_bool;

        case STRING:
            return strcmp(attr1.value.as_string, attr2.value.as_string) == 0 ? true : false;

        default:
            return false;
    }
}
