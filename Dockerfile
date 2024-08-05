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
    openssh-server \
    && rm -rf /var/lib/apt/lists/*

# Create the SSH directory and set up the server
RUN mkdir /var/run/sshd

# Configure SSH to allow root login
RUN echo 'root:password' | chpasswd
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed -i 's/PermitRootLogin without-password/PermitRootLogin yes/' /etc/ssh/sshd_config

# Allow SSH service to run in the background
EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]

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

# Allowing to run as a root
ENV OMPI_ALLOW_RUN_AS_ROOT=1
ENV OMPI_ALLOW_RUN_AS_RO OT_CONFIRM=1

# Sleep indefinitely to allow for command line interaction
CMD ["sleep", "infinity"]
