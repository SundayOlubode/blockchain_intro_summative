# ALU Private Blockchain Network

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
