/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/25 10:28:50 by juramos           #+#    #+#             */
/*   Updated: 2025/03/19 12:15:47 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "IRC.hpp"

class Message;

class Server {
private:
	// *** Attributes ***
    int 									_server_fd;
    int 									_port;
    std::string								_password;
    std::vector<struct pollfd>				_pollfds;
    std::map<unsigned int, Client*>			_clients;
	std::map<const std::string, Channel*>	_channels;
    
	// *** Methods ***
	// ServerConnection.cpp
    void 				setUpServerSocket();
    void 				handleNewConnection();
    bool				handleClientMessage(struct pollfd& pfd);
    void 				removeClient(unsigned int client_id);

    // ServerCommands.cpp
    void 				handleCapCommand(Message &message);
    void				handleNickCommand(Message &message);
    void				handleModeCommand(Message &message);
    void				handlePingCommand(Message &message);
    bool				handlePassCommand(Message &message);
    void				handleUserCommand(Message &message);
    void				handleJoinCommand(Message &message);
    void				handlePrivmsgCommand(Message &message);
    void				handleInviteCommand(Message &message);
    void				handleTopicCommand(Message &message);
    void				handleKickCommand(Message &message);
    void				handleQuitCommand(Message &message);
	
	// ServerUtils.cpp
    void 				broadcastNickChange(Client* client, const std::string &oldNick, const std::string &newNick);
    bool 				checkUniqueNick(std::string nick);
    void 				tryRegister(Client* client);
    unsigned int 		fetchClientIdFromPid(int fd);
	
public:
	// Server.cpp
	// - Constructor & Destructor
    Server(int port, const std::string& password);
    ~Server();
    
	// - Getters
	int					getPort() const;
	const std::string	&getPassword() const;

	// - Start, Cleanup
    void 				start();
    void 				cleanup();
    
};

#endif