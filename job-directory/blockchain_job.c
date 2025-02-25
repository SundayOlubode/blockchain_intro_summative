#ifndef BLOCKCHAIN_JOB_H
#define BLOCKCHAIN_JOB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>

/* Maximum lengths for job listing fields */
#define MAX_TITLE_LENGTH 100
#define MAX_COMPANY_LENGTH 100
#define MAX_LOCATION_LENGTH 100
#define MAX_DESCRIPTION_LENGTH 500
#define HASH_LENGTH 64

/**
 * struct JobListing - Structure for storing job information
 * @id: Unique identifier for the job
 * @title: Job title
 * @company: Company name
 * @location: Job location
 * @description: Job description
 * @timestamp: Time when job was posted
 * @prevHash: Hash of previous block
 * @currentHash: Hash of current block
 * @next: Pointer to next job listing
 */
typedef struct JobListing
{
        int id;
        char title[MAX_TITLE_LENGTH];
        char company[MAX_COMPANY_LENGTH];
        char location[MAX_LOCATION_LENGTH];
        char description[MAX_DESCRIPTION_LENGTH];
        time_t timestamp;
        char prevHash[HASH_LENGTH + 1];
        char currentHash[HASH_LENGTH + 1];
        struct JobListing *next;
} JobListing;

/**
 * struct JobBlockchain - Structure for managing the blockchain
 * @head: Pointer to first job listing
 * @count: Number of job listings
 */
typedef struct JobBlockchain
{
        JobListing *head;
        int count;
} JobBlockchain;

/* Function prototypes */
JobBlockchain *initialize_blockchain(void);
void calculate_hash(JobListing *job);
int add_job_listing(JobBlockchain *chain, const char *title,
                    const char *company, const char *location, const char *description);
JobListing *search_jobs(JobBlockchain *chain, const char *keyword);
int verify_integrity(JobBlockchain *chain);
void free_blockchain(JobBlockchain *chain);
void print_job(JobListing *job);

#endif

/**
 * generate_hash - Generate SHA-256 hash from input string
 * @input: Input string to hash
 * @output: Buffer to store resulting hash
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
 * initialize_blockchain - Create new blockchain
 * Return: Pointer to new blockchain or NULL on failure
 */
JobBlockchain *initialize_blockchain(void)
{
        JobBlockchain *chain = malloc(sizeof(JobBlockchain));

        if (chain == NULL)
                return (NULL);

        chain->head = NULL;
        chain->count = 0;
        return (chain);
}

/**
 * calculate_hash - Calculate hash for job listing
 * @job: Job listing to hash
 */
void calculate_hash(JobListing *job)
{
        char buffer[MAX_TITLE_LENGTH + MAX_COMPANY_LENGTH +
                    MAX_LOCATION_LENGTH + MAX_DESCRIPTION_LENGTH +
                    HASH_LENGTH + 100];

        sprintf(buffer, "%d%s%s%s%s%ld%s", job->id, job->title,
                job->company, job->location, job->description,
                job->timestamp, job->prevHash);

        generate_hash(buffer, job->currentHash);
}

/**
 * add_job_listing - Add new job to blockchain
 * @chain: Blockchain to add to
 * @title: Job title
 * @company: Company name
 * @location: Job location
 * @description: Job description
 * Return: 1 on success, 0 on failure
 */
int add_job_listing(JobBlockchain *chain, const char *title,
                    const char *company, const char *location,
                    const char *description)
{
        JobListing *new_job = malloc(sizeof(JobListing));
        JobListing *last = chain->head;

        if (new_job == NULL)
                return (0);

        new_job->id = chain->count + 1;
        strncpy(new_job->title, title, MAX_TITLE_LENGTH - 1);
        strncpy(new_job->company, company, MAX_COMPANY_LENGTH - 1);
        strncpy(new_job->location, location, MAX_LOCATION_LENGTH - 1);
        strncpy(new_job->description, description, MAX_DESCRIPTION_LENGTH - 1);
        new_job->timestamp = time(NULL);
        new_job->next = NULL;

        if (chain->head == NULL)
        {
                /* First block */
                strcpy(new_job->prevHash, "0000000000000000000000000000000000000000000000000000000000000000");
                calculate_hash(new_job);
                chain->head = new_job;
        }
        else
        {
                /* Find the last block in the chain */
                while (last->next != NULL)
                        last = last->next;

                /* Set new block's prevHash to last block's currentHash */
                strcpy(new_job->prevHash, last->currentHash);
                calculate_hash(new_job);

                /* Append to the end of the chain */
                last->next = new_job;
        }

        chain->count++;
        return (1);
}

