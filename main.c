/* main.c */
#include "alu_blockchain.h"
#include "config.h"

/* Function prototypes */
void display_menu(void);
void display_payment_menu(void);
void display_cafeteria_menu(void);
void get_string_input(const char *prompt, char *buffer, size_t size);
void clear_input_buffer(void);
int process_payment(Blockchain *chain, Wallet *wallet);
void print_transaction_history(Blockchain *chain, Wallet *wallet);
void configure_system(void);

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

        printf("\nALU Private Blockchain Network\n\n");

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

        /* Create institutional wallets */
        create_institutional_wallets();

        /* Load existing profiles */
        if (!load_profiles_from_file())
        {
                printf("No existing profiles found. Starting fresh.\n");
        }

        printf("\nWelcome to ALU Payment System\n");
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
                {
                        char name[MAX_NAME];
                        char program[50];
                        char department[50];
                        char role[30];
                        int year;
                        void *profile = NULL;
                        StudentProfileWithWallet *studentDetail = NULL;
                        VendorProfileWithWallet *vendorDetail = NULL;
                        StaffProfileWithWallet *staffDetail = NULL;

                        printf("\n=== Create New Wallet ===\n");
                        get_string_input("Enter your name: ", name, MAX_NAME);
                        get_string_input("Enter your ALU email: ", email, MAX_EMAIL);

                        if (!verify_email_domain(email))
                        {
                                printf("Invalid email domain. Must be %s, %s, or %s\n",
                                       STUDENT_DOMAIN, STAFF_DOMAIN, VENDOR_DOMAIN);
                                break;
                        }

                        if (check_email_exists(email))
                        {
                                printf("Email already registered\n");
                                break;
                        }

                        /* Create appropriate profile based on email domain */
                        if (strstr(email, STUDENT_DOMAIN))
                        {
                                printf("Enter year of study (1-4): ");
                                scanf("%d", &year);
                                clear_input_buffer();
                                get_string_input("Enter program: ", program, 50);
                                studentDetail = create_student_profile(name, email, year, program);
                                profile = studentDetail ? &studentDetail->profile : NULL;
                                current_wallet = studentDetail ? &studentDetail->wallet : NULL;
                        }
                        else if (strstr(email, STAFF_DOMAIN))
                        {
                                get_string_input("Enter department: ", department, 50);
                                get_string_input("Enter role: ", role, 30);
                                staffDetail = create_staff_profile(name, email, department, role);
                                profile = staffDetail ? &staffDetail->profile : NULL;
                                current_wallet = staffDetail ? &staffDetail->wallet : NULL;
                        }
                        else if (strstr(email, VENDOR_DOMAIN))
                        {
                                char kitchen_name[MAX_NAME];
                                get_string_input("Enter kitchen name: ", kitchen_name, MAX_NAME);
                                vendorDetail = create_vendor_profile(kitchen_name, email);
                                profile = vendorDetail ? &vendorDetail->profile : NULL;
                                current_wallet = vendorDetail ? &vendorDetail->wallet : NULL;
                        }

                        if (!profile)
                        {
                                printf("Failed to create profile.\n");
                                break;
                        }

                        if (current_wallet)
                        {
                                printf("\nWallet created successfully!\n");
                                printf("Address: %s\n", current_wallet->address);
                                printf("Private Key: %s\n", current_wallet->private_key);
                                printf("IMPORTANT: Save your private key to access your wallet later!\n");
                                printf("Initial balance: %.2f LT\n", current_wallet->balance);
                        }
                        else
                                printf("Failed to create wallet.\n");
                }
                break;

                case 2: /* Load Existing Wallet */
                        printf("\n=== Load Existing Wallet ===\n");
                        get_string_input("Enter your private key: ", private_key, HASH_LENGTH + 1);

                        if (current_wallet)
                        {
                                printf("Logging out of current wallet...\n");
                                free(current_wallet);
                        }

                        current_wallet = load_wallet_by_key(private_key);
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

                        current_wallet = load_wallet_by_public_key(current_wallet->address);

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
                        current_wallet = load_wallet_by_public_key(current_wallet->address);
                        printf("\n=== Wallet Balance ===\n");
                        printf("Email: %s\n", current_wallet->email);
                        printf("Address: %s\n", current_wallet->address);
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
                        // configure_system();
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
        printf("4. Health Insurance\n");
        printf("5. Token Transfer\n");
        printf("\nSelect payment type (1-5): ");
}

/**
 * display_cafeteria_menu - Show available kitchens
 */
