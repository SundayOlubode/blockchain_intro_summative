/* config.h */
#ifndef CONFIG_H
#define CONFIG_H

#include "alu_blockchain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define CONFIG_FILE "config.txt"
#define BACKUP_FILE "blockchain_backup.dat"

typedef struct
{
        unsigned int initial_supply;
        unsigned int circulating_supply;
        unsigned int block_reward;
        unsigned int max_transactions;
        char backup_directory[256];
        int auto_backup;
        int backup_interval;
} Config;

Config *load_config(void);
void save_config(Config *config);
int backup_blockchain(const Blockchain *chain);
int restore_blockchain(Blockchain **chain);
void create_default_config(void);

#endif /* CONFIG_H */