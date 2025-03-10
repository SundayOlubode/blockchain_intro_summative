/* wallet_storage.c */
#include "alu_blockchain.h"

/**
 * get_user_type_from_email - Determine user type from email domain
 * @email: Email to check
 * Return: UserType enum value
 */
UserType get_user_type_from_email(const char *email)
{
        if (strstr(email, STUDENT_DOMAIN))
                return STUDENT;
        else if (strstr(email, STAFF_DOMAIN))
                return STAFF;
        else if (strstr(email, VENDOR_DOMAIN))
                return VENDOR;
        return VENDOR; /* Default case */
}

// /**
//  * verify_email_domain - Verify if email domain is valid
//  * @email: Email to verify
//  * Return: 1 if valid, 0 if invalid
//  */
// int verify_email_domain(const char *email)
// {
//         return (strstr(email, STUDENT_DOMAIN) ||
//                 strstr(email, STAFF_DOMAIN) ||
//                 strstr(email, VENDOR_DOMAIN));
// }

/**
 * save_wallet - Save wallet data in binary format
 * @email: Owner's email
 * @private_key: Wallet's private key
 * @address: Wallet's address
 * @kitchen_name: Kitchen name (for vendors)
 * Return: 1 on success, 0 on failure
 */
int save_wallet(const char *email, const char *private_key,
                const char *address, const char *kitchen_name)
{
        FILE *file;
        StoredWallet wallet = {0};

        strncpy(wallet.email, email, MAX_EMAIL - 1);
        strncpy(wallet.private_key, private_key, HASH_LENGTH - 1);
        strncpy(wallet.address, address, HASH_LENGTH - 1);
        wallet.balance = 100.0; // Initial balance
        wallet.user_type = get_user_type_from_email(email);

        if (kitchen_name && wallet.user_type == VENDOR)
                strncpy(wallet.kitchen_name, kitchen_name, MAX_NAME - 1);

        file = fopen(WALLETS_FILE, "ab"); // Append in binary mode
        if (!file)
        {
                return 0;
        }

        fwrite(&wallet, sizeof(StoredWallet), 1, file);

        fclose(file);
        return 1;
}

/**
 * update_wallet_record - Update wallet record in the wallet file
 * @updated_wallet: The wallet with updated information
 * Return: 1 on success, 0 on failure
 */
int update_wallet_record(const Wallet *updated_wallet)
{
        if (!updated_wallet)
                return 0;

        FILE *file = fopen(WALLETS_FILE, "rb");
        if (!file)
        {
                printf("Error opening wallet file.\n");
                return 0;
        }

        StoredWallet *wallets = NULL;
        size_t wallet_count = 0;
        StoredWallet temp_wallet;

        // Load all wallets into memory
        while (fread(&temp_wallet, sizeof(StoredWallet), 1, file))
        {
                StoredWallet *new_wallets = realloc(wallets, (wallet_count + 1) * sizeof(StoredWallet));
                if (!new_wallets)
                {
                        free(wallets);
                        fclose(file);
                        printf("Memory allocation error.\n");
                        return 0;
                }
                wallets = new_wallets;
                wallets[wallet_count++] = temp_wallet;
        }
        fclose(file);

        // Find the wallet by address and update it
        int updated = 0;
        for (size_t i = 0; i < wallet_count; i++)
        {
                if (strcmp(wallets[i].address, updated_wallet->address) == 0)
                {
                        // Update wallet details
                        wallets[i].balance = updated_wallet->balance;
                        updated = 1;
                        break;
                }
        }

        if (!updated)
        {
                printf("Wallet record not found.\n");
                free(wallets);
                return 0;
        }

        // Overwrite the wallet file with updated records
        file = fopen(WALLETS_FILE, "wb");
        if (!file)
        {
                printf("Error opening wallet file for writing.\n");
                free(wallets);
                return 0;
        }

        fwrite(wallets, sizeof(StoredWallet), wallet_count, file);
        fclose(file);
        free(wallets);

        return 1;
}

/**
 * check_email_exists - Check if email is already registered (binary search)
 * @email: Email to check
 * Return: 1 if exists, 0 if not
 */
int check_email_exists(const char *email)
{
        FILE *file;
        StoredWallet wallet;

        file = fopen(WALLETS_FILE, "rb");
        if (!file)
        {
                return 0; // No wallets exist yet
        }

        while (fread(&wallet, sizeof(StoredWallet), 1, file))
        {
                if (strcmp(wallet.email, email) == 0)
                {
                        fclose(file);
                        return 1; // Email exists
                }
        }

        fclose(file);
        return 0; // Email not found
}

