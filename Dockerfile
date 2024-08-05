# Use an official Ubuntu as a base image
FROM ubuntu:22.04

# Install necessary packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libsodium-dev \
    libopenmpi-dev \
    openmpi-bin \
    pkg-config \
    git \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /usr/src/app

# Clone the main Secrecy repository
RUN git clone https://github.com/Snafkin547/Secrecy.git && \
cd Secrecy && \
git checkout bdbaf17e72f05bebaeab6a27506d82a03c4e59ec

# Create include/external-lib directory and clone the sql-parser repository inside it
RUN mkdir -p Secrecy/include/external-lib \
    && git clone https://github.com/mfaisal97/sql-parser.git Secrecy/include/external-lib/sql-parser

# Set the working directory to Secrecy
WORKDIR /usr/src/app/Secrecy

# Create build directory and run CMake
RUN mkdir build && cd build && cmake .. && make

# Sleep indefinitely to allow for command line interaction
CMD ["sleep", "infinity"]
