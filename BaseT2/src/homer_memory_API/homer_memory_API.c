#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include <string.h> //para strcmp
#include <stdbool.h> // bool, true, false
#include <stdint.h>
#include "homer_memory_API.h"

char* path;

// funciones generales

void mount_memory(char* memory_path) {
    path = memory_path;
}

void list_processes() {
    FILE* mem = fopen(path, "rb");
    if (!mem) return;

    for (int i = 0; i < 32; i++) {
        fseek(mem, i * 256, SEEK_SET);

        uint8_t estado;
        fread(&estado, 1, 1, mem);

        if (estado == 0x01) {
            char name[15] = {0};
            fread(name, 1, 14, mem);
            name[14] = '\0';

            uint8_t pid;
            fread(&pid, 1, 1, mem);

            printf("%d %s\n", pid, name);
        }
    }

    fclose(mem);
}

int processes_slots() {
    FILE* mem = fopen(path, "rb");
    if (!mem) return;

    int entradas_libres = 0;

    for (int i = 0; i < 32; i++) {
        fseek(mem, i * 256, SEEK_SET);

        uint8_t estado;
        fread(&estado, 1, 1, mem);

        if (estado == 0x00) entradas_libres++;
    }
    return entradas_libres;
}

void list_files(int process_id) {
    FILE* mem = fopen(path, "rb");
    if (!mem) return;

    for (int i = 0; i < 32; i++) {
        fseek(mem, i * 256, SEEK_SET);

        uint8_t estado;
        fread(&estado, 1, 1, mem);
        if (estado != 0x01) continue;

        char process_name[15] = {0};
        fread(process_name, 1, 14, mem);

        uint8_t pid;
        fread(&pid, 1, 1, mem);

        if ((int)pid != process_id) continue;

        for (int j = 0; j < 10; j++) {
            uint8_t validez;
            fread(&validez, 1, 1, mem);

            char archive_name[15] = {0};
            fread(archive_name, 1, 14, mem);
            archive_name[14] = '\0';

            uint8_t size_bytes[5];
            fread(size_bytes, 1, 5, mem);

            uint64_t file_size = 0;
            for (int k = 0; k < 5; k++) {
                file_size |= ((uint64_t)size_bytes[k] << (8 * k));
            }

            uint32_t vaddr;
            fread(&vaddr, 4, 1, mem);

            if (validez == 0x01) {
                uint16_t vpn = (vaddr >> 15) & 0xFFF;
                printf("%x %lu %x %s\n", vpn, file_size, vaddr, archive_name);
            }
        }
        break;
    }
    fclose(mem);
}

void frame_bitmap_status() {
    FILE* mem = fopen(path, "rb");
    if (!mem) return;

    fseek(mem, PCB_SIZE + IPT_SIZE, SEEK_SET);

    int usados = 0;
    int libres = 0;

    for (int i = 0; i < BITMAP_SIZE; i++) {
        uint8_t byte;
        fread(&byte, 1, 1, mem);

        for (int bit = 0; bit < 8; bit++) {
            if (byte & (1 << bit)) usados++;
            else libres++;
        }
    }
    printf("USADOS: %d LIBRES: %d\n", usados, libres);

    fclose(mem);
}

int format_memory(char* memory_path) {
    FILE* f = fopen(memory_path, "wb");
    if (!f) return -1;

    long total = PCB_SIZE + IPT_SIZE + BITMAP_SIZE + (2LL * 1024 * 1024 * 1024);

    if (fseek(f, total - 1, SEEK_SET) != 0) {
        fclose(f);
        return -1;
    }
    fputc(0, f);

    fclose(f);
    return 0;
}

// funciones procesos

