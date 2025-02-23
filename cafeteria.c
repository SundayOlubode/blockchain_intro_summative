#include "alu_blockchain.h"

/* Predefined kitchen vendors - static to limit scope to this file */
static const VendorProfile KITCHENS[NUM_KITCHENS] = {
    {.vendor_id = 9001,
     .kitchen_name = Pascal_Kitchen,
     .email = "pascal@aluvendor.com",
     .wallet_address = Pascal_Kitchen_ADDRESS,
     .balance = 0.0},
    {.vendor_id = 9002,
     .kitchen_name = Pius_Cuisine,
     .email = "pius@aluvendor.com",
     .wallet_address = Pius_Cuisine_ADDRESS,
     .balance = 0.0},
    {.vendor_id = 9003,
     .kitchen_name = Joshua_Kitchen,
     .email = "joshua@aluvendor.com",
     .wallet_address = Joshua_Kitchen_ADDRESS,
     .balance = 0.0}};

/* Keep track of active kitchens */
static int active_kitchen_count = 3;

/**
 * get_kitchen_vendor - Get kitchen vendor by index
 * @index: Kitchen index (0-based)
 * Return: Pointer to kitchen vendor or NULL if invalid
 */
const VendorProfile *get_kitchen_vendor(int index)
{
        if (index < 0 || index >= active_kitchen_count)
                return NULL;
        return &KITCHENS[index];
}

/**
 * is_kitchen_email - Check if email belongs to a kitchen vendor
 * @email: Email to check
 * Return: 1 if kitchen vendor, 0 if not
 */
int is_kitchen_email(const char *email)
{
        int i;

        for (i = 0; i < active_kitchen_count; i++)
        {
                if (strcmp(KITCHENS[i].email, email) == 0)
                        return 1;
        }
        return 0;
}

/**
 * get_kitchen_address - Get kitchen wallet address by name
 * @kitchen_name: Name of the kitchen
 * Return: Kitchen's wallet address or NULL if not found
 */
const char *get_kitchen_address(const char *kitchen_name)
{
        int i;

        for (i = 0; i < active_kitchen_count; i++)
        {
                if (strcmp(KITCHENS[i].kitchen_name, kitchen_name) == 0)
                        return KITCHENS[i].wallet_address;
        }
        return NULL;
}

/**
 * display_all_kitchens - Display all active kitchen vendors
 */
void display_all_kitchens(void)
{
        int i;

        printf("\n=== Available Kitchens ===\n");
        for (i = 0; i < active_kitchen_count; i++)
        {
                printf("%d. %s\n", i + 1, KITCHENS[i].kitchen_name);
        }
}