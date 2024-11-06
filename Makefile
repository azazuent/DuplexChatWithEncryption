CC=gcc
build: build_server build_client
	openssl rand -out ./des_key.bin 8
clean:
	rm -rf ./build/

rebuild: clean build

build_server:
	mkdir -p ./build & $(CC) ./code/server.c -lcrypto -o ./build/server -Wno-deprecated-declarations
clean_server:
	rm -f ./build/server

build_client:
	mkdir -p ./build & $(CC) ./code/client.c -lcrypto -o ./build/client -Wno-deprecated-declarations
clean_client:
	rm -f ./build/client

test:
	mkdir -p ./build && $(CC) ./code/test.c -lcrypto -o ./build/test -Wno-deprecated-declarations && ./build/test
