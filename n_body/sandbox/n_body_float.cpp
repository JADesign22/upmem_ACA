#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>  // Für Dateioperationen

// Gravitationskonstante
const float G_f = 6.67430e-11f;

// Struktur zur Darstellung eines Körpers (float)
struct Body_f {
    float mass;
    float x, y, z;
    float vx, vy, vz;
};

// Funktion zur Berechnung der Gravitationskraft zwischen zwei Körpern (float)
void computeForce_f(const Body_f &a, const Body_f &b, float &fx, float &fy, float &fz) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dz = b.z - a.z;
    float dist = sqrtf(dx*dx + dy*dy + dz*dz);
    float force = (G_f * a.mass * b.mass) / (dist * dist * dist); // dist^3 für die Richtung

    fx = force * dx;
    fy = force * dy;
    fz = force * dz;
}

// Hauptfunktion zur Durchführung der N-Body-Simulation (float)
void nbody_f(std::vector<Body_f> &bodies, float dt, int steps, const std::string &filename) {
    int n = bodies.size();
    std::ofstream outfile(filename);

        // Ergebnisse in die Datei schreiben (float)
    outfile << "step;body_id;x;y;z\n"; // CSV-Header
    for (int step = 0; step < steps; ++step) {
        // Kräfte initialisieren
        std::vector<float> fx(n, 0.0f), fy(n, 0.0f), fz(n, 0.0f);

        // Kräfte berechnen
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                float fx_ij, fy_ij, fz_ij;
                computeForce_f(bodies[i], bodies[j], fx_ij, fy_ij, fz_ij);
                fx[i] += fx_ij;
                fy[i] += fy_ij;
                fz[i] += fz_ij;
                fx[j] -= fx_ij;
                fy[j] -= fy_ij;
                fz[j] -= fz_ij;
            }
        }

        // Positionen und Geschwindigkeiten aktualisieren
        for (int i = 0; i < n; ++i) {
            bodies[i].vx += fx[i] / bodies[i].mass * dt;
            bodies[i].vy += fy[i] / bodies[i].mass * dt;
            bodies[i].vz += fz[i] / bodies[i].mass * dt;

            bodies[i].x += bodies[i].vx * dt;
            bodies[i].y += bodies[i].vy * dt;
            bodies[i].z += bodies[i].vz * dt;
        }

        // Ergebnisse in die Datei schreiben
        for (int i = 0; i < n; ++i) {
            outfile << step << ";" << i << ";" << bodies[i].x << ";" << bodies[i].y << ";" << bodies[i].z << "\n";
        }
    }
    outfile.close();
}

int main() {
    // Beispiel mit 3 Körpern (float)
    std::vector<Body_f> bodies_f = {
        {1.0e24f, 0, 0, 0, 0, 0, 0},
        {1.0e24f, 1, 0, 0, 0, 1, 0},
        {1.0e24f, 0, 1, 0, -1, 0, 0}
    };

    float dt_f = 1e-3f; // Zeitschritt
    int steps = 100;    // Anzahl der Simulationsschritte

    nbody_f(bodies_f, dt_f, steps, "nbody_float.txt");

    return 0;
}