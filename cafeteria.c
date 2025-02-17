/* cafeteria.c */
#include "alu_blockchain.h"

/**
 * get_kitchen_vendor - Get kitchen vendor by index
 * @index: Kitchen index (0-based)
 * Return: Pointer to kitchen vendor or NULL if invalid
 */
const VendorProfile *get_kitchen_vendor(int index)
{
        if (index < 0 || index >= NUM_KITCHENS)
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
        for (i = 0; i < NUM_KITCHENS; i++)
        {
                if (strcmp(KITCHENS[i].email, email) == 0)
                        return 1;
        }
        return 0;
}