#pragma once
#include "types.h"

#define MAX_BOOKS 32
#define CHUNK_SIZE 1024

bool storageInit();
int storageGetBooks(Book* out, int maxCount);
int storageReadBookChunk(const Book* book, int offset, char* buf, int bufSize);
void safeLog(const char* format, ...);
