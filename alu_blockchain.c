/* alu_blockchain.c */
#include "alu_blockchain.h"

/**
 * generate_hash - Generate SHA-256 hash
 * @input: Input string
 * @output: Output hash buffer
 */
void generate_hash(const char *input, char *output)
{
        EVP_MD_CTX *mdctx;
        const EVP_MD *md;
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hash_len;
        unsigned int i;

        mdctx = EVP_MD_CTX_new();
        md = EVP_sha256();

        EVP_DigestInit_ex(mdctx, md, NULL);
        EVP_DigestUpdate(mdctx, input, strlen(input));
        EVP_DigestFinal_ex(mdctx, hash, &hash_len);

        for (i = 0; i < hash_len; i++)
                sprintf(output + (i * 2), "%02x", hash[i]);
        output[64] = '\0';

        EVP_MD_CTX_free(mdctx);
}

/**
 * initialize_blockchain - Initialize new blockchain
 * Return: Pointer to new blockchain or NULL on failure
 */
Blockchain *initialize_blockchain(void)
{
        Blockchain *chain;
        Block *genesis;
        time_t now;
        char temp[512];

        chain = malloc(sizeof(Blockchain));
        if (!chain)
                return NULL;

        /* Initialize token */
        strncpy(chain->token.token_name, TOKEN_NAME, 49);
        strncpy(chain->token.symbol, TOKEN_SYMBOL, 4);
        chain->token.total_supply = INITIAL_SUPPLY;
        chain->token.circulating_supply = 0;

        /* Create genesis block */
        genesis = malloc(sizeof(Block));
        if (!genesis)
        {
                free(chain);
                return NULL;
        }

        genesis->index = 0;
        strcpy(genesis->previous_hash,
               "0000000000000000000000000000000000000000000000000000000000000000");
        time(&now);
        strftime(genesis->timestamp, 30, "%Y-%m-%d %H:%M:%S", localtime(&now));
        genesis->nonce = 0;
        genesis->transaction_count = 0;
        genesis->next = NULL;

        /* Calculate genesis block hash */
        sprintf(temp, "%u%s%s%u",
                genesis->index, genesis->previous_hash,
                genesis->timestamp, genesis->nonce);
        generate_hash(temp, genesis->current_hash);

        chain->genesis = genesis;
        chain->latest = genesis;
        chain->block_count = 1;

        return chain;
}

/**
 * verify_email_domain - Verify if email domain is valid
 * @email: Email to verify
 * Return: 1 if valid, 0 if invalid
 */
int verify_email_domain(const char *email)
{
        const char *domain;

        if (!email)
                return 0;

        domain = strstr(email, "@");
        if (!domain)
                return 0;

        if (strcmp(domain, STUDENT_DOMAIN) == 0 ||
            strcmp(domain, STAFF_DOMAIN) == 0 ||
            strcmp(domain, VENDOR_DOMAIN) == 0)
                return 1;

        return 0;
}

// /**
//  * create_wallet - Create new wallet
//  * @email: User email
//  * Return: New wallet or NULL on failure
//  */
// Wallet *create_wallet(const char *email)
// {
//         Wallet *wallet;
//         char temp[MAX_EMAIL + 20];
//         time_t now;

//         if (!verify_email_domain(email))
//                 return NULL;

//         wallet = malloc(sizeof(Wallet));
//         if (!wallet)
//                 return NULL;

//         strncpy(wallet->email, email, MAX_EMAIL - 1);
//         wallet->email[MAX_EMAIL - 1] = '\0';
//         wallet->balance = 100.0; /* Initial balance for testing */

//         /* Generate wallet address */
//         time(&now);
//         sprintf(temp, "%s%ld", email, now);
//         generate_hash(temp, wallet->address);

//         /* Generate private key */
//         sprintf(temp, "%s%ld%s", email, now, wallet->address);
//         generate_hash(temp, wallet->private_key);

//         return wallet;
// }

/**
 * create_transaction - Create new transaction
 * @chain: Blockchain
 * @from: Sender wallet
 * @to_address: Recipient address
 * @amount: Transaction amount
 * @type: Transaction type
 * Return: 1 on success, 0 on failure
 */
