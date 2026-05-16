#pragma once
#include <stdint.h>
 
typedef struct homer_File {
    int      process_id;
    char     file_name[15];
    uint64_t file_size;
    uint32_t vaddr;
    char     mode;
} homerFile;