void display_cafeteria_menu(void)
{
        printf("\n=== Select Kitchen ===\n");
        printf("1. Pascal's Kitchen\n");
        printf("2. Pius' Kitchen\n");
        printf("3. Joshua's Kitchen\n");
        printf("\nSelect kitchen (1-3): ");
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
 * verify_transaction - Verifies if a transaction is valid
 * @wallet: Sender's wallet
 * @to_address: Recipient's wallet address
 * @amount: Amount to be transferred
 * Return: 1 if valid, 0 if invalid
 */
int verify_transaction(const Wallet *wallet, const char *to_address, double amount)
{
        if (!wallet || !to_address || amount <= 0)
        {
                printf("Invalid transaction parameters.\n");
                return 0;
        }

        // Check if the sender has enough balance
        if (amount > wallet->balance)
        {
                printf("Insufficient balance.\n");
                return 0;
        }

        // Ensure the recipient address is not the same as the sender's address
        if (strcmp(wallet->address, to_address) == 0)
        {
                printf("Cannot send tokens to your own address.\n");
                return 0;
        }

        // Load recipient's wallet using public key
        Wallet *recipient = load_wallet_by_public_key(to_address);
        if (!recipient)
        {
                printf("Recipient wallet not found.\n");
                return 0;
        }
        return 1;
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
                break;

        case 2: /* Cafeteria Payment */
        {
                int kitchen_choice;
                trans_type = CAFETERIA_PAYMENT;
                display_cafeteria_menu();

                if (scanf("%d", &kitchen_choice) != 1 ||
                    kitchen_choice < 1 || kitchen_choice > 3)
                {
                        printf("Invalid kitchen selection.\n");
                        clear_input_buffer();
                        return 0;
                }
                clear_input_buffer();

                const VendorProfile *kitchen = get_kitchen_vendor(kitchen_choice - 1);
                if (!kitchen)
                {
                        printf("Failed to get kitchen information.\n");
                        return 0;
                }

                strcpy(to_address, kitchen->wallet_address);
                recipient_name = kitchen->kitchen_name;
                break;
        }

        case 3: /* Library Fine */
                trans_type = LIBRARY_FINE;
                strcpy(to_address, SCHOOL_LIBRARY_ADDRESS);
                recipient_name = "ALU Library";
                break;

        case 4: /* Health Insurance */
                trans_type = HEALTH_INSURANCE;
                strcpy(to_address, HEALTH_INSURANCE_ADDRESS);
                recipient_name = "ALU Health Insurance";
                break;

        case 5: /* Token Transfer */
                trans_type = TOKEN_TRANSFER;
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

        /* Verify the transaction */
        if (!verify_transaction(wallet, to_address, amount))
                return 0;

        printf("\nPayment Summary:\n");
        printf("Type: %s\n",
               type_choice == 1 ? "Tuition Fee" : type_choice == 2 ? "Cafeteria Payment"
                                              : type_choice == 3   ? "Library Fine"
                                              : type_choice == 4   ? "Health Insurance"
                                                                   : "Token Transfer");
        printf("Recipient: %s\n", recipient_name);
        printf("Amount: %.2f %s\n", amount, chain->token.symbol);
        printf("\nConfirm payment? (y/n): ");

        char confirm;
        scanf(" %c", &confirm);
        clear_input_buffer();

        if (confirm != 'y' && confirm != 'Y')
        {
                printf("Payment cancelled.\n");
                return 0;
        }

        /* Create the transaction */
        if (create_transaction(chain, wallet, to_address, amount, trans_type))
        {
                wallet->balance -= amount;

                // Load recipient's wallet and update balance
                Wallet *recipient_wallet = load_wallet_by_public_key(to_address);
                if (recipient_wallet)
                {
                        recipient_wallet->balance += amount;
                        update_wallet_record(wallet);           // Update sender
                        update_wallet_record(recipient_wallet); // Update recipient
                        free(recipient_wallet);
                }

                printf("\nPayment successful!\n");
                printf("New balance: %.2f %s\n", wallet->balance, chain->token.symbol);
                return 1;
        }
        else
        {
                printf("\nPayment failed. Please try again.\n");
                return 0;
        }
}

/**
 * create_institutional_wallets - Create wallets for institutional addresses
 * Return: 1 on success, 0 on failure
 */
int create_institutional_wallets(void)
{
        // Array of institutional wallet details
        const struct
        {
                const char *address;
                const char *email;
                const char *private_key;
                const char *name;
                UserType user_type;
        } institutions[] = {
            {SCHOOL_TUITION_ADDRESS, "tuition@alueducation.com", "tuition_pivate_key", "ALU Tuition Account", INSTITUTION},
            {SCHOOL_LIBRARY_ADDRESS, "library@alueducation.com", "library_pivate_key", "ALU Library", INSTITUTION},
            {HEALTH_INSURANCE_ADDRESS, "insurance@alueducation.com", "insurance_pivate_key", "ALU Health Insurance", INSTITUTION}};

        // Iterate over each institutional wallet and save it
        for (size_t i = 0; i < sizeof(institutions) / sizeof(institutions[0]); i++)
        {
                StoredWallet wallet = {0};

                strncpy(wallet.address, institutions[i].address, HASH_LENGTH - 1);
                strncpy(wallet.email, institutions[i].email, MAX_EMAIL - 1);
                strncpy(wallet.private_key, institutions[i].private_key, HASH_LENGTH - 1);
                strncpy(wallet.name, institutions[i].name, MAX_NAME - 1);
                wallet.balance = 0.0;
                wallet.user_type = institutions[i].user_type;

                // Save wallet to file
                if (!save_wallet(wallet.email, wallet.private_key, wallet.address, wallet.name))
                {
                        printf("Failed to save wallet for: %s\n", institutions[i].name);
                        return 0;
                }
        }

        printf("Institutional wallets created successfully.\n");
        return 1;
}

/* Continue with the rest of main.c... */