/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/25 11:04:39 by juramos           #+#    #+#             */
/*   Updated: 2025/02/27 18:32:03 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IRC.hpp"

Server::Server(void): _port(6667), _password("password") {
		setUpServerSocket();
}

Server::Server(int port, const std::string& password):
	_port(port), _password(password) {
		setUpServerSocket();
}

int	Server::getPort() const { return _port; }

const std::string	&Server::getPassword() const { return _password; }

Server::~Server() {
	close(_server_fd);
}

/**/

void Server::start() {
    Message::initCommandMap();
    _pollfds.clear(); // Ensure no leftovers

    struct pollfd server_pollfd = {_server_fd, POLLIN, 0};
    _pollfds.push_back(server_pollfd);

    while (true) {
        int ret = poll(_pollfds.data(), _pollfds.size(), -1);

        if (ret < 0) {
            if (errno == EINTR) {
                continue; // Ignore interrupts (e.g., signal received)
            }
            error("Fatal error in poll", true, true);
        }

        // Iterate in reverse to safely erase closed sockets
        for (size_t i = _pollfds.size(); i-- > 0;) {
            if (_pollfds[i].revents & POLLIN) {
                if (_pollfds[i].fd == _server_fd) {
                    handleNewConnection();
                } else {
                    // If `handleClientMessage()` returns false, remove client
                    if (!handleClientMessage(_pollfds[i])) {
                        unsigned int client_id = fetchClientIdFromPid(_pollfds[i].fd);
                        if (client_id > 0) {
                            removeClient(client_id);
                        } 
                        else {
                            // Just close the fd if we couldn't find a client (shouldn't happen)
                            close(_pollfds[i].fd);
                            _pollfds.erase(_pollfds.begin() + i);
                        }
                    }
                }
            }
        }
        // deleteClients(); 
    }
}

