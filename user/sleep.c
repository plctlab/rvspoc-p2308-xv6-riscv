#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc < 2 || argc > 2){
    fprintf(2, "Usage: sleep time\n");
    exit(1);
  }

  if(sleep(atoi(argv[1])) < 0){
    fprintf(2, "sleep: cannot sleep for %s clock ticks\n", argv[1]);
  }

  exit(0);
}
