#!/bin/bash

# firstly make the bash executable by running: chmod +x Build_report.sh in terminal 
# Usage: ./Build_report.sh [file_name_without_extension]
# This will then build the specified C++ file using the Intel OneAPI SYCL compiler inside the Docker container and generate a report.

# The name of your C++ file (now including folder paths if you pass them)
FILE_NAME=${1:-keccak_hls}

echo "Launching Intel OneAPI SYCL compilation for: ${FILE_NAME}.cpp"

# Execute build inside the container using the modern icpx compiler
docker exec -t intel_builder bash -c "
    source /opt/intel/oneapi/2025.0/oneapi-vars.sh --force && \
    cd /workspace && \
    icpx -fsycl -fintelfpga -Xstarget=Agilex7 -Xshardware -fsycl-link=early ${FILE_NAME}.cpp -o ${FILE_NAME}_report.a
"

echo "Build complete! Check the ${FILE_NAME}_report.prj/reports/report.html file."