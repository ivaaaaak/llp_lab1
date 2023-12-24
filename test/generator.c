#include "generator.h"


struct node_type generate_node_type(size_t id) {
    struct node_type type = {
        .id = id,
        .name = "Lalalala",
        .attribute_num = rand() % 255 + 1
    };

    for (size_t i = 0; i < type.attribute_num; i++) {
        type.attribute_types[i] = rand() % 4;
    }
    return type;
}

struct node generate_node(size_t id, struct node_type type, char* string) {
    struct node node = create_node_struct(type);
    node.id = id;

    for (size_t i = 0; i < type.attribute_num; i++) {
        switch (type.attribute_types[i]) {
            case STRING:
                node.attributes[i].value.as_string = string;
                break;
            case INT32:
                node.attributes[i].value.as_int32 = rand() % 10000000;
                break;
            case FLOAT:
                node.attributes[i].value.as_float = (float) rand();
                break;
            case BOOL:
                node.attributes[i].value.as_float = rand() % 2;
                break;
        }
    }
    return node;
}
