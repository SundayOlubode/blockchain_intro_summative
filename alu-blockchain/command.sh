rm -r ./backups ./wallets.dat ./transactions.dat ./txpool.dat ./kitchens.txt ./profiles.dat
gcc -Wall -Werror -Wextra -pedantic -std=c99 main.c alu_blockchain.c config.c wallet.c profile.c -o alu_payment -lssl -lcrypto
./alu_payment