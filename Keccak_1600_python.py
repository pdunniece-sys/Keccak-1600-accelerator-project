# pure_sha3.py

# Round constants for the Iota step
RC = [0x0000000000000001, 0x0000000000008082, 0x800000000000808A,
      0x8000000080008000, 0x000000000000808B, 0x0000000080000001,
      0x8000000080008081, 0x8000000000008009, 0x000000000000008A,
      0x0000000000000088, 0x0000000080008009, 0x000000008000000A,
      0x000000008000808B, 0x800000000000008B, 0x8000000000008089,
      0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
      0x000000000000800A, 0x800000008000000A, 0x8000000080008081,
      0x8000000000008080, 0x0000000080000001, 0x8000000080008008]

# Rotation constants for the Rho step
ROT = [[0, 36, 3, 41, 18],
       [1, 44, 10, 45, 2],
       [62, 6, 43, 15, 61],
       [28, 55, 25, 21, 56],
       [27, 20, 39, 8, 14]]

def rotl64(a, n):
    """Helper function to rotate a 64-bit integer left by n bits."""
    return ((a << n) | (a >> (64 - n))) & 0xFFFFFFFFFFFFFFFF

def theta(state):
    C = [state[x][0] ^ state[x][1] ^ state[x][2] ^ state[x][3] ^ state[x][4] for x in range(5)]
    D = [C[(x - 1) % 5] ^ rotl64(C[(x + 1) % 5], 1) for x in range(5)]
    for x in range(5):
        for y in range(5):
            state[x][y] ^= D[x]
    return state

def rho_and_pi(state):
    B = [[0 for _ in range(5)] for _ in range(5)]
    for x in range(5):
        for y in range(5):
            # Pi rearranges the lanes, Rho applies the rotation
            B[y][(2 * x + 3 * y) % 5] = rotl64(state[x][y], ROT[x][y])
    return B

def chi(state):
    # We need a temporary copy so we don't overwrite data we still need to read
    temp = [[state[x][y] for y in range(5)] for x in range(5)]
    for x in range(5):
        for y in range(5):
            state[x][y] = temp[x][y] ^ ((~temp[(x + 1) % 5][y]) & temp[(x + 2) % 5][y])
    return state

def iota(state, round_index):
    state[0][0] ^= RC[round_index]
    return state

def keccak_f1600(state):
    """Runs the 24 scrambling rounds on the 1600-bit state."""
    for i in range(24):
        state = theta(state)
        state = rho_and_pi(state)
        state = chi(state)
        state = iota(state, i)
    return state

def simulate_heavy_workload(num_blocks=10000):
    """Simulates hashing a large file block by block."""
    # Initialize a state matrix of 5x5 containing 64-bit integers
    state = [[0 for _ in range(5)] for _ in range(5)]
    
    print(f"Running Keccak-f[1600] permutation for {num_blocks} blocks...")
    for _ in range(num_blocks):
        state = keccak_f1600(state)
    print("Done!")

if __name__ == "__main__":
    simulate_heavy_workload()