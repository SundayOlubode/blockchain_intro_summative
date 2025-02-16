/* main.c */
#include "alu_blockchain.h"
#include "config.h"

/* Function prototypes */
void display_menu(void);
void display_payment_menu(void);
void get_string_input(const char *prompt, char *buffer, size_t size);
void clear_input_buffer(void);
int process_payment(Blockchain *chain, Wallet *wallet);
void print_transaction_history(Blockchain *chain, Wallet *wallet);
void configure_system(void);

/**
 * display_menu - Show main menu options
 */
void display_menu(void)
{
        printf("\n=== ALU Payment System ===\n");
        printf("1. Create New Wallet\n");
        printf("2. Load Existing Wallet\n");
        printf("3. Make Payment\n");
        printf("4. View Balance\n");
        printf("5. View Transaction History\n");
        printf("6. View Blockchain Status\n");
        printf("7. Backup Blockchain\n");
        printf("8. Restore Blockchain\n");
        printf("9. Configure System\n");
        printf("10. Exit\n");
        printf("\nEnter your choice (1-10): ");
}

/**
 * display_payment_menu - Show payment type options
 */
void display_payment_menu(void)
{
        printf("\n=== Payment Types ===\n");
        printf("1. Tuition Fee\n");
        printf("2. Cafeteria Payment\n");
        printf("3. Library Fine\n");
        printf("4. Token Transfer\n");
        printf("\nSelect payment type (1-4): ");
}

/**
 * get_string_input - Get string input safely
 * @prompt: Prompt to display
 * @buffer: Buffer to store input
 * @size: Buffer size
 */
void get_string_input(const char *prompt, char *buffer, size_t size)
{
        printf("%s", prompt);
        if (fgets(buffer, size, stdin) != NULL)
        {
                size_t len = strlen(buffer);
                if (len > 0 && buffer[len - 1] == '\n')
                        buffer[len - 1] = '\0';
        }
}

/**
 * clear_input_buffer - Clear input buffer
 */
void clear_input_buffer(void)
{
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
                ;
}

/**
 * process_payment - Handle payment transaction
 * @chain: Blockchain
 * @wallet: User's wallet
 * Return: 1 on success, 0 on failure
 */
int process_payment(Blockchain *chain, Wallet *wallet)
{
        int type_choice;
        char to_address[HASH_LENGTH + 1];
        double amount;
        TransactionType trans_type;
        const char *recipient_name;

        if (!chain || !wallet)
                return 0;

        display_payment_menu();
        if (scanf("%d", &type_choice) != 1)
        {
                clear_input_buffer();
                return 0;
        }
        clear_input_buffer();

        switch (type_choice)
        {
        case 1: /* Tuition Fee */
                trans_type = TUITION_FEE;
                strcpy(to_address, SCHOOL_TUITION_ADDRESS);
                recipient_name = "ALU Tuition Account";
                printf("\nSelected: Tuition Fee Payment\n");
                printf("Recipient: %s\n", recipient_name);
                break;

        case 2: /* Cafeteria Payment */
                trans_type = CAFETERIA_PAYMENT;
                strcpy(to_address, CAFETERIA_ADDRESS);
                recipient_name = "ALU Cafeteria";
                printf("\nSelected: Cafeteria Payment\n");
                printf("Recipient: %s\n", recipient_name);
                break;

        case 3: /* Library Fine */
                trans_type = LIBRARY_FINE;
                strcpy(to_address, SCHOOL_LIBRARY_ADDRESS);
                recipient_name = "ALU Library";
                printf("\nSelected: Library Fine Payment\n");
                printf("Recipient: %s\n", recipient_name);
                break;

        case 4: /* Token Transfer */
                trans_type = TOKEN_TRANSFER;
                printf("\nSelected: Token Transfer\n");
                get_string_input("Enter recipient's wallet address: ", to_address, HASH_LENGTH + 1);
                recipient_name = "Custom Address";
                break;

        default:
                printf("Invalid payment type.\n");
                return 0;
        }

        printf("Enter amount in %s: ", chain->token.symbol);
        if (scanf("%lf", &amount) != 1)
        {
                clear_input_buffer();
                return 0;
        }
        clear_input_buffer();

        if (amount <= 0 || amount > wallet->balance)
        {
                printf("Invalid amount or insufficient balance.\n");
                return 0;
        }

        printf("\nPayment Summary:\n");
        printf("Type: %s\n",
               type_choice == 1 ? "Tuition Fee" : type_choice == 2 ? "Cafeteria Payment"
                                              : type_choice == 3   ? "Library Fine"
                                                                   : "Token Transfer");
        printf("Recipient: %s\n", recipient_name);
        printf("Amount: %.2f %s\n", amount, chain->token.symbol);
        printf("\nConfirm payment? (y/n): ");

        {
                char confirm;
                scanf(" %c", &confirm);
                clear_input_buffer();

                if (confirm != 'y' && confirm != 'Y')
                {
                        printf("Payment cancelled.\n");
                        return 0;
                }
        }

        if (create_transaction(chain, wallet, to_address, amount, trans_type))
        {
                printf("\nPayment successful!\n");
                printf("New balance: %.2f %s\n", wallet->balance - amount, chain->token.symbol);
                return 1;
        }
        else
        {
                printf("\nPayment failed. Please try again.\n");
                return 0;
        }
}

