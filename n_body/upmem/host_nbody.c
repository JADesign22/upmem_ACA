/*
 * Project: UPmem ACA, N-body simulation
 * Course: Advanced Computer Architecture SS24, University of Heidelberg
 * Authors: Albrecht, Burr, Jahnel
 */

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

#define NOF_BODIES 4

// Structure to represent a body (float)
typedef struct {
    float mass;
    float x, y, z;
    float vx, vy, vz;
    float padding; // Padding byte for 8-byte alignment
} Body_f;

void write_results_to_file(const char *filename, Body_f *bodies, size_t num_bodies);

// Method to create dummy data of n bodies. The dummy data is based on n, not on random values
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

    // Print all bodies
    for (size_t i = 0; i < NOF_BODIES; ++i) {
        printf("Body %zu: mass=%f, x=%f, y=%f, z=%f, vx=%f, vy=%f, vz=%f\n", i, bodies[i].mass, bodies[i].x, bodies[i].y, bodies[i].z, bodies[i].vx, bodies[i].vy, bodies[i].vz);
    }
    
    // DPU initialization
    struct dpu_set_t dpu_set, dpu;
    DPU_ASSERT(dpu_alloc(1, NULL, &dpu_set));
    printf("DPU allocated\n");

    // Load DPU program
    DPU_ASSERT(dpu_load(dpu_set, DPU_BINARY, NULL));
    printf("DPU loaded\n");

    // Copy data to the DPU
    DPU_FOREACH(dpu_set, dpu) {
        DPU_ASSERT(dpu_copy_to(dpu, "mram_bodies", 0, bodies, sizeof(Body_f) * NOF_BODIES));
    }
    printf("Data copied to DPU\n");

    // Launch DPU program
    DPU_ASSERT(dpu_launch(dpu_set, DPU_SYNCHRONOUS));
    printf("DPU launched\n");

    // Display DPU logs (optional)
    DPU_FOREACH(dpu_set, dpu) {
        DPU_ASSERT(dpu_log_read(dpu, stdout));
    }

    // Copy data back from the DPU
    DPU_FOREACH(dpu_set, dpu) {
        DPU_ASSERT(dpu_copy_from(dpu, "mram_bodies", 0, bodies, sizeof(Body_f) * NOF_BODIES));
    }

    // Write results to a text file
    write_results_to_file("nbody_results.csv", bodies, NOF_BODIES);

    // Free DPU
    DPU_ASSERT(dpu_free(dpu_set));

    return 0;
}

void write_results_to_file(const char *filename, Body_f *bodies, size_t num_bodies) {
    FILE *outfile = fopen(filename, "w");
    if (outfile == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Write CSV header
    fprintf(outfile, "step;body_id;x;y;z\n");

    // Write data
    for (size_t i = 0; i < num_bodies; ++i) {
        fprintf(outfile, "%d;%zu;%f;%f;%f\n", 0, i, bodies[i].x, bodies[i].y, bodies[i].z);
    }

    fclose(outfile);
}