#include "tests.h"

int test_add(struct database* db, 
            FILE* test_result, 
            size_t count,
            size_t frequency_of_verification) {

    uint64_t k = count / frequency_of_verification;

    uint64_t nodes_num[k];
    uint64_t file_size[k];
    uint64_t time[k];
    
    size_t node_id = 0;
    char* string = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

    struct node_type node_type = generate_node_type(0);
    create_node_type(db, node_type);

    for (int i = 0; i < count; i++) {

        if (i % frequency_of_verification == 0) {
            nodes_num[i / frequency_of_verification] = i + 1;

            clock_t t1 = clock();
            create_node(db, generate_node(node_id, node_type, string));
            clock_t t2 = clock();
            
            node_id++;
            time[i / frequency_of_verification] = ((double)(t2 - t1) / CLOCKS_PER_SEC) * 1000;
            fseek(db->file, 0, SEEK_END);
            file_size[i / frequency_of_verification] =  ftell(db->file);

        } else {
            create_node(db, generate_node(node_id, node_type, string));
            node_id++;
        }
    }

    fwrite(&k, sizeof(uint64_t), 1, test_result);
    fwrite(nodes_num, sizeof(uint64_t), k, test_result);
    fwrite(file_size, sizeof(uint64_t), k, test_result);
    fwrite(time, sizeof(uint64_t), k, test_result);
    return 0;
}

int test_delete(struct database* db, 
                    FILE* test_result, 
                    size_t count, 
                    size_t frequency_of_verification) {

    uint64_t k = count / frequency_of_verification;

    uint64_t nodes_num[k];
    uint64_t time[k];

    char* string = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    struct node_type node_type = generate_node_type(0);
    create_node_type(db, node_type);

    for (int i = 0; i < count; i++) {
        create_node(db, generate_node(i, node_type, string));
    }

    for (int i = 0; i < count; i++) {
        if (i % frequency_of_verification == 0) {
            nodes_num[i / frequency_of_verification] = i + 1;

            clock_t t1 = clock();
            delete_node_by_id(db, i);
            clock_t t2 = clock();
            time[i / frequency_of_verification] = ((double)(t2 - t1) / CLOCKS_PER_SEC) * 1000;

        } else {
            delete_node_by_id(db, i);
        }
    }

    fwrite(&k, sizeof(uint64_t), 1, test_result);
    fwrite(nodes_num, sizeof(uint64_t), k, test_result);
    fwrite(time, sizeof(uint64_t), k, test_result);
    return 0;
}


int test_add_and_delete(struct database* db, 
                            struct node_type node_type,
                            FILE* test_result, 
                            size_t add_count, 
                            size_t delete_count, 
                            uint64_t count) {

    uint64_t file_size[count];
    uint64_t nodes_num[count];

    char* string = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

    for (int i = 0; i < count; i++) {

        for (int j = 0; j < add_count; j++) {
            create_node(db, generate_node(j, node_type, string));
        }

        for (int j = 0; j < delete_count; j++) {
            delete_node_by_id(db, j);
        }

        fseek(db->file, 0, SEEK_END);
        file_size[i] = ftell(db->file);
        nodes_num[i] = (i + 1) * (add_count - delete_count);
    }
    
    fwrite(&count, sizeof(uint64_t), 1, test_result);
    fwrite(nodes_num, sizeof(uint64_t), count, test_result);
    fwrite(file_size, sizeof(uint64_t), count, test_result);
    return 0;
}

int test_match(struct database* db, 
                        FILE* test_result, 
                        struct node_type node_type,
                        size_t count, 
                        size_t frequency_of_verification) {

    uint64_t k = count / frequency_of_verification;

    uint64_t nodes_num[k];
    uint64_t time[k];

    char* string = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

    for (int i = 0; i < count; i++) {
        create_node(db, generate_node(i, node_type, string));
    }

    struct node node = create_node_struct(node_type);

    for (int i = 0; i < count; i++) {

        if (i % frequency_of_verification == 0) {
            clock_t t1 = clock();
            match_node_by_id(db, node_type, i, &node);
            clock_t t2 = clock();

            time[i / frequency_of_verification] = ((double)(t2 - t1) / CLOCKS_PER_SEC) * 1000;
            nodes_num[i / frequency_of_verification] = i + 1;
        } else {
            match_node_by_id(db, node_type, i, &node);
        }
    }

    fwrite(&k, sizeof(uint64_t), 1, test_result);
    fwrite(nodes_num, sizeof(uint64_t), k, test_result);
    fwrite(time, sizeof(uint64_t), k, test_result);
    return 0;
}

int test_update(struct database* db, 
                    FILE* test_result, 
                    size_t count, 
                    size_t frequency_of_verification) {

    uint64_t k = count / frequency_of_verification;

    uint64_t nodes_num[k];
    uint64_t time[k];
    
    size_t node_id = 0;
    struct node_type node_type = generate_node_type(0);
    create_node_type(db, node_type);
    char* string = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

    size_t id_array[count];

    for (int i = 0; i < count; i++) {
        create_node(db, generate_node(node_id, node_type, string));
        id_array[i] = node_id;
        node_id++;
    }

    struct attribute new_attr;

    switch (node_type.attribute_types[0]) {
        case STRING:
            new_attr.value.as_string = "AAAN";
            break;
        case INT32:
            new_attr.value.as_int32 = rand() % 10000000;
            break;
        case FLOAT:
            new_attr.value.as_float = (float) rand();
            break;
        case BOOL:
            new_attr.value.as_float = rand() % 2;
            break;
    }

    for (int i = 0; i < count; i++) {
        if (i % frequency_of_verification == 0) {
            clock_t t1 = clock();
            update_node_by_id(db, node_type, id_array[i], 1, new_attr);
            clock_t t2 = clock();

            time[i / frequency_of_verification] = ((double)(t2 - t1) / CLOCKS_PER_SEC) * 1000;
            nodes_num[i / frequency_of_verification] = i + 1;
        } else {
            update_node_by_id(db, node_type, id_array[i], 1, new_attr);
        }
    }

    fwrite(&k, sizeof(uint64_t), 1, test_result);
    fwrite(nodes_num, sizeof(uint64_t), k, test_result);
    fwrite(time, sizeof(uint64_t), k, test_result);
    return 0;
}


int run_tests(char* db_file_path, char* tests_res_file_path) {
    struct database db;

    if (create_db(db_file_path, &db) != 0) {
        return 1;
    }

    FILE* tests_res_file = fopen(tests_res_file_path, "wb+");

    if (tests_res_file == NULL) {
        return 1;
    } 

    test_add(&db, tests_res_file, 10000, 100);

    // test_delete(&db, tests_res_file, 800, 10);

    // struct node_type node_type = generate_node_type(0);
    // create_node_type(&db, node_type);

    // test_add_and_delete(&db, node_type, tests_res_file, 500, 400, 4);

    // test_match(&db, tests_res_file, node_type, 800, 10);
    
    // test_update(&db, tests_res_file, 800, 10);

    destroy_db(&db);
    fclose(tests_res_file);
    return 0; 
}