#!/bin/bash

# Get user input for role
read -p "Enter your role (type 1, 2, or 3): " ROLE
read -p "Enter your private IP: " MINE

# Based on the role, ask for the appropriate IPs
if [ "$ROLE" -eq 1 ]; then
    read -p "Next. Enter role2's private IP: " SUCC
    read -p "Finally, Enter role3's private IP: " PRED

    # Check S3 Access
    S3_BUCKET_NAME="secrecy-bucket1"

elif [ "$ROLE" -eq 2 ]; then
    read -p "Next. Enter role3's private IP: " SUCC
    read -p "Finally, Enter role1's private IP: " PRED

    # Check S3 Access
    S3_BUCKET_NAME="secrecy-bucket2"

elif [ "$ROLE" -eq 3 ]; then
    read -p "Next. Enter role1's private IP: " SUCC
    read -p "Finally, Enter role2's private IP: " PRED

    # No S3 access needed for role 3 as per your logic

else
    echo "Invalid role! Please enter 1, 2, or 3."
    exit 1
fi

# Check S3 Access (for roles 1 and 2 only)
if [ "$ROLE" -eq 1 ] || [ "$ROLE" -eq 2 ]; then
    echo "Checking S3 access..."
    aws s3 ls s3://$S3_BUCKET_NAME > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "S3 access confirmed for bucket: $S3_BUCKET_NAME"
    else
        echo "S3 access failed for bucket: $S3_BUCKET_NAME. Please name the bucket name as instructed in the guide"
        exit 1
    fi
fi

check_connection() {
    local IP=$1
    local ROLE=$2

    RESULT=$(nmap -p 22 $IP 2>&1)

    # Check if nmap output contains certain strings that indicate a failure
    if echo "$RESULT" | grep -q "Error\|illegal\|down"; then
        echo "Connection check failed to peer role ($ROLE) at IP: $IP. Ensure that the EC2 instance, VPC, Peering Connection, and IP are correct."
        exit 1
    else
        echo "Connection check successful to peer role ($ROLE) at IP: $IP"
    fi
}

# Check Ping to successor VPC
check_connection "$SUCC" "successor"
check_connection "$PRED" "predecessor"

if [ "$ROLE" -eq 1 ]; then
    # Create hostfile.txt and store the values
    echo -e "$MINE\n$SUCC\n$PRED" > hostfile.txt
    # Confirm where the file has been created
    echo "File hostfile.txt created with the following IPs:"
    cat hostfile.txt
fi