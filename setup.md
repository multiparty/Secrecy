# Secrecy OpenMPI Setup Guide

This document is a step-by-step guide to setting up a Secrecy Server and OPEN MPI implementation on AWS EC2 instances.

## Introduction

This guide navigates you through how to set up an AWS environment and get the MPC vehicle up and running.

### Prerequisites
- AWS Account
- SSH client installed on your local machine


## Before You Start
**Designate each party to roles 1, 2, and 3**
This step is particularly important to avoid confusion. You will see why in a second.

## 1) Create VPC 

1. Navigate to the AWS VPC console and hit **Create VPC**.

<img width="800" alt="image" src="https://github.com/user-attachments/assets/8398b2cb-77fe-4eb9-b263-207eab301219">

2. Select **VPC and more**.
3. Name your VPC in the following manner:
   - role-1: secrecy1
   - role-2: secrecy2
   - role-3: secrecy3
4. Pick the respective IPv4 CIDR block as follows:
   - role-1: 10.0.0.0/16
   - role-2: 10.1.0.0/16
   - role-3: 10.2.0.0/16
5. Create a VPC with **1 zone** and **public** and zero **private subnets**.

<table>
  <tr>
    <td><img src="https://github.com/user-attachments/assets/7c2d3bf1-17bc-425c-ac65-0f1701ab5564" alt="VPC Setup" width="400"></td>
    <td><img src="https://github.com/user-attachments/assets/e88406f0-382a-4cbb-b837-df9bafff6d60" alt="VPC Subnet Configuration" width="400"></td>
  </tr>
</table>

## 2) Launch EC2 Instance

<img src="https://github.com/user-attachments/assets/264abd86-06d2-44c7-bbbb-523a80ee6f86" alt="EC2 Instances" width="800">

1. Name Instance as follows:
   - role-1: secrecy1
   - role-2: secrecy2
   - role-3: secrecy3
   
   <img width="700" alt="image" src="https://github.com/user-attachments/assets/69ae4a3a-afef-487b-a447-9a478fd80d79">

2. Select Amazon Linux

3. Pick t2.micro as an instance size.

4. Generate a key pair if you haven't and save the key to your local machine.

   <img src="https://github.com/user-attachments/assets/da61f0c5-eeef-4df5-ad55-2a4e40936e79" alt="Key Pair" width="600">

5. Hit **Edit** in Network settings, pick the VPC you've just created in step 1:
   - role-1: secrecy1
   - role-2: secrecy2
   - role-3: secrecy3

7. Enable **Auto-assign public IP**

<table>
  <tr>
    <td><img src="https://github.com/user-attachments/assets/d6387771-ebb7-48d9-92e8-91e06d2c0431" alt="Network Settings" width="800"></td>
    <td><img width="689" alt="image" src="https://github.com/user-attachments/assets/e353478a-e39b-494e-83ad-a34e6bd1f314"></td>
  </tr>
</table>

## 3) Setup Subnets
1. Go to the VPC Dashboard
2. In the left-hand navigation pane, select **"Subnets"**.
3. Click **"Create subnet"**
<img width="800" alt="image" src="https://github.com/user-attachments/assets/a9dc2141-7f0f-4e34-8cbb-ef9db2714ae4">

4. Select your VPC from the drop-down. Then the following settings pane pops up.
  - Name subnet as follows:
   - role-1: SubScy1
   - role-2: SubScy2
   - role-3: SubScy3
  - Set Availability Zone 'us-east-1a'
  - IPv4 VPC CIDR block should be your VPC CIDR block
  - IPv4 subnet CIDR block should be set according to your role:
    - role-1: 10.0.16.0/20
    - role-2: 10.1.16.0/20
    - role-3: 10.2.16.0/20
  - Hit **"Create subnet"** 

## 4) Create VPC Peering Connection
**You are making only one connection**
Some people might get confused and try to create two connections because your instance communicates with two other instances. 
However, you only need to create **ONE** connection, as one of your two peers will also establish a connection with you.

The resulting connections will form a triangle, connecting all participants.

1. Go to the VPC Dashboard
2. In the left-hand navigation pane, select **"Peering Connections"**.
3. Click **"Create Peering Connection"**. Make sure that other parties have already created VPCs at this point
   <img width="800" alt="image" src="https://github.com/user-attachments/assets/4af09183-c24f-4789-8902-67d276199cbd">
4. Fill in parameters, 
  - Name: Give the Peering Connection a name as below:
      - role-1: secrecy12
      - role-2: secrecy23
      - role-3: secrecy31
  - VPC ID(Requester): Select your VPC
  - VPC ID(Accepter): Select in the following way:
      - role-1: secrecy2
      - role-2: secrecy3
      - role-3: secrecy1
  - If your partners/other parties use a separate AWS account, select "another account" and enter their Account ID.

