# 1. Job Blockchain System

A secure job listing management system implementing blockchain principles to ensure data integrity.

## How to Compile and Run

1. Navigate to the job-directory:

   ```
   cd job-directory
   ```

2. Run the commands.sh script:

   ```
   ./commands.sh
   ```

   If the script is not executable, make it executable first:

   ```
   chmod 755 commands.sh
   ```

3. The program will compile and run automatically.

## Blockchain Implementation

This application implements core blockchain principles by:

- Structuring job listings as blocks in a chain
- Using cryptographic hashing to create unique fingerprints of each block
- Linking blocks sequentially with each block referencing the previous block's hash
- Providing integrity verification to detect any tampering with the chain

## Hashing Mechanism

The system uses SHA-256 hashing (via OpenSSL) to ensure data integrity:

- Each job block's hash is calculated from its content (job details, timestamp) and the previous block's hash
- Any change to a block's data would result in a completely different hash value
- The verification process can detect tampering by recalculating hashes and comparing them to stored values
- The chain integrity is verified by ensuring each block's previous hash matches the actual hash of the previous block

## Usage

The application provides a simple menu-driven interface with options to:

1. Add new job listings
2. View all job listings
3. Search jobs by keyword
4. Verify blockchain integrity
5. Exit the program

## Files

- `blockchain_job.h`: Header file with structure definitions and function prototypes
- `blockchain_job.c`: Implementation of the blockchain functionality
- `main.c`: Main program with user interface
- `commands.sh`: Script to compile and run the program

## Requirements

- GCC compiler
- OpenSSL development libraries

#

# 2. ALU Private Blockchain Network

A blockchain-based payment system for the ALU community that enables secure transactions using Leaders Token (LT).

## How to Compile and Run

1. Navigate to the alu-blockchain directory:

   ```
   cd alu-blockchain
   ```

2. Run the command.sh script:

   ```
   ./command.sh
   ```

   If the script is not executable, make it executable first:

   ```
   chmod 755 command.sh
   ```

3. The program will compile and run automatically.

## Features

- Wallet creation and management for students and vendors
- Secure transactions using blockchain technology
- Multiple payment types (Tuition, Cafeteria, Library, Health Insurance)
- Transaction history tracking
- Block mining with rewards
- Blockchain integrity verification
- Backup and restore functionality

## Blockchain Implementation

The system implements a private blockchain that:

- Records all transactions in immutable blocks
- Uses cryptographic hashing to secure the chain
- Maintains transaction history for all users
- Verifies integrity of the entire blockchain

## Usage

The application provides a menu-driven interface with options to:

1. Create a new wallet
2. Load an existing wallet
3. Initiate transactions
4. View balance
5. View transaction history
6. View blockchain details
7. Mine new blocks
8. Check blockchain status
9. Backup the blockchain
10. Restore from backup

## Special Accounts

- Pre-loaded student wallets
- Vendor wallets (e.g., Pius Cuisine, Joshua's Kitchen)
- System accounts for tuition, library, and other institutional services

## Requirements

- GCC compiler
- OpenSSL development libraries