/**
 * load_wallet_by_key - Load wallet using private key
 * @private_key: Private key to search for
 * Return: Loaded wallet or NULL if not found
 */
Wallet *load_wallet_by_key(const char *private_key)
{
        FILE *file;
        StoredWallet stored_wallet;
        Wallet *wallet = NULL;

        if (!private_key || private_key[0] == '\0')
        {
                printf("Invalid private key\n");
                return NULL;
        }

        file = fopen(WALLETS_FILE, "rb");
        if (!file)
        {
                printf("Failed to open wallets file\n");
                return NULL;
        }

        printf("Searching for wallet with private key: %s\n", private_key);

        // Search for the wallet by comparing private keys
        while (fread(&stored_wallet, sizeof(StoredWallet), 1, file))
        {

                if (strcmp(stored_wallet.private_key, private_key) == 0)
                {
                        // Allocate memory for Wallet
                        wallet = malloc(sizeof(Wallet));
                        if (!wallet)
                        {
                                printf("Failed to allocate memory for wallet\n");
                                fclose(file);
                                return NULL;
                        }

                        // Copy all relevant fields from stored_wallet
                        strncpy(wallet->email, stored_wallet.email, MAX_EMAIL - 1);
                        wallet->email[MAX_EMAIL - 1] = '\0';

                        strncpy(wallet->private_key, stored_wallet.private_key, HASH_LENGTH - 1);
                        wallet->private_key[HASH_LENGTH - 1] = '\0';

                        strncpy(wallet->address, stored_wallet.address, HASH_LENGTH - 1);
                        wallet->address[HASH_LENGTH - 1] = '\0';

                        wallet->balance = stored_wallet.balance;
                        wallet->user_type = stored_wallet.user_type;

                        break;
                }
        }

        fclose(file);

        return wallet;
}

/**
 * load_wallet_by_public_key - Load wallet using public key
 * @public_key: Public key to search for
 * Return: Loaded wallet or NULL if not found
 */
Wallet *load_wallet_by_public_key(const char *public_key)
{
        FILE *file;
        StoredWallet stored_wallet;
        Wallet *wallet = NULL;

        file = fopen(WALLETS_FILE, "rb");
        if (!file)
                return NULL;

        while (fread(&stored_wallet, sizeof(StoredWallet), 1, file))
        {
                if (strcmp(stored_wallet.address, public_key) == 0) // Compare public key
                {
                        wallet = malloc(sizeof(Wallet));
                        if (wallet)
                        {
                                strncpy(wallet->email, stored_wallet.email, MAX_EMAIL - 1);
                                wallet->email[MAX_EMAIL - 1] = '\0'; // Ensure null termination

                                strncpy(wallet->private_key, stored_wallet.private_key, HASH_LENGTH - 1);
                                wallet->private_key[HASH_LENGTH - 1] = '\0'; // Ensure null termination

                                strncpy(wallet->address, stored_wallet.address, HASH_LENGTH - 1);
                                wallet->address[HASH_LENGTH - 1] = '\0'; // Ensure null termination

                                wallet->balance = stored_wallet.balance;
                                wallet->user_type = stored_wallet.user_type;
                                // strncpy(wallet->kitchen_name, stored_wallet.kitchen_name, MAX_NAME - 1);
                        }
                        break;
                }
        }

        fclose(file);
        return wallet;
}

/**
 * create_wallet - Create a new wallet with verified email
 * @email: User's email
 * Return: New Wallet struct or NULL on failure
 */
Wallet *load_wallet_by_email(const char *email)
{
        FILE *file;
        StoredWallet stored_wallet;
        Wallet *wallet = NULL;

        file = fopen(WALLETS_FILE, "rb");
        if (!file)
                return NULL;

        while (fread(&stored_wallet, sizeof(StoredWallet), 1, file))
        {
                if (strcmp(stored_wallet.email, email) == 0) // Compare public key
                {
                        wallet = malloc(sizeof(Wallet));
                        if (wallet)
                        {
                                strncpy(wallet->email, stored_wallet.email, MAX_EMAIL - 1);
                                wallet->email[MAX_EMAIL - 1] = '\0'; // Ensure null termination

                                strncpy(wallet->private_key, stored_wallet.private_key, HASH_LENGTH - 1);
                                wallet->private_key[HASH_LENGTH - 1] = '\0'; // Ensure null termination

                                strncpy(wallet->address, stored_wallet.address, HASH_LENGTH - 1);
                                wallet->address[HASH_LENGTH - 1] = '\0'; // Ensure null termination

                                wallet->balance = stored_wallet.balance;
                                wallet->user_type = stored_wallet.user_type;
                                // strncpy(wallet->kitchen_name, stored_wallet.kitchen_name, MAX_NAME - 1);
                        }
                        break;
                }
        }

        fclose(file);
        return wallet;
}

