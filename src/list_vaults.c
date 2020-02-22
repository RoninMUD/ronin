/*
**  File: list_vaults.c                                     Part of DIKUMUD
**  Will list the vaults a player has access to without causing machine lag.
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VAULTS 50

#define UPPER(c)    (((c) >= 'a' && (c) <= 'z') ? ((c) + ('A' - 'a')) : (c))
#define CAP(string) (*(string) = UPPER(*(string)), string)

int main(int argc, char **argv) {
  char name[255], buf[4096];
  FILE *fd;

  if (!argv[1]) {
    printf("Unable to list access to vaults.\n\r");

    exit(0);
  }

  snprintf(name, sizeof(name), "%s", argv[1]);

  remove("vault/access.list");

  snprintf(buf, sizeof(buf), "grep -lx %s vault/*.name > vault/access.list", name);

  system(buf);

  printf("You have access to the following vaults...\n\r\n\r");

  snprintf(buf, sizeof(buf), "vault/%s.vault", name);

  if ((fd = fopen(buf, "r"))) {
    printf("%s\n\r", CAP(name));

    fclose(fd);
  }

  if (!(fd = fopen("vault/access.list", "r"))) {
    printf("Unable to list access to further vaults.\n\r");

    exit(0);
  }

  memset(name, 0, sizeof(name));

  for (int i = 0; !feof(fd) && (i < MAX_VAULTS); i++) {
    if (fscanf(fd, "vault/%[a-z].name\n", name)) {
      printf("%s\n\r", CAP(name));
    }
  }

  fclose(fd);

  exit(0);
}
