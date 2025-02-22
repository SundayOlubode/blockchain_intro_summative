/* alu_blockchain.c */
#include "alu_blockchain.h"
#include "config.h"

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

// /**
//  * initialize_blockchain - Initialize new blockchain
//  * Return: Pointer to new blockchain or NULL on failure
//  */
// Blockchain *initialize_blockchain(void)
// {
//         Blockchain *chain;
//         Block *genesis;
//         time_t now;
//         char temp[512];

//         chain = malloc(sizeof(Blockchain));
//         if (!chain)
//                 return NULL;

//         /* Initialize token */
//         strncpy(chain->token.token_name, TOKEN_NAME, 49);
//         strncpy(chain->token.symbol, TOKEN_SYMBOL, 4);
//         chain->token.total_supply = INITIAL_SUPPLY;
//         chain->token.circulating_supply = CIRCULATING_SUPPLY;

//         /* Create genesis block */
//         genesis = malloc(sizeof(Block));
//         if (!genesis)
//         {
//                 free(chain);
//                 return NULL;
//         }

//         genesis->index = 0;
//         strcpy(genesis->previous_hash,
//                "0000000000000000000000000000000000000000000000000000000000000000");
//         time(&now);
//         strftime(genesis->timestamp, 30, "%Y-%m-%d %H:%M:%S", localtime(&now));
//         genesis->nonce = 0;
//         genesis->transaction_count = 0;
//         genesis->next = NULL;

//         /* Calculate genesis block hash */
//         sprintf(temp, "%u%s%s%u",
//                 genesis->index, genesis->previous_hash,
//                 genesis->timestamp, genesis->nonce);
//         generate_hash(temp, genesis->current_hash);

//         chain->genesis = genesis;
//         chain->latest = genesis;
//         chain->block_count = 1;

//         return chain;
// }

/**
 * initialize_blockchain - Initialize blockchain system
 * Return: Pointer to initialized blockchain
 */
