#include <iostream>
#include <cstdint>
#include <array>
#include <sycl/sycl.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>

using namespace sycl;

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

inline uint64_t rotl64(uint64_t a, int n) {
    return (n == 0) ? a : ((a << n) | (a >> (64 - n)));
}

using PipeIn = ext::intel::pipe<class KeccakInPipe, std::array<uint64_t, 25>, 8>;
using PipeOut = ext::intel::pipe<class KeccakOutPipe, std::array<uint64_t, 25>, 8>;

int main() {
    try {
        #if defined(FPGA_EMULATOR)
            queue q{ext::intel::fpga_emulator_selector_v};
        #else
            queue q{ext::intel::fpga_selector_v};
        #endif

        std::cout << "Target Device: " << q.get_device().get_info<info::device::name>() << "\n";

        int num_blocks = 5000000; 
        std::cout << "Synthesising Partial Pipeline\n";

        // KERNEL 1: Data Feeder
        q.submit([&](handler& h) {
            h.single_task<class DataFeeder>([=]() {
                for (int i = 0; i < num_blocks; ++i) {
                    std::array<uint64_t, 25> input_block; 
                    #pragma unroll
                    for(int k = 0; k < 25; ++k) {
                        // using the hex constant to show compiler therre is data present, 
                        // not only just that there is data but that it is differennt
                        // this stops the compiler optimising away the whole loop
                        input_block[k] = (uint64_t)(i + k) * 0x123456789ABCDEF; 
                    }
                    PipeIn::write(input_block);
                }
            });
        });

        // KERNEL 2: The Core Pipeline (Partial Pipelining)
        q.submit([&](handler& h) {
            h.single_task<class KeccakPipeline>([=]() [[intel::kernel_args_restrict]] {
                
                // State OUTSIDE the loop -> Sequential feedback SHA-3 Sponge structure
                std::array<uint64_t, 25> state = {0}; 
                
                // Initiation Interval forced.
                [[intel::initiation_interval(10)]] 
                for (int i = 0; i < num_blocks; ++i) {
                    std::array<uint64_t, 25> input = PipeIn::read();
                    
                    state[0] ^= input[0]; 
                    
                    uint64_t C[5], D[5];
                    uint64_t t0, t1, t2, t3, t4;

                    #pragma unroll 24
                    for (int r = 0; r < 24; ++r) {
                        
                        // THETA
                        C[0] = state[0] ^ state[5] ^ state[10] ^ state[15] ^ state[20];
                        C[1] = state[1] ^ state[6] ^ state[11] ^ state[16] ^ state[21];
                        C[2] = state[2] ^ state[7] ^ state[12] ^ state[17] ^ state[22];
                        C[3] = state[3] ^ state[8] ^ state[13] ^ state[18] ^ state[23];
                        C[4] = state[4] ^ state[9] ^ state[14] ^ state[19] ^ state[24];

                        D[0] = C[4] ^ rotl64(C[1], 1);
                        D[1] = C[0] ^ rotl64(C[2], 1);
                        D[2] = C[1] ^ rotl64(C[3], 1);
                        D[3] = C[2] ^ rotl64(C[4], 1);
                        D[4] = C[3] ^ rotl64(C[0], 1);

                        #pragma unroll
                        for (int x = 0; x < 5; ++x) {
                            state[x] ^= D[x]; state[x+5] ^= D[x]; state[x+10] ^= D[x]; state[x+15] ^= D[x]; state[x+20] ^= D[x];
                        }

                        // RHO & PI
                        t1 = state[1];
                        state[1]  = rotl64(state[6], 44);
                        state[6]  = rotl64(state[9], 20);
                        state[9]  = rotl64(state[22], 61);
                        state[22] = rotl64(state[14], 39);
                        state[14] = rotl64(state[20], 18);
                        state[20] = rotl64(state[2], 62);
                        state[2]  = rotl64(state[12], 43);
                        state[12] = rotl64(state[13], 25);
                        state[13] = rotl64(state[19], 8);
                        state[19] = rotl64(state[23], 56);
                        state[23] = rotl64(state[15], 41);
                        state[15] = rotl64(state[4], 27);
                        state[4]  = rotl64(state[24], 14);
                        state[24] = rotl64(state[21], 2);
                        state[21] = rotl64(state[8], 55);
                        state[8]  = rotl64(state[16], 45);
                        state[16] = rotl64(state[5], 36);
                        state[5]  = rotl64(state[3], 28);
                        state[3]  = rotl64(state[18], 21);
                        state[18] = rotl64(state[17], 15);
                        state[17] = rotl64(state[11], 10);
                        state[11] = rotl64(state[7], 6);
                        state[7]  = rotl64(state[10], 3);
                        state[10] = rotl64(t1, 1);

                        // CHI
                        #pragma unroll
                        for (int y = 0; y < 25; y += 5) {
                            t0 = state[y + 0];
                            t1 = state[y + 1];
                            t2 = state[y + 2];
                            t3 = state[y + 3];
                            t4 = state[y + 4];

                            state[y + 0] = t0 ^ ((~t1) & t2);
                            state[y + 1] = t1 ^ ((~t2) & t3);
                            state[y + 2] = t2 ^ ((~t3) & t4);
                            state[y + 3] = t3 ^ ((~t4) & t0);
                            state[y + 4] = t4 ^ ((~t0) & t1);
                        }

                        // IOTA
                        state[0] ^= RC[r];
                        
                        // ---  User defined pipelining --- 
                        // eg to add a single register would be, if r == 11, then add a register stage after round 11 
                        // eg to add a register stage every other round
                        if ((r * 10) / 24 != ((r - 1) * 10) / 24 || r == 0) { //
                            #pragma unroll
                            for (int k = 0; k < 25; ++k) {
                                state[k] = sycl::ext::intel::fpga_reg(state[k]);
                            }
                        }
                    }
                    
                    PipeOut::write(state);
                }
            });
        });

        // KERNEL 3: Data Consumer
        q.submit([&](handler& h) {
            h.single_task<class DataConsumer>([=]() {
                for (int i = 0; i < num_blocks; ++i) {
                    std::array<uint64_t, 25> output_block = PipeOut::read();
                    if (i == num_blocks - 1) {
                        (void)output_block[0]; 
                    }
                }
            });
        }).wait(); 

        std::cout << "Kernel execution complete!\n";

    } catch (exception const& e) {
        std::cout << "SYCL exception caught: " << e.what() << '\n';
        return 1;
    }

    return 0;
}