int create_transaction(Blockchain *chain, Wallet *from,
                       const char *to_address, double amount,
                       TransactionType type)
{
        Transaction transaction;
        char temp[256];
        Block *latest;

        if (!chain || !from || !to_address || amount <= 0 || from->balance < amount)
                return 0;

        latest = chain->latest;
        if (!latest || latest->transaction_count >= MAX_TRANSACTIONS)
                return 0;

        strncpy(transaction.from_address, from->address, HASH_LENGTH);
        strncpy(transaction.to_address, to_address, HASH_LENGTH);
        transaction.amount = amount;
        transaction.type = type;
        time(&transaction.timestamp);

        /* Generate transaction signature */
        sprintf(temp, "%s%s%.2f%ld",
                transaction.from_address, transaction.to_address,
                amount, transaction.timestamp);
        generate_hash(temp, transaction.signature);

        latest->transactions[latest->transaction_count++] = transaction;
        return 1;
}

/**
 * validate_chain - Validate blockchain integrity
 * @chain: Blockchain to validate
 * Return: 1 if valid, 0 if compromised
 */
int validate_chain(Blockchain *chain)
{
        Block *current;
        char temp[512];
        char calc_hash[HASH_LENGTH + 1];
        unsigned int i;

        if (!chain || !chain->genesis)
                return 0;

        current = chain->genesis;
        while (current->next != NULL)
        {
                /* Verify link to next block */
                if (strcmp(current->next->previous_hash, current->current_hash) != 0)
                        return 0;

                /* Verify current block's hash */
                sprintf(temp, "%u%s%s%u",
                        current->index, current->previous_hash,
                        current->timestamp, current->nonce);
                generate_hash(temp, calc_hash);

                if (strcmp(calc_hash, current->current_hash) != 0)
                        return 0;

                /* Verify transactions in block */
                for (i = 0; i < (unsigned int)current->transaction_count; i++)
                {
                        /* Add transaction verification if needed */
                        (void)i; /* Suppress unused variable warning */
                }

                current = current->next;
        }
        return 1;
}

/**
 * print_transaction_history - Print transaction history for a wallet
 * @chain: Blockchain
 * @wallet: Wallet to check transactions for
 */
void print_transaction_history(Blockchain *chain, Wallet *wallet)
{
        Block *current;
        Transaction *trans;
        int i;
        int found = 0;

        if (!chain || !wallet)
                return;

        current = chain->genesis;
        while (current != NULL)
        {
                for (i = 0; i < current->transaction_count; i++)
                {
                        trans = &current->transactions[i];
                        if (strcmp(trans->from_address, wallet->address) == 0 ||
                            strcmp(trans->to_address, wallet->address) == 0)
                        {
                                found = 1;
                                printf("\nBlock #%d\n", current->index);
                                printf("Type: ");
                                switch (trans->type)
                                {
                                case TUITION_FEE:
                                        printf("Tuition Fee\n");
                                        break;
                                case CAFETERIA_PAYMENT:
                                        printf("Cafeteria Payment\n");
                                        break;
                                case LIBRARY_FINE:
                                        printf("Library Fine\n");
                                        break;
                                case HEALTH_INSURANCE:
                                        printf("Health Insurance\n");
                                        break;
                                case TOKEN_TRANSFER:
                                        printf("Token Transfer\n");
                                        break;
                                }
                                printf("Amount: %.2f %s\n", trans->amount, TOKEN_SYMBOL);
                                printf("From: %.16s...\n", trans->from_address);
                                printf("To: %.16s...\n", trans->to_address);
                                printf("Time: %s", ctime(&trans->timestamp));
                        }
                }
                current = current->next;
        }

        if (!found)
                printf("No transactions found.\n");
}

/**
 * cleanup_blockchain - Free blockchain memory
 * @chain: Blockchain to cleanup
 */
void cleanup_blockchain(Blockchain *chain)
{
        Block *current;
        Block *next;

        if (!chain)
                return;

        current = chain->genesis;
        while (current)
        {
                next = current->next;
                free(current);
                current = next;
        }

        free(chain);
}