Blockchain *initialize_blockchain(void)
{
        Blockchain *chain = NULL;

        /* Load configuration */
        Config *config = load_config();
        if (!config)
        {
                printf("Error: Failed to load config. Exiting...\n");
                return NULL;
        }

        /* Try to restore from backup */
        if (restore_blockchain(&chain))
        {
                printf("Blockchain restored from backup.\n");
        }
        else
        {
                /* No backup found, create new blockchain */
                printf("No backup found. Creating new blockchain...\n");

                chain = malloc(sizeof(Blockchain));
                if (!chain)
                {
                        printf("Error: Memory allocation failed.\n");
                        free(config);
                        return NULL;
                }

                /* Initialize genesis block */
                chain->genesis = malloc(sizeof(Block));
                if (!chain->genesis)
                {
                        printf("Error: Memory allocation failed.\n");
                        free(chain);
                        free(config);
                        return NULL;
                }

                /* Set genesis block properties */
                chain->block_count = 1;
                chain->latest = chain->genesis;
                chain->genesis->index = 0;
                chain->genesis->previous_hash[0] = '0000000000000000000000000000000000000000000000000000000000000000';
                chain->genesis->timestamp = time(NULL);
                chain->genesis->reward = config->block_reward;
                chain->genesis->transactions = 0;
                chain->genesis->next = NULL;

                /* Set initial token supply */
                chain->token.total_supply = config->initial_supply;
                chain->token.circulating_supply = CIRCULATING_SUPPLY;
                chain->token.token_name = TOKEN_NAME;

                /* Save blockchain as a new backup */
                backup_blockchain(chain);
        }

        free(config);
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

/**
 * add_transaction - Adds a transaction to the latest block
 * @chain: Blockchain
 * @transaction: Transaction to add
 * Return: 1 on success, 0 on failure
 */
int add_transaction(Blockchain *chain, Transaction *transaction)
{
        Block *latest;

        if (!chain || !transaction)
                return 0;

        latest = chain->latest;
        if (latest->transaction_count >= MAX_TRANSACTIONS)
                return 0;

        latest->transactions[latest->transaction_count++] = *transaction;
        return 1;
}

/**
 * create_block - Creates a new block and adds it to the blockchain
 * @chain: Blockchain
 * Return: Pointer to the new block, NULL on failure
 */
Block *create_block(Blockchain *chain)
{
        Block *new_block, *latest;
        time_t now;
        char temp[512];

        if (!chain || !chain->latest)
                return NULL;

        new_block = malloc(sizeof(Block));
        if (!new_block)
                return NULL;

        latest = chain->latest;
        new_block->index = latest->index + 1;
        strncpy(new_block->previous_hash, latest->current_hash, HASH_LENGTH);
        time(&now);
        strftime(new_block->timestamp, 30, "%Y-%m-%d %H:%M:%S", localtime(&now));
        new_block->nonce = 0;
        new_block->transaction_count = 0;
        new_block->next = NULL;

        /* Generate block hash */
        sprintf(temp, "%u%s%s%u", new_block->index, new_block->previous_hash,
                new_block->timestamp, new_block->nonce);
        generate_hash(temp, new_block->current_hash);

        /* Link new block to blockchain */
        latest->next = new_block;
        chain->latest = new_block;
        chain->block_count++;

        return new_block;
}

/**
 * validate_block - Validates a newly created block
 * @chain: Blockchain
 * @block: Block to validate
 * Return: 1 if valid, 0 if invalid
 */
int validate_block(Blockchain *chain, Block *block)
{
        char temp[512];
        char computed_hash[HASH_LENGTH + 1];

        if (!chain || !block)
                return 0;

        /* Check previous hash */
        if (strcmp(block->previous_hash, chain->latest->current_hash) != 0)
                return 0;

        /* Recompute hash */
        sprintf(temp, "%u%s%s%u", block->index, block->previous_hash,
                block->timestamp, block->nonce);
        generate_hash(temp, computed_hash);

        if (strcmp(computed_hash, block->current_hash) != 0)
                return 0;

        return 1;
}

/**
 * select_validator - Selects a validator based on PoS
 * @chain: Blockchain
 * Return: Pointer to the selected validator's wallet
 */
Wallet *select_validator()
{
        Wallet *selected_wallet = NULL;
        double total_balance = 0, random_value, cumulative_weight = 0;
        FILE *file;
        StoredWallet stored_wallet;
        // Wallet *current_wallet = NULL;

        // Open the wallets file
        file = fopen(WALLETS_FILE, "rb");
        if (!file)
                return NULL;

        // Calculate total balance of all wallets
        while (fread(&stored_wallet, sizeof(StoredWallet), 1, file))
        {
                total_balance += stored_wallet.balance;
        }
        fclose(file);

        if (total_balance == 0)
                return NULL;

        // Generate a random value within the total balance
        srand(time(NULL));
        random_value = ((double)rand() / RAND_MAX) * total_balance;

        // Reopen the file to traverse it again and select wallet
        file = fopen(WALLETS_FILE, "rb");
        if (!file)
                return NULL;

        // Select wallet based on probability weight
        while (fread(&stored_wallet, sizeof(StoredWallet), 1, file))
        {
                cumulative_weight += stored_wallet.balance;
                if (random_value <= cumulative_weight)
                {
                        selected_wallet = malloc(sizeof(Wallet));
                        if (selected_wallet)
                        {
                                strncpy(selected_wallet->email, stored_wallet.email, MAX_EMAIL - 1);
                                strncpy(selected_wallet->private_key, stored_wallet.private_key, HASH_LENGTH - 1);
                                strncpy(selected_wallet->address, stored_wallet.address, HASH_LENGTH - 1);
                                selected_wallet->balance = stored_wallet.balance;
                                selected_wallet->user_type = stored_wallet.user_type;
                                // selected_wallet->next = NULL;
                        }
                        break;
                }
        }

        fclose(file);
        return selected_wallet;
}
