/* alu_blockchain.c */
#include "alu_blockchain.h"
#include "config.h"

int tx_count = 0;

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
 * initialize_blockchain - Initialize blockchain system
 * Return: Pointer to initialized blockchain
 */
Blockchain *initialize_blockchain(void)
{
        Blockchain *chain = NULL;
        time_t now;
        int block_idx = 0;
        int block_cnt = 0;
        char temp[512];

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
                chain->block_count = ++block_cnt;
                chain->latest = chain->genesis;
                chain->genesis->index = block_idx++;
                strcpy(chain->genesis->previous_hash,
                       "0000000000000000000000000000000000000000000000000000000000000000");
                time(&now);
                strftime(chain->genesis->timestamp, 30, "%Y-%m-%d %H:%M:%S", localtime(&now));
                chain->genesis->reward = config->block_reward;
                chain->genesis->transaction_count = 1;
                chain->genesis->next = NULL;

                /* Set genesis as latest */
                chain->latest = chain->genesis;

                /* Generate block hash */
                sprintf(temp, "%u%s%s%u", chain->genesis->index, chain->genesis->previous_hash,
                        chain->genesis->timestamp, chain->genesis->nonce);
                generate_hash(temp, chain->genesis->current_hash);

                /* Set initial token supply */
                printf("Total Supply: %u\n", config->initial_supply);
                chain->token.total_supply = config->initial_supply;
                chain->token.circulating_supply = CIRCULATING_SUPPLY;
                strncpy(chain->token.token_name, TOKEN_NAME, 50);
                strncpy(chain->token.symbol, TOKEN_SYMBOL, 4);

                /**= Set tx_count to zero */
                tx_count = 0;

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
 * initiate_transaction - Create new transaction and store it in TX_FILE
 * @chain: Blockchain
 * @from: Sender wallet
 * @to_address: Recipient address
 * @amount: Transaction amount
 * @type: Transaction type
 * Return: 1 on success, 0 on failure
 */
int initiate_transaction(Blockchain *chain, Wallet *from, const char *to_address, double amount, TransactionType type)
{
        Transaction transaction;
        Wallet *recipient;
        char temp[256];
        FILE *file;
        FILE *tx_pool;

        if (!chain || !from || !to_address || amount <= 0 || from->balance < amount)
                return 0;

        /* Check unspent balance to prevent double-spending */
        double available_balance = get_unspent_balance(from->address);
        if (available_balance < amount)
        {
                printf("Double-spending detected! Not enough unspent balance.\n");
                return 0;
        }

        recipient = load_wallet_by_public_key(to_address);
        if (!recipient)
        {
                printf("Recipient wallet not found\n");
                return 0;
        }

        /* Prepare transaction record */
        strncpy(transaction.from_address, from->address, HASH_LENGTH - 1);
        transaction.from_address[HASH_LENGTH - 1] = '\0';

        strncpy(transaction.to_address, to_address, HASH_LENGTH - 1);
        transaction.to_address[HASH_LENGTH - 1] = '\0';

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

        /* Append transaction to TX_POOL */
        tx_pool = fopen(TX_POOL, "ab");
        if (!tx_pool)
        {
                printf("Error opening transactions pool file.\n");
                return 0;
        }

        fwrite(&transaction, sizeof(Transaction), 1, file);
        fclose(file);
        fwrite(&transaction, sizeof(Transaction), 1, tx_pool);
        fclose(tx_pool);

        /* Save updated wallets */
        update_wallet_record(from);
        update_wallet_record(recipient);

        printf("\nTransaction successfully recorded.\n");

        /*Increment tx count */
        tx_count++;

        if (tx_count >= 3)
        {
                printf("\nTransaction pool full. Creating new block...\n");
                sleep(2);
                mine_block(chain);
        }

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

        printf("\nWallet Address: %s\n", wallet->address);

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
                        printf("From: %s\n", transaction.from_address);
                        printf("To: %s\n", transaction.to_address);
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
int add_transaction(Block *new_block, Transaction *transaction)
{
        if (!new_block || !transaction)
                return 0;

        if (new_block->transaction_count >= MAX_TRANSACTIONS)
                return 0;

        new_block->transactions[new_block->transaction_count++] = *transaction;
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
        strncpy(new_block->previous_hash, latest->current_hash, HASH_LENGTH - 1);
        new_block->previous_hash[HASH_LENGTH - 1] = '\0'; // Ensure null termination

        time(&now);
        strftime(new_block->timestamp, 30, "%Y-%m-%d %H:%M:%S", localtime(&now));
        new_block->nonce = 0;
        new_block->transaction_count = 0;
        new_block->next = NULL;
        new_block->reward = BLOCK_REWARD;

        /* Generate block hash */
        sprintf(temp, "%u%s%s%u", new_block->index, new_block->previous_hash,
                new_block->timestamp, new_block->nonce);
        generate_hash(temp, new_block->current_hash);

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
        {
                printf("Previous hash mismatch! Expected: %s, Found: %s\n",
                       chain->latest->current_hash, block->previous_hash);
                return 0;
        }

        /* Recompute hash */
        sprintf(temp, "%u%s%s%u", block->index, block->previous_hash,
                block->timestamp, block->nonce);
        generate_hash(temp, computed_hash);

        if (strcmp(computed_hash, block->current_hash) != 0)
        {
                printf("Hash mismatch! Computed: %s, Expected: %s\n",
                       computed_hash, block->current_hash);
                return 0;
        }

        printf("✅ Block validation successful!\n");
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
                                selected_wallet->email[MAX_EMAIL - 1] = '\0'; // Ensure null termination
                                strncpy(selected_wallet->private_key, stored_wallet.private_key, HASH_LENGTH - 1);
                                selected_wallet->private_key[HASH_LENGTH - 1] = '\0'; // Ensure null termination
                                strncpy(selected_wallet->address, stored_wallet.address, HASH_LENGTH - 1);
                                selected_wallet->address[HASH_LENGTH - 1] = '\0'; // Ensure null termination
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

/**
 * mine_block - Mines a new block, validates it, and rewards the validator
 * @chain: Blockchain
 * Return: Pointer to the mined block, NULL on failure
 */
Block *mine_block(Blockchain *chain)
{
        Block *new_block, *latest;
        Transaction *tx_pool;
        Wallet *validator;
        double mining_reward = 2.0; /* Adjust reward as needed */

        if (!chain)
        {
                printf("Blockchain not initialized.\n");
                return NULL;
        }

        latest = chain->latest;

        printf("\n⛏️    Mining new block...\n");
        sleep(1.5);

        new_block = create_block(chain);
        if (!new_block)
        {
                printf("Block creation failed.\n");
                return NULL;
        }

        sleep(1.0);
        if (!validate_block(chain, new_block))
        {
                printf("❌ Block validation failed. Discarding block.\n");
                return NULL;
        }

        /* Extract transactions from TX_POOL */
        sleep(1.0);
        tx_pool = extract_transactions();
        if (tx_pool)
        {
                int i = 0;
                while (i < tx_count && strlen(tx_pool[i].from_address) > 0)
                {
                        /* Add transactions */
                        if (!add_transaction(new_block, &tx_pool[i]))
                        {
                                printf("Failed to add transaction to block #%d\n", new_block->index);
                        }
                        i++;
                }
                free(tx_pool);
        }

        /* Select a validator */
        validator = select_validator(chain);
        if (!validator)
        {
                printf("No validator selected. Block reward skipped.\n");
        }
        else
        {
                validator->balance += mining_reward;
                update_wallet_record(validator);
                printf("\nBlock mined by %s %s. Reward: %.2f\n", validator->email, validator->address, mining_reward);
        }

        /* Link new block to blockchain first */
        latest->next = new_block;
        chain->latest = new_block;
        chain->block_count++;

        /* Backup blockchain */
        backup_blockchain(chain);

        printf("New block #%d created with %d transaction(s)\n", new_block->index + 1, new_block->transaction_count);

        /* Reset tx_count for next pool */
        tx_count = 0;

        /* Return mined block */
        return new_block;
}

/**
 * print_blockchain - Print all blocks in the blockchain
 * @chain: Pointer to the blockchain
 */
void print_blockchain(const Blockchain *chain)
{
        Block *current;
        int i;

        if (!chain)
        {
                printf("Blockchain is NULL.\n");
                return;
        }

        printf("\n========== ALU PRIVATE BLOCKCHAIN ==========\n");
        printf("Total Blocks: %d\n", chain->block_count);
        printf("Token Name: %s (%s)\n", chain->token.token_name, chain->token.symbol);
        printf("Total Supply: %u\n", chain->token.total_supply);
        printf("Circulating Supply: %u\n", chain->token.circulating_supply);
        printf("=====================================\n\n");

        current = chain->genesis;
        for (i = 0; current; i++)
        {
                printf("----- Block #%d -----\n", current->index + 1);
                printf("Timestamp: %s\n", current->timestamp);
                printf("Previous Hash: %s\n", current->previous_hash);
                printf("Transactions: %d\n", current->transaction_count);
                printf("Reward: %u\n", current->reward);
                printf("Hash: %s\n", current->current_hash);
                printf("---------------------\n\n");

                current = current->next;
        }
}

/**
 * extract_transactions - Extracts transactions from TX_POOL and returns an array
 * Return: Pointer to an array of transactions (must be freed after use), NULL on failure
 */
Transaction *extract_transactions()
{
        FILE *file;
        Transaction *transactions;
        int count = 0;

        /* Allocate memory for transactions */
        transactions = malloc(MAX_TRANSACTIONS * sizeof(Transaction));
        if (!transactions)
                return NULL;

        file = fopen(TX_POOL, "rb");
        if (!file)
        {
                free(transactions);
                return NULL;
        }

        /* Read transactions from the file */
        while (fread(&transactions[count], sizeof(Transaction), 1, file) == 1)
        {
                count++;
                if (count >= MAX_TRANSACTIONS)
                        break; /* Prevent overflow */
        }
        printf("Extracted %d transactions from pool\n", count);

        fclose(file);

        /* Clear TX_POOL file after extraction */
        file = fopen(TX_POOL, "wb");
        if (file)
                fclose(file);

        return transactions;
}

/**
 * get_unspent_balance - Checks the unspent balance of a wallet
 * @address: Wallet address to check
 * Return: Available balance including initial balance
 */
double get_unspent_balance(const char *address)
{
        FILE *file = fopen(TX_FILE, "rb");
        if (!file)
        {
                printf("Error opening transaction history file.\n");
                return 100.00; // Default balance for new wallets
        }

        double balance = 100.00; // Start with initial balance
        Transaction tx;

        /* Read all transactions from history */
        while (fread(&tx, sizeof(Transaction), 1, file))
        {
                if (strcmp(tx.from_address, address) == 0)
                        balance -= tx.amount; // Deduct spent amount

                if (strcmp(tx.to_address, address) == 0)
                        balance += tx.amount; // Add received amount
        }

        fclose(file);
        return balance;
}