/**
 * search_jobs - Search for jobs by keyword
 * @chain: Blockchain to search
 * @keyword: Search keyword
 * Return: List of matching jobs or NULL if none found
 */
JobListing *search_jobs(JobBlockchain *chain, const char *keyword)
{
        JobListing *results = NULL, *current = chain->head;
        JobListing *last_result = NULL, *match;

        while (current != NULL)
        {
                if (strstr(current->title, keyword) ||
                    strstr(current->company, keyword) ||
                    strstr(current->description, keyword))
                {
                        match = malloc(sizeof(JobListing));
                        if (match == NULL)
                        {
                                current = current->next;
                                continue;
                        }
                        memcpy(match, current, sizeof(JobListing));
                        match->next = NULL;

                        if (results == NULL)
                        {
                                results = match;
                                last_result = match;
                        }
                        else
                        {
                                last_result->next = match;
                                last_result = match;
                        }
                }
                current = current->next;
        }
        return (results);
}

/**
 * verify_integrity - Check blockchain integrity
 * @chain: Blockchain to verify
 * Return: 1 if intact, 0 if compromised
 */
/**
 * verify_integrity - Check blockchain integrity and write results to file
 * @chain: Blockchain to verify
 * Return: 1 if intact, 0 if compromised
 */
int verify_integrity(JobBlockchain *chain)
{
        JobListing *current;
        char temp_hash[HASH_LENGTH + 1];
        FILE *output_file;
        int block_count = 0;
        int is_valid = 1;

        /* Open output file for writing */
        output_file = fopen("output.txt", "w");
        if (output_file == NULL)
        {
                perror("Error opening output.txt");
                return (-1);
        }

        fprintf(output_file, "=== BLOCKCHAIN INTEGRITY VERIFICATION ===\n\n");

        if (chain->head == NULL)
        {
                fprintf(output_file, "Blockchain is empty. Integrity valid.\n");
                fclose(output_file);
                return (1);
        }

        /* First pass: check hash recalculation and collect block info */
        current = chain->head;
        while (current != NULL)
        {
                block_count++;
                fprintf(output_file, "Block #%d:\n", block_count);
                fprintf(output_file, "  Title: %s\n", current->title);
                fprintf(output_file, "  Current Hash: %s\n", current->currentHash);
                if (current->next != NULL)
                {
                        fprintf(output_file, "  Next Block: %s\n", current->next->title);
                }

                /* Verify hash hasn't been tampered with */
                strcpy(temp_hash, current->currentHash);
                calculate_hash(current);
                if (strcmp(temp_hash, current->currentHash) != 0)
                {
                        fprintf(output_file, "  HASH TAMPERING: Recalculated hash doesn't match stored hash\n");
                        fprintf(output_file, "  Stored hash: %s\n", temp_hash);
                        fprintf(output_file, "  Recalculated hash: %s\n", current->currentHash);
                        is_valid = 0;
                }
                else
                {
                        /* Restore the original hash */
                        strcpy(current->currentHash, temp_hash);
                }

                current = current->next;
        }

        /* Second pass: check chain integrity - this matches original logic */
        current = chain->head;
        while (current->next != NULL)
        {
                /* Check if the prevHash of the next block matches this block's currentHash */
                if (strcmp(current->currentHash, current->next->prevHash) != 0)
                {
                        fprintf(output_file, "\nBlock #%d -> Block #%d integrity error:\n",
                                block_count - (current->next->next ? 2 : 1),
                                block_count - (current->next->next ? 1 : 0));
                        fprintf(output_file, "  Current block hash: %s\n", current->currentHash);
                        fprintf(output_file, "  Next block prevHash: %s\n", current->next->prevHash);
                        fprintf(output_file, "  HASH MISMATCH: Chain integrity compromised\n");
                        is_valid = 0;
                }

                current = current->next;
        }

        fprintf(output_file, "\n=== VERIFICATION SUMMARY ===\n");
        fprintf(output_file, "Total blocks: %d\n", block_count);
        fprintf(output_file, "Blockchain integrity: %s\n",
                is_valid ? "INTACT" : "COMPROMISED");

        fclose(output_file);
        return (is_valid);
}

/**
 * free_blockchain - Free all blockchain memory
 * @chain: Blockchain to free
 */
void free_blockchain(JobBlockchain *chain)
{
        JobListing *current = chain->head;
        JobListing *temp;

        while (current != NULL)
        {
                temp = current;
                current = current->next;
                free(temp);
        }
        free(chain);
}

