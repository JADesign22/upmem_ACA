#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>  // Für Dateioperationen

// Gravitationskonstante
const double G_d = 6.67430e-11;

// Struktur zur Darstellung eines Körpers (double)
struct Body_d {
    double mass;
    double x, y, z;
    double vx, vy, vz;
};

// Funktion zur Berechnung der Gravitationskraft zwischen zwei Körpern (double)
void computeForce_d(const Body_d &a, const Body_d &b, double &fx, double &fy, double &fz) {
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double dz = b.z - a.z;
    double dist = sqrt(dx*dx + dy*dy + dz*dz);
    double force = (G_d * a.mass * b.mass) / (dist * dist * dist); // dist^3 für die Richtung

    fx = force * dx;
    fy = force * dy;
    fz = force * dz;
}

// Hauptfunktion zur Durchführung der N-Body-Simulation (double)
void nbody_d(std::vector<Body_d> &bodies, double dt, int steps, const std::string &filename) {
    int n = bodies.size();
    std::ofstream outfile(filename);

    // Ergebnisse in die Datei schreiben (double)
    outfile << "step;body_id;x;y;z\n"; // CSV-Header
    for (int step = 0; step < steps; ++step) {
        // Kräfte initialisieren
        std::vector<double> fx(n, 0.0), fy(n, 0.0), fz(n, 0.0);

        // Kräfte berechnen
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                double fx_ij, fy_ij, fz_ij;
                computeForce_d(bodies[i], bodies[j], fx_ij, fy_ij, fz_ij);
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
    // Beispiel mit 3 Körpern (double)
    std::vector<Body_d> bodies_d = {
        {1.0e24, 0, 0, 0, 0, 0, 0},
        {1.0e24, 1, 0, 0, 0, 1, 0},
        {1.0e24, 0, 1, 0, -1, 0, 0}
    };

    double dt_d = 1e-3; // Zeitschritt
    int steps = 100;    // Anzahl der Simulationsschritte

    nbody_d(bodies_d, dt_d, steps, "nbody_double.txt");

    return 0;
}