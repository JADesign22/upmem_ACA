#include <stdint.h>
#include <stdio.h>
#include <mram.h>
#include <defs.h>

// Struktur zur Darstellung eines Körpers (float)
typedef struct {
    float mass;
    float x, y, z;
    float vx, vy, vz;
    float padding;  // Auffüllbyte für 8-Byte-Ausrichtung
} Body_f;

__mram_noinit Body_f mram_body;

int main() {
    // WRAM Kopie des Körpers
    Body_f local_body;

    // fill local body with some values
    local_body.mass = 1.0f;
    local_body.x = 1.0f;
    local_body.y = 2.0f;
    local_body.z = 1.0f;
    local_body.vx = 1.0f;
    local_body.vy = 1.0f;
    local_body.vz = 1.0f;

    // Schreibe den veränderten Körper zurück ins MRAM
    mram_write(&local_body, &mram_body, sizeof(Body_f));

    // print the x-coordinate of the body
    printf("x-coordinate of the body: %f\n", local_body.x);

    // Create new varibale for body and read form MRAM
    Body_f new_body;
    mram_read(&mram_body, &new_body, sizeof(Body_f));

    // print the y-coordinate of the body
    printf("y-coordinate of the body: %f\n", new_body.y);


    return 0;
}