/**
 * create_wallet - Create a new wallet with verified email
 * @email: User's email
 * @kitchen_name: Optional kitchen name (for VENDOR user type)
 * Return: New Wallet struct or NULL on failure
 */
Wallet *create_wallet(const char *email, const char *kitchen_name)
{
        Wallet *wallet;
        char temp[MAX_EMAIL + 20];
        time_t now;
        UserType type;

        if (!verify_email_domain(email))
        {
                printf("Invalid email domain\n");
                return NULL;
        }

        if (check_email_exists(email))
        {
                printf("Email already registered\n");
                return NULL;
        }

        type = get_user_type_from_email(email);

        // If user is VENDOR but no kitchen name is provided, return error
        if (type == VENDOR && (!kitchen_name || kitchen_name[0] == '\0'))
        {
                printf("Kitchen name is required for vendors\n");
                return NULL;
        }

        wallet = malloc(sizeof(Wallet));
        if (!wallet)
                return NULL;

        strncpy(wallet->email, email, MAX_EMAIL - 1);
        wallet->user_type = type;
        wallet->balance = 100.0; // Initial balance

        time(&now);
        sprintf(temp, "%s%ld", email, now);
        generate_hash(temp, wallet->address);

        sprintf(temp, "%s%ld%s", email, now, wallet->address);
        generate_hash(temp, wallet->private_key);

        // Save wallet with kitchen name if user type is VENDOR
        if (!save_wallet(email, wallet->private_key, wallet->address,
                         type == VENDOR ? kitchen_name : NULL))
        {
                free(wallet);
                return NULL;
        }

        return wallet;
}

/**
 * reload_wallet - Reload wallet from file using public key
 * @current_wallet: Current wallet to reload
 * Return: Updated wallet or NULL if not found
 */
Wallet *reload_wallet(Wallet *current_wallet)
{
        if (!current_wallet)
                return NULL;

        Wallet *temp_wallet = load_wallet_by_public_key(current_wallet->address);

        if (temp_wallet)
        {
                // Only free current_wallet if it's dynamically allocated
                if (current_wallet != temp_wallet)
                        free(current_wallet);

                return temp_wallet;
        }

        return current_wallet; // Keep the old one if loading fails
}

/**
 * add_kitchen - Add a new kitchen vendor to the list
 * @vendor_id: Unique vendor ID
 * @kitchen_name: Name of the kitchen
 * @email: Vendor's email
 * @wallet_address: Wallet address for the kitchen
 */
void add_kitchen(const char *kitchen_name, const char *email, const char *wallet_address)
{
        FILE *file = fopen("kitchens.txt", "a"); // Open the file in append mode
        if (!file)
        {
                printf("Failed to open file for appending!\n");
                return;
        }

        // Write the kitchen information to the file
        fprintf(file, "%s,%s,%s,%.2f\n", kitchen_name, email, wallet_address, 100.0); // Default balance set to 100.0

        fclose(file);
}

void trim_newline(char *str)
{
        size_t len = strlen(str);
        if (len > 0 && str[len - 1] == '\n')
                str[len - 1] = '\0';
}

/**
 * get_kitchen_vendor - Get kitchen vendor by index
 * @kitchen_index: Index of the kitchen vendor
 * Return: Pointer to the kitchen vendor or NULL if not found
 */
VendorProfile *get_kitchen_vendor(int kitchen_index)
{
        FILE *file = fopen("kitchens.txt", "r");
        if (!file)
        {
                printf("Failed to open kitchens file.\n");
                return NULL;
        }

        char line[256];
        int current_index = 0;
        while (fgets(line, sizeof(line), file))
        {
                if (current_index == kitchen_index)
                {
                        VendorProfile *kitchen = malloc(sizeof(VendorProfile));
                        if (!kitchen)
                        {
                                printf("Memory allocation failed.\n");
                                fclose(file);
                                return NULL;
                        }

                        // Debugging: Print the line being read
                        printf("Raw line from file: %s\n", line);

                        sscanf(line, "%99[^,],%99[^,],%255[^,],%lf",
                               kitchen->kitchen_name, kitchen->email,
                               kitchen->wallet_address, &kitchen->balance);
                        trim_newline(kitchen->wallet_address); // Remove newline if present

                        fclose(file);
                        return kitchen;
                }
                current_index++;
        }

        fclose(file);
        return NULL;
}
