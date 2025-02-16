#ifndef ALU_BLOCKCHAIN_H
#define ALU_BLOCKCHAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <regex.h>

#define MAX_EMAIL 100
#define MAX_NAME 100
#define HASH_LENGTH 64
#define MAX_TRANSACTIONS 100
#define TOKEN_NAME "Leaders Token"
#define TOKEN_SYMBOL "LT"
#define INITIAL_SUPPLY 1000000
#define BLOCK_REWARD 50

/* User Types */
typedef enum {
    STUDENT,
    STAFF,
    INTERN,
    VENDOR
} UserType;

/* Transaction Types */
typedef enum {
    TUITION_FEE,
    CAFETERIA_PAYMENT,
    LIBRARY_FINE,
    TOKEN_TRANSFER
} TransactionType;

/* Student Structure */
typedef struct {
    char email[MAX_EMAIL];
    char name[MAX_NAME];
    int student_id;
    double balance;
} Student;

/* Wallet Structure */
typedef struct {
    char address[HASH_LENGTH + 1];
    char private_key[HASH_LENGTH + 1];
    double balance;
    char email[MAX_EMAIL];
    UserType user_type;
} Wallet;

/* Vendor Structure */
typedef struct {
    char email[MAX_EMAIL];
    char name[MAX_NAME];
    int vendor_id;
    double balance;
    char service_type[50];
} Vendor;

/* Token Structure */
typedef struct {
    char token_name[50];
    char symbol[5];
    unsigned int total_supply;
    unsigned int circulating_supply;
} Token;

/* Transaction Structure */
typedef struct {
    char from_address[HASH_LENGTH + 1];
    char to_address[HASH_LENGTH + 1];
    double amount;
    TransactionType type;
    time_t timestamp;
    char signature[HASH_LENGTH + 1];
} Transaction;

/* Block Structure */
typedef struct Block {
    unsigned int index;
    char previous_hash[HASH_LENGTH + 1];
    char timestamp[30];
    unsigned int nonce;
    Transaction transactions[MAX_TRANSACTIONS];
    int transaction_count;
    char current_hash[HASH_LENGTH + 1];
    struct Block* next;
} Block;

/* Blockchain Structure */
typedef struct {
    Block* genesis;
    Block* latest;
    int block_count;
    Token token;
} Blockchain;

#define SCHOOL_TUITION_ADDRESS "0000000000000000000000000000000000000000000000000000000000000001"
#define SCHOOL_LIBRARY_ADDRESS "0000000000000000000000000000000000000000000000000000000000000002"
#define CAFETERIA_ADDRESS     "0000000000000000000000000000000000000000000000000000000000000003"

/* Function Prototypes */
Blockchain* initialize_blockchain(void);
int verify_email_domain(const char* email);
Wallet* create_wallet(const char* email);
Wallet* load_wallet(const char* private_key);
int create_transaction(Blockchain* chain, Wallet* from, 
                      const char* to_address, double amount,
                      TransactionType type);
int validate_chain(Blockchain* chain);
void cleanup_blockchain(Blockchain* chain);
void print_transaction_history(Blockchain* chain, Wallet* wallet);

#endif /* ALU_BLOCKCHAIN_H */