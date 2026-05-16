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
    if (!mem) return -1;

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

// funciones auxiliares archivos

static int find_pcb_slot(FILE* mem, int process_id) {
    for (int i = 0; i < 32; i++) {
        fseek(mem, i * 256, SEEK_SET);
        uint8_t estado;
        fread(&estado, 1, 1, mem);
        if (estado != 0x01) continue;
 
        fseek(mem, i * 256 + 15, SEEK_SET);
        uint8_t pid;
        fread(&pid, 1, 1, mem);
        if ((int)pid == process_id) return i;
    }
    return -1;
}

static long file_entry_offset(int pcb_slot, int j) {
    return (long)pcb_slot * 256 + 16 + (long)j * 24;
}

static uint8_t read_file_entry(FILE* mem, int pcb_slot, int j,
                                char name_out[15], uint64_t* size_out, uint32_t* vaddr_out) {
    fseek(mem, file_entry_offset(pcb_slot, j), SEEK_SET);
    uint8_t validez;
    fread(&validez, 1, 1, mem);
 
    char name[15] = {0};
    fread(name, 1, 14, mem);
    name[14] = '\0';
 
    uint8_t size_bytes[5];
    fread(size_bytes, 1, 5, mem);
    uint64_t sz = 0;
    for (int k = 0; k < 5; k++) sz |= ((uint64_t)size_bytes[k] << (8 * k));
 
    uint32_t va;
    fread(&va, 4, 1, mem);
 
    if (name_out) { memcpy(name_out, name, 15); }
    if (size_out)  *size_out  = sz;
    if (vaddr_out) *vaddr_out = va;
    return validez;
}

static void write_file_entry(FILE* mem, int pcb_slot, int j,
                              uint8_t validez, const char* name,
                              uint64_t file_size, uint32_t vaddr) {
    fseek(mem, file_entry_offset(pcb_slot, j), SEEK_SET);
    fwrite(&validez, 1, 1, mem);
 
    char name_buf[14] = {0};
    if (name) strncpy(name_buf, name, 14);
    fwrite(name_buf, 1, 14, mem);
 
    uint8_t size_bytes[5];
    for (int k = 0; k < 5; k++) size_bytes[k] = (uint8_t)((file_size >> (8 * k)) & 0xFF);
    fwrite(size_bytes, 1, 5, mem);
 
    fwrite(&vaddr, 4, 1, mem);
}

static uint8_t read_ipt_entry(FILE* mem, uint16_t pfn,
                               uint16_t* pid_out, uint16_t* vpn_out) {
    fseek(mem, PCB_SIZE + (long)pfn * 3, SEEK_SET);
    uint8_t entry[3];
    fread(entry, 1, 3, mem);
    uint32_t raw = entry[0] | ((uint32_t)entry[1] << 8) | ((uint32_t)entry[2] << 16);
 
    uint8_t  validez = raw & 0x1;
    uint16_t pid     = (raw >> 1)  & 0x3FF;
    uint16_t vpn     = (raw >> 11) & 0xFFF;
 
    if (pid_out) *pid_out = pid;
    if (vpn_out) *vpn_out = vpn;
    return validez;
}

static void write_ipt_entry(FILE* mem, uint16_t pfn, uint8_t validez,
                             uint16_t pid, uint16_t vpn) {
    uint32_t raw = ((uint32_t)(validez & 0x1))
                 | ((uint32_t)(pid & 0x3FF) << 1)
                 | ((uint32_t)(vpn & 0xFFF) << 11);
    uint8_t entry[3] = { raw & 0xFF, (raw >> 8) & 0xFF, (raw >> 16) & 0xFF };
    fseek(mem, PCB_SIZE + (long)pfn * 3, SEEK_SET);
    fwrite(entry, 1, 3, mem);
}

static void bitmap_set(FILE* mem, uint16_t pfn, uint8_t val) {
    long off = PCB_SIZE + IPT_SIZE + pfn / 8;
    fseek(mem, off, SEEK_SET);
    uint8_t byte;
    fread(&byte, 1, 1, mem);
    if (val) byte |=  (1 << (pfn % 8));
    else     byte &= ~(1 << (pfn % 8));
    fseek(mem, off, SEEK_SET);
    fwrite(&byte, 1, 1, mem);
}

