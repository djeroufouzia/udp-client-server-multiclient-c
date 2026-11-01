# UDP Client-Server Communication System with Multiple Clients

## Project Overview

This project implements a UDP-based client-server communication system in C.

A single UDP server manages multiple clients and allows bidirectional communication between the server and each registered client.

The server uses `select()` to monitor incoming UDP messages and keyboard input, while each client uses POSIX threads (`pthread`) to handle sending and receiving operations concurrently.

**Architecture:** One Server – Multiple Clients.

## Features

- UDP communication using IPv4 sockets.
- One server handling multiple clients.
- Client identification using IP addresses and port numbers.
- Client registration and management.
- I/O multiplexing using `select()`.
- Concurrent sending and receiving using POSIX threads.
- Individual server-to-client messaging.
- Command-line interface.

## Technologies

- C Programming Language
- UDP / IPv4
- POSIX Sockets API
- POSIX Threads (pthreads)
- `select()`
- GCC
- macOS / Linux

## Project Structure

```text
udp-client-server-multiclient-c/
├── src/
│   ├── server.c
│   └── client.c
├── screenshots/
├── README.md
└── .gitignore
```

## Compilation

Compile the server:

```bash
gcc src/server.c -o server
```

Compile the client:

```bash
gcc src/client.c -o client -pthread
```

## Execution

1. Start the server:

```bash
./server
```

2. Open another terminal and start the first client:

```bash
./client
```

3. Open additional terminals to launch more clients.

4. Send messages from each client to the server.

5. Use the server terminal to send individual replies to registered clients.

## Architecture

### Server

- Creates and binds a UDP socket.
- Uses `select()` to monitor the UDP socket and standard input.
- Receives messages using `recvfrom()`.
- Identifies clients using their IP addresses and port numbers.
- Stores registered clients in an array.
- Sends individual messages using `sendto()`.

### Client

- Creates a UDP socket.
- Uses the main thread to read user input and send messages.
- Uses a receiver thread to receive server messages.
- Uses `sendto()` and `recvfrom()` for communication.


## Learning Objectives

- Understand UDP socket programming in C.
- Implement client-server communication.
- Manage multiple clients.
- Understand I/O multiplexing with `select()`.
- Implement concurrent operations using POSIX threads.
- Handle network messages and client addresses.

## Future Improvements

- Add client usernames.
- Implement broadcast messaging.
- Add message acknowledgements.
- Improve client disconnection handling.
- Add authentication and secure communication.
