/* main.c */
#include "alu_blockchain.h"
#include "config.h"

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
        sleep(1);
        if (!chain)
        {
                printf("Failed to initialize blockchain\n");
                free(config);
                return 1;
        }

        /* Create institutional wallets */
        create_institutional_wallets();

        sleep(1);

        /* Create Vendor wallets */
        create_vendor_wallets();

        /* Load existing profiles */
        if (!load_profiles_from_file())
        {
                printf("\nStarting fresh....\n");
                sleep(1.5);
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
                        if (current_wallet)
                        {
                                printf("Your wallet has already been loaded.\n");
                                printf("Please continue with the current wallet or log out first.\n");
                                break;
                        }

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
                        // get_string_input("Enter your name: ", name, MAX_NAME);
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
                                // printf("Enter year of study (1-4): ");
                                // scanf("%d", &year);
                                // clear_input_buffer();
                                // get_string_input("Enter program: ", program, 50);
                                year = 3;
                                studentDetail = create_student_profile(name, email, year, program);
                                profile = studentDetail ? &studentDetail->profile : NULL;
                                current_wallet = studentDetail ? &studentDetail->wallet : NULL;
                                printf("Student wallet address: %s\n", current_wallet->address);
                                printf("Student wallet private key: %s\n", current_wallet->private_key);
                        }
                        else if (strstr(email, STAFF_DOMAIN))
                        {
                                // get_string_input("Enter department: ", department, 50);
                                // get_string_input("Enter role: ", role, 30);
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
                        sleep(1);
                        if (current_wallet)
                                printf("Wallet loaded successfully!\n");
                        else
                                printf("Failed to load wallet. Check your private key.\n");
                        break;

                case 3: /* Initiate Transaction */
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
                        print_transaction_history(current_wallet);
                        break;

                case 6: /* View Blocks */
                        print_blockchain(chain);
                        break;

                case 7: /* Mine Block */
                        mine_block(chain);
                        break;

                case 8: /* View Blockchain Status */
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

                case 9: /* Backup Blockchain */
                        printf("\n=== Backup Blockchain ===\n");
                        if (backup_blockchain(chain))
                                printf("Blockchain backed up successfully!\n");
                        else
                                printf("Failed to backup blockchain.\n");
                        break;

                case 10: /* Restore Blockchain */
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
                case 11: /* Exit */
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
        printf("3. Initiate Transaction\n");
        printf("4. View Balance\n");
        printf("5. View Transaction History\n");
        printf("6. View Blockchain\n");
        printf("7. Mine Block\n");
        printf("8. View Blockchain Status\n");
        printf("9. Backup Blockchain\n");
        printf("10. Restore Blockchain\n");
        printf("11. Exit\n");
        printf("\nEnter your choice (1-11): ");
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

// /**
//  * display_cafeteria_menu - Show available kitchens
//  */
void display_cafeteria_menu(void)
{
        FILE *file = fopen("kitchens.txt", "r"); // Open the file in read mode
        if (!file)
        {
                printf("\nNo kitchens available at the moment.\n");
                return;
        }

        printf("\n=== Select Kitchen ===\n");

        char line[256];
        int index = 1;
        while (fgets(line, sizeof(line), file))
        {
                char kitchen_name[MAX_NAME];
                sscanf(line, "%99[^,]", kitchen_name); // Read the kitchen name before the first comma
                printf("%d. %s\n", index++, kitchen_name);
        }

        fclose(file);

        printf("\nSelect kitchen (1-%d): ", index - 1);
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
        free(recipient);
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

                if (scanf("%d", &kitchen_choice) != 1 || kitchen_choice < 1)
                {
                        printf("Invalid kitchen selection.\n");
                        clear_input_buffer();
                        return 0;
                }
                clear_input_buffer();

                VendorProfile *kitchen = get_kitchen_vendor(kitchen_choice - 1); // Get the kitchen based on the selection
                printf("Selected kitchen: %s\n", kitchen->kitchen_name);
                printf("Wallet Address: %s\n", kitchen->wallet_address);
                if (!kitchen)
                {
                        printf("Failed to get kitchen information.\n");
                        return 0;
                }

                strcpy(to_address, kitchen->wallet_address);
                printf("Kitchen Address: %s\n", to_address);
                recipient_name = kitchen->kitchen_name;
                free(kitchen); // Don't forget to free the memory after usage
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
        if (initiate_transaction(chain, wallet, to_address, amount, trans_type))
        {
                wallet->balance -= amount;

                // Load recipient's wallet and update balance
                Wallet *recipient_wallet = load_wallet_by_public_key(to_address);
                if (recipient_wallet)
                {
                        recipient_wallet->balance += amount;
                        update_wallet_record(wallet);
                        update_wallet_record(recipient_wallet);
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
 * create_institutional_wallets - Create wallets for institutions
 * Return: 1 on success, 0 on failure
 */
int create_institutional_wallets(void)
{
        printf("\nPreloading school wallets...\n");
        sleep(1);
        const struct
        {
                const char *address;
                const char *email;
                const char *private_key;
        } institutions[] = {
            {SCHOOL_TUITION_ADDRESS, "tuition@alu.edu", "tuition_key_placeholder"},
            {SCHOOL_LIBRARY_ADDRESS, "library@alu.edu", "library_key_placeholder"},
            {HEALTH_INSURANCE_ADDRESS, "insurance@alu.edu", "insurance_key_placeholder"}};

        for (size_t i = 0; i < sizeof(institutions) / sizeof(institutions[0]); i++)
        {
                // Check if wallet already exists
                Wallet *existing_wallet = load_wallet_by_public_key(institutions[i].address);
                if (existing_wallet)
                {
                        free(existing_wallet);
                        continue;
                }

                // Save wallet
                if (!save_wallet(institutions[i].email, institutions[i].private_key, institutions[i].address, NULL))
                {
                        printf("Failed to save wallet for %s\n", institutions[i].email);
                        return 0;
                }
        }

        return 1;
}

/**
 * create_vendor_wallets - Create wallets for kitchen vendors with predefined addresses
 * Return: 1 on success, 0 on failure
 */
int create_vendor_wallets(void)
{
        printf("\nPreloading vendor wallets...\n");
        sleep(1);

        const struct
        {
                const char *kitchen_name;
                const char *email;
                const char *public_key;
                const char *private_key;
        } vendors[] = {
            {Pius_Cuisine, "pius@vendor.com", Pius_Cuisine_ADDRESS, "pius_key_placeholder"},
            {Joshua_Kitchen, "joshua@vendor.com", Joshua_Kitchen_ADDRESS, "joshua_key_placeholder"},
            {Pascal_Kitchen, "pascal@vendor.com", Pascal_Kitchen_ADDRESS, "pascal_key_placeholder"}};

        for (size_t i = 0; i < sizeof(vendors) / sizeof(vendors[0]); i++)
        {
                // Check if wallet already exists
                Wallet *existing_wallet = load_wallet_by_public_key(vendors[i].public_key);
                if (existing_wallet)
                {
                        free(existing_wallet);
                        continue;
                }

                add_kitchen(vendors[i].kitchen_name, vendors[i].email, vendors[i].public_key);

                if (!save_wallet(vendors[i].email, vendors[i].private_key, vendors[i].public_key, vendors[i].kitchen_name))
                {
                        printf("Failed to save wallet for %s\n", vendors[i].kitchen_name);
                        return 0;
                }
        }

        return 1;
}
