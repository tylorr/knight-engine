#include "watch.h"

#include <stdio.h>

void WatchDirectory(void *arg)
{
    char *path = (char*)arg;
    printf("Path: %s", path);
}
