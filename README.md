# DuplexChatWithEncryption
Full duplex chat in C supporting encrypting messages

This chat supports three types of communication:
1) No encryption
2) DES encryption with a key provided in a file
3) DES encryption with a key generated through Diffie-Hellman key exchange

Build with make

Usage: server.sh <port> <-n/-des/-dh>
       client.sh <hostname> <port> <-n/-des/-dh>
