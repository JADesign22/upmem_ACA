/*
 * Project: UPmem ACA, N-body simulation
 * Course: Advanced Computer Architecture SS24, University of Heidelberg
 * Authors: Albrecht, Burr, Jahnel
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <alloc.h>
#include <mram.h>
#include <barrier.h>
#include <defs.h>

#define NOF_BODIES 4
#define NOF_STEPS 4

// Structure to represent a body (float)
typedef struct {
    float mass;
    float x, y, z;
    float vx, vy, vz;
    float padding; // Padding byte for 8-byte alignment
} Body_f;

//Force vectors
float fx[NOF_BODIES], fy[NOF_BODIES], fz[NOF_BODIES];

// Force combinations for better load balancing
int force_combinations[NOF_BODIES * (NOF_BODIES - 1) / 2][2];

// MRAM and WRAM data
__mram_noinit Body_f mram_bodies[NOF_BODIES];
Body_f local_bodies[NOF_BODIES];

// Synchronization barrier
BARRIER_INIT(my_barrier, NR_TASKLETS);

// Math methods
float fabsf(float x);
float sqrtf(float x);
float fast_sqrtf(float x);

// Gravitational constant and time step
#define G 6.67430e-11f
#define DT 1e-3f

// Calculate the force between two bodies
void computeForce_f(const Body_f *a, const Body_f *b, float *fx, float *fy, float *fz) {
    float dx = b->x - a->x;
    float dy = b->y - a->y;
    float dz = b->z - a->z;
    float dist = fast_sqrtf(dx * dx + dy * dy + dz * dz);
    float dist3 = dist * dist * dist;
    float force = (G * a->mass * b->mass) / dist3;
    *fx = force * dx;
    *fy = force * dy;
    *fz = force * dz;
}

int main() {
    printf("Tasklet %d: Enter Main\n", me());

    // Copy data from MRAM to WRAM
    if (me() == 0) {
        mram_read(mram_bodies, local_bodies, sizeof(local_bodies));
        printf("Tasklet %d: Data copied from MRAM to WRAM\n", me());

        // Calculate the number of possible force combinations
        int row = 0;
        for (int i = 0; i < NOF_BODIES; ++i) {
            for (int j = i + 1; j < NOF_BODIES; ++j) {
                force_combinations[row][0] = i;
                force_combinations[row][1] = j;
                row++;
            }
        }
        printf("Number of possible force combinations: %d\n", row);
    }

    // Calculation of forces and update of positions and velocities
    for (int step = 0; step < NOF_STEPS; ++step) {
        // Initialize forces
        if (me() == 0) {
            for (int i = 0; i < NOF_BODIES; ++i) {
                fx[i] = fy[i] = fz[i] = 0.0f;
            }
        }

        barrier_wait(&my_barrier);

        // Calculate forces based on force combinations
        int iteration_counter = 0;
        for (int row = me(); row < NOF_BODIES * (NOF_BODIES - 1) / 2; row += NR_TASKLETS) {
            int i = force_combinations[row][0];
            int j = force_combinations[row][1];

            float fx_ij, fy_ij, fz_ij;
            computeForce_f(&local_bodies[i], &local_bodies[j], &fx_ij, &fy_ij, &fz_ij);

            fx[i] += fx_ij;
            fy[i] += fy_ij;
            fz[i] += fz_ij;
            fx[j] -= fx_ij;
            fy[j] -= fy_ij;
            fz[j] -= fz_ij;

            iteration_counter++;
        }

        printf("Tasklet %d: Force calculation done with %d iterations\n", me(), iteration_counter);
        barrier_wait(&my_barrier);

        // Update positions and velocities
        for (int i = me(); i < NOF_BODIES; i += NR_TASKLETS) {
            float vx = fx[i] / local_bodies[i].mass * DT;
            float vy = fy[i] / local_bodies[i].mass * DT;
            float vz = fz[i] / local_bodies[i].mass * DT;
            
            local_bodies[i].vx += vx;
            local_bodies[i].vy += vy;
            local_bodies[i].vz += vz;
            local_bodies[i].x += vx * DT;
            local_bodies[i].y += vy * DT;
            local_bodies[i].z += vz * DT;
        }

        printf("Tasklet %d: Position update done\n", me());
        barrier_wait(&my_barrier);
    }

    // Copy data from WRAM back to MRAM
    if (me() == 0) {
        mram_write(local_bodies, mram_bodies, sizeof(local_bodies));
        printf("Tasklet %d: Data copied from WRAM to MRAM\n", me());
    }

    return 0;
}

// Math methods
float fabsf(float x) {
    return x < 0 ? -x : x;
}

float sqrtf(float x) {
    float guess = x / 2.0f;
    float epsilon = 0.00001f;
    while (fabsf(guess * guess - x) > epsilon) {
        guess = (guess + x / guess) / 2.0f;
    }
    return guess;
}

float fast_sqrtf(float x) {
    union {
        float f;
        uint32_t i;
    } conv = { x };
    conv.i = 0x5f3759df - (conv.i >> 1);
    conv.f *= 1.5f - (x * 0.5f * conv.f * conv.f);
    return x * conv.f;
}