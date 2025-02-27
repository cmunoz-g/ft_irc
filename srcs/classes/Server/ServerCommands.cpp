#include "IRC.hpp"

void Server::handleCapCommand(Message &message) {
    if (message.getParams().empty()) { // Prevent accessing invalid indexes
        _clients[message.getSenderId()]->receiveMessage(":" + SERVER_NAME + " 410 :Invalid CAP command\r\n");
        return;
    }

    std::string capSubcommand = message.getParams()[0];

    if (capSubcommand == "LS") { 
        std::string response = ":" + SERVER_NAME + " CAP * LS :multi-prefix\r\n";
        _clients[message.getSenderId()]->receiveMessage(response);
    } 
    else if (capSubcommand == "REQ") { 
        if (message.getParams().size() < 2) { // Prevent missing param error
            _clients[message.getSenderId()]->receiveMessage(":" + SERVER_NAME + " 410 :Missing capability request\r\n");
            return;
        }

        std::string response = ":" + SERVER_NAME + " CAP * ACK :" + message.getParams()[1] + "\r\n";
        _clients[message.getSenderId()]->receiveMessage(response);
    } 
    else if (capSubcommand == "END") { 
		std::stringstream ss;
		ss << "[LOG] [ID:" << message.getSenderId() << "] Capability negotiation ended for the client" << std::endl;
		std::cout << ss.str();

        _clients[message.getSenderId()]->setCapNegotiationStatus(true);
    } 
    else { 
        // Unknown CAP command handling
        std::string response = ":" + SERVER_NAME + " 410 :Unknown CAP subcommand\r\n";
        _clients[message.getSenderId()]->receiveMessage(response);
    }
}

void Server::handleNickCommand(Message &message) { 
    Client *client = _clients[message.getSenderId()];
    std::string nickname = message.getParams()[0];

    if (!_password.empty() && !client->isAuthenticated()) {
        client->receiveMessage(":" + SERVER_NAME + " 451 " + nickname + " :You have not registered\r\n");
        return;
    }

    if (checkUniqueNick(nickname)) {
        client->setNickname(nickname);

        std::stringstream ss;
        ss << "[LOG] [ID:" << message.getSenderId() << "] NICK command handled, nickname saved as " 
           << client->getNickname() << std::endl;
        std::cout << ss.str();
    } 
    else {
        std::string response = ":" + SERVER_NAME + " 433 " + nickname + " :Nickname is already in use\r\n";
        client->receiveMessage(response);
    }
}


void Server::handleModeCommand(Message &message) {
    Client *client = _clients[message.getSenderId()];
    std::string nickname = client->getNickname();
    std::string response;

    if (message.getParams().empty()) {
        response = ":" + SERVER_NAME + " 461 " + nickname + " MODE :Not enough parameters\r\n";
    } else {
        std::string target = message.getParams()[0];

        if (target[0] == '#') { // Handling Channel Modes
            if (_channels.find(target) == _channels.end()) {
                response = ":" + SERVER_NAME + " 403 " + nickname + " " + target + " :No such channel\r\n";
                client->receiveMessage(response);
                return;
            }

            Channel *channel = _channels[target];

            if (message.getParams().size() == 1) { // Retrieving current channel modes
                response = ":" + SERVER_NAME + " 324 " + nickname + " " + target + " " + channel->getModes() + "\r\n";
            } else { // Setting channel modes
                std::string modeString = message.getParams()[1];

                // Validate each mode before applying
                for (size_t i = 0; i < modeString.length(); i++) {
                    char mode = modeString[i];
                    if (mode != '+' && mode != '-' && !isValidMode(mode, true)) {
                        response = ":" + SERVER_NAME + " 501 " + nickname + " :Unknown MODE flag\r\n";
                        client->receiveMessage(response);
                        return;
                    }
                }

                if (channel->isOperator(client)) {
                    channel->setModesFromString(modeString, message.getParams(), client);
                    
                    response = ":" + client->getNickname() + "!" + client->getUsername() + "@" + SERVER_NAME + 
                               " MODE " + target + " " + modeString + "\r\n";
                } else {
                    response = ":" + SERVER_NAME + " 482 " + nickname + " " + target + " :You're not a channel operator\r\n";
                }
            }
        } 
        
        else if (target == client->getNickname()) { // Handling User Modes
            if (message.getParams().size() == 1) { // Retrieving current user modes
                response = ":" + SERVER_NAME + " 221 " + nickname + " " + client->getModes() + "\r\n";
            } else { // Setting user modes
                std::string modeString = message.getParams()[1];

                // Validate each user mode
                for (size_t i = 0; i < modeString.length(); i++) {
                    char mode = modeString[i];
                    if (mode != '+' && mode != '-' && !isValidMode(mode, false)) {
                        response = ":" + SERVER_NAME + " 501 " + nickname + " :Unknown MODE flag\r\n";
                        client->receiveMessage(response);
                        return;
                    }
                }

                client->setModesFromString(modeString);
                
                response = ":" + client->getNickname() + "!" + client->getUsername() + "@" + SERVER_NAME + 
                           " MODE " + nickname + " " + modeString + "\r\n";
            }
        } 
        
        else {
            response = ":" + SERVER_NAME + " 401 " + nickname + " " + target + " :No such nick/channel\r\n";
        }
    }

    client->receiveMessage(response);
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
		std::stringstream ss;
		ss << "[LOG] [ID:" << message.getSenderId() << "] PASS command handled, incorrect password" << std::endl;
		std::cout << ss.str();

	}
	else {
		_clients[message.getSenderId()]->setAuthenticated(true);
		std::stringstream ss;
		ss << "[LOG] [ID:" << message.getSenderId() << "] PASS command handled, client authenticated" << std::endl;
		std::cout << ss.str();
	}
}

