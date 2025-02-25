/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/25 11:04:39 by juramos           #+#    #+#             */
/*   Updated: 2025/02/25 12:43:44 by cmunoz-g         ###   ########.fr       */
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

// Server::Server(Server &toCopy): _port(toCopy.getPort()), _password(toCopy.getPassword()) {}

// Server	&Server::operator=(Server &other) {
// 	if (this != &other)
// 	{
// 		// TODO
// 		return *this;
// 	}
// 	return *this;
// }

int	Server::getPort() const { return _port; }

const std::string	&Server::getPassword() const { return _password; }

Server::~Server() {
	close(_server_fd);
}

unsigned int Server::fetchClientIdFromPid(int fd) {
	for (int i = 1; _clients[i]; i++)
		if (_clients[i]->getSocket() == fd)
			return (_clients[i]->getId());
	return (-1);
}

void	Server::setUpServerSocket() {
	// Crear socket
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd < 0)
		throw std::runtime_error("Error creando socket");

	// Configurar socket como no bloqueante
	fcntl(_server_fd, F_SETFL, O_NONBLOCK);

	// Permitir reutilización del puerto
	int opt = 1;
	setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// Configurar dirección del servidor
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(_port);

	// Vincular socket
	if (bind(_server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
		throw std::runtime_error("Error en bind");

	// Escuchar conexiones
	if (listen(_server_fd, 10) < 0)
		throw std::runtime_error("Error en listen");
}

void	Server::handleNewConnection(std::vector<struct pollfd> &pollfds) {
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	
	int client_fd = accept(_server_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0) {
		if (errno != EWOULDBLOCK)
			std::cerr << "Error aceptando conexión" << std::endl;
		return;
	}

	// Configurar nuevo socket cliente como no bloqueante
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	// Añadir nuevo cliente a poll
	struct pollfd client_pollfd = {client_fd, POLLIN, 0};
	pollfds.push_back(client_pollfd);

	// Añadir nuevo cliente al map
	static int id = 1;
	_clients[id] = new Client(client_fd, id);
	// el mismo objeto, con la misma dir. de memoria tanto en server como en channel
	
	id++;
		
	std::cout << "Nueva conexión aceptada" << std::endl;
}

void Server::handleClientMessage(struct pollfd& pfd) {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = recv(pfd.fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_read <= 0) {
            if (bytes_read == 0 || errno != EWOULDBLOCK) {
                // Cliente desconectado o error
                close(pfd.fd);
                pfd.fd = -1; // Marcar para eliminar
            }
            return;
        }

        buffer[bytes_read] = '\0';
		unsigned int client_id = fetchClientIdFromPid(pfd.fd);
		if (client_id > 0) {
			Client *client = _clients[client_id];
			
			client->appendToBuffer(buffer);
			std::string buf = client->getBuffer();
			size_t pos;
			
			while ((pos = buf.find("\r\n")) != std::string::npos) {
				std::string singleCommand = buf.substr(0, pos);
				buf.erase(0, pos + 2);
				
				client->setBuffer(singleCommand);
				
				Message newMessage(client);
				newMessage.printMessageDebug();
				
				switch (newMessage.getCommandType()) {
					case IRC::CMD_CAP:
						handleCapCommand(newMessage);
						break;
					case IRC::CMD_NICK:
						handleNickCommand(newMessage);
						break;
					case IRC::CMD_USER:
						handleUserCommand(newMessage);
						break;
					case IRC::CMD_PASS:
						handlePassCommand(newMessage);
						break;
					case IRC::CMD_PRIVMSG:
						handlePrivmsgCommand(newMessage);
						break;
					case IRC::CMD_JOIN:
						handleJoinCommand(newMessage);
						break;
					case IRC::CMD_INVITE:
						handleInviteCommand(newMessage);
						break;
					case IRC::CMD_TOPIC:
						handleTopicCommand(newMessage);
						break;
					case IRC::CMD_MODE:
						handleModeCommand(newMessage);
						break;
					case IRC::CMD_PING:
						handlePingCommand(newMessage);
						break;
					case IRC::CMD_KICK:
						handleKickCommand(newMessage);
						break;
					case IRC::CMD_QUIT:
						handleQuitCommand(newMessage);
						break;
					default:
						break;
				}
			}
			client->setBuffer(buf);
		}
}

void Server::handleCapCommand(Message &message) {
	// :<server_name> CAP <client_id> LS :
	if (message.getParams()[0] == "LS") { // && message.getParams()[1] == "302" hace falta???
		std::string response = ":" + SERVER_NAME + " CAP * LS :multi-prefix\r\n";
		_clients[message.getSenderId()]->receiveMessage(response);
	}
	else if (message.getParams()[0] == "REQ") {
		std::string response = ":" + SERVER_NAME + " CAP * ACK :" + message.getParams()[1] + "\r\n"; // Ahora mismo mandamos lo que pide Irssi, mirar si hay que implementarlos requerimientos diferente o que
		_clients[message.getSenderId()]->receiveMessage(response);
	}
	else if (message.getParams()[0] == "END") {
		std::cout << "Capability negotiation ended for client " << _clients[message.getSenderId()] << std::endl; // esta devolviendo el id en hex?
		_clients[message.getSenderId()]->setCapNegotiationStatus(true);
	}
	else {
		// checkear
	} 
}

/* 
 433     ERR_NICKNAMEINUSE
                        "<nick> :Nickname is already in use"

                - Returned when a NICK message is processed that results
                  in an attempt to change to a currently existing
                  nickname.
*/

void Server::handleNickCommand(Message &message) { // llega un unico param que es el nick
	std::string nickname = message.getParams()[0];
	if (checkUniqueNick(nickname)) {
		_clients[message.getSenderId()]->setNickname(nickname);
		std::cout << "NICK command handled, nickname saved as " << _clients[message.getSenderId()]->getNickname() << std::endl;
	} 
	else {
		std::string response = ":" + SERVER_NAME + " 433 " + nickname + " :Nickname is already in use\r\n";
	}
}

void Server::handleModeCommand(Message &message) {
	Client *client = _clients[message.getSenderId()];
	std::string nickname = client->getNickname();
	std::string response;

	if (message.getParams().empty()) {
		response = ":" + SERVER_NAME + " 461 " + nickname + " MODE :Not enough parameters\r\n";
	}
	else {
		std::string target = message.getParams()[0];
		
		if (target[0] == '#') { // Channel modes
			if (_channels.find(target) == _channels.end()) {
				response = ":" + SERVER_NAME + " 403 " + nickname + " " + target + " :No such channel\r\n";
				client->receiveMessage(response);
				return;
			}
			Channel *channel = _channels[target];
			if (message.getParams().size() == 1) {
				response = ":" + SERVER_NAME + " 324 " + nickname + " " + target + " " + channel->getModes() + "\r\n";
			}
			else { // Setting channel modes
				std::string modeString = message.getParams()[1];
				if (channel->isOperator(client)) {
					channel->setModesFromString(modeString, message.getParams(), client);
					response = ":" + nickname + " MODE " + target + " " + modeString + "\r\n";
				}
				else {
					response = ":" + SERVER_NAME + " 482 " + nickname + " " + target + " :You're not channel operator\r\n";
				}
			}
			
		}
		
		else if (target == client->getNickname()) { // User modes
			if (message.getParams().size() == 1) {
				response = ":" + SERVER_NAME + " 221 " + nickname + " " + client->getModes() + "\r\n";
			}
			else { // Setting user modes
				std::string modeString = message.getParams()[1];
				client->setModesFromString(modeString);
				response = ":" + nickname + " MODE " + nickname + " " + modeString + "\r\n";
			}
		}
		else {
			response = ":" + SERVER_NAME + " 401 " + nickname + " " + target + " :No such nick/channel\r\n";
		}
	}

	client->receiveMessage(response);

	// if (message.getParams().size() == 1) {
	// 	std::string channelName = message.getParams()[0];
	// 	response = ":" + SERVER_NAME + " 324 " + nickname + " " + channelName + " +o" + "\r\n"; // hay que adaptar esto para enviar los modos concretos que tenga el canal
	// }
	// else {
	// 	response = ":" + nickname + " MODE " + nickname + " +i\r\n"; // +i es el modo invisible que el cliente pide al servidor, mirar si hay que implementarlo en el servidor
	// }
	// _clients[message.getSenderId()]->receiveMessage(response);
}

void Server::handlePingCommand(Message &message) {
	std::string token = message.getParams().size() == 0 ? SERVER_NAME : message.getParams()[0];
	if (!token.empty() && token[0] == ':') {
		token = token.substr(1);
	}

	std::string response = "PONG :" + token + "\r\n";
	_clients[message.getSenderId()]->receiveMessage(response);
}

void Server::handlePassCommand(Message &message) {
	std::string nickname = _clients[message.getSenderId()]->getNickname();

	if (_clients[message.getSenderId()]->isAuthenticated()) {
		std::string response = ":" + SERVER_NAME + " 462 " + nickname + " :You may not reregister\r\n";
		_clients[message.getSenderId()]->receiveMessage(response);
	}
	else if (message.getParams().empty()) {
		std::string response = ":" + SERVER_NAME + " 461 " + nickname + " PASS :Not enough parameters\r\n";
		_clients[message.getSenderId()]->receiveMessage(response);
	}
	else if (message.getParams()[0] != getPassword()) {
		std::string response = ":" + SERVER_NAME + " 464 " + nickname + " :Password incorrect\r\n";
		_clients[message.getSenderId()]->receiveMessage(response);
	}
	else {
		_clients[message.getSenderId()]->setAuthenticated(true);
		std::cout << "Pass command handled, client authenticated" << std::endl;
	}
}

void Server::handleUserCommand(Message &message) {
	Client *client = _clients[message.getSenderId()];
	client->setUsername(message.getParams()[0]);
	std::cout << "User command handled, username saved as " << client->getUsername() << std::endl;

	if (!client->getNickname().empty() && !client->getUsername().empty() && client->isAuthenticated()) {
		std::string response = ":" + SERVER_NAME + " 001 " + client->getNickname() + "\r\n";
		client->receiveMessage(response);
	}
}

void Server::handleJoinCommand(Message &message) { // revisar el join a varios canales simultaneamente
	if (message.getParams()[0].empty()) // para gestionar el autojoin de irssi
		return;
	std::string channelName = message.getParams()[0];
	Client *client = _clients[message.getSenderId()];
	
	if (_channels.find(channelName) == _channels.end()) { // if channel doesn't exist, we create it
		_channels[channelName] = new Channel(channelName, client);
		if (message.getParams().size() > 1) {
			_channels[channelName]->setPassword(message.getParams()[1]);
			_channels[channelName]->setMode(IRC::MODE_K, true);
		}
		std::string topic_response = ":" + SERVER_NAME + " 331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n";
		client->receiveMessage(topic_response);
		_channels[channelName]->sendNames(client);
		std::string mode_response = ":" + SERVER_NAME + " 324 " + client->getNickname() + " " + channelName + " +o" + "\r\n";
		client->receiveMessage(mode_response);

		_channels[channelName]->addClient(client);
		_channels[channelName]->addOperator(client);
	}
	
	else {
		Channel *channel = _channels[channelName];
		if (channel->hasClient(client)) {
			std::string response = ":" + SERVER_NAME + " 443 " + client->getNickname() + " " + channelName + " :is already on channel\r\n";
			client->receiveMessage(response);
			return;
		}
		if (channel->hasMode(IRC::MODE_I)) { // && !channel->isInvited(client) GESTIONAR INVITACIONES
			std::string response = ":" + SERVER_NAME + " 473 " + client->getNickname() + " " + channelName + " :Cannot join channel (+i)\r\n";
			client->receiveMessage(response);
			return;
		}
		if (message.getParams().size() > 1 && channel->hasMode(IRC::MODE_K) && channel->getPassword() != message.getParams()[1]) { // y si no tiene contrasena el canal y el cliente pone una?
			std::string response = ":" + SERVER_NAME + " 475 " + client->getNickname() + " " + channelName + " :Cannot join channel (+k)\r\n";
			client->receiveMessage(response);
			return;
		}
		if (channel->addClient(client) == false) {
			std::string response = ":" + SERVER_NAME + " 471 " + client->getNickname() + " " + channelName + " :Cannot join channel (+l)\r\n";
			client->receiveMessage(response);
			return;
		}
		std::string response = ":" + client->getNickname() + " JOIN " + channelName + "\r\n";
		client->receiveMessage(response);
		std::string topic_response;
		if (channel->hasMode(IRC::MODE_T))
			topic_response = ":" + SERVER_NAME + " 332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n";
		else
			topic_response = ":" + SERVER_NAME + " 331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n";
		client->receiveMessage(topic_response);
		channel->sendNames(client);
		channel->addClient(client);
		channel->broadcastMessage(":" + client->getNickname() + " JOIN " + channelName + "\r\n", client);
	}
}

void Server::handlePrivmsgCommand(Message &message) {
	Client *sender = _clients[message.getSenderId()];

	if (message.getParams().empty()) {
		std::string response = ":" + SERVER_NAME + " 411 " + sender->getNickname() + " :No recipient given (PRIVMSG)\r\n";
		sender->receiveMessage(response);
		return;
	}

	if (message.getParams().size() < 2 || message.getParams()[1].empty()) {
		std::string response = ":" + SERVER_NAME + " 412 " + sender->getNickname() + " :No text to send\r\n";
		sender->receiveMessage(response);
		return;
	}

	std::string target = message.getParams()[0];
    std::string msg = message.getParams()[1];

	if (target[0] == '#') { // Target is a channel
		if (_channels.find(target) == _channels.end()) {
			std::string response = ":" + SERVER_NAME + " 403 " + sender->getNickname() + " " + target + " :No such channel\r\n";
			sender->receiveMessage(response);
			return;
		}
		Channel *channel = _channels[target];
		if (!channel->hasClient(sender)) {
			std::string response = ":" + SERVER_NAME + " 404 " + sender->getNickname() + " " + target + " :Cannot send to channel\r\n";
			sender->receiveMessage(response);
			return;
		}
		channel->broadcastMessage(":" + sender->getNickname() + " PRIVMSG " + target + " :" + msg + "\r\n", sender);
	}
	else { // Target is a user
		std::map<unsigned int, Client*>::iterator it = _clients.begin();
		while (it != _clients.end()) {
			if (it->second->getNickname() == target) {
				it->second->receiveMessage(":" + sender->getNickname() + " PRIVMSG " + target + " :" + msg + "\r\n");
				return;
			}
			++it;
		}
		std::string response = ":" + SERVER_NAME + " 401 " + sender->getNickname() + " " + target + " :No such nick\r\n";
		sender->receiveMessage(response);

	}
}

void Server::handleInviteCommand(Message &message) {
	Client *inviter = _clients[message.getSenderId()];
	
	if (message.getParams().size() < 2) {
		std::string response = ":" + SERVER_NAME + " 461 " + inviter->getNickname() + " INVITE :Not enough parameters\r\n"; // Revisar si es el comando correcto
		inviter->receiveMessage(response);
		return;
	}

	std::string nickname = message.getParams()[0];
	std::string channelName = message.getParams()[1];

	if (_channels.find(channelName) == _channels.end()) {
		std::string response = ":" + SERVER_NAME + " 403 " + inviter->getNickname() + " " + channelName + " :No such channel\r\n";
	}

	Channel *channel = _channels[channelName];
	if (!channel->hasClient(inviter)) {
		std::string response = ":" + SERVER_NAME + " 442 " + inviter->getNickname() + " " + channelName + " :You're not on that channel\r\n";
		return;
	}
	
	Client *invitee = NULL;
	std::map<unsigned int, Client*>::iterator it = _clients.begin();
	while (it != _clients.end()) {
		if (it->second->getNickname() == nickname) {
			invitee = it->second;
			break;
		}
		++it;
	}
	
	if (!invitee) {
		std::string response = ":" + SERVER_NAME + " 401 " + inviter->getNickname() + " " + nickname + " :No such nick\r\n";
		inviter->receiveMessage(response);
		return;
	}

	if (channel->hasMode(IRC::MODE_I) && !channel->isOperator(inviter)) {
		std::string response = ":" + SERVER_NAME + " 482 " + inviter->getNickname() + " " + channelName + " :You're not channel operator\r\n";
		inviter->receiveMessage(response);
		return;
	}

	std::string inviteResponse = ":" + inviter->getNickname() + " INVITE " + nickname + " :" + channelName + "\r\n";
	invitee->receiveMessage(inviteResponse);

	std::string inviterResponse = ":" + SERVER_NAME + " 341 " + inviter->getNickname() + " " + nickname + " " + channelName + "\r\n";
	inviter->receiveMessage(inviterResponse);
	
	channel->addInvitedClient(invitee);
}

void Server::handleTopicCommand(Message &message) {
	Client *client = _clients[message.getSenderId()];

	if (message.getParams().empty()) {
		std::string response = ":" + SERVER_NAME + " 461 TOPIC :Not enough parameters\r\n";
		client->receiveMessage(response);
		return;
	}
	
	std::string channelName = message.getParams()[0];

	if (_channels.find(channelName) == _channels.end()) {
		std::string response = ":" + SERVER_NAME + " 403 " + channelName + " :No such channel\r\n";
		client->receiveMessage(response);
		return;
	}
	
	Channel *channel = _channels[channelName];
	if (message.getParams().size() == 1) { // VIEW Topic: Checks current topic
		if (channel->getTopic().empty()) {
			std::string response = ":" + SERVER_NAME + " 331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n";
			client->receiveMessage(response);
		}
		else {
			std::string response = ":" + SERVER_NAME + " 332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n";
			client->receiveMessage(response);
		}
		return;
	}

	std::string newTopic = message.getParams()[1];
	
	if (!channel->setTopic(client, newTopic)) {
		std::string response = ":" + SERVER_NAME + " 482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n";
		return;
	}

	std::string topic_response = ":" + client->getNickname() + "!" + client->getUsername() + "@" + SERVER_NAME + " TOPIC " + channelName + " :" + newTopic + "\r\n";
	channel->broadcastMessage(topic_response, NULL);
}

void Server::handleKickCommand(Message &message) {
	Client *kicker = _clients[message.getSenderId()];

	if (message.getParams().size() < 2) {
		std::string response = ":" + SERVER_NAME + " 461 KICK :Not enough parameters\r\n";
		kicker->receiveMessage(response);
		return;
	}

	std::string channelName = message.getParams()[0];
	std::string targetName = message.getParams()[1];
	std::string reason = (message.getParams().size() > 2) ? message.getParams()[2] : "No reason";

	if (_channels.find(channelName) == _channels.end()) {
		std::string response = ":" + SERVER_NAME + " 403 " + channelName + " :No such channel\r\n";
		kicker->receiveMessage(response);
		return;
	}

	Channel *channel = _channels[channelName];
	if (!channel->hasClient(kicker)) {
		std::string response = ":" + SERVER_NAME + " 442 " + kicker->getNickname() + " " + channelName + " :You're not on that channel\r\n";
		kicker->receiveMessage(response);
		return;
	}
	if (!channel->isOperator(kicker)) {
		std::string response = ":" + SERVER_NAME + " 482 " + channelName + " :You're not a channel operator\r\n";
        kicker->receiveMessage(response);
        return;
	}

	Client *target = NULL;
	std::map<unsigned int, Client*>::iterator it = _clients.begin();
	while (it != _clients.end()) {
		if (targetName == it->second->getNickname()) {
			target = it->second;
			break;
		}
		++it;
	}

	if (!target) {
		std::string response = ":" + SERVER_NAME + " 401 " + targetName + " :No such nick\r\n";
        kicker->receiveMessage(response);
        return;
	}

	std::string kickResponse = ":" + kicker->getNickname() + "!" + kicker->getUsername() + "@" + SERVER_NAME + " KICK " + channelName + " " + targetName + " :" + reason + "\r\n";
	channel->broadcastMessage(kickResponse, NULL);
	target->leaveChannel(channel);
	channel->removeClient(target); 
	
	target->receiveMessage(kickResponse);
}

void Server::handleQuitCommand(Message &message) {
	Client *client = _clients[message.getSenderId()];
	std::string reason = (message.getParams().empty()) ? "Client quit" : message.getParams()[0];
	std::string quitResponse = ":" + client->getNickname() + "!" + client->getUsername() + "@" + SERVER_NAME + " QUIT :" + reason + "\r\n";
	
	std::map<const std::string, Channel*>::iterator it = _channels.begin();
	while (it != _channels.end()) {
		Channel *channel = it->second;
		++it;
		if (channel->hasClient(client)) {
			channel->broadcastMessage(quitResponse, client);
			channel->removeClient(client);
		}
	}

	client->receiveMessage(quitResponse);
	client->cleanup();
	delete client;
	_clients.erase(message.getSenderId());
}

/**/

bool Server::checkUniqueNick(std::string nick) { // esto hay que testearlo
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

void Server::start() {
	std::vector<struct pollfd> pollfds;
	Message::initCommandMap();
	
	struct pollfd server_pollfd = {_server_fd, POLLIN, 0};
	pollfds.push_back(server_pollfd);

	while (true) {
		// Poll espera eventos en los sockets
		int ret = poll(pollfds.data(), pollfds.size(), -1);
		if (ret < 0)
			throw std::runtime_error("Error en poll");

		// Revisar todos los file descriptors
		for (size_t i = 0; i < pollfds.size(); i++) {
			if (pollfds[i].revents & POLLIN) {
				if (pollfds[i].fd == _server_fd) {
					// Nueva conexión en el socket servidor // Nota; Seguro???? 
					handleNewConnection(pollfds);
				} else {
					// Mensaje de un cliente existente
					handleClientMessage(pollfds[i]);
				}
			}
		}
		deleteClients();
	}
}
