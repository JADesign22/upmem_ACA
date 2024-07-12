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

#define NOF_BODIES 64

// Struktur zur Darstellung eines Körpers (float)
typedef struct {
    float mass;
    float x, y, z;
    float vx, vy, vz;
    float padding;  // Auffüllbyte für 8-Byte-Ausrichtung
} Body_f;

void write_results_to_file(const char *filename, Body_f *bodies, size_t num_bodies);

// methode to create dummy data of n bodies. The dummy data is based on n not on random values
void create_dummy_data(Body_f *bodies, size_t num_bodies) {
    for (size_t i = 0; i < num_bodies; ++i) {
        bodies[i].mass = 1.0e10f;
        bodies[i].x = num_bodies % (i + 1);
        bodies[i].y = num_bodies % (i + 2);
        bodies[i].z = num_bodies % (i + 3);
        bodies[i].vx = num_bodies % (i + 1);
        bodies[i].vy = num_bodies % (i + 2);
        bodies[i].vz = num_bodies % (i + 3);
    }
}

int main() {

    Body_f bodies[NOF_BODIES];
    create_dummy_data(bodies, NOF_BODIES);

    //print all bodies
    for (size_t i = 0; i < NOF_BODIES; ++i) {
        printf("Body %zu: mass=%f, x=%f, y=%f, z=%f, vx=%f, vy=%f, vz=%f\n", i, bodies[i].mass, bodies[i].x, bodies[i].y, bodies[i].z, bodies[i].vx, bodies[i].vy, bodies[i].vz);
    }
    
    // DPU-Initialisierung
    struct dpu_set_t dpu_set, dpu;
    DPU_ASSERT(dpu_alloc(1, NULL, &dpu_set));
    printf("DPU allocated\n");

    // DPU-Programm laden
    DPU_ASSERT(dpu_load(dpu_set, DPU_BINARY, NULL));
    printf("DPU loaded\n");

    // Daten in die DPU kopieren
    DPU_FOREACH(dpu_set, dpu) {
        DPU_ASSERT(dpu_copy_to(dpu, "mram_bodies", 0, bodies, sizeof(Body_f) * NOF_BODIES));
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
        DPU_ASSERT(dpu_copy_from(dpu, "mram_bodies", 0, bodies, sizeof(Body_f) * NOF_BODIES));
    }

    // Ergebnisse in eine Textdatei schreiben
    write_results_to_file("nbody_results.csv", bodies, NOF_BODIES);

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