static uint16_t find_free_pfn(FILE* mem) {
    fseek(mem, PCB_SIZE + IPT_SIZE, SEEK_SET);
    for (int i = 0; i < BITMAP_SIZE; i++) {
        uint8_t byte;
        fread(&byte, 1, 1, mem);
        if (byte == 0xFF) continue;
        for (int bit = 0; bit < 8; bit++) {
            if (!((byte >> bit) & 1)) return (uint16_t)(i * 8 + bit);
        }
    }
    return 0xFFFF;
}

static long physical_addr(uint16_t pfn, uint16_t offset) {
    return DATA_OFFSET + ((long)pfn << 15) + offset;
}
 
static uint16_t vpn_to_pfn(FILE* mem, int pid, uint16_t vpn) {
    for (int pfn = 0; pfn < 65536; pfn++) {
        uint16_t p, v;
        uint8_t valid = read_ipt_entry(mem, (uint16_t)pfn, &p, &v);
        if (valid && (int)p == pid && v == vpn) return (uint16_t)pfn;
    }
    return 0xFFFF;
}

static uint32_t next_free_vaddr(FILE* mem, int pcb_slot) {
    uint32_t max_end = 0;
    for (int j = 0; j < 10; j++) {
        uint64_t sz; uint32_t va;
        uint8_t v = read_file_entry(mem, pcb_slot, j, NULL, &sz, &va);
        if (!v) continue;
        uint32_t vpn    = (va >> 15) & 0xFFF;
        uint32_t offset = va & 0x7FFF;
        uint32_t start  = (vpn << 15) | offset;
        uint32_t end    = start + (uint32_t)sz;
        if (end > max_end) max_end = end;
    }
    uint32_t vpn    = (max_end >> 15) & 0xFFF;
    uint32_t offset = max_end & 0x7FFF;
    return (vpn << 15) | offset;
}

// funciones para archivos

homerFile* open_file(int process_id, char* file_name, char mode) {
    FILE* mem = fopen(path, "rb");
    if (!mem) return NULL;
 
    int pcb_slot = find_pcb_slot(mem, process_id);
    if (pcb_slot == -1) { fclose(mem); return NULL; }
 
    if (mode == 'r') {
        for (int j = 0; j < 10; j++) {
            char name[15];
            uint64_t sz; uint32_t va;
            uint8_t v = read_file_entry(mem, pcb_slot, j, name, &sz, &va);
            if (!v) continue;
            if (strcmp(name, file_name) == 0) {
                fclose(mem);
                homerFile* hf = malloc(sizeof(homerFile));
                hf->process_id = process_id;
                strncpy(hf->file_name, file_name, 14);
                hf->file_name[14] = '\0';
                hf->file_size = sz;
                hf->vaddr     = va;
                hf->mode      = 'r';
                return hf;
            }
        }
        fclose(mem);
        return NULL;
    } else if (mode == 'w') {
        for (int j = 0; j < 10; j++) {
            char name[15];
            uint8_t v = read_file_entry(mem, pcb_slot, j, name, NULL, NULL);
            if (v && strcmp(name, file_name) == 0) {
                fclose(mem);
                return NULL;
            }
        }
        fclose(mem);
        homerFile* hf = malloc(sizeof(homerFile));
        hf->process_id = process_id;
        strncpy(hf->file_name, file_name, 14);
        hf->file_name[14] = '\0';
        hf->file_size = 0;
        hf->vaddr     = 0;
        hf->mode      = 'w';
        return hf;
    }
 
    fclose(mem);
    return NULL;
}
 
int read_file(homerFile* file_desc, char* dest) {
    if (!file_desc || file_desc->mode != 'r') return -1;
 
    FILE* mem = fopen(path, "rb");
    if (!mem) return -1;
 
    FILE* out = fopen(dest, "wb");
    if (!out) { fclose(mem); return -1; }
 
    int pid         = file_desc->process_id;
    uint32_t vaddr  = file_desc->vaddr;
    uint64_t remain = file_desc->file_size;
    int bytes_read  = 0;
 
    uint32_t vpn    = (vaddr >> 15) & 0xFFF;
    uint32_t offset = vaddr & 0x7FFF;
 
    while (remain > 0) {
        uint16_t pfn = vpn_to_pfn(mem, pid, (uint16_t)vpn);
        if (pfn == 0xFFFF) break;
 
        uint32_t bytes_in_page = 32768 - offset;
        uint32_t to_read = (remain < bytes_in_page) ? (uint32_t)remain : bytes_in_page;
 
        fseek(mem, physical_addr(pfn, (uint16_t)offset), SEEK_SET);
        uint8_t buf[32768];
        fread(buf, 1, to_read, mem);
        fwrite(buf, 1, to_read, out);
 
        bytes_read += (int)to_read;
        remain     -= to_read;
        vpn++;
        offset = 0;
    }
 
    fclose(out);
    fclose(mem);
    return bytes_read;
}
 
