/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/26 10:07:13 by juramos           #+#    #+#             */
/*   Updated: 2025/03/19 11:29:23 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IRC.hpp"

// *** Constructors & Destructor ***
Client::Client(int socket, unsigned int id): _socket(socket), _nickname(""),
	_username(""), _buffer(""), _authenticated(false), _registered(false), _passwordAttempts(0), _id(id) {}

Client::~Client() {}

// *** Getters, Setters ***
int Client::getSocket() const {
    return _socket;
}

std::string const   Client::getNickname() const {
    return _nickname;
}

std::string const   Client::getUsername() const {
    return _username;
}

unsigned int    Client::getId() const {
    return _id;
}

std::map<const std::string, Channel*> Client::getChannels() const {
    return _channels;
}

void        Client::setNickname(const std::string& nickname) {
	_nickname = nickname;
}

void        Client::setUsername(const std::string& username) {
	_username = username;
}

void        Client::setAuthenticated(bool status) {
	_authenticated = status;
}

void        Client::setRegistered(bool status) {
    _registered = status;
}

void		Client::setCapNegotiationStatus(bool status) {
	_capNegotiation = status;
}

void		Client::setBuffer(const std::string &buffer) {
	_buffer = buffer;
}

// *** Member Functions ***
// Checks
bool	Client::isAuthenticated() const {
    return _authenticated;
}

bool	Client::isRegistered() const {
    return _registered;
}

bool    Client::isCapNegotiationDone() const {
    return _capNegotiation;
}

// Buffer Operations
void	Client::appendToBuffer(const std::string& data) {
	_buffer.append(data);
}

std::string const	Client::getBuffer() const {
	return _buffer;
}

void	Client::clearBuffer(void) {
	_buffer.clear();
}

// Channel Operations
void	Client::joinChannel(Channel *channel) {
	if (isInChannel(channel)) {
		error("Client is already on the channel", false, false);
		return ;
	}
	_channels.insert(std::make_pair(channel->getName(), channel));
}

void Client::leaveChannel(const Channel *channel) {
    if (!isInChannel(channel)) {
		error("Client is not on the channel", false, false);
        return;
    }

    std::map<const std::string, Channel*>::iterator it = _channels.find(channel->getName());
    
    if (it == _channels.end()) {
		error("Channel not found in client's list", false, false);
        return;
    }

    if (isOperator(channel)) {
        removeOperatorStatus(channel);
    }

    _channels.erase(it);
}

void Client::leaveAllChannels() {
    while (!_channels.empty()) {
        Channel *channel = _channels.begin()->second;
        leaveChannel(channel);   
        channel->removeClient(this);
    }
}

bool	Client::isInChannel(const Channel *channel) const {
	for (std::map<const std::string, Channel*>::const_iterator it = _channels.begin(); it != _channels.end(); ++it) {
		if (it->second->getName() == channel->getName())
			return (true);
	}
	return (false);
}

// Operator Operations
void Client::setOperatorStatus(Channel *channel) {
    if (!isInChannel(channel)) {
        error("Client is not on the channel", false, false);
        return;
    }

    if (isOperator(channel)) {
        error("Client is already an operator", false, false);
        return;
    }

    std::map<std::string, Channel*>::iterator it = _channels.find(channel->getName());
    if (it == _channels.end()) {
        error("Channel not found in client's list", false, false);
        return; 
    }

    channel->addOperator(this);
    _op_channels[channel->getName()] = channel;
}

void	Client::removeOperatorStatus(const Channel *channel) {
	if (!isInChannel(channel)) {
		error("Client is not on the channel", false, false);
		return ;
	}
	else if (!isOperator(channel)) {
		error("Client is not an operator", false, false);
		return ;
	}

	std::map<const std::string, Channel*>::iterator it = _op_channels.begin();
	while (it->second->getName() != channel->getName())
		++it;
	
	_op_channels.erase(it);
}

bool	Client::isOperator(const Channel *channel) const {
	for (std::map<const std::string, Channel*>::const_iterator it = _op_channels.begin(); it != _op_channels.end(); ++it) {
		if (it->second->getName() == channel->getName())
			return (true);
	}
	return (false);
}

// Communication
void Client::receiveMessage(const std::string &message) {
    ssize_t bytesSent = ::send(_socket, message.c_str(), message.size(), 0);

    if (bytesSent < 0) {
        std::stringstream ss;
        ss << "Error sending message to client " << _id;
        error(ss.str(), false, true);
    } 
    else if (static_cast<size_t>(bytesSent) < message.size()) {
        std::stringstream ss;
        ss << "Partial send: Only " << bytesSent << " bytes sent to client " << _id;
        error(ss.str(), false, true);
    } 
    else {
		std::stringstream ss;
		ss << MAGENTA << "[MSG] " << RESET << "[ID:" << _id << "] Message sent to client: " << message;
		std::cout << ss.str();
    }
}

// Mode Operations
void Client::setMode(IRC::ClientMode mode, bool enabled) {
    std::vector<unsigned int>::iterator it = std::find(_modes.begin(), _modes.end(), mode);
    if (enabled && it == _modes.end()) {
        _modes.push_back(mode);
    } else if (!enabled && it != _modes.end()) {
        _modes.erase(it);
    }
}

void Client::setModesFromString(const std::string& modeString) {
    bool adding = true;

    for (size_t i = 0; i < modeString.length(); ++i) {
        char c = modeString[i];

        if (c == '+') {
            adding = true;  
        } else if (c == '-') {
            adding = false;
        } else {
            IRC::ClientMode modeEnum = IRC::C_MODE_NONE;

            switch (c) {
                case 'i':
                    modeEnum = IRC::C_MODE_I;
                    break;
                case 'o':
                    modeEnum = IRC::C_MODE_O;
                    break;
                default:
                    break;
            }

            // Handle adding or removing modes
            if (adding) {
                if (!hasMode(modeEnum)) {
                    _modes.push_back(modeEnum); // Add mode if not present
                }
            } else {
				std::vector<unsigned int>::iterator it = std::find(_modes.begin(), _modes.end(), modeEnum);
                if (it != _modes.end()) {
                    _modes.erase(it); // Remove mode if present
                }
            }
        }
    }
}

bool Client::hasMode(IRC::ClientMode mode) const {
    return std::find(_modes.begin(), _modes.end(), mode) != _modes.end();
}

std::string Client::getModes() const {
    std::string modes = "+";

    for (size_t i = 0; i < _modes.size(); ++i) {
        switch (_modes[i]) {
            case IRC::C_MODE_I:
                modes += "i"; 
                break;
            case IRC::C_MODE_O:
                modes += "o";
                break;
            default:
                break;
        }
    }

    // If no modes are active, return an empty string
    if (modes.length() == 1) // Only contains '+'
        return "";

    return modes;
}

// Password Operations
void Client::addPasswordAttempt() {
    _passwordAttempts++;
}

unsigned int Client::getPasswordAttempts() const {
    return _passwordAttempts;
}

// Cleanup
void	Client::cleanup() {
	for (std::map<const std::string, Channel*>::const_iterator it = _channels.begin(); it != _channels.end(); ++it) {
		it->second->removeClient(this);
	}
	_channels.clear();

	for (std::map<const std::string, Channel*>::const_iterator it = _op_channels.begin(); it != _op_channels.end(); ++it) {
		it->second->removeOperator(this);
	}
	_op_channels.clear();
    
    // Prevents reusing closed socket
	if (_socket != -1) { 
        close(_socket);
        _socket = -1;  
    }
}