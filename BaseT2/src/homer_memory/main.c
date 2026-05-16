#include "../homer_memory_API/homer_memory_API.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uso: ./homer_memory <archivo.bin>\n");
        return 1;
    }
 
    // format_memory y mount_memory
    format_memory(argv[1]);
    mount_memory(argv[1]);
 
    // start_process
    start_process(1, "atenea_guau");
    start_process(2, "gia_miau");
    start_process(3, "zeus_pio");
    start_process(4, "luna_muu");
 
    // list_processes
    printf("=== Procesos activos ===\n");
    list_processes();

    printf("\n");
 
    // processes_slots
    printf("PCB slots libres: %d\n\n", processes_slots());
 
    // file_table_slots
    printf("File table slots pid=1: %d\n", file_table_slots(1));
 
    // Crear archivos locales
    FILE* f;
    f = fopen("patrona.txt", "w");
    fprintf(f, "Atenea es la patrona de la casa.\n");
    fclose(f);
 
    f = fopen("sofa.txt", "w");
    fprintf(f, "Gia duerme todo el día en el sofá\n");
    fclose(f);
 
    f = fopen("gusano.txt", "w");
    fprintf(f, "Zeus encuentra un gusano.\n");
    fclose(f);
 
    f = fopen("duerme.txt", "w");
    fprintf(f, "Luna duerme todo el día.\n");
    fclose(f);
 
    homerFile* hf;

    hf = open_file(1, "patrona.txt", 'w');
    write_file(hf, "patrona.txt");
    close_file(hf);
 
    hf = open_file(2, "sofa.txt", 'w');
    write_file(hf, "sofa.txt");
    close_file(hf);
 
    hf = open_file(3, "gusano.txt", 'w');
    write_file(hf, "gusano.txt");
    close_file(hf);
 
    hf = open_file(4, "duerme.txt", 'w');
    write_file(hf, "duerme.txt");
    close_file(hf);
 
    // list_files
    printf("\n--- Archivos pid=1 (atenea_guau) ---\n");
    list_files(1);
    printf("\n--- Archivos pid=2 (gia_miau) ---\n");
    list_files(2);
    printf("\n--- Archivos pid=3 (zeus_pio) ---\n");
    list_files(3);
    printf("\n--- Archivos pid=4 (luna_muu) ---\n");
    list_files(4);

    printf("\n");

    printf("File table slots pid=1: %d\n", file_table_slots(1));
 
    // frame_bitmap_status
    printf("\n--- Bitmap tras escrituras ---\n");
    frame_bitmap_status();
 
    // Leer un archivo
    printf("\n--- Leyendo patrona.txt de atena_guau ---\n");
    hf = open_file(1, "patrona.txt", 'r');
    read_file(hf, "patrona_copia.txt");
    close_file(hf);
    f = fopen("patrona_copia.txt", "r");
    char buf[256];
    while (fgets(buf, sizeof(buf), f)) printf("  %s", buf);
    fclose(f);
 
    // delete_file
    printf("\n--- Borrando sofa.txt de gia_miau ---\n");
    delete_file(2, "sofa.txt");
    printf("\n--- Archivos pid=2 (gia_miau) --- <--- Después del delete\n\n\n");
    list_files(2);

    printf("\n--- Bitmap tras eliminación de archivo ---\n");
    frame_bitmap_status();
 
    // finish_process
    printf("\n--- Terminando proceso luna_muu (pid=4) ---\n\n");
    finish_process(4);

    printf("=== Procesos activos ===\n");
    list_processes();

    printf("\n");
  
    printf("\n--- Bitmap tras eliminación de proceso ---\n");
    frame_bitmap_status();
 
    // clear_all_processes
    printf("\n=== Cerrando todos los procesos ===\n");
    printf("Procesos cerrados: %d\n\n", clear_all_processes());
    list_processes();

    printf("\n--- Bitmap tras eliminación de todos los procesos ---\n");
    frame_bitmap_status();

    printf("\n");

    printf("PCB slots libres: %d\n", processes_slots());
 
    return 0;
}