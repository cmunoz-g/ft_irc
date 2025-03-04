#include "IRC.hpp"

unsigned int Server::fetchClientIdFromPid(int fd) {
    for (std::map<unsigned int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->getSocket() == fd) {
            return it->first;
        }
    }
    std::stringstream ss;
	ss << "Client with file descriptor " << fd << " not found" << std::endl;
    error(ss.str(), true, false);
    return 0; 
}


void Server::setUpServerSocket() {
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0) {
        error("Error creating socket", true, true);
    }

    if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) < 0) {
        error("Error setting socket to non-blocking mode", true, true);
    }

    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        error("Error setting SO_REUSEADDR", true, true);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_port);

    if (bind(_server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        error("Error in bind", true, true);
    }

    if (listen(_server_fd, 10) < 0) {
        error("Error in listen", true, true);
    }
}


void Server::handleNewConnection() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    int client_fd = accept(_server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) {
            return;
        }
        error("Error accepting connection", false, true);
        return;
    }

    fcntl(client_fd, F_SETFL, O_NONBLOCK); // Sets non-blocking mode

    struct pollfd client_pollfd = {client_fd, POLLIN, 0};
    _pollfds.push_back(client_pollfd);

    // Finds the first available ID
    static unsigned int next_id = 1;
    while (_clients.find(next_id) != _clients.end()) {
        next_id++;
    }

    // Adds new client
    _clients[next_id] = new Client(client_fd, next_id);
    std::cout << "[LOG] [ID:" << next_id << "] New connection accepted" << std::endl;
}


bool Server::handleClientMessage(struct pollfd& pfd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = recv(pfd.fd, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_read <= 0) {
        if (bytes_read == 0 || errno != EWOULDBLOCK) {
            // Client disconnected or error occurred
            return false; // Signal that the client should be removed
        }
        return true; // No data, but client still connected
    }

    buffer[bytes_read] = '\0';
    unsigned int client_id = fetchClientIdFromPid(pfd.fd);

    if (client_id == 0) {
        std::cout << "[ERROR] Could not find client for fd " << pfd.fd << std::endl;
        return false; // Signal to remove this fd
    }

    std::cout << "[DEBUG] Handling client message" << " CLIENT ID: " << client_id << std::endl;

    if (client_id > 0) {
        Client *client = _clients[client_id];
      
        // Append data to client's buffer
        client->appendToBuffer(buffer);
        std::string buf = client->getBuffer();
        size_t pos;
        

        // Store all commands in a vector if irssi sends several commands together
        std::vector<std::string> commands;
        while ((pos = buf.find("\r\n")) != std::string::npos) {
            commands.push_back(buf.substr(0, pos)); // Store the full command
            buf.erase(0, pos + 2);
        }

        // Process /PASS first
        for (size_t i = 0; i < commands.size(); i++) {
            if (commands[i].substr(0, 4) == "PASS") { 
                client->setBuffer(commands[i]); 
                Message newMessage(client);
                handlePassCommand(newMessage);
                break; 
            }
        }

        for (size_t i = 0; i < commands.size(); i++) {
            client->setBuffer(commands[i]); // Set message buffer for parsing
            Message newMessage(client);
            newMessage.printMessageDebug();
            switch (newMessage.getCommandType()) {
                case IRC::CMD_NICK: handleNickCommand(newMessage); break;
                case IRC::CMD_USER: handleUserCommand(newMessage); break;
                case IRC::CMD_CAP: handleCapCommand(newMessage); break;
                case IRC::CMD_PRIVMSG: handlePrivmsgCommand(newMessage); break;
                case IRC::CMD_JOIN: handleJoinCommand(newMessage); break;
                case IRC::CMD_INVITE: handleInviteCommand(newMessage); break;
                case IRC::CMD_TOPIC: handleTopicCommand(newMessage); break;
                case IRC::CMD_MODE: handleModeCommand(newMessage); break;
                case IRC::CMD_PING: handlePingCommand(newMessage); break;
                case IRC::CMD_KICK: handleKickCommand(newMessage); break;
                case IRC::CMD_QUIT:
                    handleQuitCommand(newMessage);
                    return false;
                    break;
                default: break;
            }
        }

        client->setBuffer(buf);
    }

    return true; // Client is still connected
}

bool Server::checkUniqueNick(std::string nick) { // Test
	std::map<unsigned int, Client*>::iterator it = _clients.begin();

	while (it != _clients.end()) {
		if (it->second->getNickname() == nick)
			return (false); // not unique
		++it;
	}
	return (true);
}

void Server::deleteClients() {
	std::map<unsigned int, Client*>::iterator it = _clients.begin();

	while (it != _clients.end()) {
		if (it->second->getSocket() == -1) {
			it->second->cleanup();
			std::map<unsigned int, Client*>::iterator toErase = it;
			++it;
			_clients.erase(toErase);
		}
		else {
			++it;
		}
	} 
}

void Server::removeClient(unsigned int client_id) {
    std::map<unsigned int, Client*>::iterator it = _clients.find(client_id);
    if (it == _clients.end())
        return;
    
    Client* client = it->second;
    int fd = client->getSocket();
    
    // Remove from all channels
    std::map<const std::string, Channel*>::iterator ch_it;
    for (ch_it = _channels.begin(); ch_it != _channels.end(); ++ch_it) {
        if (ch_it->second->hasClient(client)) {
            ch_it->second->removeClient(client);
        }
    }
    
    // Remove from pollfds
    for (size_t i = 0; i < _pollfds.size(); ++i) {
        if (_pollfds[i].fd == fd) {
            close(fd); // Close socket ONLY HERE
            _pollfds.erase(_pollfds.begin() + i);
            break;
        }
    }
    
    // Remove from clients map
    _clients.erase(it);
    delete client; // Delete the client object
    
    std::cout << "[LOG] [ID:" << client_id << "] Client fully removed from server" << std::endl;
    std::cout << "[DEBUG] Number of clients after deletion: " << _clients.size() << std::endl;
}