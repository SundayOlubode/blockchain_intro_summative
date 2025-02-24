/* config.c */
#include "alu_blockchain.h"
#include "config.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * create_default_config - Create default configuration file
 */
void create_default_config(void)
{
        FILE *file;

        file = fopen(CONFIG_FILE, "w");
        if (!file)
                return;

        fprintf(file, "# ALU Blockchain Configuration\n");
        fprintf(file, "initial_supply=1000000\n");
        fprintf(file, "block_reward=2\n");
        fprintf(file, "max_transactions=100\n");
        fprintf(file, "backup_directory=./backups\n");
        fprintf(file, "auto_backup=1\n");
        fprintf(file, "backup_interval=10\n");

        fclose(file);
}

/**
 * load_config - Load configuration from file
 * Return: Config structure or NULL on failure
 */
Config *load_config(void)
{
        FILE *file;
        Config *config;
        char line[256];
        char *value;

        config = malloc(sizeof(Config));
        if (!config)
                return NULL;

        /* Set default values */
        config->initial_supply = INITIAL_SUPPLY;
        config->circulating_supply = CIRCULATING_SUPPLY;
        config->block_reward = BLOCK_REWARD;
        config->max_transactions = 100;
        strcpy(config->backup_directory, "./backups");
        config->auto_backup = 1;
        config->backup_interval = 10;

        file = fopen(CONFIG_FILE, "r");
        if (!file)
        {
                create_default_config();
                return config;
        }

        while (fgets(line, sizeof(line), file))
        {
                /* Skip comments and empty lines */
                if (line[0] == '#' || line[0] == '\n')
                        continue;

                /* Remove newline */
                line[strcspn(line, "\n")] = 0;

                value = strchr(line, '=');
                if (!value)
                        continue;
                *value = '\0';
                value++;

                if (strcmp(line, "initial_supply") == 0)
                        config->initial_supply = (unsigned int)atoi(value);
                else if (strcmp(line, "block_reward") == 0)
                        config->block_reward = (unsigned int)atoi(value);
                else if (strcmp(line, "max_transactions") == 0)
                        config->max_transactions = (unsigned int)atoi(value);
                else if (strcmp(line, "backup_directory") == 0)
                        strncpy(config->backup_directory, value, 255);
                else if (strcmp(line, "auto_backup") == 0)
                        config->auto_backup = atoi(value);
                else if (strcmp(line, "backup_interval") == 0)
                        config->backup_interval = atoi(value);
        }

        fclose(file);
        return config;
}

/**
 * save_config - Save configuration to file
 * @config: Configuration to save
 */
void save_config(Config *config)
{
        FILE *file;

        if (!config)
                return;

        file = fopen(CONFIG_FILE, "w");
        if (!file)
                return;

        fprintf(file, "# ALU Blockchain Configuration\n");
        fprintf(file, "initial_supply=%u\n", config->initial_supply);
        fprintf(file, "block_reward=%u\n", config->block_reward);
        fprintf(file, "max_transactions=%u\n", config->max_transactions);
        fprintf(file, "backup_directory=%s\n", config->backup_directory);
        fprintf(file, "auto_backup=%d\n", config->auto_backup);
        fprintf(file, "backup_interval=%d\n", config->backup_interval);

        fclose(file);
}

/**
 * backup_blockchain - Backup blockchain to file
 * @chain: Blockchain to backup
 * Return: 1 on success, 0 on failure
 */
int backup_blockchain(const Blockchain *chain)
{
        FILE *file;
        Block *current;
        char backup_path[512];
        time_t now;
        struct tm *timeinfo;
        Config *config;

        if (!chain)
                return 0;

        config = load_config();
        if (!config)
                return 0;

/* Create backup directory if it doesn't exist */
#ifdef _WIN32
        mkdir(config->backup_directory);
#else
        mkdir(config->backup_directory, 0777);
#endif

        /* Create backup filename with timestamp */
        time(&now);
        timeinfo = localtime(&now);
        sprintf(backup_path, "%s/backup_%04d%02d%02d_%02d%02d%02d.dat",
                config->backup_directory,
                timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
                timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

        file = fopen(backup_path, "wb");
        if (!file)
        {
                free(config);
                return 0;
        }

        /* Write blockchain metadata */
        fwrite(&chain->block_count, sizeof(int), 1, file);
        fwrite(&chain->token, sizeof(Token), 1, file);

        /* Write all blocks */
        current = chain->genesis;
        while (current)
        {
                fwrite(current, sizeof(Block), 1, file);
                current = current->next;
        }

        fclose(file);
        free(config);
        return 1;
}

/**
 * restore_blockchain - Restore blockchain from backup
 * @chain: Pointer to blockchain pointer
 * Return: 1 on success, 0 on failure
 */
int restore_blockchain(Blockchain **chain)
{
        FILE *file;
        Block *current;
        Block *next;
        int block_count;
        int i;
        Config *config;
        char latest_backup[512] = {0};
        DIR *dir;
        struct dirent *entry;
        time_t latest_time = 0;
        char backup_path[512];
        struct stat st;

        config = load_config();
        if (!config)
                return 0;

        /* Find latest backup file */
        dir = opendir(config->backup_directory);
        if (!dir)
        {
                free(config);
                return 0;
        }

        while ((entry = readdir(dir)))
        {
                if (strstr(entry->d_name, "backup_") && strstr(entry->d_name, ".dat"))
                {
                        sprintf(backup_path, "%s/%s", config->backup_directory, entry->d_name);
                        if (stat(backup_path, &st) == 0)
                        {
                                if (st.st_mtime > latest_time)
                                {
                                        latest_time = st.st_mtime;
                                        strcpy(latest_backup, backup_path);
                                }
                        }
                }
        }
        closedir(dir);

        if (!latest_backup[0])
        {
                free(config);
                return 0;
        }

        file = fopen(latest_backup, "rb");
        if (!file)
        {
                free(config);
                return 0;
        }

        /* Free existing blockchain if any */
        if (*chain)
        {
                current = (*chain)->genesis;
                while (current)
                {
                        next = current->next;
                        free(current);
                        current = next;
                }
                free(*chain);
        }

        /* Create new blockchain */
        *chain = malloc(sizeof(Blockchain));
        if (!*chain)
        {
                fclose(file);
                free(config);
                return 0;
        }

        /* Read blockchain metadata */
        fread(&block_count, sizeof(int), 1, file);
        fread(&(*chain)->token, sizeof(Token), 1, file);

        /* Read all blocks */
        (*chain)->genesis = NULL;
        (*chain)->latest = NULL;
        (*chain)->block_count = 0;

        for (i = 0; i < block_count; i++)
        {
                current = malloc(sizeof(Block));
                if (!current)
                {
                        fclose(file);
                        free(config);
                        /* TODO: Cleanup partial blockchain */
                        return 0;
                }

                fread(current, sizeof(Block), 1, file);
                current->next = NULL;

                if (!(*chain)->genesis)
                {
                        (*chain)->genesis = current;
                        (*chain)->latest = current;
                }
                else
                {
                        (*chain)->latest->next = current;
                        (*chain)->latest = current;
                }
                (*chain)->block_count++;
        }

        fclose(file);
        free(config);
        return 1;
}