#include <iostream>
#include <cstdint>
#include <chrono>
#include <fstream>
#include <vector>
#include <string>

// Round constants for the Iota step
const uint64_t RC[24] = {
    0x0000000000000001, 0x0000000000008082, 0x800000000000808A,
    0x8000000080008000, 0x000000000000808B, 0x0000000080000001,
    0x8000000080008081, 0x8000000000008009, 0x000000000000008A,
    0x0000000000000088, 0x0000000080008009, 0x000000008000000A,
    0x000000008000808B, 0x800000000000008B, 0x8000000000008089,
    0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
    0x000000000000800A, 0x800000008000000A, 0x8000000080008081,
    0x8000000000008080, 0x0000000080000001, 0x8000000080008008
};

// Rotation constants for the Rho step
const int ROT[5][5] = {
    {0, 36, 3, 41, 18},
    {1, 44, 10, 45, 2},
    {62, 6, 43, 15, 61},
    {28, 55, 25, 21, 56},
    {27, 20, 39, 8, 14}
};

// Helper function to rotate a 64-bit integer left by n bits
inline uint64_t rotl64(uint64_t a, int n) {
    if (n == 0) return a;
    return (a << n) | (a >> (64 - n));
}

void theta(uint64_t state[5][5]) {
    uint64_t C[5];
    uint64_t D[5];
    
    for (int x = 0; x < 5; ++x) {
        C[x] = state[x][0] ^ state[x][1] ^ state[x][2] ^ state[x][3] ^ state[x][4];
    }
    
    for (int x = 0; x < 5; ++x) {
        D[x] = C[(x + 4) % 5] ^ rotl64(C[(x + 1) % 5], 1); // (x - 1) % 5 is functionally (x + 4) % 5
    }
    
    for (int x = 0; x < 5; ++x) {
        for (int y = 0; y < 5; ++y) {
            state[x][y] ^= D[x];
        }
    }
}

void rho_and_pi(uint64_t state[5][5]) {
    uint64_t B[5][5] = {0};
    
    for (int x = 0; x < 5; ++x) {
        for (int y = 0; y < 5; ++y) {
            B[y][(2 * x + 3 * y) % 5] = rotl64(state[x][y], ROT[x][y]);
        }
    }
    
    // Copy B back to state
    for (int x = 0; x < 5; ++x) {
        for (int y = 0; y < 5; ++y) {
            state[x][y] = B[x][y];
        }
    }
}

void chi(uint64_t state[5][5]) {
    uint64_t temp[5][5];
    
    for (int x = 0; x < 5; ++x) {
        for (int y = 0; y < 5; ++y) {
            temp[x][y] = state[x][y];
        }
    }
    
    for (int x = 0; x < 5; ++x) {
        for (int y = 0; y < 5; ++y) {
            state[x][y] = temp[x][y] ^ ((~temp[(x + 1) % 5][y]) & temp[(x + 2) % 5][y]);
        }
    }
}

void iota(uint64_t state[5][5], int round_index) {
    state[0][0] ^= RC[round_index];
}

void keccak_f1600(uint64_t state[5][5]) {
    for (int i = 0; i < 24; ++i) {
        theta(state);
        rho_and_pi(state);
        chi(state);
        iota(state, i);
    }
}


// The benchmarking harness
// Global variable to trick the compiler and prevent Dead Code Elimination
volatile uint64_t global_sink = 0;

// The benchmarking harness
void benchmark_and_save(const std::string& filename) {
    std::ofstream file(filename);
    
    // Create the CSV headers
    file << "Blocks,Run,Time_ms\n";

    // Logarithmic block scaling + your massive 50M workload
    std::vector<int> block_counts = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000};
    int num_runs = 5;

    std::cout << "Starting benchmark. Saving to " << filename << "...\n";

    for (int blocks : block_counts) {
        std::cout << "Testing " << blocks << " blocks...\n";
        
        for (int run = 1; run <= num_runs; ++run) {
            uint64_t state[5][5] = {0}; 
            
            // Start the high-resolution timer
            auto start = std::chrono::high_resolution_clock::now();
            
            for (int i = 0; i < blocks; ++i) {
                keccak_f1600(state);
            }
            
            // Stop the timer immediately after the loop
            auto end = std::chrono::high_resolution_clock::now();
            
            // TRICK THE COMPILER: Force it to evaluate the final state
            // Because this is a volatile global variable, the compiler CANNOT delete the loop above.
            global_sink ^= state[0][0]; 

            std::chrono::duration<double, std::milli> duration = end - start;
            
            // Save the raw data point to the CSV
            file << blocks << "," << run << "," << duration.count() << "\n";
        }
    }
    
    file.close();
    std::cout << "Benchmarking complete!\n";
}

int main() {
    // Run this for the readable script
    benchmark_and_save("readable_results.csv");
    return 0;
}