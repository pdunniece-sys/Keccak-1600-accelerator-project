# Keccak Hardware Accelerator

This repository contains C++ and SYCL implementations of Keccak-f[1600], along with benchmarking, plotting, and Intel FPGA build scripts.

## What Is Here

- `Build_report_2.sh` - runs an Intel oneAPI SYCL build inside Docker and points at the generated report in `*_report.prj/reports/report.html`.
- `plot.ipynb` - notebook for exploring the accelerator results and generated figures.
- `requirements.txt` - Python dependencies used by the notebooks and plotting workflow.
- `CPP/Software_cpp_and_Graphing/Unoptimised_C++.cpp` - baseline CPU benchmark that writes `readable_results.csv`.
- `CPP/Software_cpp_and_Graphing/optimised_graphing.cpp` - optimized CPU benchmark that writes `optimised_results.csv`.
- `CPP/Software_cpp_and_Graphing/graphing.ipynb` - notebook that compares the benchmark CSV files and produces `cpp_execution_scaling.png`.
- `CPP/Keccak_HLS/Pipelining/pipelining.cpp` - SYCL FPGA version that experiments with partial pipelining and register staging.
- `CPP/Keccak_HLS/Unrolling/Optimised_cpp.cpp` - SYCL FPGA version that folds the Keccak round function into a single kernel flow.
- `CPP/reports/` - generated Intel HLS project outputs and reports.

## Running The Benchmarks

The CPU benchmarks are self-contained C++ programs. Compile and run them from `CPP/Software_cpp_and_Graphing/` or from the repository root with your preferred compiler.

Example:

```bash
g++ -O3 -std=c++17 CPP/Software_cpp_and_Graphing/Unoptimised_C++.cpp -o unoptimised && ./unoptimised
g++ -O3 -std=c++17 CPP/Software_cpp_and_Graphing/optimised_graphing.cpp -o optimised && ./optimised
```

Each program writes a CSV in the same folder it is run from:

- `readable_results.csv`
- `optimised_results.csv`

## Plotting Results

Open `CPP/Software_cpp_and_Graphing/graphing.ipynb` after generating the CSV files to compare execution time scaling and regenerate `cpp_execution_scaling.png`.

The top-level `plot.ipynb` notebook contains additional analysis and visualisation work for the accelerator results.

## Building FPGA Reports

`Build_report_2.sh` expects the source file name without the `.cpp` extension. It compiles the target inside the `intel_builder` Docker container using Intel oneAPI SYCL tools.

Example:

```bash
./Build_report_2.sh CPP/Keccak_HLS/Pipelining/pipelining
./Build_report_2.sh CPP/Keccak_HLS/Unrolling/Optimised_cpp
```

After the build finishes, check the generated report in the corresponding `*_report.prj/reports/report.html` directory.

## Dependencies

- Python 3
- Jupyter
- `numpy`
- A C++17-capable compiler for the CPU benchmarks
- Docker and Intel oneAPI SYCL / FPGA tooling for the HLS build script

## Notes

- The `.accellerators_venv/` folder is the local Python virtual environment.
- The contents under `CPP/reports/` are generated build artifacts and are safe to regenerate.
