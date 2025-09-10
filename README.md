# ft\_irc

Internet Relay Chat (IRC) server implemented in C++. Supports multiple clients, channel management, and standard IRC commands over TCP/IP using non-blocking sockets and a poll-based event loop.


## Build and run

Requires a C++ compiler (e.g., `g++`) and a POSIX environment (Linux, macOS).

```bash
git clone https://github.com/cmunoz-g/ft_irc.git
cd ft_irc
make
```

This produces the `ircserv` executable.

```bash
./ircserv <port> <password>
```

* `<port>`: TCP port to listen on.
* `<password>`: Connection password required from clients.


## Usage

Connect using any IRC client or Netcat:

```bash
irssi --connect=127.0.0.1 --port=6667 --password=secret
# or
nc -C 127.0.0.1 6667
```

### Supported commands

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


## Features

* **Multi-client support:** Concurrent connections handled with non-blocking sockets and `poll()`.
* **Authentication:** Clients must send `PASS`, `NICK`, and `USER` before joining.
* **Channels:** Create/join channels, set topics, keys, user limits, and invite-only mode.
* **Messaging:** Forward private and channel messages. Supports `PING/PONG`.
* **Operators:** Channel operators can `KICK`, `INVITE`, set `TOPIC`, and change `MODE` flags (`i`, `t`, `k`, `l`, `o`).
* **Capabilities:** Basic `CAP` subcommands (`LS`, `REQ`, `END`).
* **Shutdown:** Handles `QUIT` cleanly and frees client resources.


## Implementation details

The server follows an event-driven model:

1. **Server socket:** A single listening socket is created, set non-blocking, and registered with `poll()`.
2. **Event loop:** `poll()` monitors listening and client sockets for input.
3. **Connections:** New sockets accepted, set non-blocking, and tracked.
4. **Message parsing:** Input is buffered until complete commands (`\r\n`) are received, then parsed by `Message`.
5. **Command dispatch:** Commands routed to handler methods in `Server`.
6. **Clients:** Each `Client` tracks its socket, state, buffer, modes, and channels.
7. **Channels:** `Channel` manages members, operators, modes, topics, and invitations.


## Code structure

* **main.cpp**: Entry point, signal setup, argument parsing, server start.
* **Server.hpp / Server.cpp**: Core event loop, connection handling, command routing.
* **ServerConnection.cpp**: Socket setup, `accept()`, non-blocking configuration, client removal.
* **ServerCommands.cpp**: IRC command handlers.
* **ServerUtils.cpp**: Utilities for broadcasting, nickname checks, registration.
* **signal.cpp**: Signal handling for clean shutdown.
* **utils.cpp**: Logging, error handling, string conversion, mode validation.
* **Client.cpp**: Client state, buffers, modes, and channel tracking.
* **Channel.cpp**: Channel state: members, operators, modes, topics, invites.
* **Message.cpp**: Command parsing and mapping to enums.


