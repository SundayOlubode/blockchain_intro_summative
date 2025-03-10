#ifndef ALU_BLOCKCHAIN_H
#define ALU_BLOCKCHAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdbool.h>

/* Constants */
#define MAX_EMAIL 100
#define MAX_NAME 100
#define HASH_LENGTH 65
#define MAX_TRANSACTIONS 100
#define PROFILES_FILE "profiles.dat"
#define WALLETS_FILE "wallets.dat"
#define TX_FILE "transactions.dat"
#define TX_POOL "txpool.dat"
#define NUM_KITCHENS 10

/* Token definitions */
#define TOKEN_NAME "Leaders Token"
#define TOKEN_SYMBOL "LT"
#define INITIAL_SUPPLY 1000000
#define CIRCULATING_SUPPLY 1000000
#define BLOCK_REWARD 2

/* Domain definitions */
#define STUDENT_DOMAIN "@alustudent.com"
#define STAFF_DOMAIN "@alueducation.com"
#define VENDOR_DOMAIN "@aluvendor.com"

/* Vendor*/
#define Pius_Cuisine "Pius Cuisine"
#define Joshua_Kitchen "Joshua's Kitchen"
#define Pascal_Kitchen "Pascal's Kitchen"

/* Vendor Address */
#define Pius_Cuisine_ADDRESS "0x000pius000000cuisine000000"
#define Joshua_Kitchen_ADDRESS "0x000joshua000000kitchen000000"
#define Pascal_Kitchen_ADDRESS "0x000pascal000000kitchen000000"

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
        INTERN,
        INSTITUTION
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

typedef struct VendorProfile
{
        int vendor_id;
        char kitchen_name[50];
        char email[50];
        char wallet_address[HASH_LENGTH];
        double balance;
        struct VendorProfile *next;
} VendorProfile;

extern VendorProfile *vendor_list; // Declare, but don't initialize

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
        unsigned int reward;
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
        char name[MAX_NAME];
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
        char name[MAX_NAME];
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

/* Function prototypes */
void generate_hash(const char *input, char *output);
Blockchain *initialize_blockchain(void);
int verify_email_domain(const char *email);
Wallet *create_wallet(const char *email, const char *kitchen_name);
Wallet *load_wallet_by_key(const char *private_key);
int initiate_transaction(Blockchain *chain, Wallet *from,
                         const char *to_address, double amount,
                         TransactionType type);
int validate_chain(Blockchain *chain);
void cleanup_blockchain(Blockchain *chain);
void print_transaction_history(Wallet *wallet);
Wallet *load_wallet_by_public_key(const char *public_key);
int update_wallet_record(const Wallet *updated_wallet);
int create_institutional_wallets(void);
int add_transaction(Block *new_block, Transaction *transaction);
Block *create_block(Blockchain *chain);
int validate_block(Blockchain *chain, Block *block);
Wallet *select_validator();
void print_blockchain(const Blockchain *chain);
Transaction *extract_transactions();
Wallet *reload_wallet(Wallet *current_wallet);
int create_vendor_wallets(void);
double get_unspent_balance(const char *address);
Block *mine_block(Blockchain *chain);
void add_kitchen(const char *kitchen_name, const char *email, const char *wallet_address);
VendorProfile *get_kitchen_vendor(int kitchen_index);
StudentProfileWithWallet *create_student_profile(const char *email);
StaffProfileWithWallet *create_staff_profile(const char *email);
VendorProfileWithWallet *create_vendor_profile(const char *name, const char *email);
int save_profiles_to_file(void);
int load_profiles_from_file(void);
int check_email_exists(const char *email);
UserType get_user_type_from_email(const char *email);
int save_wallet(const char *email, const char *private_key,
                const char *address, const char *kitchen_name);
void display_menu(void);
void display_payment_menu(void);
void display_cafeteria_menu(void);
void get_string_input(const char *prompt, char *buffer, size_t size);
void clear_input_buffer(void);
int process_payment(Blockchain *chain, Wallet *wallet);
int get_recipient_address(char *to_address, const char **recipient_name);
Wallet *load_wallet_by_email(const char *email);

#endif /* ALU_BLOCKCHAIN_H */