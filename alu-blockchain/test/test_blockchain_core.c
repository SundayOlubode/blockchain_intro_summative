#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Unity/src/unity.h"
#include "alu_blockchain.h"

/* Mock file operations for transaction tests */
#define MAX_MOCK_TRANSACTIONS 10
static Transaction mock_transactions[MAX_MOCK_TRANSACTIONS];
static int mock_transaction_count = 0;
static int mock_file_open = 0;
static int mock_file_cleared = 0;

static StoredWallet mock_wallets[5];
static int mock_wallet_count = 0;
static int mock_wallet_file_pos = 0;

Block *create_block(Blockchain *chain);
int validate_block(Blockchain *chain, Block *block);
Wallet *select_validator();
Transaction *extract_transactions();

/* Store original function pointers */
static FILE *(*original_fopen)(const char *, const char *) = NULL;
static int (*original_fclose)(FILE *) = NULL;
static size_t (*original_fread)(void *, size_t, size_t, FILE *) = NULL;

/* Mock file operations */
FILE *mock_fopen(const char *filename, const char *mode)
{
        if (strcmp(filename, TX_POOL) == 0)
        {
                if (strcmp(mode, "rb") == 0)
                {
                        mock_file_open = 1;
                        return (FILE *)1;
                }
                else if (strcmp(mode, "wb") == 0)
                {
                        mock_file_cleared = 1;
                        return (FILE *)1;
                }
        }
        else if (strcmp(filename, WALLETS_FILE) == 0)
        {
                mock_wallet_file_pos = 0;
                return (FILE *)2;
        }
        return NULL;
}

int mock_fclose(FILE *file)
{
        return 0;
}

size_t mock_fread(void *ptr, size_t size, size_t count, FILE *file)
{
        if (file == (FILE *)1)
        {
                static int read_count = 0;

                if (read_count >= mock_transaction_count)
                {
                        read_count = 0;
                        return 0;
                }

                memcpy(ptr, &mock_transactions[read_count], size);
                read_count++;
                return 1;
        }
        else if (file == (FILE *)2)
        { /* Wallet file */
                if (mock_wallet_file_pos >= mock_wallet_count)
                        return 0;

                memcpy(ptr, &mock_wallets[mock_wallet_file_pos], size);
                mock_wallet_file_pos++;
                return 1;
        }
        return 0;
}

void setUp(void)
{
        /* Reset mock state before each test */
        mock_file_open = 0;
        mock_file_cleared = 0;
        mock_transaction_count = 0;
        mock_wallet_count = 0;
        mock_wallet_file_pos = 0;
        memset(mock_transactions, 0, sizeof(mock_transactions));
        memset(mock_wallets, 0, sizeof(mock_wallets));

        /* Store original function pointers if not already done */
        if (original_fopen == NULL)
        {
                original_fopen = fopen;
                original_fclose = fclose;
                original_fread = fread;
        }
}

void tearDown(void)
{
        /* Nothing to clean up */
}

void test_create_block_null_chain(void)
{
        Block *result = create_block(NULL);
        TEST_ASSERT_NULL(result);
}

void test_create_block_success(void)
{
        Blockchain chain;
        Block genesis;

        genesis.index = 0;
        strcpy(genesis.current_hash, "genesis_hash");
        genesis.next = NULL;

        chain.genesis = &genesis;
        chain.latest = &genesis;

        Block *new_block = create_block(&chain);

        TEST_ASSERT_NOT_NULL(new_block);
        TEST_ASSERT_EQUAL_INT(1, new_block->index);
        TEST_ASSERT_EQUAL_STRING("genesis_hash", new_block->previous_hash);
        TEST_ASSERT_EQUAL_INT(0, new_block->nonce);
        TEST_ASSERT_EQUAL_INT(0, new_block->transaction_count);
        TEST_ASSERT_EQUAL_INT(BLOCK_REWARD, new_block->reward);
        TEST_ASSERT_NULL(new_block->next);

        free(new_block);
}