int write_file(homerFile* file_desc, char* src) {
    if (!file_desc || file_desc->mode != 'w') return -1;
 
    FILE* in = fopen(src, "rb");
    if (!in) return -1;
 
    fseek(in, 0, SEEK_END);
    long src_size = ftell(in);
    rewind(in);
 
    FILE* mem = fopen(path, "r+b");
    if (!mem) { fclose(in); return -1; }
 
    int pid = file_desc->process_id;
    int pcb_slot = find_pcb_slot(mem, pid);
    if (pcb_slot == -1) { fclose(mem); fclose(in); return -1; }
 
    int ft_slot = -1;
    for (int j = 0; j < 10; j++) {
        uint8_t v = read_file_entry(mem, pcb_slot, j, NULL, NULL, NULL);
        if (!v) { ft_slot = j; break; }
    }
    if (ft_slot == -1) { fclose(mem); fclose(in); return -1; }
 
    uint32_t start_vaddr = next_free_vaddr(mem, pcb_slot);
    uint32_t vpn    = (start_vaddr >> 15) & 0xFFF;
    uint32_t offset = start_vaddr & 0x7FFF;
    uint32_t MAX_VPN = 0x1000;
 
    int bytes_written = 0;
    long remain = src_size;
    uint32_t file_vaddr = (vpn << 15) | offset;
 
    while (remain > 0 && vpn < MAX_VPN) {
        uint16_t pfn = vpn_to_pfn(mem, pid, (uint16_t)vpn);
        if (pfn == 0xFFFF) {
            pfn = find_free_pfn(mem);
            if (pfn == 0xFFFF) break;
            bitmap_set(mem, pfn, 1);
            write_ipt_entry(mem, pfn, 1, (uint16_t)pid, (uint16_t)vpn);
        }
 
        uint32_t space_in_page = 32768 - offset;
        uint32_t to_write = (remain < (long)space_in_page) ? (uint32_t)remain : space_in_page;
 
        uint8_t buf[32768];
        fread(buf, 1, to_write, in);
        fseek(mem, physical_addr(pfn, (uint16_t)offset), SEEK_SET);
        fwrite(buf, 1, to_write, mem);
 
        bytes_written += (int)to_write;
        remain        -= (long)to_write;
        vpn++;
        offset = 0;
    }
    fclose(in);
 
    file_desc->file_size = (uint64_t)bytes_written;
    file_desc->vaddr     = file_vaddr;
 
    write_file_entry(mem, pcb_slot, ft_slot, 0x01, file_desc->file_name,
                     (uint64_t)bytes_written, file_vaddr);
 
    fclose(mem);
    return bytes_written;
}
 
void delete_file(int process_id, char* file_name) {
    FILE* mem = fopen(path, "r+b");
    if (!mem) return;
 
    int pcb_slot = find_pcb_slot(mem, process_id);
    if (pcb_slot == -1) { fclose(mem); return; }
 
    int ft_slot = -1;
    uint64_t file_size = 0;
    uint32_t vaddr = 0;
    for (int j = 0; j < 10; j++) {
        char name[15];
        uint8_t v = read_file_entry(mem, pcb_slot, j, name, &file_size, &vaddr);
        if (v && strcmp(name, file_name) == 0) { ft_slot = j; break; }
    }
    if (ft_slot == -1) { fclose(mem); return; }
 
    uint32_t vpn_start = (vaddr >> 15) & 0xFFF;
    uint32_t off_start = vaddr & 0x7FFF;
    uint64_t end_byte  = (uint64_t)(vpn_start << 15) + off_start + file_size;
    uint32_t vpn_end   = (uint32_t)((end_byte - 1) >> 15);
 
    for (uint32_t vpn = vpn_start; vpn <= vpn_end; vpn++) {
        uint16_t pfn = vpn_to_pfn(mem, process_id, (uint16_t)vpn);
        if (pfn == 0xFFFF) continue;
        write_ipt_entry(mem, pfn, 0, 0, 0);
        bitmap_set(mem, pfn, 0);
    }
 
    uint8_t zero_entry[24] = {0};
    fseek(mem, file_entry_offset(pcb_slot, ft_slot), SEEK_SET);
    fwrite(zero_entry, 1, 24, mem);
 
    fclose(mem);
}
 
void close_file(homerFile* file_desc) {
    free(file_desc);
}