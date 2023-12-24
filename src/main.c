#include "../test/tests.h"

int main (int argc, char *argv[]) {
     if (argc < 3) {
          printf("Specify path to the database file and tests results file - lab1 <db file path> <tests results file>");
          return 1;
     }
     return run_tests(argv[1], argv[2]);
}