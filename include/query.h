#ifndef QUERY_H
#define QUERY_H

#include "database.h"


int create_node_type(struct database* db, const struct node_type type);
int create_edge_type(struct database* db, struct edge_type type);
int delete_node_type(struct database* db, uint8_t type_id);
int delete_edge_type(struct database* db, uint8_t type_id);

int create_node(struct database* db, struct node node);
int create_edge(struct database* db, struct edge edge);
int match_node_by_attr(struct database* db, struct node_type node_type, uint8_t attr_num, struct attribute attr, struct node* node);
int match_node_by_id(struct database* db, struct node_type node_type, uint32_t node_id, struct node* node);
int match_node_by_edge(struct database* db, struct node_type node_type, uint32_t dst_node_id, struct node* node);
int update_node_by_id(struct database* db, struct node_type node_type, uint32_t node_id, uint8_t attr_num, struct attribute attr);
int delete_node_by_id(struct database* db, uint32_t node_id);

#endif
