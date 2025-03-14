/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/25 12:07:15 by cmunoz-g          #+#    #+#             */
/*   Updated: 2025/03/12 11:50:07 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IRC.hpp"

std::map<std::string, IRC::CommandType> Message::_commandMap;

Message::Message(const Client *client) {
	_senderSocket = client->getSocket();
    _senderId = client->getId();
	parse(client->getBuffer());
    setCommandType();
	setReceiver();
}

Message::~Message(void) {}

const std::string& Message::getPrefix() const { return _command._prefix; }
const std::string& Message::getCommand() const { return _command._command; }
const std::vector<std::string>& Message::getParams() const { return _command._params; }
IRC::CommandType Message::getCommandType() const { return _commandType; }
int Message::getSenderSocket() const { return _senderSocket; }
unsigned int    Message::getSenderId() const { return _senderId; }
std::string Message::getReceiverChannel() const { return _receiverChannel; }

void	Message::parse(const std::string& buffer) {
    std::string msg = buffer;
    
    // Eliminar \r\n del final si existe
    size_t end = msg.find("\r\n");
    if (end != std::string::npos)
        msg = msg.substr(0, end);

    // Extraer prefix si existe
    if (!msg.empty() && msg[0] == ':') {
        size_t space = msg.find(' ');
        if (space == std::string::npos) {
            _command._prefix = "";
            return;
        }
        _command._prefix = msg.substr(1, space - 1);
        msg = msg.substr(space + 1);
    }

    // Extraer comando
    size_t space = msg.find(' ');
    if (space == std::string::npos) {
        _command._command = msg;
        return; // No hay parámetros
    }
    
    _command._command = msg.substr(0, space);
    msg = msg.substr(space + 1);

    // Extraer parámetros
    _command._params.clear();
    size_t start = 0;
    
    // Procesar todos los parámetros
    while (start < msg.length()) {
        // Si encontramos un ':', tomar todo lo que queda como último parámetro
        if (msg[start] == ':') {
            _command._params.push_back(msg.substr(start + 1));
            break;
        }

        // Encontrar el siguiente espacio
        space = msg.find(' ', start);
        
        // Si no hay más espacios, este es el último parámetro
        if (space == std::string::npos) {
            _command._params.push_back(msg.substr(start));
            break;
        }

        // Añadir el parámetro encontrado
        _command._params.push_back(msg.substr(start, space - start));
        start = space + 1;

        // Saltar espacios adicionales
        while (start < msg.length() && msg[start] == ' ')
            start++;
    }
}

void	Message::setReceiver(void)
{
	for (std::vector<std::string>::iterator it = _command._params.begin(); it < _command._params.end(); it++)
	{
		if (!(*it).empty() && (*it).at(0) == '#')
			_receiverChannel = (*it);
	}
}

void Message::initCommandMap() {
    if (_commandMap.empty()) {
        _commandMap.insert(std::make_pair("CAP", IRC::CMD_CAP));
        _commandMap.insert(std::make_pair("PASS", IRC::CMD_PASS));
        _commandMap.insert(std::make_pair("NICK", IRC::CMD_NICK));
        _commandMap.insert(std::make_pair("USER", IRC::CMD_USER));
        _commandMap.insert(std::make_pair("JOIN", IRC::CMD_JOIN));
        _commandMap.insert(std::make_pair("PRIVMSG", IRC::CMD_PRIVMSG));
        _commandMap.insert(std::make_pair("KICK", IRC::CMD_KICK));
        _commandMap.insert(std::make_pair("INVITE", IRC::CMD_INVITE));
        _commandMap.insert(std::make_pair("TOPIC", IRC::CMD_TOPIC));
        _commandMap.insert(std::make_pair("MODE", IRC::CMD_MODE));
        _commandMap.insert(std::make_pair("QUIT", IRC::CMD_QUIT));
        _commandMap.insert(std::make_pair("PING", IRC::CMD_PING));
        _commandMap.insert(std::make_pair("PONG", IRC::CMD_PONG));
        _commandMap.insert(std::make_pair("WHO", IRC::CMD_WHO));
    }
}

void    Message::setCommandType(void) {
    std::map<std::string, IRC::CommandType>::const_iterator it = _commandMap.find(_command._command);
    if (it != _commandMap.end()) {
        _commandType = it->second;
    }
    else {
        _commandType = IRC::CMD_UNKNOWN;
    }
}

void    Message::printMessageDebug(int client_id) const {
    std::cout << GREEN << "[CMD] " << RESET "[ID:" << client_id << "] Handling " << getCommand() << " command" << std::endl;
    std::cout << "type :" << _commandType << std::endl;
	std::cout << "command :" << this->getCommand() << std::endl; 
	std::cout << "prefix :" << this->getPrefix() << std::endl; 
    const std::vector<std::string> params = this->getParams();
	int i = 1;
	for (std::vector<std::string>::const_iterator it = params.begin(); it != params.end(); ++it) {
        std::cout << "param" << i << ":" << *it << std::endl; 
        i++;
	}
}