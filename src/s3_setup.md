# AWS S3 Setup Guide

This guide provides step-by-step instructions for setting up AWS S3 to work with an EC2 Secrecy instance. 
You'll learn how to create an S3 bucket for user input, set up IAM roles, and add permissions to an existing S3 bucket for output.

## Step 1: Create an S3 Bucket for User Input

1. **Log in to the AWS Management Console**  
   Open the AWS Management Console and navigate to the S3 service.

2. **Create a New S3 Bucket**  
   - Click on the "Create bucket" button.
   - Enter a unique name for your bucket (e.g., `secrecy-bucket1`).
   - Click "Create bucket."

## Step 2: Create an IAM Role for EC2 to Access S3

1. **Navigate to the IAM Service**  
   Go to the AWS Management Console and navigate to the **IAM** service.

2. **Create a New IAM Role**  
   - Click on "Roles" in the sidebar and then click the "Create role" button.
   - Choose **AWS service** and then **EC2** as the trusted entity.
   - Click "Next: Permissions."

3. **Attach S3 Full Access Policy**  
   - In the permissions step, search for `AmazonS3FullAccess`.
   - Select the checkbox next to `AmazonS3FullAccess` to grant full access to S3.
   - Click click "Next: Review."

4. **Review and Create Role**  
   - Enter a name for your role (e.g., `Secrecy-role1`).
   - Review the settings and click "Create role."

5. **Attach IAM Role to EC2 Instance**  
   - Go to the **EC2 Dashboard** in the AWS Management Console.
   - Select your EC2 instance.
   - Click on "Actions" > "Security" > "Modify IAM Role."
   - Choose the newly created IAM role (`EC2-S3-Access-Role`) and click "Update IAM Role."

## Step 3: Add Permissions to an Existing S3 Bucket for Output

You have an existing S3 bucket (e.g., `secrecy-bucket1`) and want to allow your EC2 instance to access it:

1. **Navigate to the IAM Role**  
   Go back to the **IAM** service and select the `EC2-S3-Access-Role` you created earlier.

2. **Add Inline Policy for Specific Bucket Access**  
   - Under the **Permissions** tab, click **Add inline policy**.
   - Choose the **JSON** tab on the right and enter the following policy:

   ```json
   {
       "Version": "2012-10-17",
       "Statement": [
           {
               "Effect": "Allow",
               "Action": [
                   "s3:PutObject",
                   "s3:GetObject",
                   "s3:DeleteObject",
                   "s3:ListBucket"
               ],
               "Resource": [
                   "arn:aws:s3:::your-bucket",
                   "arn:aws:s3:::your-bucket/*",
                   "arn:aws:s3:::output-bucket",
                   "arn:aws:s3:::output-bucket/*"
               ]
           }
       ]
   }
