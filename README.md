# 💬 ft\_irc 💬

**Internet Relay Chat (IRC) Server**

A compliant IRC server implemented in C++98, capable of handling multiple simultaneous clients, channel management, and standard IRC commands over TCP/IP using non-blocking sockets and a poll-based event loop.

---

## Table of Contents

* [Features](#features)
* [Architecture](#architecture)
* [Getting Started](#getting-started)

  * [Prerequisites](#prerequisites)
  * [Build](#build)
  * [Run](#run)
* [Usage](#usage)

  * [Connecting with a Client](#connecting-with-a-client)
  * [Supported Commands](#supported-commands)
* [Code Structure](#code-structure)
* [Authors](#authors)

---

## Features

* **Multi-client support**: Handles concurrent connections without blocking using non-blocking sockets and `poll()`.
* **Authentication**: Clients must send a password (`PASS`), nickname (`NICK`), and username (`USER`) before joining.
* **Channel management**: Create/join channels, set topics, keys, user limits, invite-only mode.
* **Messaging**: Forward channel and private messages (`PRIVMSG`), support for `PING/PONG` keep-alive.
* **Operator commands**: Channel operators can `KICK`, `INVITE`, change `TOPIC`, and adjust `MODE` flags (`i`, `t`, `k`, `l`, `o`).
* **Capability negotiation**: Basic support for CAP subcommands (`LS`, `REQ`, `END`).
* **Graceful shutdown**: Handles `QUIT` commands and cleans up client state.

## Architecture

The server follows an event-driven design:

1. **Server Socket**: A single listening socket is created, set non-blocking, and registered with `poll()`.
2. **Event Loop**: `poll()` monitors the listening socket and all client sockets for input events.
3. **Connection Handling**: New connections are `accept()`ed, their sockets set non-blocking, and added to the poll set.
4. **Message Processing**:

   * Data is read into per-client buffers until complete commands (`\r\n`) are assembled.
   * The `Message` class parses raw input into commands and parameters.
   * Commands are dispatched to handler methods in `Server`.
5. **Client Management**: Each `Client` object tracks its socket, registration state, buffer, modes, and channel subscriptions.
6. **Channel Management**: `Channel` objects maintain member lists, operators, modes, topics, keys, and invited clients.

## Getting Started

### Prerequisites

* A C++98-compatible compiler (e.g., `g++`).
* POSIX environment (Linux, macOS).

### Build

```bash
git clone https://github.com/cmunoz-g/ft_irc.git
cd ft_irc
make
```

This produces the `ircserv` executable.

### Run

```bash
./ircserv <port> <password>
```

* `<port>`: TCP port for the server to listen on.
* `<password>`: Connection password required by clients.

## Usage

### Connecting with a Client

ft_irc was developed using irssi as the client for testing, but you can use any standard IRC client, as well as Netcat, to connect:

```bash
irssi --connect=127.0.0.1 --port=6667 --password=secret
# or
nc -C 127.0.0.1 6667
```

### Supported Commands

| Command   | Description                                        |
| --------- | -------------------------------------------------- |
| `PASS`    | Authenticate with server password                  |
| `NICK`    | Set or change nickname                             |
| `USER`    | Register username and real name                    |
| `CAP`     | Negotiate client capabilities (`LS`, `REQ`, `END`) |
| `JOIN`    | Create or join channels                            |
| `PRIVMSG` | Send messages to users or channels                 |
| `PING`    | Keep-alive; server responds with `PONG`            |
| `MODE`    | Adjust user or channel modes                       |
| `TOPIC`   | View or set channel topics                         |
| `KICK`    | Remove a user from a channel                       |
| `INVITE`  | Invite a user to an invite-only channel            |
| `QUIT`    | Disconnect from the server                         |

## Code Structure

* **main.cpp**: Sets up signal handlers, parses arguments, and starts the server.
* **Server.hpp / Server.cpp**: Core event loop, socket setup, connection dispatching, command routing.
* **ServerConnection.cpp**: Socket creation, `accept()`, non-blocking configuration, client removal.
* **ServerCommands.cpp**: Handlers for each IRC command.
* **ServerUtils.cpp**: Utility functions (broadcast, nickname uniqueness, registration flow).
* **signal.cpp**: Handles termination signals (`SIGINT`, `SIGTERM`) to gracefully stop the server by updating the global running flag.
* **utils.cpp**: General-purpose utilities: error logging/throwing, string-to-int conversion, and validation for modes and nicknames.
* **Client.cpp**: Tracks client state, buffers, modes, channel subscriptions, message sending.
* **Channel.cpp**: Manages channel state: members, operators, modes, topics, invites.
* **Message.cpp**: Parses raw input into commands and parameters; maps to enum types.

---

Happy chatting!

