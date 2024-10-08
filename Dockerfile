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
    nano \
    netcat \
    iptables \
    telnet \
    && rm -rf /var/lib/apt/lists/*

# Create the SSH directory and set up the server
RUN mkdir /var/run/sshd
RUN echo 'root:password' | chpasswd
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed -i 's/PermitRootLogin without-password/PermitRootLogin yes/' /etc/ssh/sshd_config

# Generate SSH keys and configure known hosts
RUN mkdir -p /root/.ssh && ssh-keygen -t rsa -b 2048 -f /root/.ssh/id_rsa -q -N "" && \
    cat /root/.ssh/id_rsa.pub >> /root/.ssh/authorized_keys && \
    echo "Host *" >> /root/.ssh/config && \
    echo "    StrictHostKeyChecking no" >> /root/.ssh/config && \
    echo "Host *" >> /root/.ssh/config && \
    echo "    ForwardAgent yes" >> /root/.ssh/config

# Set the working directory
WORKDIR /usr/src/app

# Clone the main Secrecy repository
RUN git clone https://github.com/multiparty/Secrecy.git

# Create include/external-lib directory and clone the sql-parser repository inside it
RUN mkdir -p Secrecy/include/external-lib \
    && git clone https://github.com/mfaisal97/sql-parser.git Secrecy/include/external-lib/sql-parser

# Set the working directory to Secrecy
WORKDIR /usr/src/app/Secrecy

# Create build directory and run CMake
RUN mkdir build && cd build && cmake .. && make

# Allowing to run as a root
ENV OMPI_ALLOW_RUN_AS_ROOT=1
ENV OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1
ENV OMPI_MCA_routed=direct

# Set OpenMPI environment variables
# ENV OMPI_MCA_oob_tcp_dynamic_ports=10000-10009 \
#     OMPI_MCA_btl_tcp_port_min_v4=10000 \
#     OMPI_MCA_btl_tcp_port_range_v4=10

# Expose the necessary ports
EXPOSE 0-65535
CMD ["/usr/sbin/sshd", "-D"]