/**
 * configure_system - Handle system configuration
 */
void configure_system(void)
{
        Config *config;
        int choice;

        config = load_config();
        if (!config)
        {
                printf("Failed to load configuration.\n");
                return;
        }

        printf("\nCurrent Configuration:\n");
        printf("1. Initial Supply: %u\n", config->initial_supply);
        printf("2. Block Reward: %u\n", config->block_reward);
        printf("3. Max Transactions per Block: %u\n", config->max_transactions);
        printf("4. Backup Directory: %s\n", config->backup_directory);
        printf("5. Auto Backup: %s\n", config->auto_backup ? "Enabled" : "Disabled");
        printf("6. Backup Interval: %d blocks\n", config->backup_interval);
        printf("7. Save and Exit\n");
        printf("\nSelect setting to modify (1-7): ");

        if (scanf("%d", &choice) != 1)
        {
                clear_input_buffer();
                free(config);
                return;
        }
        clear_input_buffer();

        switch (choice)
        {
        case 1:
                printf("Enter new initial supply: ");
                scanf("%u", &config->initial_supply);
                break;
        case 2:
                printf("Enter new block reward: ");
                scanf("%u", &config->block_reward);
                break;
        case 3:
                printf("Enter new max transactions per block: ");
                scanf("%u", &config->max_transactions);
                break;
        case 4:
                get_string_input("Enter new backup directory: ",
                                 config->backup_directory, 256);
                break;
        case 5:
                printf("Enable auto backup (1/0): ");
                scanf("%d", &config->auto_backup);
                break;
        case 6:
                printf("Enter new backup interval: ");
                scanf("%d", &config->backup_interval);
                break;
        case 7:
                break;
        default:
                printf("Invalid choice.\n");
        }

        save_config(config);
        free(config);
}

/**
 * main - Entry point
 * Return: 0 on success, 1 on failure
 */
