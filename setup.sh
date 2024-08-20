#!/bin/bash

# Prompt user for IP addresses interactively
read -p "Enter the first IP address: " IP1
read -p "Enter the second IP address: " IP2
read -p "Enter the third IP address: " IP3

# Verify that the user has entered all required IPs
if [ -z "$IP1" ] || [ -z "$IP2" ] || [ -z "$IP3" ]; then
    echo "Error: All three IP addresses must be provided."
    exit 1
fi

# Install DevTools and packages
sudo yum groupinstall -y "Development Tools" \
&& sudo yum install -y cmake libsodium libsodium-devel openmpi openmpi-devel pkg-config git openssh-server nano

git clone https://github.com/multiparty/Secrecy.git

# Create the directory for the external library and clone the sql-parser repository
mkdir -p Secrecy/include/external-lib \
&& git clone https://github.com/mfaisal97/sql-parser.git Secrecy/include/external-lib/sql-parser

# Update the PATH for OpenMPI
echo 'export PATH=$PATH:/usr/lib64/openmpi/bin' >> ~/.bashrc \
&& export PATH=$PATH:/usr/lib64/openmpi/bin

# Verify that OpenMPI compilers are in the PATH
which mpicc && which mpicxx

# Build the Secrecy project
cd Secrecy \
&& mkdir build && cd build && cmake .. && make

# Create a hostfile with the provided IP addresses
echo -e "$IP1\n$IP2\n$IP3" > hostfile.txt

echo "Setup completed!"