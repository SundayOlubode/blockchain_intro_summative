UNITY_DIR = Unity/src
INCLUDES = -I$(UNITY_DIR) -I.
TEST_DIR = test
LIBS = -lcrypto -lssl  # Add OpenSSL libraries

# Your implementation source files
SRC_FILES = ./alu_blockchain.c ./wallet.c ./config.c ./profile.c

all: test

test: test_runner
	./test_runner

test_runner: $(TEST_DIR)/test_blockchain_core.c $(SRC_FILES)
	gcc $(INCLUDES) -o test_runner $(TEST_DIR)/test_blockchain_core.c $(SRC_FILES) $(UNITY_DIR)/unity.c $(LIBS)

clean:
	rm -f test_runner