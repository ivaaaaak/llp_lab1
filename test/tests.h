#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include <time.h>
#include "generator.h"
#include "../include/query.h"

int run_tests(char* db_file_path, char* tests_res_file_path);

#endif