int main(void)
{
        Blockchain *chain;
        Wallet *current_wallet = NULL;
        char email[MAX_EMAIL];
        char private_key[HASH_LENGTH + 1];
        int choice;
        Config *config;

        /* Load configuration */
        config = load_config();
        if (!config)
        {
                printf("Failed to load configuration. Creating default configuration.\n");
                create_default_config();
                config = load_config();
                if (!config)
                {
                        printf("Failed to create configuration. Exiting.\n");
                        return 1;
                }
        }

        /* Initialize blockchain */
        chain = initialize_blockchain();
        if (!chain)
        {
                printf("Failed to initialize blockchain\n");
                free(config);
                return 1;
        }

        printf("Welcome to ALU Payment System\n");
        printf("Token: %s (%s)\n", chain->token.token_name, chain->token.symbol);
        printf("Total Supply: %u %s\n", chain->token.total_supply, chain->token.symbol);

        while (1)
        {
                display_menu();
                if (scanf("%d", &choice) != 1)
                {
                        printf("Invalid input. Please enter a number.\n");
                        clear_input_buffer();
                        continue;
                }
                clear_input_buffer();

                switch (choice)
                {
                case 1: /* Create New Wallet */
                        printf("\n=== Create New Wallet ===\n");
                        get_string_input("Enter your ALU email: ", email, MAX_EMAIL);

                        if (!verify_email_domain(email))
                        {
                                printf("Invalid email domain. Must be alueducation.com, "
                                       "alustudent.com, or si.alueducation.com\n");
                                break;
                        }

                        if (current_wallet)
                        {
                                printf("Logging out of current wallet...\n");
                                free(current_wallet);
                        }

                        current_wallet = create_wallet(email);
                        if (current_wallet)
                        {
                                printf("\nWallet created successfully!\n");
                                printf("Address: %s\n", current_wallet->address);
                                printf("Private Key: %s\n", current_wallet->private_key);
                                printf("IMPORTANT: Save your private key to access your wallet later!\n");
                        }
                        else
                                printf("Failed to create wallet.\n");
                        break;

                case 2: /* Load Existing Wallet */
                        printf("\n=== Load Existing Wallet ===\n");
                        get_string_input("Enter your private key: ", private_key, HASH_LENGTH + 1);

                        if (current_wallet)
                        {
                                printf("Logging out of current wallet...\n");
                                free(current_wallet);
                        }

                        current_wallet = load_wallet(private_key);
                        if (current_wallet)
                                printf("Wallet loaded successfully!\n");
                        else
                                printf("Failed to load wallet. Check your private key.\n");
                        break;

                case 3: /* Make Payment */
                        if (!current_wallet)
                        {
                                printf("Please create or load a wallet first.\n");
                                break;
                        }

                        if (process_payment(chain, current_wallet))
                        {
                                printf("Payment completed successfully!\n");
                                if (config->auto_backup &&
                                    chain->block_count % config->backup_interval == 0)
                                {
                                        printf("Performing auto backup...\n");
                                        backup_blockchain(chain);
                                }
                        }
                        else
                                printf("Payment failed.\n");
                        break;

                case 4: /* View Balance */
                        if (!current_wallet)
                        {
                                printf("Please create or load a wallet first.\n");
                                break;
                        }
                        printf("\n=== Wallet Balance ===\n");
                        printf("Address: %.16s...\n", current_wallet->address);
                        printf("Balance: %.2f %s\n", current_wallet->balance,
                               chain->token.symbol);
                        break;

                case 5: /* View Transaction History */
                        if (!current_wallet)
                        {
                                printf("Please create or load a wallet first.\n");
                                break;
                        }

                        printf("\n=== Transaction History ===\n");
                        print_transaction_history(chain, current_wallet);
                        break;

                case 6: /* View Blockchain Status */
                        printf("\n=== Blockchain Status ===\n");
                        printf("Total Blocks: %d\n", chain->block_count);
                        printf("Token Name: %s (%s)\n", chain->token.token_name,
                               chain->token.symbol);
                        printf("Total Supply: %u\n", chain->token.total_supply);
                        printf("Circulating Supply: %u\n", chain->token.circulating_supply);

                        if (validate_chain(chain))
                                printf("Blockchain Integrity: Valid\n");
                        else
                                printf("Blockchain Integrity: COMPROMISED!\n");
                        break;

                case 7: /* Backup Blockchain */
                        printf("\n=== Backup Blockchain ===\n");
                        if (backup_blockchain(chain))
                                printf("Blockchain backed up successfully!\n");
                        else
                                printf("Failed to backup blockchain.\n");
                        break;

                case 8: /* Restore Blockchain */
                        printf("\n=== Restore Blockchain ===\n");
                        printf("Warning: This will replace the current blockchain.\n");
                        printf("Are you sure? (y/n): ");
                        {
                                char confirm;
                                scanf(" %c", &confirm);
                                clear_input_buffer();
                                if (confirm == 'y' || confirm == 'Y')
                                {
                                        if (restore_blockchain(&chain))
                                                printf("Blockchain restored successfully!\n");
                                        else
                                                printf("Failed to restore blockchain.\n");
                                }
                        }
                        break;

                case 9: /* Configure System */
                        configure_system();
                        break;

                case 10: /* Exit */
                        printf("\nThank you for using ALU Payment System!\n");
                        if (current_wallet)
                                free(current_wallet);
                        cleanup_blockchain(chain);
                        free(config);
                        return 0;

                default:
                        printf("Invalid choice. Please try again.\n");
                }
        }

        return 0;
}