5. Click **"Create Peering Connection"**
  <img width="500" alt="image" src="https://github.com/user-attachments/assets/befcb6a1-6a54-4bc4-8cef-a02e5e8e0176">

6. Go back to the **"Peering Connections"** Dashboard.
7. Select with a radio button your incoming Peer Connection request
      - role-1: secrecy31
      - role-2: secrecy12
      - role-3: secrecy23
8. Click **"Actions"** at the right top, and hit **Accept request**

## 5) Update Route Tables

** Ensure that everyone has created a peering connection before implementing this step

1. Go to the **Route Tables** section in the VPC Dashboard.
2. Select the Route Table associated with the subnets in each VPC from the list.
3. Click **Edit routes** in the Routes tab:
   <img width="800" alt="image" src="https://github.com/user-attachments/assets/68ab564c-138c-43b8-89ad-d87a1a257577">
4. Click **"Add route"** to add a new route:
   - **Destination**: The CIDR block of the peered VPC
   - Add the CIDR blocks in the following way:
     - role-1:
       - 10.1.0.0/16
       - 10.2.0.0/16
     - role-2:
       - 10.0.0.0/16
       - 10.2.0.0/16
     - role-3:
       - 10.0.0.0/16
       - 10.1.0.0/16

   - **Target**: Select the Peering Connection and select corresponding subnets (e.g., `pcx-xxxxxx`) 
     <img width="800" alt="image" src="https://github.com/user-attachments/assets/6da851d3-17b4-4bf3-b27b-de6a4a97f3c8">
6. Click **Save routes**.

## 6) Update Security Groups and Network ACLs
1. Go to the EC2 Dashboard.
2. Select your instance, choose the **Security** tab, and hit the pop-up link.
   <img width="800" alt="image" src="https://github.com/user-attachments/assets/48931b07-9a17-4a8f-bfbb-81e7ff2f96f7">
3. This will take you to Security Groups Dashboard. Click on the Security Group ID
   <img width="800" alt="image" src="https://github.com/user-attachments/assets/35dba1ee-1cd4-4fb9-a05b-f1ced5a78436">
4. Click **"Edit inbound rules"**
6. Add two inbound rules to allow traffic from the peered VPC’s CIDR blocks:
   - **Type**: Select `All TCP`
   - **Source**: Enter the CIDR block of the peered VPC
     - role-1:
       -  10.1.0.0/16
       -  10.2.0.0/16
     - role-2:
       -  10.0.0.0/16
       -  10.2.0.0/16
     - role-3:
       -  10.0.0.0/16
       -  10.1.0.0/16

## 7) Access Instance and Network Configuration

With these steps so far, you should be able to access the EC2 instance and are ready to launch the Secrecy app.

1. Exchange Private IP Address with other participants

2. Try running the command below in your terminal:

```
ssh -i /path/to/your/key.pem ec2-user@your-public-IP
```

3. If you successfully SSH into your instance, run the following commands:

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
&& mkdir build && cd build && cmake .. && make -j
```
  
4. Generate an SSH key pair on each instance by running the following command:

```
ssh-keygen -t rsa -N "" -f ~/.ssh/id_rsa
```

and below command will show your pubic key just generated:

```
cat ~/.ssh/id_rsa.pub
```

5. Exchange public keys with other participants. To grant them access, add their public keys to your authorized_keys file.

```
nano ~/.ssh/authorized_keys
```

Then, paste their public keys by pressing Command + V (on macOS) or the appropriate paste command for your operating system.

6. In order to enable agent forwarding and disable strict host key checking for all SSH connections, you can add the following configuration to your SSH config file by editing it with:

```
nano ~/.ssh/config

```

Then add:

```
Host *
    ForwardAgent yes
Host *
    StrictHostKeyChecking no
```
This allows you to use the SSH keys stored on your local machine to authenticate to other remote servers through an intermediary server, without having to copy your private key to the intermediary.


Make files accessible by:

```
chmod 600 ~/.ssh/id_rsa
chmod 600 ~/.ssh/config
```

## 8) Initiate MPI program
Designate one of two parties with a dataset as an initializing party, and only the initializing party executes the following steps.

You'll need to create a host file in the build directory to run the MPI process. You can do this automatically by running the following command:

```
 echo -e "<public IP of your instance>\n<public IP of another party's instance>\n<public IP of last instance>" > hostfile.txt
```
Make sure you replace <IPs>, before running the command.

Alternatively, you can create it manually by opening the file in a text editor:

```
nano hostfile.txt
```

The resulting hostfile should resemble the following format (The order matters):

<img width="600" alt="image" src="https://github.com/user-attachments/assets/0d37b700-cb77-4653-ae44-4c24021e4149">

Once the host file is prepared, the initiating party can run the Secrecy algorithm with the command:

```
mpirun -np 3 --hostfile hostfile.txt ./test_join_sail ./../sample1.csv ./../sample2.csv
```

