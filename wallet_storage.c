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
                printf("Checking: %s vs Stored: %s\n", email, wallet.email); // Debugging
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

        file = fopen(WALLETS_FILE, "rb");
        if (!file)
                return NULL;

        while (fread(&stored_wallet, sizeof(StoredWallet), 1, file))
        {
                if (strcmp(stored_wallet.private_key, private_key) == 0)
                {
                        wallet = malloc(sizeof(Wallet));
                        if (wallet)
                        {
                                strncpy(wallet->email, stored_wallet.email, MAX_EMAIL - 1);
                                strncpy(wallet->private_key, stored_wallet.private_key, HASH_LENGTH);
                                strncpy(wallet->address, stored_wallet.address, HASH_LENGTH);
                                wallet->balance = stored_wallet.balance;
                                wallet->user_type = stored_wallet.user_type;
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
