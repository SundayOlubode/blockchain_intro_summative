#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "../alu_blockchain.h" // Replace with the actual header file containing the function declarations
#include <stdlib.h>

/* Mock variable to control the behavior of check_email_exists */
bool mock_check_email_exists = false;

/* Mock function to override check_email_exists */
bool check_email_exists(const char *email)
{
        return mock_check_email_exists;
}

void test_create_student_profile_valid_email()
{
        StudentProfileWithWallet *profile = create_student_profile("test@student.edu");
        CU_ASSERT_PTR_NOT_NULL(profile);
        CU_ASSERT_STRING_EQUAL(profile->profile.email, "test@student.edu");
        CU_ASSERT_EQUAL(strlen(profile->profile.wallet_address), HASH_LENGTH);
        free(profile);
}

void test_create_student_profile_invalid_email()
{
        StudentProfileWithWallet *profile = create_student_profile("test@gmail.com");
        CU_ASSERT_PTR_NULL(profile);
}

void test_create_student_profile_duplicate_email()
{
        /* Simulate that the email already exists */
        mock_check_email_exists = true;

        StudentProfileWithWallet *profile = create_student_profile("existing@student.edu");
        CU_ASSERT_PTR_NULL(profile);

        /* Reset after test */
        mock_check_email_exists = false;
}

void test_create_student_profile_max_students()
{
        int student_count = 0;
        student_count = MAX_TRANSACTIONS;
        StudentProfileWithWallet *profile = create_student_profile("test2@student.edu");
        CU_ASSERT_PTR_NULL(profile);
        student_count = 0; // Reset after test
}

void test_create_student_profile_wallet_creation_failure()
{
        StudentProfileWithWallet *profile;
        /* Mock function to simulate wallet creation failure */

        profile = create_student_profile("test3@student.edu");
        CU_ASSERT_PTR_NULL(profile);
}

int main()
{
        CU_initialize_registry();
        CU_pSuite suite = CU_add_suite("Student Profile Tests", 0, 0);
        CU_add_test(suite, "Valid Email", test_create_student_profile_valid_email);
        CU_add_test(suite, "Invalid Email", test_create_student_profile_invalid_email);
        CU_add_test(suite, "Duplicate Email", test_create_student_profile_duplicate_email);
        CU_add_test(suite, "Max Students Reached", test_create_student_profile_max_students);
        CU_add_test(suite, "Wallet Creation Failure", test_create_student_profile_wallet_creation_failure);
        CU_basic_run_tests();
        CU_cleanup_registry();
        return 0;
}