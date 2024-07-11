#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <alloc.h>
#include <mram.h>
#include <perfcounter.h>
#include <barrier.h>
#include <defs.h>

// Gravitational constant
#define G 6.67430e-11f
#define DT 1e-3f

// Number of bodies
#define nof_bodies 4

// Structure to represent a body (float)
typedef struct {
    float mass;
    float x, y, z;
    float vx, vy, vz;
    float padding;  // Padding byte for 8-byte alignment
} Body_f;

__mram_noinit Body_f mram_bodies[nof_bodies];
float fx[nof_bodies], fy[nof_bodies], fz[nof_bodies];

Body_f local_bodies[nof_bodies];

BARRIER_INIT(my_barrier, NR_TASKLETS);

// Math methods
float fabsf(float x);
float sqrtf(float x);

// Calculate the force between two bodies
void computeForce_f(const Body_f *a, const Body_f *b, float *fx, float *fy, float *fz) {
    float dx = b->x - a->x;
    float dy = b->y - a->y;
    float dz = b->z - a->z;
    float dist = sqrtf(dx * dx + dy * dy + dz * dz);
    float dist3 = dist * dist * dist;
    float force = (G * a->mass * b->mass) / dist3;

    *fx = force * dx;
    *fy = force * dy;
    *fz = force * dz;
}

int main() {
    printf("Tasklet %d: Enter Main\n", me());

    // Initialize the performance counter
    //perfcounter_config(COUNT_CYCLES, true);

    // Copy data from MRAM to WRAM
    if (me() == 0) {
        mram_read(mram_bodies, local_bodies, sizeof(local_bodies));
        printf("Tasklet %d: Data copied from MRAM to WRAM\n", me());
    }

    // Calculation of forces and update of positions and velocities
    for (int step = 0; step < 2; ++step) {
        // Initialize forces
        if (me() == 0) {
            for (int i = 0; i < nof_bodies; ++i) {
                fx[i] = fy[i] = fz[i] = 0.0f;
            }
        }

        barrier_wait(&my_barrier);

        // Calculate forces
        for (int i = me(); i < nof_bodies; i += NR_TASKLETS) {
            for (int j = i + 1; j < nof_bodies; ++j) {
                float fx_ij, fy_ij, fz_ij;
                computeForce_f(&local_bodies[i], &local_bodies[j], &fx_ij, &fy_ij, &fz_ij);

                fx[i] += fx_ij;
                fy[i] += fy_ij;
                fz[i] += fz_ij;
                fx[j] -= fx_ij;
                fy[j] -= fy_ij;
                fz[j] -= fz_ij;

                // Print body index i and j and force calculation done
                printf("Tasklet %d: Force calculation done for bodies %d <> %d\n", me(), i, j);
            }
        }

        // Print tasklet ID and force calculation done
        printf("Tasklet %d: Force calculation done\n", me());

        barrier_wait(&my_barrier);

        // Update positions and velocities
        for (int i = me(); i < nof_bodies; i += NR_TASKLETS) {
            float vx = fx[i] / local_bodies[i].mass * DT;
            float vy = fy[i] / local_bodies[i].mass * DT;
            float vz = fz[i] / local_bodies[i].mass * DT;

            // Print local body vx
            printf("Tasklet %d: local_bodies[%d].vx: %f\n", me(), i, local_bodies[i].vx);

            local_bodies[i].vx += vx;
            local_bodies[i].vy += vy;
            local_bodies[i].vz += vz;

            local_bodies[i].x += vx * DT;
            local_bodies[i].y += vy * DT;
            local_bodies[i].z += vz * DT;
        }

        // Print tasklet ID and position update done
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