#pragma once
#include "types.h"

#define MAX_BOOKS 32

bool storageInit();
int storageGetBooks(Book* out, int maxCount);
