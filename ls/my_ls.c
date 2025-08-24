// Explained in: https://x.com/zuhaitz_dev/status/1957384265403892003

#include <stdio.h>
#include <dirent.h> // For opendir, readdir, closedir
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv)
{
  // For this example we are gonna use the current directory.
  const char *dir_path = ".";

  DIR *d;

  struct dirent *dir_entry;

  // SUCCESS = Pointer to a DIR stream
  // FAIL    = NULL
  d = opendir(dir_path);

  if (NULL == d)
  {
    fprintf(stderr,
            "Error: Could not open directory '%s': %s\n",
            dir_path,
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  printf("Listing contents of: %s\n", dir_path);
  printf("-----------------------\n");

  while ((NULL != (dir_entry = readdir(d))))
  {
    printf("%s\n", dir_entry->d_name);
  }

  if (-1 == closedir(d))
  {
    fprintf(stderr,
            "Error:Could not close directory '%s': %s\n",
            dir_path,
            strerror(errno));
  }

  return EXIT_SUCCESS;
}