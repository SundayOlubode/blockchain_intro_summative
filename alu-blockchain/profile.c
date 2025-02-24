/* profile.c */
#include "alu_blockchain.h"

/* Static profile storage */
static StudentProfile students[MAX_TRANSACTIONS];
static StaffProfile staff[MAX_TRANSACTIONS];
static VendorProfile vendors[MAX_TRANSACTIONS];
static int student_count = 0;
static int staff_count = 0;
static int vendor_count = 0;

static unsigned int next_student_id = 1000;
static unsigned int next_staff_id = 5000;
static unsigned int next_vendor_id = 9000;

/**
 * save_profiles_to_file - Save all profiles to file
 * Return: 1 on success, 0 on failure
 */
int save_profiles_to_file(void)
{
        FILE *file;

        file = fopen(PROFILES_FILE, "wb");
        if (!file)
                return 0;

        /* Write counters */
        fwrite(&student_count, sizeof(int), 1, file);
        fwrite(&staff_count, sizeof(int), 1, file);
        fwrite(&vendor_count, sizeof(int), 1, file);
        fwrite(&next_student_id, sizeof(unsigned int), 1, file);
        fwrite(&next_staff_id, sizeof(unsigned int), 1, file);
        fwrite(&next_vendor_id, sizeof(unsigned int), 1, file);

        /* Write profiles */
        fwrite(students, sizeof(StudentProfile), student_count, file);
        fwrite(staff, sizeof(StaffProfile), staff_count, file);
        fwrite(vendors, sizeof(VendorProfile), vendor_count, file);

        fclose(file);
        return 1;
}

/**
 * load_profiles_from_file - Load all profiles from file
 * Return: 1 on success, 0 if file doesn't exist or error
 */
int load_profiles_from_file(void)
{
        FILE *file;

        file = fopen(PROFILES_FILE, "rb");
        if (!file)
                return 0;

        /* Read counters */
        fread(&student_count, sizeof(int), 1, file);
        fread(&staff_count, sizeof(int), 1, file);
        fread(&vendor_count, sizeof(int), 1, file);
        fread(&next_student_id, sizeof(unsigned int), 1, file);
        fread(&next_staff_id, sizeof(unsigned int), 1, file);
        fread(&next_vendor_id, sizeof(unsigned int), 1, file);

        /* Read profiles */
        fread(students, sizeof(StudentProfile), student_count, file);
        fread(staff, sizeof(StaffProfile), staff_count, file);
        fread(vendors, sizeof(VendorProfile), vendor_count, file);

        fclose(file);
        return 1;
}

/**
 * create_student_profile - Create new student profile and wallet
 * @name: Student's full name
 * @email: Student's email address
 * @year: Year of study
 * @program: Study program/major
 * Return: Combined profile and wallet struct or NULL on failure
 */
StudentProfileWithWallet *create_student_profile(const char *email)
{
        Wallet *wallet;
        StudentProfileWithWallet *combined;

        if (!email)
                return NULL;

        if (strstr(email, STUDENT_DOMAIN) == NULL)
        {
                printf("Invalid email domain. Must use %s\n", STUDENT_DOMAIN);
                return NULL;
        }

        if (check_email_exists(email))
        {
                printf("Email already registered\n");
                return NULL;
        }

        if (student_count >= MAX_TRANSACTIONS)
        {
                printf("Maximum number of students reached\n");
                return NULL;
        }

        /* Create wallet first */
        wallet = create_wallet(email, NULL);
        if (!wallet)
        {
                printf("Failed to create wallet\n");
                return NULL;
        }

        /* Allocate memory for combined struct */
        combined = (StudentProfileWithWallet *)malloc(sizeof(StudentProfileWithWallet));
        if (!combined)
        {
                printf("Failed to allocate memory for profile and wallet\n");
                free(wallet);
                return NULL;
        }

        /* Initialize profile */
        combined->profile.student_id = next_student_id++;
        strncpy(combined->profile.email, email, MAX_EMAIL - 1);
        strncpy(combined->profile.wallet_address, wallet->address, HASH_LENGTH);

        memcpy(&combined->wallet, wallet, sizeof(Wallet));

        printf("\nStudent Profile Created:\n");
        printf("Email: %s\n", combined->profile.email);
        printf("Wallet Address: %s\n", combined->profile.wallet_address);

        student_count++;
        free(wallet); // Free temporary wallet

        save_profiles_to_file();
        return combined;
}

/**
 * create_staff_profile - Create new staff profile
 * @name: Staff member's name
 * @email: Staff email
 * @department: Department/faculty
 * @role: Staff role
 * Return: New staff profile or NULL on failure
 */