int start_process(int process_id, char* process_name) {
    FILE* mem = fopen(path, "r+b");
    if (!mem) return -1;

    for (int i = 0; i < 32; i++) {
        fseek(mem, i * 256, SEEK_SET);

        uint8_t estado;
        fread(&estado, 1, 1, mem);
        if (estado != 0x01) continue;

        fseek(mem, i * 256 + 15, SEEK_SET);
        uint8_t pid;
        fread(&pid, 1, 1, mem);

        if ((int)pid == process_id) {
            fclose(mem);
            return -1;
        }
    }

    int slot = -1;
    for (int i = 0; i < 32; i++) {
        fseek(mem, i * 256, SEEK_SET);

        uint8_t estado;
        fread(&estado, 1, 1, mem);

        if (estado == 0x00) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        fclose(mem);
        return -1;
    }

    fseek(mem, slot * 256, SEEK_SET);

    uint8_t estado = 0x01;
    fwrite(&estado, 1, 1, mem);

    char name[14] = {0};
    strncpy(name, process_name, 14);
    fwrite(name, 1, 14, mem);

    uint8_t pid = (uint8_t)process_id;
    fwrite(&pid, 1, 1, mem);

    uint8_t zeros[240] = {0};
    fwrite(zeros, 1, 240, mem);

    fclose(mem);
    return 0;
}

int finish_process(int process_id) {
    FILE* mem = fopen(path, "r+b");
    if (!mem) return -1;

    int slot = -1;
    for (int i = 0; i < 32; i++) {
        fseek(mem, i * 256, SEEK_SET);

        uint8_t estado;
        fread(&estado, 1, 1, mem);
        if (estado != 0x01) continue;

        fseek(mem, i * 256 + 15, SEEK_SET);
        uint8_t pid;
        fread(&pid, 1, 1, mem);

        if ((int)pid == process_id) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        fclose(mem);
        return -1;
    }

    for (int pfn = 0; pfn < 65536; pfn++) {
        fseek(mem, PCB_SIZE + pfn * 3, SEEK_SET);

        uint8_t entry[3];
        fread(entry, 1, 3, mem);

        uint32_t raw = entry[0] | ((uint32_t)entry[1] << 8) | ((uint32_t)entry[2] << 16);

        uint8_t validez = raw & 0x1;
        uint16_t pid = (raw >> 1) & 0x3FF;

        if (validez == 1 && (int)pid == process_id) {
            fseek(mem, PCB_SIZE + pfn * 3, SEEK_SET);
            uint8_t zeros[3] = {0};
            fwrite(zeros, 1, 3, mem);

            long bitmap_offset = PCB_SIZE + IPT_SIZE + pfn / 8;
            fseek(mem, bitmap_offset, SEEK_SET);
            uint8_t byte;
            fread(&byte, 1, 1, mem);

            byte &= ~(1 << (pfn % 8));

            fseek(mem, bitmap_offset, SEEK_SET);
            fwrite(&byte, 1, 1, mem);
        }
    }

    fseek(mem, slot * 256, SEEK_SET);
    uint8_t zeros[256] = {0};
    fwrite(zeros, 1, 256, mem);

    fclose(mem);
    return 0;
}

int clear_all_processes() {
    FILE* mem = fopen(path, "rb");
    if (!mem) return 0;

    int terminados = 0;

    uint8_t pids[32];
    int count = 0;

    for (int i = 0; i < 32; i++) {
        fseek(mem, i * 256, SEEK_SET);

        uint8_t estado;
        fread(&estado, 1, 1, mem);
        if (estado != 0x01) continue;

        fseek(mem, i * 256 + 15, SEEK_SET);
        uint8_t pid;
        fread(&pid, 1, 1, mem);

        pids[count++] = pid;
    }

    fclose(mem);

    for (int i = 0; i < count; i++) {
        if (finish_process((int)pids[i]) == 0) {
            terminados++;
        }
    }
    return terminados;
}

int file_table_slots(int process_id) {
    FILE* mem = fopen(path, "rb");
    if (!mem) return 0;

    int slot = -1;

    for (int i = 0; i < 32; i++) {

        fseek(mem, i * 256, SEEK_SET);
        uint8_t estado;
        fread(&estado, 1, 1, mem);

        if (estado != 0x01) continue;

        fseek(mem, i * 256 + 15, SEEK_SET);
        uint8_t pid;
        fread(&pid, 1, 1, mem);

        if ((int) pid == process_id) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        fclose(mem);
        return -1;
    }

    int contador = 0;

    for (int i = 0; i < 10; i++) {
        uint8_t estado_archivo;
        fread(&estado_archivo, 1, 1, mem);

        if (estado_archivo != 0x01) contador++;

        fseek(mem, 23, SEEK_CUR);
    }

    fclose(mem);
    return contador;
}

// funciones archivos