/**
 * print_job - Display job listing information
 * @job: Job to display
 */
void print_job(JobListing *job)
{
        printf("\nJob ID: %d\n", job->id);
        printf("Title: %s\n", job->title);
        printf("Company: %s\n", job->company);
        printf("Location: %s\n", job->location);
        printf("Description: %s\n", job->description);
        printf("Timestamp: %s", ctime(&job->timestamp));
        printf("Previous Hash: %.16s...\n", job->prevHash);
        printf("Current Hash: %.16s...\n", job->currentHash);
}

/**
 * clear_input_buffer - Clears input buffer
 */
void clear_input_buffer(void)
{
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
                ;
}

/**
 * get_string_input - Gets string input safely
 * @prompt: Prompt to display
 * @buffer: Buffer to store input
 * @size: Size of buffer
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
 * display_menu - Displays main menu
 */
void display_menu(void)
{
        printf("\n=== Blockchain Job Directory ===\n");
        printf("1. Add new job listing\n");
        printf("2. View all job listings\n");
        printf("3. Search jobs\n");
        printf("4. Verify blockchain integrity\n");
        printf("5. Exit\n");
        printf("Enter your choice (1-5): ");
}

/**
 * main - Entry point
 * Return: 0 on success, 1 on failure
 */
int main(void)
{
        JobBlockchain *chain = initialize_blockchain();
        JobListing *current, *results, *temp;
        int choice;
        char title[MAX_TITLE_LENGTH];
        char company[MAX_COMPANY_LENGTH];
        char location[MAX_LOCATION_LENGTH];
        char description[MAX_DESCRIPTION_LENGTH];
        char keyword[50];

        if (chain == NULL)
        {
                printf("Failed to initialize blockchain\n");
                return (1);
        }

        /* Initialize with sample job listings */
        add_job_listing(chain, "Backend Developer", "TechFlow Inc",
                        "Remote", "Looking for a skilled backend developer with 3+ years of "
                                  "experience in Python, Django, and PostgreSQL. Knowledge of RESTful "
                                  "APIs and microservices architecture required.");

        add_job_listing(chain, "Data Engineer", "DataMinds Corp",
                        "New York, NY", "Seeking experienced data engineer for ETL pipeline "
                                        "development. Must have strong SQL skills and experience with Apache "
                                        "Spark and AWS services. Big data experience is a plus.");

        add_job_listing(chain, "DevOps Specialist", "CloudNet Solutions",
                        "San Francisco, CA", "Join our team as a DevOps specialist. "
                                             "Experience with Docker, Kubernetes, and CI/CD pipelines required. "
                                             "AWS certification preferred.");

        printf("Initialized blockchain with 3 sample job listings.\n");

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
                case 1:
                        printf("\n=== Add New Job Listing ===\n");
                        get_string_input("Enter job title: ", title, MAX_TITLE_LENGTH);
                        get_string_input("Enter company name: ", company, MAX_COMPANY_LENGTH);
                        get_string_input("Enter location: ", location, MAX_LOCATION_LENGTH);
                        get_string_input("Enter job description: ", description, MAX_DESCRIPTION_LENGTH);

                        if (add_job_listing(chain, title, company, location, description))
                                printf("Job listing added successfully!\n");
                        else
                                printf("Failed to add job listing.\n");
                        break;

                case 2:
                        printf("\n=== All Job Listings ===\n");
                        current = chain->head;
                        if (current == NULL)
                                printf("No job listings found.\n");
                        while (current != NULL)
                        {
                                print_job(current);
                                current = current->next;
                        }
                        break;

                case 3:
                        printf("\n=== Search Jobs ===\n");
                        get_string_input("Enter search keyword: ", keyword, sizeof(keyword));
                        results = search_jobs(chain, keyword);
                        current = results;

                        if (current == NULL)
                                printf("No matching jobs found.\n");
                        while (current != NULL)
                        {
                                print_job(current);
                                temp = current;
                                current = current->next;
                                free(temp);
                        }
                        break;

                case 4:
                        printf("\n=== Blockchain Integrity Check ===\n");
                        if (verify_integrity(chain))
                                printf("Blockchain integrity verified: No tampering detected\n");
                        else
                                printf("Warning: Blockchain integrity compromised!\n");
                        break;

                case 5:
                        printf("\nExiting program. Goodbye!\n");
                        free_blockchain(chain);
                        return (0);

                default:
                        printf("Invalid choice. Please try again.\n");
                }
        }
}