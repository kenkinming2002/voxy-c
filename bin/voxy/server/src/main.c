#include "application.h"

#include <stdlib.h>

int main(int argc, char *argv[])
{
  struct application application;
  if(application_init(&application, argc, argv) != 0)
    return EXIT_FAILURE;

  application_run(&application);
  application_fini(&application);
}

