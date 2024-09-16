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
        echo "S3 access failed for bucket: $S3_BUCKET_NAME"
    fi
fi

# Check Ping to successor VPC
echo "Pinging peer instance (successor) with IP: $SUCC"
ping -c 4 $SUCC > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "Ping successful to peer instance (successor) at IP: $SUCC"
else
    echo "Ping failed to peer instance (successor) at IP: $SUCC"
fi

# Check Ping to predecessor VPC
echo "Pinging peer instance (predecessor) with IP: $PRED"
ping -c 4 $PRED > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "Ping successful to peer instance (predecessor) at IP: $PRED"
else
    echo "Ping failed to peer instance (predecessor) at IP: $PRED"
fi


if [ "$ROLE" -eq 1 ]; then
    # Create hostfile.txt and store the values
    echo -e "$MINE\n$SUCC\n$PRED" > hostfile.txt
    # Confirm where the file has been created
    echo "File hostfile.txt created with the following contents:"
    cat hostfile.txt
fi