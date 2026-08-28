#include "memory.h"

#include <stdlib.h>
#include <errno.h>

void *
sc_allocarray(size_t nmemb, size_t size) {
    if (size && nmemb > SIZE_MAX / size) {
        errno = ENOMEM;
        return NULL;
    }
    return malloc(nmemb * size);
}
