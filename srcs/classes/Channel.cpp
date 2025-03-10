/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/28 10:53:06 by juramos           #+#    #+#             */
/*   Updated: 2025/03/10 12:34:19 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IRC.hpp"

Channel::Channel(const std::string& name, Client* creator) : _name(name), _topic(""), _password(""), _userLimit(0) {
    _clients.insert(std::make_pair(creator->getId(), creator));
    _operators.insert(std::make_pair(creator->getId(), creator));
}

Channel::~Channel() {
}

const std::string& Channel::getName() const {
    return _name;
}

const std::string& Channel::getTopic() const {
    return _topic;
}

const std::string& Channel::getPassword() const {
    return _password;
}

size_t Channel::getUserCount() const {
    return _clients.size();
}

size_t Channel::getUserLimit() const {
    return _userLimit;
}

std::string Channel::getModes() const {
    std::string modes = "+";

    if (hasMode(IRC::MODE_I))
        modes += "i";
    if (hasMode(IRC::MODE_T))
        modes += "t";
    if (!_password.empty())
        modes += "k";
    if (_userLimit > 0)
        modes += "l";

    return modes == "+" ? "" : modes;
}

std::map<unsigned int, Client*> Channel::getClients() const {
    return _clients;
}

bool Channel::isOperator(Client* client) const {
    std::map<unsigned int, Client*>::const_iterator it = _operators.find(client->getId());
    return it != _operators.end();
}

void Channel::setMode(IRC::ChannelMode mode, bool enabled) {
    std::vector<unsigned int>::iterator it = std::find(_modes.begin(), _modes.end(), mode);
    if (enabled && it == _modes.end()) {
        _modes.push_back(mode);
    } else if (!enabled && it != _modes.end()) {
        _modes.erase(it);
    }
}

bool Channel::setModesFromString(const std::string& modeString, const std::vector<std::string>& params) {
    bool adding = true;
    size_t paramIndex = 2;

    for (size_t i = 0; i < modeString.length(); ++i) {
        char c = modeString[i];

        if (c == '+') {
            adding = true;
        } else if (c == '-') {
            adding = false;
        } else {
            IRC::ChannelMode modeEnum = IRC::MODE_NONE; // Default to no mode

            // Mapping the mode character to the ChannelMode enum
            switch (c) {
                case 'i':
                    modeEnum = IRC::MODE_I;
                    break;
                case 't':
                    modeEnum = IRC::MODE_T;
                    break;
                case 'k':
                    modeEnum = IRC::MODE_K;
                    break;
                case 'l':
                    modeEnum = IRC::MODE_L;
                    break;
                case 'o':
                    modeEnum = IRC::MODE_O;
                    break;
                default:
                    break;
            }

            // Handle adding or removing the mode
            if (adding) {
                if (!hasMode(modeEnum)) {
                    _modes.push_back(modeEnum); // Add mode if not already set
                }

                // Handle modes that require parameters
                if (modeEnum == IRC::MODE_K && paramIndex < params.size()) {
                    _password = params[paramIndex++];
                } 
                else if (modeEnum == IRC::MODE_L && paramIndex < params.size()) {
                    _userLimit = stringToInt(params[paramIndex++]);
                } 
                else if (modeEnum == IRC::MODE_O && paramIndex < params.size()) {
                    Client* targetClient = NULL;
                    for (std::map<unsigned int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
                        if (it->second->getNickname() == params[paramIndex]) {
                            targetClient = it->second;
                            break;
                        }
                    }

                    if (targetClient) {
                        addOperator(targetClient);
                    }
                    else
                        return false;
                }
            } else { // Removing modes
                std::vector<unsigned int>::iterator it = std::find(_modes.begin(), _modes.end(), modeEnum);
                if (it != _modes.end()) {
                    _modes.erase(it); // Remove mode if present
                }

                // Handle parameter clearing for specific modes
                if (modeEnum == IRC::MODE_K) {
                    _password.clear();
                } 
                else if (modeEnum == IRC::MODE_L) {
                    _userLimit = 0;
                } 
                else if (modeEnum == IRC::MODE_O && paramIndex < params.size()) {
                    Client* targetClient = NULL;
                    for (std::map<unsigned int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
                        if (it->second->getNickname() == params[paramIndex]) {
                            targetClient = it->second;
                            break;
                        }
                    }

                    if (targetClient) {
                        removeOperator(targetClient);
                    }
                    else
                        return false;
                }
            }
        }
    }
    return true;
}


bool Channel::hasMode(IRC::ChannelMode mode) const {
    return std::find(_modes.begin(), _modes.end(), mode) != _modes.end();
}

void Channel::setPassword(const std::string& pass) {
    _password = pass;
}

void Channel::setUserLimit(size_t limit) {
    _userLimit = limit;
}

bool Channel::checkPassword(const std::string& pass) const {
    return _password == pass;
}

bool Channel::setTopic(Client* client, const std::string& newTopic) {
    if (!isOperator(client) && hasMode(IRC::MODE_T)) {
        return false;
    }
    _topic = newTopic;
    return true;
}

bool Channel::addClient(Client* client) {
    if (getUserLimit() > 0 && (getUserCount() >= getUserLimit()))
        return false;
    
    _clients.insert(std::make_pair(client->getId(), client));
    return true;
}

void Channel::removeClient(Client* client) {
    std::map<unsigned int, Client*>::iterator it = _clients.find(client->getId());
    if (it != _clients.end()) {
        _clients.erase(it);
    }
    removeOperator(client);
}

bool Channel::hasClient(Client* client) const {
    return _clients.find(client->getId()) != _clients.end();
}

void Channel::addInvitedClient(Client* client) {
    _invitedClients.insert(std::make_pair(client->getId(), client));
}

bool Channel::isInvitedClient(Client* client) const {
    return _invitedClients.find(client->getId()) != _invitedClients.end();
}

void Channel::removeInvitedClient(Client *client) {
    std::map<unsigned int, Client*>::iterator it = _invitedClients.find(client->getId());
    if (it != _invitedClients.end()) {
        _invitedClients.erase(it);
    }
}

void Channel::addOperator(Client* client) {
    _operators.insert(std::make_pair(client->getId(), client));
}

void Channel::removeOperator(Client* client) {
    std::map<unsigned int, Client*>::iterator it = _operators.find(client->getId());
    if (it != _operators.end()) {
        _operators.erase(it);
    }
}

void Channel::inviteClient(Client* operator_client, Client* target) {
    if (isOperator(operator_client))
        addInvitedClient(target);
}

void Channel::broadcastMessage(const std::string& message, Client* exclude) {
    std::map<unsigned int, Client*>::iterator it;
    for (it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second != exclude) {
            it->second->receiveMessage(message);
        }
    }
}

void Channel::sendNames(Client* client) const {
    std::string namesList;
    std::map<unsigned int, Client*>::const_iterator it;
    
    for (it = _clients.begin(); it != _clients.end(); ++it) {
        if (isOperator(it->second))
            namesList += "@";
        namesList += it->second->getNickname() + " ";
    }

    std::string serverReply;
    serverReply = ":" + SERVER_NAME + " 353 " + client->getNickname() + " = " + _name + " :" + namesList + "\r\n";
    client->receiveMessage(serverReply);

    std::string endReply = ":" + SERVER_NAME + " 366 " + 
                          client->getNickname() + " " + 
                          _name + " :End of /NAMES list" + "\r\n";
    client->receiveMessage(endReply);
}

bool Channel::canModifyTopic(Client* client) const {
    return isOperator(client) || !hasMode(IRC::MODE_T);
}

bool Channel::isInviteOnly() const {
    return hasMode(IRC::MODE_I);
}

void Channel::notifyModeChange(Client* changer, char mode, bool enabled, const std::string& param) {
    std::ostringstream message;
    
    message << ":" << changer->getNickname() << "!" << changer->getUsername()
            << "@" << SERVER_NAME << " MODE " << _name << " "
            << (enabled ? "+" : "-") << mode;

    if (!param.empty()) {
        message << " " << param;
    }

    broadcastMessage(message.str());
}

