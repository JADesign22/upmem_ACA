#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <alloc.h>
#include <mram.h>
#include <perfcounter.h>
#include <barrier.h>
#include <defs.h>

// Gravitationskonstante
#define G 6.67430e-11f
#define DT 1e-3f

// Struktur zur Darstellung eines Körpers (float)
typedef struct {
    float mass;
    float x, y, z;
    float vx, vy, vz;
    float padding;  // Auffüllbyte für 8-Byte-Ausrichtung
} Body_f;

#define nof_bodies 3

__mram_noinit Body_f mram_bodies[3];
float fx[3], fy[3], fz[3];

Body_f local_bodies[3];

BARRIER_INIT(my_barrier, 3);

// math methodes
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

    // Initialisieren der perfcounter
    //perfcounter_config(COUNT_CYCLES, true);
    
    

    // Kopiere Daten von MRAM in WRAM
    if (me() == 0) {
        mram_read(mram_bodies, local_bodies, sizeof(local_bodies));
        printf("Tasklet %d: Data copied from MRAM to WRAM\n", me());
    }


    // Berechnung der Kräfte und Aktualisierung der Positionen und Geschwindigkeiten
    for (int step = 0; step < 2; ++step) {
        // Kräfte initialisieren
        if (me() == 0) {
            for (int i = 0; i < 3; ++i) {
                fx[i] = fy[i] = fz[i] = 0.0f;
            }
        }

        barrier_wait(&my_barrier);


        
        // Kräfte berechnen
        for (int i = me(); i < 3; i += NR_TASKLETS) {
            for (int j = i + 1; j < 3; ++j) {
                float fx_ij, fy_ij, fz_ij;
                computeForce_f(&local_bodies[i], &local_bodies[j], &fx_ij, &fy_ij, &fz_ij);
                
                fx[i] += fx_ij;
                fy[i] += fy_ij;
                fz[i] += fz_ij;
                fx[j] -= fx_ij;
                fy[j] -= fy_ij;
                fz[j] -= fz_ij;

                // print body index i and j and force calculation done
                printf("Tasklet %d: Force calculation done for bodies %d <> %d\n", me(), i, j);
                
            }
            
        }
        //print tasklet id and force calculation done
        printf("Tasklet %d: Force calculation done\n", me());

        barrier_wait(&my_barrier);
        
        // Positionen und Geschwindigkeiten aktualisieren
        for (int i = me(); i < 3; i += NR_TASKLETS) {
            float vx = fx[i] / local_bodies[i].mass * DT;
            float vy = fy[i] / local_bodies[i].mass * DT;
            float vz = fz[i] / local_bodies[i].mass * DT;

            //print localbodie vx
            printf("Tasklet %d: local_bodies[%d].vx: %f\n", me(), i, local_bodies[i].vx);
            
            local_bodies[i].vx += vx;
            local_bodies[i].vy += vy;
            local_bodies[i].vz += vz;
            
            local_bodies[i].x += vx * DT;
            local_bodies[i].y += vy * DT;
            local_bodies[i].z += vz * DT;
            
        }
       
        //print tasklet id and position update done
        printf("Tasklet %d: Position update done\n", me());
        barrier_wait(&my_barrier);
         
        
        
    }

    // Kopiere Daten von WRAM zurück in MRAM
        if (me() == 0) {
            mram_write(local_bodies, mram_bodies, sizeof(local_bodies));
            printf("Tasklet %d: Data copied from WRAM to MRAM\n", me());
        }
    
    return 0;
}







// math methodes
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