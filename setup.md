# Secrecy OpenMPI Setup Guide

This document is a step-by-step guide to setting up a Secrecy Server and OPEN MPI implementation on AWS EC2 instances.

## Introduction

In this guide, you will learn how to create a Virtual Private Cloud (VPC), launch and configure EC2 instances, and set up Secrecy with OpenMPI. This guide assumes you have a basic understanding of AWS services and SSH.

### Prerequisites
- AWS Account
- Basic knowledge of EC2 and VPC
- SSH client installed on your local machine

## 1) Create VPC 

1. Navigate to the AWS VPC console and hit **Create VPC**.

<img width="800" alt="image" src="https://github.com/user-attachments/assets/8398b2cb-77fe-4eb9-b263-207eab301219">

2. Select **VPC and more**.
3. Name your VPC
4. Create a VPC with **1 zone** and **public** and zero **private subnets**.

<table>
  <tr>
    <td><img src="https://github.com/user-attachments/assets/7c2d3bf1-17bc-425c-ac65-0f1701ab5564" alt="VPC Setup" width="400"></td>
    <td><img src="https://github.com/user-attachments/assets/e88406f0-382a-4cbb-b837-df9bafff6d60" alt="VPC Subnet Configuration" width="400"></td>
  </tr>
</table>

## 2) Launch EC2 Instance

<img src="https://github.com/user-attachments/assets/264abd86-06d2-44c7-bbbb-523a80ee6f86" alt="EC2 Instances" width="800">

1. Name Instance and select Amazon Linux
   
   <img width="700" alt="image" src="https://github.com/user-attachments/assets/69ae4a3a-afef-487b-a447-9a478fd80d79">

2. Pick an appropriate instance size.

3. Generate a key pair if you haven't and save the key to your local machine.

   <img src="https://github.com/user-attachments/assets/da61f0c5-eeef-4df5-ad55-2a4e40936e79" alt="Key Pair" width="600">

4. Hit **Edit** in Network settings and pick the VPC you've just created in step 1
   - **Create new subnet**
   - Enable **Auto-assign public IP**
   - Select **Create security group**

<table>
  <tr>
    <td><img src="https://github.com/user-attachments/assets/d6387771-ebb7-48d9-92e8-91e06d2c0431" alt="Network Settings" width="800"></td>
    <td><img width="689" alt="image" src="https://github.com/user-attachments/assets/e353478a-e39b-494e-83ad-a34e6bd1f314"></td>
  </tr>
</table>


## 3) Open Ports in Instances

1. Go to the EC2 instance dashboard.
2. Click the line of your instance (e.g., Secrecy-node#).
3. Save the Public IP address and exchange it with other participants.

   <img src="https://github.com/user-attachments/assets/c194e122-f442-4745-8989-2d4840c2d8d3" alt="EC2 Dashboard" width="800">

4. Open the **Security** tab and Hit **Security Groups**, which opens a new browser tab.

   <img src="https://github.com/user-attachments/assets/4d6e9a60-c62a-4234-8f7a-ce8f07caeff7" alt="Security Tab" width="800">

6. Click the value under **Security group ID**.

   <img src="https://github.com/user-attachments/assets/4d81375e-415f-409e-926c-548fb2f5c9ee" alt="Security Group ID" width="600">


- Under the **inbound rules**, open the **Edit inbound rules** button.

  <img src="https://github.com/user-attachments/assets/b22c5546-0908-454a-8642-98f2e92fe523" alt="Edit Inbound Rules" width="800">

- Add rules so that Port 22 is accessible from your local machine and all ports (0-65535) are accessible from the public IP addresses of the other two instances.

  <img src="https://github.com/user-attachments/assets/befaa0a7-99bb-4add-9c1a-3ea1d739627f" alt="Inbound Rules Configuration" width="800">

## 4) Access Instance

With these steps so far, you should be able to access the EC2 instance and are ready to launch the Secrecy app.

Exchange public IP addresses with other participants.

Try running the command below in your terminal:

```
ssh -i /path/to/your/key.pem ec2-user@your-public-IP
```

If you successfully SSH into your instance, run the following commands, after replacing <public IP of instance> with actual public IP addresses of your and other participants:

```
sudo yum groupinstall -y "Development Tools" \
&& sudo yum install -y cmake libsodium libsodium-devel openmpi openmpi-devel pkg-config git openssh-server nano \
&& git clone https://github.com/multiparty/Secrecy.git \
&& mkdir -p Secrecy/include/external-lib \
&& git clone https://github.com/mfaisal97/sql-parser.git Secrecy/include/external-lib/sql-parser \
&& echo 'export PATH=$PATH:/usr/lib64/openmpi/bin' >> ~/.bashrc \
&& export PATH=$PATH:/usr/lib64/openmpi/bin \
&& which mpicc && which mpicxx \
&& cd Secrecy \
&& mkdir build && cd build && cmake .. && make
```
  
Generate an SSH key pair on each instance by running the following command:

```
ssh-keygen -t rsa -N "" -f ~/.ssh/id_rsa && cat ~/.ssh/id_rsa.pub
```

Exchange public keys with other participants.
Paste their IP addresses in authorized_keys, which can be opened by:

```
nano ~/.ssh/authorized_keys
```

and paste by **"command + v "**.

## 5) Make a hostfile and run MPI program

If you are initiating the MPI process, make a host file in the build directory by running the following command:

```
 echo -e "<public IP of your instance>\n<public IP of another party's instance>\n<public IP of last instance>" > hostfile.txt
```

or manually by:

```
nano hostfile.txt
```

The resulting hostfile should look like this:

<img width="206" alt="image" src="https://github.com/user-attachments/assets/cc577fd0-24b0-4ab8-8620-9e894dd9a3a9">

The initiating party should be able to run the Secrecy algorithm by running the following command:

```
mpirun -np 3 --hostfile hostfile.txt ./test_join_sail ./../sample1.json ./../sample2.json
```

