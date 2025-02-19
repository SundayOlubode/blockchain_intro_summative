#ifndef ALU_BLOCKCHAIN_H
#define ALU_BLOCKCHAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

/* Constants */
#define MAX_EMAIL 100
#define MAX_NAME 100
#define HASH_LENGTH 64
#define MAX_TRANSACTIONS 100
#define PROFILES_FILE "profiles.dat"
#define WALLETS_FILE "wallets.dat"
#define NUM_KITCHENS 10

/* Token definitions */
#define TOKEN_NAME "Leaders Token"
#define TOKEN_SYMBOL "LT"
#define INITIAL_SUPPLY 1000000

/* Domain definitions */
#define STUDENT_DOMAIN "@alustudent.com"
#define STAFF_DOMAIN "@alueducation.com"
#define VENDOR_DOMAIN "@aluvendor.com"

/* Institutional addresses */
#define SCHOOL_TUITION_ADDRESS "0000000000000000000000000000000000000000000000000000000000000001"
#define SCHOOL_LIBRARY_ADDRESS "0000000000000000000000000000000000000000000000000000000000000002"
#define HEALTH_INSURANCE_ADDRESS "0000000000000000000000000000000000000000000000000000000000000003"

/* Enums */
typedef enum
{
        STUDENT,
        STAFF,
        VENDOR,
        INTERN
} UserType;

typedef enum
{
        TUITION_FEE,
        CAFETERIA_PAYMENT,
        LIBRARY_FINE,
        HEALTH_INSURANCE,
        TOKEN_TRANSFER
} TransactionType;

/* Basic Structures */
typedef struct
{
        unsigned int student_id;
        char name[MAX_NAME];
        char email[MAX_EMAIL];
        int year_of_study;
        char program[50];
        char wallet_address[HASH_LENGTH + 1];
} StudentProfile;

typedef struct
{
        unsigned int staff_id;
        char name[MAX_NAME];
        char email[MAX_EMAIL];
        char department[50];
        char role[30];
        char wallet_address[HASH_LENGTH + 1];
} StaffProfile;

typedef struct
{
        unsigned int vendor_id;
        char kitchen_name[MAX_NAME];
        char email[MAX_EMAIL];
        char wallet_address[HASH_LENGTH + 1];
        double balance;
} VendorProfile;

/* Token Structure */
typedef struct
{
        char token_name[50];
        char symbol[5];
        unsigned int total_supply;
        unsigned int circulating_supply;
} Token;

/* Blockchain related structures */
typedef struct
{
        char from_address[HASH_LENGTH + 1];
        char to_address[HASH_LENGTH + 1];
        double amount;
        TransactionType type;
        time_t timestamp;
        char signature[HASH_LENGTH + 1];
} Transaction;

typedef struct Block
{
        unsigned int index;
        char previous_hash[HASH_LENGTH + 1];
        char timestamp[30];
        unsigned int nonce;
        Transaction transactions[MAX_TRANSACTIONS];
        int transaction_count;
        char current_hash[HASH_LENGTH + 1];
        struct Block *next;
} Block;

typedef struct
{
        Block *genesis;
        Block *latest;
        int block_count;
        struct
        {
                char token_name[50];
                char symbol[5];
                unsigned int total_supply;
                unsigned int circulating_supply;
        } token;
} Blockchain;

/* Wallet structures */
typedef struct
{
        char address[HASH_LENGTH + 1];
        char private_key[HASH_LENGTH + 1];
        char email[MAX_EMAIL];
        double balance;
        UserType user_type;
} Wallet;

typedef struct
{
        char email[MAX_EMAIL];
        char private_key[HASH_LENGTH + 1];
        char address[HASH_LENGTH + 1];
        double balance;
        UserType user_type;
        char kitchen_name[MAX_NAME];
} StoredWallet;

typedef struct
{
        StudentProfile profile;
        Wallet wallet;
} StudentProfileWithWallet;

typedef struct
{
        StaffProfile profile;
        Wallet wallet;
} StaffProfileWithWallet;

typedef struct
{
        VendorProfile profile;
        Wallet wallet;
} VendorProfileWithWallet;

const VendorProfile *get_kitchen_vendor(int index);
void print_kitchen_info(const VendorProfile *kitchen);

/* Function prototypes */
void generate_hash(const char *input, char *output);
Blockchain *initialize_blockchain(void);
int verify_email_domain(const char *email);
Wallet *create_wallet(const char *email, const char *kitchen_name);
Wallet *load_wallet_by_key(const char *private_key);
int create_transaction(Blockchain *chain, Wallet *from,
                       const char *to_address, double amount,
                       TransactionType type);
int validate_chain(Blockchain *chain);
void cleanup_blockchain(Blockchain *chain);
void print_transaction_history(Blockchain *chain, Wallet *wallet);

/* Profile management */
StudentProfileWithWallet *create_student_profile(const char *name, const char *email,
                                                 int year, const char *program);
StaffProfileWithWallet *create_staff_profile(const char *name, const char *email,
                                             const char *department, const char *role);
VendorProfileWithWallet *create_vendor_profile(const char *name, const char *email);
int save_profiles_to_file(void);
int load_profiles_from_file(void);
int check_email_exists(const char *email);
UserType get_user_type_from_email(const char *email);
int save_wallet(const char *email, const char *private_key,
                const char *address, const char *kitchen_name);
Wallet *create_student_wallet(StudentProfile *profile, const char *name, const char *email,
                              int year, const char *program);

#endif /* ALU_BLOCKCHAIN_H */