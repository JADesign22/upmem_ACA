#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <chrono>
#include <random>

// Gravitational constant
const float G_f = 6.67430e-11f;

// Structure to represent a body (float)
struct Body_f {
    float mass;
    float x, y, z;
    float vx, vy, vz;
};

// Function to compute gravitational force between two bodies (float)
void computeForce_f(const Body_f &a, const Body_f &b, float &fx, float &fy, float &fz) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dz = b.z - a.z;
    float dist = sqrtf(dx*dx + dy*dy + dz*dz);
    float force = (G_f * a.mass * b.mass) / (dist * dist * dist); // dist^3

    fx = force * dx;
    fy = force * dy;
    fz = force * dz;
}

// Main function to perform the N-Body simulation (float)
void nbody_f(std::vector<Body_f> &bodies, float dt, int steps, const std::string &filename) {
    int n = bodies.size();
    std::ofstream outfile(filename);

    // Write results to file (float)
    outfile << "step;body_id;x;y;z\n"; // CSV header
    for (int step = 0; step < steps; ++step) {
        // Initialize forces
        std::vector<float> fx(n, 0.0f), fy(n, 0.0f), fz(n, 0.0f);

        // Compute forces
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

        // Update positions and velocities
        for (int i = 0; i < n; ++i) {
            bodies[i].vx += fx[i] / bodies[i].mass * dt;
            bodies[i].vy += fy[i] / bodies[i].mass * dt;
            bodies[i].vz += fz[i] / bodies[i].mass * dt;

            bodies[i].x += bodies[i].vx * dt;
            bodies[i].y += bodies[i].vy * dt;
            bodies[i].z += bodies[i].vz * dt;
        }

        // Write results to file
        for (int i = 0; i < n; ++i) {
            outfile << step << ";" << i << ";" << bodies[i].x << ";" << bodies[i].y << ";" << bodies[i].z << "\n";
        }
    }
    outfile.close();
}

// Function to generate a list of bodies with random values
std::vector<Body_f> generateBodies(int numBodies) {
    std::vector<Body_f> bodies;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1.0, 1e24);
    std::uniform_real_distribution<> pos(-1e11, 1e11);
    std::uniform_real_distribution<> vel(-1e3, 1e3);

    for (int i = 0; i < numBodies; ++i) {
        bodies.push_back({ static_cast<float>(dis(gen)), static_cast<float>(pos(gen)), static_cast<float>(pos(gen)), static_cast<float>(pos(gen)),
                           static_cast<float>(vel(gen)), static_cast<float>(vel(gen)), static_cast<float>(vel(gen)) });
    }
    return bodies;
}

int main() {
    std::vector<int> numBodiesList = { 10, 50, 100, 200, 400, 800, 1000, 2000 };  // Different numbers of bodies
    float dt_f = 1e-3f; // Time step
    int steps = 100;    // Number of simulation steps

    std::ofstream resultFile("nbody_results.txt");
    resultFile << "numOfBodies;steps;duration\n"; // CSV header

    for (int numBodies : numBodiesList) {
        std::vector<Body_f> bodies_f = generateBodies(numBodies);
        std::string filename = "nbody_float_" + std::to_string(numBodies) + ".txt";

        // Start timing
        auto start = std::chrono::high_resolution_clock::now();

        nbody_f(bodies_f, dt_f, steps, filename);

        // End timing
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = end - start;

        // Print and save the results
        std::cout << "Number of bodies: " << numBodies << " - Time: " << duration.count() << " seconds\n";
        resultFile << numBodies << ";" << steps << ";" << duration.count() << "\n";
    }

    resultFile.close();
    return 0;
}