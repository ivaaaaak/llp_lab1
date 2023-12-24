#ifndef GENERATOR_H
#define GENERATOR_H

#include "../include/graph.h"

struct node_type generate_node_type(size_t id);
struct node generate_node(size_t id, struct node_type type, char* string);

#endif