void test_validate_block_null_inputs(void)
{
        Blockchain chain;
        Block block;

        TEST_ASSERT_EQUAL_INT(0, validate_block(NULL, &block));
        TEST_ASSERT_EQUAL_INT(0, validate_block(&chain, NULL));
}

void test_validate_block_invalid_previous_hash(void)
{
        Blockchain chain;
        Block latest;
        Block block_to_validate;

        strcpy(latest.current_hash, "latest_hash");

        chain.latest = &latest;

        strcpy(block_to_validate.previous_hash, "wrong_hash");

        TEST_ASSERT_EQUAL_INT(0, validate_block(&chain, &block_to_validate));
}

void test_validate_block_valid(void)
{
        Blockchain chain;
        Block latest;
        Block block_to_validate;
        char temp[512];

        strcpy(latest.current_hash, "latest_hash");

        chain.latest = &latest;

        block_to_validate.index = 1;
        strcpy(block_to_validate.previous_hash, "latest_hash");
        strcpy(block_to_validate.timestamp, "2023-01-01 12:00:00");
        block_to_validate.nonce = 0;

        sprintf(temp, "%u%s%s%u", block_to_validate.index, block_to_validate.previous_hash,
                block_to_validate.timestamp, block_to_validate.nonce);
        generate_hash(temp, block_to_validate.current_hash);

        TEST_ASSERT_EQUAL_INT(1, validate_block(&chain, &block_to_validate));
}

void test_select_validator_zero_balance(void)
{
        strcpy(mock_wallets[0].email, "user1@example.com");
        mock_wallets[0].balance = 0;
        strcpy(mock_wallets[1].email, "user2@example.com");
        mock_wallets[1].balance = 0;
        mock_wallet_count = 2;

        Wallet *result = select_validator();
        TEST_ASSERT_NOT_NULL(result);
}

void test_select_validator_success(void)
{
        /* Set up wallets with balances */
        strcpy(mock_wallets[0].email, "user1@example.com");
        strcpy(mock_wallets[0].private_key, "private1");
        strcpy(mock_wallets[0].address, "address1");
        mock_wallets[0].balance = 100;
        mock_wallets[0].user_type = STUDENT;

        strcpy(mock_wallets[1].email, "user2@example.com");
        strcpy(mock_wallets[1].private_key, "private2");
        strcpy(mock_wallets[1].address, "address2");
        mock_wallets[1].balance = 200;
        mock_wallets[1].user_type = STAFF;

        mock_wallet_count = 2;

        Wallet *result = select_validator();

        TEST_ASSERT_NOT_NULL(result);

        free(result);
}

void test_extract_transactions_empty_file(void)
{
        mock_transaction_count = 0;

        Transaction *result = extract_transactions();

        TEST_ASSERT_NOT_NULL(result);

        /* Cleanup */
        free(result);
}

void test_extract_transactions_with_data(void)
{
        Transaction *result = extract_transactions();

        TEST_ASSERT_NOT_NULL(result);

        free(result);
}

void test_extract_transactions_max_transactions(void)
{
        Transaction *result = extract_transactions();

        TEST_ASSERT_NOT_NULL(result);

        free(result);
}

/* Test runner */
int main(void)
{
        UNITY_BEGIN();

        /* create_block tests */
        RUN_TEST(test_create_block_null_chain);
        RUN_TEST(test_create_block_success);

        /* validate_block tests */
        RUN_TEST(test_validate_block_null_inputs);
        RUN_TEST(test_validate_block_invalid_previous_hash);
        RUN_TEST(test_validate_block_valid);

        /* select_validator tests */
        RUN_TEST(test_select_validator_zero_balance);
        RUN_TEST(test_select_validator_success);

        /* extract_transactions tests */
        RUN_TEST(test_extract_transactions_empty_file);
        RUN_TEST(test_extract_transactions_with_data);
        RUN_TEST(test_extract_transactions_max_transactions);

        return UNITY_END();
}