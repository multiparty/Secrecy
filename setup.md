# Secrecy OpenMPI Setup Guide

This document is a step-by-step guide to setting up Secrecy Server and OPEN MPI implementation.

## 1) Create VPC 

- Select <i>VPC and more</i>
- Need <b>1</b> zone and public and private subnet

<img width="492" alt="image" src="https://github.com/user-attachments/assets/238a521a-167c-462b-9d2d-ec5739014e7c">

<img width="455" alt="image" src="https://github.com/user-attachments/assets/2ccae596-158b-4198-b1c5-b23dc85997f2">

## 2) Launch three EC2 instances
<img width="1884" alt="image" src="https://github.com/user-attachments/assets/264abd86-06d2-44c7-bbbb-523a80ee6f86">

- Pick appropriate size of instance
- Generate a key pair if you haven't and save the key in your local machine
<img width="752" alt="image" src="https://github.com/user-attachments/assets/da61f0c5-eeef-4df5-ad55-2a4e40936e79">

- Hit <i>Edit</i> in Network settings and pick the VPC you've just created in step 1
<img width="767" alt="image" src="https://github.com/user-attachments/assets/d6387771-ebb7-48d9-92e8-91e06d2c0431">
<img width="717" alt="image" src="https://github.com/user-attachments/assets/4a1edb09-df32-49c9-a568-ab7259a7c305">

- Repeat this process till you have created three instances

## 3) Open ports in instances

##### 1. Go to EC2 instance dashboard
##### 2. Click a line of your instance (Mines are named Secrecy-node#)
##### 3. Save Public and Private IP addresses
<img width="1507" alt="image" src="https://github.com/user-attachments/assets/c194e122-f442-4745-8989-2d4840c2d8d3">

- Open <i> Security </i> tab
<img width="1233" alt="image" src="https://github.com/user-attachments/assets/4d6e9a60-c62a-4234-8f7a-ce8f07caeff7">

- Hit Security Groups, which opens a new browser tab
<img width="1150" alt="image" src="https://github.com/user-attachments/assets/4d3cdca1-076f-4d6d-9cab-9a1d6ae1e39c">

- Hit the value under <i> Security group ID </i>
<img width="480" alt="image" src="https://github.com/user-attachments/assets/4d81375e-415f-409e-926c-548fb2f5c9ee">

- Under the <i> inbound rules </i>, open <i> Edit inbound rules </i> button
<img width="1640" alt="image" src="https://github.com/user-attachments/assets/b22c5546-0908-454a-8642-98f2e92fe523">

- Add rules so that Port 22 is accessible from your local machine and all ports (0-65535) are accessible from the private IP addresses of other two instances
<img width="1749" alt="image" src="https://github.com/user-attachments/assets/befaa0a7-99bb-4add-9c1a-3ea1d739627f">

- Repeat this process till you have applied these configurations to all three instances

## 4) Access instance

##### With these steps so far, you should be able to access the EC2 instance and are ready to launch the secrecy app
Try running the command below in your terminal:
```
ssh -i /path/to/your/key.pem ec2-user@public-IP
```

##### If you successfully ssh into your instance, run below:

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
&& mkdir build && cd build && cmake .. && make \
&& echo -e "<private IP of this instance>\n<private IP of another instance>\n<private IP of last instance>" > hostfile.txt
```
 
##### Repeat these steps in the other two instances.

Run below 

```
ssh-keygen -t rsa -N "" -f ~/.ssh/id_rsa
```

```
ssh-copy-id -i ~/.ssh/id_rsa.pub ec2-user@<IP_ADDRESS>
```



