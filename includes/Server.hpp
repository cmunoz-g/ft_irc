/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/25 10:28:50 by juramos           #+#    #+#             */
/*   Updated: 2025/03/06 10:19:13 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "IRC.hpp"

class Message;

class Server {
private:
    int _server_fd;
    int _port;
    std::string _password;
    std::vector<struct pollfd> _pollfds;
    std::map<unsigned int, Client*>		_clients; // _client_fd no es único, puesto que cuando se desconecta se setea a -1. Se crea una variable _id dentro de Client, inicializada solo desde Server y que asegura que sea única
	std::map<const std::string, Channel*>	_channels;
    
    void setUpServerSocket();
	Server(Server &toCopy);
	Server	&operator=(Server &other);
public:
	Server(void);
    Server(int port, const std::string& password);
    ~Server();

	int	getPort() const;
	const std::string	&getPassword() const;
    void handleNewConnection();
    bool handleClientMessage(struct pollfd& pfd);
    void start();
    void deleteClients();
    void removeClient(unsigned int client_id);
    unsigned int fetchClientIdFromPid(int fd);
    void tryRegister(Client* client);

    // checks
    bool checkUniqueNick(std::string nick);

    // commands
    void handleCapCommand(Message &message);
    void handleNickCommand(Message &message);
    void handleModeCommand(Message &message);
    void handlePingCommand(Message &message);
    bool handlePassCommand(Message &message);
    void handleUserCommand(Message &message);
    void handleJoinCommand(Message &message);
    void handlePrivmsgCommand(Message &message);
    void handleInviteCommand(Message &message);
    void handleTopicCommand(Message &message);
    void handleKickCommand(Message &message);
    void handleQuitCommand(Message &message);
    
};

#endif