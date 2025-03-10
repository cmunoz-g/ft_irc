/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/25 11:04:39 by juramos           #+#    #+#             */
/*   Updated: 2025/03/10 15:18:51 by cmunoz-g         ###   ########.fr       */
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
    _pollfds.clear(); // Ensures no leftovers

    struct pollfd server_pollfd = {_server_fd, POLLIN, 0};
    _pollfds.push_back(server_pollfd);

    while (g_running) {
        int ret = poll(_pollfds.data(), _pollfds.size(), -1);
        std::cout << "[DEBUG] poll returned " << ret 
                  << ", errno=" << errno
                  << ", g_running=" << g_running
                  << std::endl;
    
        if (ret < 0) {
            if (errno == EINTR) {
                // print debug
                std::cout << "[DEBUG] EINTR => g_running=" << g_running << std::endl;
                if (!g_running) {
                    std::cout << "[DEBUG] Breaking out of main loop" << std::endl;
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

    std::cout << "[LOG] Server cleanup complete" << std::endl;
}
