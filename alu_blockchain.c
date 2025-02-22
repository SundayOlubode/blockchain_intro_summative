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
 * create_transaction - Create new transaction and store it in TX_FILE
 * @chain: Blockchain
 * @from: Sender wallet
 * @to_address: Recipient address
 * @amount: Transaction amount
 * @type: Transaction type
 * Return: 1 on success, 0 on failure
 */
int create_transaction(Blockchain *chain, Wallet *from, const char *to_address, double amount, TransactionType type)
{
        Transaction transaction;
        Wallet *recipient;
        char temp[256];
        FILE *file;

        if (!chain || !from || !to_address || amount <= 0 || from->balance < amount)
                return 0;

        recipient = load_wallet_by_public_key(to_address);
        if (!recipient)
        {
                printf("Recipient wallet not found\n");
                return 0;
        }

        /* Prepare transaction record */
        strncpy(transaction.from_address, from->address, HASH_LENGTH);
        strncpy(transaction.to_address, to_address, HASH_LENGTH);
        transaction.amount = amount;
        transaction.type = type;
        time(&transaction.timestamp);

        /* Generate transaction signature */
        sprintf(temp, "%s%s%.2f%ld", transaction.from_address, transaction.to_address, amount, transaction.timestamp);
        generate_hash(temp, transaction.signature);

        /* Append transaction to TX_FILE */
        file = fopen(TX_FILE, "ab");
        if (!file)
        {
                printf("Error opening transactions file.\n");
                return 0;
        }
        fwrite(&transaction, sizeof(Transaction), 1, file);
        fclose(file);

        /* Save updated wallets */
        update_wallet_record(from);
        update_wallet_record(recipient);

        printf("Transaction successfully recorded.\n");
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
 * @wallet: Wallet to check transactions for
 */
void print_transaction_history(Wallet *wallet)
{
        FILE *file;
        Transaction transaction;
        int found = 0;

        if (!wallet)
                return;

        file = fopen(TX_FILE, "rb");
        if (!file)
        {
                printf("No transaction history available.\n");
                return;
        }

        printf("\nTransaction History for Wallet: %.16s...\n", wallet->address);

        while (fread(&transaction, sizeof(Transaction), 1, file))
        {
                if (strcmp(transaction.from_address, wallet->address) == 0 || strcmp(transaction.to_address, wallet->address) == 0)
                {
                        found = 1;
                        printf("\nTransaction Type: %s\n",
                               transaction.type == TUITION_FEE ? "Tuition Fee" : transaction.type == CAFETERIA_PAYMENT ? "Cafeteria Payment"
                                                                             : transaction.type == LIBRARY_FINE        ? "Library Fine"
                                                                             : transaction.type == HEALTH_INSURANCE    ? "Health Insurance"
                                                                                                                       : "Token Transfer");
                        printf("Amount: %.2f %s\n", transaction.amount, TOKEN_SYMBOL);
                        printf("From: %.16s...\n", transaction.from_address);
                        printf("To: %.16s...\n", transaction.to_address);
                        printf("Time: %s", ctime(&transaction.timestamp));
                        printf("Status: %s\n", strcmp(transaction.from_address, wallet->address) == 0 ? "Sent" : "Received");
                }
        }
        fclose(file);

        if (!found)
                printf("No transactions found for this wallet.\n");
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