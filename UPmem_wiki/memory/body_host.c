#include <assert.h>
#include <dpu.h>
#include <dpu_log.h>
#include <stdio.h>

#ifndef DPU_BINARY
#define DPU_BINARY "./body_dpu"
#endif

// Struktur zur Darstellung eines Körpers (float)
typedef struct {
    float mass;
    float x, y, z;
    float vx, vy, vz;
    float padding;  // Auffüllbyte für 8-Byte-Ausrichtung
} Body_f;

int main(void) {
    // Beispielkörper
    Body_f body = {1.0e24f, 0, 0, 0, 0, 0, 0, 0};

    // DPU-Initialisierung
    struct dpu_set_t dpu_set, dpu;
    DPU_ASSERT(dpu_alloc(1, NULL, &dpu_set));

    // DPU-Programm laden
    DPU_ASSERT(dpu_load(dpu_set, DPU_BINARY, NULL));

    // Ursprünglichen Körper in die DPU kopieren
    DPU_FOREACH(dpu_set, dpu) {
        DPU_ASSERT(dpu_copy_to(dpu, "mram_body", 0, &body, sizeof(Body_f)));
    }

    // DPU-Programm ausführen
    DPU_ASSERT(dpu_launch(dpu_set, DPU_SYNCHRONOUS));

    // DPU-Logs anzeigen (optional)
    DPU_FOREACH(dpu_set, dpu) {
        DPU_ASSERT(dpu_log_read(dpu, stdout));
    }

    // Veränderten Körper von der DPU zurückkopieren
    Body_f modified_body;
    DPU_FOREACH(dpu_set, dpu) {
        DPU_ASSERT(dpu_copy_from(dpu, "mram_body", 0, &modified_body, sizeof(Body_f)));
    }

    // Ergebnisse ausgeben
    printf("Original Body:\n");
    printf("Mass: %f, x: %f, y: %f, z: %f\n", body.mass, body.x, body.y, body.z);
    printf("vx: %f, vy: %f, vz: %f\n", body.vx, body.vy, body.vz);

    printf("Modified Body:\n");
    printf("Mass: %f, x: %f, y: %f, z: %f\n", modified_body.mass, modified_body.x, modified_body.y, modified_body.z);
    printf("vx: %f, vy: %f, vz: %f\n", modified_body.vx, modified_body.vy, modified_body.vz);

    // DPU freigeben
    DPU_ASSERT(dpu_free(dpu_set));

    return 0;
}