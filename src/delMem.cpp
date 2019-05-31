#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <semaphore.h>

using namespace std;

int
main(void) {
  shm_unlink("evaluator");

  return EXIT_SUCCESS;
}
