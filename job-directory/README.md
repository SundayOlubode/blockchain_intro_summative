# Job Blockchain System

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
