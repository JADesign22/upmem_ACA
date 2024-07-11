#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dpu.h>
#include <dpu_log.h>
#include <dpu_memory.h>
#include <assert.h>

#ifndef DPU_BINARY
#define DPU_BINARY "./dpu_nbody"
#endif

// Struktur zur Darstellung eines Körpers (float)
typedef struct {
    float mass;
    float x, y, z;
    float vx, vy, vz;
    float padding;  // Auffüllbyte für 8-Byte-Ausrichtung
} Body_f;

void write_results_to_file(const char *filename, Body_f *bodies, size_t num_bodies);

int main() {
    // Beispiel mit 3 Körpern (float)
    Body_f bodies[] = {
        {1.0e10f, 0, 0, 0, 0, 0, 0, 0},
        {1.0e10f, 1, 0, 0, 0, 1, 0, 0},
        {1.0e10f, 0, 0, 1, 0, 1, 0, 0},
        {1.0e10f, 0, 1, 0, -1, 0, 0, 0}
    };
    size_t num_bodies = sizeof(bodies) / sizeof(bodies[0]);

    
    // DPU-Initialisierung
    struct dpu_set_t dpu_set, dpu;
    DPU_ASSERT(dpu_alloc(1, NULL, &dpu_set));
    printf("DPU allocated\n");

    // DPU-Programm laden
    DPU_ASSERT(dpu_load(dpu_set, DPU_BINARY, NULL));
    printf("DPU loaded\n");

    // Daten in die DPU kopieren
    DPU_FOREACH(dpu_set, dpu) {
        DPU_ASSERT(dpu_copy_to(dpu, "mram_bodies", 0, bodies, sizeof(Body_f) * num_bodies));
    }
    printf("Data copied to DPU\n");

    // DPU-Programm ausführen
    DPU_ASSERT(dpu_launch(dpu_set, DPU_SYNCHRONOUS));
    printf("DPU launched\n");

    // DPU-Logs anzeigen (optional)
    DPU_FOREACH(dpu_set, dpu) {
        DPU_ASSERT(dpu_log_read(dpu, stdout));
    }

    // Daten von der DPU zurückkopieren
    DPU_FOREACH(dpu_set, dpu) {
        DPU_ASSERT(dpu_copy_from(dpu, "mram_bodies", 0, bodies, sizeof(Body_f) * num_bodies));
    }

    // Ergebnisse in eine Textdatei schreiben
    write_results_to_file("nbody_results.csv", bodies, num_bodies);

    // DPU freigeben
    DPU_ASSERT(dpu_free(dpu_set));

    return 0;
}

void write_results_to_file(const char *filename, Body_f *bodies, size_t num_bodies) {
    FILE *outfile = fopen(filename, "w");
    if (outfile == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // CSV-Header schreiben
    fprintf(outfile, "step;body_id;x;y;z\n");

    // Daten schreiben

    for (size_t i = 0; i < num_bodies; ++i) {
        fprintf(outfile, "%d;%zu;%f;%f;%f\n", 0, i, bodies[i].x, bodies[i].y, bodies[i].z);
    }
    

    fclose(outfile);
}