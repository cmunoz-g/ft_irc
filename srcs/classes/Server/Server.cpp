/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/25 11:04:39 by juramos           #+#    #+#             */
/*   Updated: 2025/03/19 11:43:26 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IRC.hpp"

// *** Constructor & Destructor

Server::Server(int port, const std::string& password): _password(password) {
	if (port < 1 || port > 65535)
		error("Invalid port number", true, false);
	this->_port = port;
	setUpServerSocket();
}

Server::~Server() {
	//close(_server_fd);
}

// *** Getters ***

int	Server::getPort() const { return _port; }

const std::string	&Server::getPassword() const { return _password; }

// *** Member Functions ***

void Server::start() {
    Message::initCommandMap();
    _pollfds.clear(); // Ensures no leftovers

    struct pollfd server_pollfd = {_server_fd, POLLIN, 0};
    _pollfds.push_back(server_pollfd);

    while (g_running) {
        int ret = poll(_pollfds.data(), _pollfds.size(), -1);
    
        if (ret < 0) {
            if (errno == EINTR) {
                if (!g_running) {
                    break;
                }
                continue;
            }
            error("Fatal error in poll", true, true);
        }

        // Iterate in reverse order so we can safely erase closed sockets
        for (size_t i = _pollfds.size(); i-- > 0;) {
            if (!g_running)
                break;

            if (_pollfds[i].revents & POLLIN) {
                if (_pollfds[i].fd == _server_fd) {
                    // Accepts new connection
                    handleNewConnection();
                } else {
                    // handleClientMessage(...) returns false if the client should be removed
                    if (!handleClientMessage(_pollfds[i])) {
                        unsigned int client_id = fetchClientIdFromPid(_pollfds[i].fd);
                        if (client_id > 0) {
                            removeClient(client_id);
                        } else {
                            // Close FD if no client is associated
                            close(_pollfds[i].fd);
                            _pollfds.erase(_pollfds.begin() + i);
                        }
                    }
                }
            }
        }
    }

    std::cout << YELLOW << "[LOG] " << RESET << "[SERVER] Server was shut down" << std::endl;
}

void Server::cleanup() {
    if (_server_fd >= 0)
        close(_server_fd);

    for (std::map<unsigned int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        delete it->second;
    _clients.clear();

    for (std::map<const std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
        delete it->second;
    _channels.clear();

    _pollfds.clear();
}
