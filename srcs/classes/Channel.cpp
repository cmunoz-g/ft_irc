/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/28 10:53:06 by juramos           #+#    #+#             */
/*   Updated: 2025/02/24 11:07:58 by cmunoz-g         ###   ########.fr       */
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
    if (!isOperator(client) && hasMode(IRC::MODE_T)) { // IRC::TOPIC_RESTRICTED
        return false;
    }
    _topic = newTopic;
    return true;
}

bool Channel::addClient(Client* client) {
    if (getUserCount() >= getUserLimit())
        return false;
    _clients.insert(std::make_pair(client->getId(), client));
    return true;
}

bool Channel::removeClient(Client* client) {
    std::map<unsigned int, Client*>::iterator it = _clients.find(client->getId());
    if (it != _clients.end()) {
        _clients.erase(it);
        return true;
    }
    return false;
}

bool Channel::hasClient(Client* client) const {
    return _clients.find(client->getId()) != _clients.end();
}

void Channel::addInvitedClient(Client* client) { // Cambiar a bool ? No se si lo necesito
    _invitedClients.insert(std::make_pair(client->getId(), client));
}

bool Channel::isInvitedClient(Client* client) const {
    return _invitedClients.find(client->getId()) != _invitedClients.end();
}

bool Channel::removeInvitedClient(Client *client) {
    std::map<unsigned int, Client*>::iterator it = _invitedClients.find(client->getId());
    if (it != _invitedClients.end()) {
        _invitedClients.erase(it);
        return true;
    }
    return false;
}

bool Channel::addOperator(Client* client) {
    if (getUserCount() >= getUserLimit()) {
        return false;
    }
    _operators.insert(std::make_pair(client->getId(), client));
    return true;
}

bool Channel::removeOperator(Client* client) {
    std::map<unsigned int, Client*>::iterator it = _operators.find(client->getId());
    if (it != _operators.end()) {
        _operators.erase(it);
        return true;
    }
    return false;
}

bool Channel::kickClient(Client* operator_client, Client* target, const std::string& reason) {
    if (!isOperator(operator_client)) {
        return false;
    }
    std::string kickMessage = ":" + operator_client->getNickname() + " KICK " + 
                             _name + " " + target->getNickname() + " :" + reason;
    broadcastMessage(kickMessage);
    return removeClient(target);
}

bool Channel::inviteClient(Client* operator_client, Client* target) {
    if (!isOperator(operator_client)) {
        return false;
    }
    return addClient(target);
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
    std::string modeStr;
    std::string message;

    if (enabled)
        modeStr = "+";
    else
        modeStr = "-";
    modeStr += mode;

    message = ":";
    message += changer->getNickname();
    message += "!";
    message += changer->getUsername();
    message += "@";
    message += SERVER_NAME;
    message += " MODE ";
    message += _name;
    message += " ";
    message += modeStr;
    
    if (!param.empty()) {
        message += " ";
        message += param;
    }
    broadcastMessage(message);
}