StaffProfileWithWallet *create_staff_profile(const char *email)
{
        Wallet *wallet;
        StaffProfileWithWallet *combined;

        if (!email)
                return NULL;

        if (strstr(email, STAFF_DOMAIN) == NULL)
        {
                printf("Invalid email domain. Must use %s\n", STAFF_DOMAIN);
                return NULL;
        }

        if (check_email_exists(email))
        {
                printf("Email already registered\n");
                return NULL;
        }

        if (staff_count >= MAX_TRANSACTIONS)
        {
                printf("Maximum number of staff reached\n");
                return NULL;
        }

        wallet = create_wallet(email, NULL);
        if (!wallet)
        {
                printf("Failed to create wallet\n");
                return NULL;
        }

        /* Allocate memory for combined struct */
        combined = (StaffProfileWithWallet *)malloc(sizeof(StaffProfileWithWallet));
        if (!combined)
        {
                printf("Failed to allocate memory for staff profile and wallet\n");
                free(wallet);
                return NULL;
        }

        /* Initialize staff profile */
        combined->profile.staff_id = next_staff_id++;
        strncpy(combined->profile.email, email, MAX_EMAIL - 1);
        strncpy(combined->profile.wallet_address, wallet->address, HASH_LENGTH);

        /* Copy wallet into combined struct */
        memcpy(&combined->wallet, wallet, sizeof(Wallet));

        printf("\nStaff Profile Created:\n");
        printf("Email: %s\n", combined->profile.email);
        printf("Wallet Address: %s\n", combined->profile.wallet_address);

        staff_count++;
        free(wallet);
        save_profiles_to_file();
        return combined;
}

/**
 * create_vendor_profile - Create new vendor profile
 * @name: Vendor's name (kitchen name)
 * @email: Vendor's email
 * Return: New vendor profile or NULL on failure
 */
VendorProfileWithWallet *create_vendor_profile(const char *kitchen_name, const char *email)
{
        Wallet *wallet;
        VendorProfileWithWallet *combined;

        if (!kitchen_name || !email)
                return NULL;

        if (strstr(email, VENDOR_DOMAIN) == NULL)
        {
                printf("Invalid email domain. Must use %s\n", VENDOR_DOMAIN);
                return NULL;
        }

        if (check_email_exists(email))
        {
                printf("Email already registered\n");
                return NULL;
        }

        if (vendor_count >= MAX_TRANSACTIONS)
        {
                printf("Maximum number of vendors reached\n");
                return NULL;
        }

        wallet = create_wallet(email, kitchen_name);
        if (!wallet)
        {
                printf("Failed to create wallet\n");
                return NULL;
        }

        /* Allocate memory for combined struct */
        combined = (VendorProfileWithWallet *)malloc(sizeof(VendorProfileWithWallet));
        if (!combined)
        {
                printf("Failed to allocate memory for vendor profile and wallet\n");
                free(wallet);
                return NULL;
        }

        /* Initialize vendor profile */
        combined->profile.vendor_id = next_vendor_id++;
        strncpy(combined->profile.kitchen_name, kitchen_name, MAX_NAME - 1);
        strncpy(combined->profile.email, email, MAX_EMAIL - 1);
        strncpy(combined->profile.wallet_address, wallet->address, HASH_LENGTH);
        combined->profile.balance = 0.0;

        /* Copy wallet into combined struct */
        memcpy(&combined->wallet, wallet, sizeof(Wallet));

        printf("\nVendor Profile Created:\n");
        printf("ID: %u\n", combined->profile.vendor_id);
        printf("Kitchen Name: %s\n", combined->profile.kitchen_name);
        printf("Email: %s\n", combined->profile.email);
        printf("Wallet Address: %s\n", combined->profile.wallet_address);

        printf("Vendor wallet address: %s\n", wallet->address);
        add_kitchen(kitchen_name, wallet->email, wallet->address);

        vendor_count++;
        free(wallet);
        save_profiles_to_file();
        return combined;
}

/**
 * get_profile_by_email - Get profile by email address
 * @email: Email to search for
 * @type: Pointer to store profile type
 * Return: Void pointer to profile or NULL if not found
 */
void *get_profile_by_email(const char *email, UserType *type)
{
        int i;

        for (i = 0; i < student_count; i++)
        {
                if (strcmp(students[i].email, email) == 0)
                {
                        *type = STUDENT;
                        return &students[i];
                }
        }

        for (i = 0; i < staff_count; i++)
        {
                if (strcmp(staff[i].email, email) == 0)
                {
                        *type = STAFF;
                        return &staff[i];
                }
        }

        for (i = 0; i < vendor_count; i++)
        {
                if (strcmp(vendors[i].email, email) == 0)
                {
                        *type = VENDOR;
                        return &vendors[i];
                }
        }

        return NULL;
}