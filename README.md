# Keccak Hardware Accelerator

This repository contains optimised implementations of the Keccak cryptographic hash function, including both Python and C++ versions with hardware acceleration support.

## Project Structure

- **`Keccak_1600_python.py`** — Pure Python implementation of Keccak-1600
- **`CPP_theta_exectutiontime_vs_code.ipynb`** — Jupyter notebook comparing execution times between θ step implementations
- **`CPP/`** — C++ implementations and hardware-related files
  - **`Optimised_Keccak/`** — Optimised Keccak implementations
    - **`Optimised_HLS/`** — Intel HLS hardware synthesis project
    - **`Unrolled/`** — Loop-unrolled optimisation variant
  - **`Graphing/`** — Data visualisation and performance analysis
    - **`optimised_graphing.cpp`** — C++ graphing utilities
    - **`Unoptimised_C++/`** — Baseline C++ implementation for comparison

## Building & Running

### Python
```bash
python Keccak_1600_python.py
```

### C++
Compile with appropriate optimisation flags:
```bash
g++ -O3 -o keccak CPP/Optimised_Keccak/Optimised_cpp.cpp
```

### HLS Synthesis
Navigate to the Intel HLS project in `CPP/Optimised_Keccak/Optimised_HLS/` and use the Intel HLS tools to synthesize.

## Performance Analysis

- See `CPP_theta_exectutiontime_vs_code.ipynb` for execution time comparisons
- Results and visualisations in `CPP/Graphing/`

## Dependencies

- Python 3.x
- Intel/Altera HLS tools (for hardware synthesis)
- C++11 or later
- Jupyter (for notebooks)

## Notes

- Virtual environment: `.accellerators_venv/`
- Profiling data: `profile_data.prof`