void Server::handleUserCommand(Message &message) {
    Client *client = _clients[message.getSenderId()];

    if (!_password.empty() && !client->isAuthenticated()) {
        client->receiveMessage(":" + SERVER_NAME + " 451 " + client->getNickname() + " :You have not registered\r\n");
        return;
    }

    client->setUsername(message.getParams()[0]);

    std::stringstream ss;
    ss << "[LOG] [ID:" << message.getSenderId() << "] USER command handled, username saved as " 
       << client->getUsername() << std::endl;
    std::cout << ss.str();

    if (!client->getNickname().empty() && !client->getUsername().empty() && client->isAuthenticated()) {
        std::string response = ":" + SERVER_NAME + " 001 " + client->getNickname() + " :Welcome to the IRC Server!\r\n";
        client->receiveMessage(response);
    }
}


void Server::handleJoinCommand(Message &message) {
    if (message.getParams().empty() || message.getParams()[0].empty()) {
        return; // No parameters, ignore (needed for Irssi auto-join)
    }

    std::string channelsParam = message.getParams()[0];
    std::vector<std::string> channels;
    std::stringstream ss(channelsParam);
    std::string channelName;

    // Split input by comma for multiple channels
    while (std::getline(ss, channelName, ',')) {
        channels.push_back(channelName);
    }

    Client *client = _clients[message.getSenderId()];

    for (size_t i = 0; i < channels.size(); i++) {
        std::string channelName = channels[i];

        // Reject channel names such as # or &
        if (channelName.length() < 2) {
            client->receiveMessage(":" + SERVER_NAME + " 403 " + client->getNickname() + " " + channelName + " :No such channel\r\n");
            continue;
        }

        // Check if the channel exists
        if (_channels.find(channelName) == _channels.end()) {
            // Channel does NOT exist, create it
            _channels[channelName] = new Channel(channelName, client);
            _channels[channelName]->addClient(client);
            _channels[channelName]->addOperator(client);

            // Handle password-protected channels (`+k`)
            if (message.getParams().size() > 1) {
                _channels[channelName]->setPassword(message.getParams()[1]);
                _channels[channelName]->setMode(IRC::MODE_K, true);
            }

            // Send channel info
            std::string topic_response = ":" + SERVER_NAME + " 331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n";
            client->receiveMessage(topic_response);
            _channels[channelName]->sendNames(client);
            std::string mode_response = ":" + SERVER_NAME + " 324 " + client->getNickname() + " " + channelName + " +o\r\n";
            client->receiveMessage(mode_response);
        } 
        else {
            Channel *channel = _channels[channelName];

            // Check if user is already in the channel
            if (channel->hasClient(client)) {
                std::string response = ":" + SERVER_NAME + " 443 " + client->getNickname() + " " + channelName + " :is already on channel\r\n";
                client->receiveMessage(response);
                continue;
            }

            // Check invite-only mode (`+i`)
            if (channel->hasMode(IRC::MODE_I) && !channel->isInvitedClient(client)) {
                std::string response = ":" + SERVER_NAME + " 473 " + client->getNickname() + " " + channelName + " :Cannot join channel (+i)\r\n";
                client->receiveMessage(response);
                continue;
            }

            // Check password-protected channels (`+k`)
            if (channel->hasMode(IRC::MODE_K)) {
                if (message.getParams().size() < 2 || channel->getPassword() != message.getParams()[1]) {
                    std::string response = ":" + SERVER_NAME + " 475 " + client->getNickname() + " " + channelName + " :Cannot join channel (+k)\r\n";
                    client->receiveMessage(response);
                    continue;
                }
            }

            // Check if channel has a user limit (`+l`)
            if (!channel->addClient(client)) {
                std::string response = ":" + SERVER_NAME + " 471 " + client->getNickname() + " " + channelName + " :Cannot join channel (+l)\r\n";
                client->receiveMessage(response);
                continue;
            }

            // Send join confirmation
            std::string joinMsg = ":" + client->getNickname() + " JOIN " + channelName + "\r\n";
            client->receiveMessage(joinMsg);
            channel->broadcastMessage(joinMsg, client);

            // Send topic information
            std::string topic_response;
            if (channel->hasMode(IRC::MODE_T)) {
                topic_response = ":" + SERVER_NAME + " 332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n";
            } else {
                topic_response = ":" + SERVER_NAME + " 331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n";
            }
            client->receiveMessage(topic_response);

            // Send user list
            channel->sendNames(client);
        }
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
		std::string response = ":" + SERVER_NAME + " 461 " + inviter->getNickname() + " INVITE :Not enough parameters\r\n";
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
    std::map<unsigned int, Client*>::iterator it_client = _clients.find(message.getSenderId());
    if (it_client == _clients.end())
        return;

    Client *client = it_client->second;
    std::string reason = (message.getParams().empty()) ? "Client quit" : message.getParams()[0];
    std::string quitResponse = ":" + client->getNickname() + "!" + client->getUsername() + "@" + SERVER_NAME + " QUIT :" + reason + "\r\n";

    // Notify all channels
    std::map<const std::string, Channel*>::iterator it;
    for (it = _channels.begin(); it != _channels.end(); ++it) {
        Channel *channel = it->second;
        if (channel->hasClient(client)) {
            channel->broadcastMessage(quitResponse, client);
        }
    }

    client->receiveMessage(quitResponse);
    
    removeClient(message.getSenderId());
}
