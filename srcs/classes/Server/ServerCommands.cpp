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
		ss << YELLOW << "[LOG] " << RESET << "[ID:" << message.getSenderId() << "] Capability negotiation ended for the client" << std::endl;
		std::cout << ss.str();

        _clients[message.getSenderId()]->setCapNegotiationStatus(true);
    } 
    else { 
        // 410 ERR_INVALIDCAPCMD
        std::string response = ":" + SERVER_NAME + " 410 :Unknown CAP subcommand\r\n";
        _clients[message.getSenderId()]->receiveMessage(response);
    }
}

void Server::handleNickCommand(Message &message) {
    Client *client = _clients[message.getSenderId()];
    
    if (!_password.empty() && !client->isAuthenticated()) {
        // 451 ERR_NOTREGISTERED 
        client->receiveMessage(":" + SERVER_NAME + " 451 * :You have not registered\r\n");
        return;
    }
    
    if (message.getParams().empty()) {
        // 431 ERR_NONICKNAMEGIVEN
        std::string response = ":" + SERVER_NAME + " 431 " 
                             + (client->getNickname().empty() ? "*" : client->getNickname()) 
                             + " :No nickname given\r\n";
        client->receiveMessage(response);
        return;
    }
    
    // The user wants to change to this nickname
    std::string nickname = message.getParams()[0];
    
    // Validate nickname
    if (!isValidNick(nickname)) {
        // 432 ERR_ERRONEUSNICKNAME
        std::string response = ":" + SERVER_NAME + " 432 " 
                             + (client->getNickname().empty() ? "*" : client->getNickname()) 
                             + " " + nickname + " :Erroneous nickname\r\n";
        client->receiveMessage(response);
        return;
    }

    // Nick uniqueness check
    if (!checkUniqueNick(nickname)) {
        // 433 ERR_NICKNAMEINUSE
        std::string currentNick = client->getNickname().empty() ? "*" : client->getNickname();
        std::string response = ":" + SERVER_NAME + " 433 " 
                             + currentNick + " " + nickname 
                             + " :Nickname is already in use\r\n";
        client->receiveMessage(response);
        return;
    }

    // At this point, the nickname is valid and unique.
    // If the client already had a nickname, broadcast the change.
    std::string oldNick = client->getNickname();
    client->setNickname(nickname);

    broadcastNickChange(client, oldNick, nickname);

    std::cout << YELLOW << "[LOG] " << RESET 
              << "[ID:" << message.getSenderId() 
              << "] NICK command handled, nickname saved as " 
              << client->getNickname() << std::endl;

    // After setting a valid nick, attempt registration
    tryRegister(client);
}

void Server::handleModeCommand(Message &message) {
    Client *client = _clients[message.getSenderId()];
    std::string nickname = client->getNickname();
    std::string response;

    if (message.getParams().empty()) {
        // 461 ERR_NEEDMOREPARAMS
        response = ":" + SERVER_NAME + " 461 " + nickname + " MODE :Not enough parameters\r\n";
    } else {
        std::string target = message.getParams()[0];

        if (target[0] == '#' || target[0] == '&') { // Handling Channel Modes
            if (_channels.find(target) == _channels.end()) {
                // 403 ERR_NOSUCHCHANNEL
                response = ":" + SERVER_NAME + " 403 " + nickname + " " + target + " :No such channel\r\n";
                client->receiveMessage(response);
                return;
            }

            Channel *channel = _channels[target];

            if (message.getParams().size() == 1) { // Retrieving current channel modes
                // 324 RPL_CHANNELMODEIS
                response = ":" + SERVER_NAME + " 324 " + nickname + " " + target + " " + channel->getModes() + "\r\n";
            } else { // Setting channel modes
                std::string modeString = message.getParams()[1];

                // Validate each mode before applying
                for (size_t i = 0; i < modeString.length(); i++) {
                    char mode = modeString[i];
                    if (mode != '+' && mode != '-' && !isValidMode(mode, true)) {
                        // 501 ERR_UMODEUNKNOWNFLAG
                        response = ":" + SERVER_NAME + " 501 " + nickname + " :Unknown MODE flag\r\n";
                        client->receiveMessage(response);
                        return;
                    }
                }

                if (channel->isOperator(client)) {
                    if (!channel->setModesFromString(modeString, message.getParams())) {
                        // 441 ERR_USERNOTINCHANNEL
                        response = ":" + SERVER_NAME + " 441 " + nickname + " " + target + " :They aren't on that channel\r\n";
                    }
                    else {
                        response = ":" + client->getNickname() + "!" + client->getUsername() + "@" + SERVER_NAME + 
                                   " MODE " + target + " " + modeString + "\r\n";
                    }
                } else {
                    /// 482 ERR_CHANOPRIVSNEEDED
                    response = ":" + SERVER_NAME + " 482 " + nickname + " " + target + " :You're not a channel operator\r\n";
                }
            }
        } 
        
        else if (target == client->getNickname()) { // Handling User Modes
            if (message.getParams().size() == 1) { // Retrieving current user modes
                // 221 RPL_UMODEIS
                response = ":" + SERVER_NAME + " 221 " + nickname + " " + client->getModes() + "\r\n";
            } else { // Setting user modes
                std::string modeString = message.getParams()[1];

                // Validate each user mode
                for (size_t i = 0; i < modeString.length(); i++) {
                    char mode = modeString[i];
                    if (mode != '+' && mode != '-' && !isValidMode(mode, false)) {
                        // 501 ERR_UMODEUNKNOWNFLAG
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
            // 401 ERR_NOSUCHNICK
            response = ":" + SERVER_NAME + " 401 " + nickname + " " + target + " :No such nick\r\n";
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

bool Server::handlePassCommand(Message &message) {
    Client *client = _clients[message.getSenderId()];
	std::string nickname = client->getNickname().empty() ? "*" : client->getNickname();

	if (client->isAuthenticated()) {
        // 462 ERR_ALREADYREGISTRED
		std::string response = ":" + SERVER_NAME + " 462 " + nickname + " :You may not reregister\r\n";
		client->receiveMessage(response);
	}
	else if (message.getParams().empty()) {
        // 461 ERR_NEEDMOREPARAMS
		std::string response = ":" + SERVER_NAME + " 461 " + nickname + " PASS :Not enough parameters\r\n";
		client->receiveMessage(response);
        return false;
	}
	else if (message.getParams()[0] != getPassword()) {
        // 464 ERR_PASSWDMISMATCH
		std::string response = ":" + SERVER_NAME + " 464 " + nickname + " :Password incorrect\r\n";
		client->receiveMessage(response);
		std::stringstream ss;
		ss << YELLOW << "[LOG] " << RESET << "[ID:" << message.getSenderId() << "] PASS command handled, incorrect password" << std::endl;
		std::cout << ss.str();
        return false;
	}
	else {
		client->setAuthenticated(true);
		std::stringstream ss;
		ss << YELLOW << "[LOG] " << RESET << "[ID:" << message.getSenderId() << "] PASS command handled, client authenticated" << std::endl;
		std::cout << ss.str();
	}
    return true;
}

void Server::handleUserCommand(Message &message) {
    Client *client = _clients[message.getSenderId()];

    if (client->isRegistered()) {
        std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
        client->receiveMessage(":" + SERVER_NAME + " 462 " + nick + " :You may not reregister\r\n");
        return;
    }
    
    if (!_password.empty() && !client->isAuthenticated()) {
        // 451 ERR_NOTREGISTERED
        client->receiveMessage(":" + SERVER_NAME + " 451 " + client->getNickname() + " :You have not registered\r\n");
        return;
    }

    // USER has 4 params (username, hostname, server, realname),
    // but you only strictly need the first param as "username."
    client->setUsername(message.getParams()[0]);
    std::cout << YELLOW << "[LOG] " << RESET << "[ID:" << message.getSenderId() << "] USER command handled, username saved as " 
              << client->getUsername() << std::endl;

    // After setting a username, attempt registration
    tryRegister(client);
}


void Server::handleJoinCommand(Message &message) {
    Client *client = _clients[message.getSenderId()];

    // If there's no channel parameter, Irssi often sends an empty JOIN in auto-join scenarios.
    // Just return in that case.
    if (message.getParams().empty() || message.getParams()[0].empty()) {
		// 461 ERR_NEEDMOREPARAMS
        std::string response = ":" + SERVER_NAME + " 461 " + client->getNickname() + " JOIN :Not enough parameters\r\n";
        client->receiveMessage(response);
        return;
    }

    // JOIN 0 is the standard way to leave all channels at once
    if (message.getParams().size() == 1 && message.getParams()[0] == "0") {
        client->leaveAllChannels();
        return;
    }

    // The first parameter is a comma-separated list of channels (no spaces)
    // e.g. "#chan1,#chan2"
    std::string channelsParam = message.getParams()[0];
    std::vector<std::string> channels;
    {
        std::stringstream ss(channelsParam);
        std::string chan;
        while (std::getline(ss, chan, ',')) {
            size_t start = chan.find_first_not_of(" \t\r\n");
            size_t end   = chan.find_last_not_of(" \t\r\n");

            // If everything was whitespace OR it's truly empty (trailing comma),
            // we push an empty string so the code below can return 403 for it.
            if (start == std::string::npos || end == std::string::npos) {
                channels.push_back("");
            } else {
                channels.push_back(chan.substr(start, end - start + 1));
            }
        }
    }

    // For multi-key logic, the second parameter can be a comma-separated list of keys
    // e.g. "key1,key2,key3"
    std::vector<std::string> keys;
    if (message.getParams().size() > 1) {
        std::string keysParam = message.getParams()[1];
        // Split by comma
        std::stringstream ss(keysParam);
        std::string key;
        while (std::getline(ss, key, ',')) {
            size_t start = key.find_first_not_of(" \t\r\n");
            size_t end   = key.find_last_not_of(" \t\r\n");
            if (start != std::string::npos && end != std::string::npos) {
                keys.push_back(key.substr(start, end - start + 1));
            } else {
                // If the key chunk is empty, we push an empty string
                keys.push_back("");
            }
        }
    }

    // Process each channel
    for (size_t i = 0; i < channels.size(); i++) {
        std::string channelName = channels[i];

        // If there's a corresponding key (i < keys.size()), apply it
        // Otherwise, no key is provided for this channel
        std::string providedKey;
        if (i < keys.size()) {
            providedKey = keys[i];
        }

        // If there's no '#' or '&', 403 ERR_NOSUCHCHANNEL. Also covers empty string
        if (channelName.size() < 2 || (channelName[0] != '#' && channelName[0] != '&')) {
            std::string errMsg = ":" + SERVER_NAME + " 403 " + client->getNickname() + " "
                                 + channelName + " :No such channel\r\n";
            client->receiveMessage(errMsg);
            continue;
        }

        // If channel doesn't exist yet, create it
        if (_channels.find(channelName) == _channels.end()) {
			if (channelName.size() > MAX_CHANNEL_LEN) {
    			channelName = channelName.substr(0, MAX_CHANNEL_LEN);
			}
            Channel *newChannel = new Channel(channelName, client);
            _channels[channelName] = newChannel;

            // If the user provided a key for a brand-new channel, set +k
            if (!providedKey.empty()) {
                newChannel->setPassword(providedKey);
                newChannel->setMode(IRC::MODE_K, true);
            }

            std::string joinMsg = ":" + client->getNickname() + "!"
                + client->getUsername() + "@" + SERVER_NAME
                + " JOIN " + channelName + "\r\n";

            client->receiveMessage(joinMsg);
            newChannel->broadcastMessage(joinMsg, client);

            // If no topic set, 331 RPL_NOTOPIC
            // otherwise 332 RPL_TOPIC
            if (newChannel->getTopic().empty()) {
                std::string noTopic = ":" + SERVER_NAME + " 331 " + client->getNickname() +
                                      " " + channelName + " :No topic is set\r\n";
                client->receiveMessage(noTopic);
            } else {
                std::string topicReply = ":" + SERVER_NAME + " 332 " + client->getNickname() +
                                         " " + channelName + " :" + newChannel->getTopic() + "\r\n";
                client->receiveMessage(topicReply);
            }

            newChannel->sendNames(client);
            client->joinChannel(newChannel);
            client->setOperatorStatus(newChannel);
        }
        else {
            // Channel already exists
            Channel *channel = _channels[channelName];

            // 443 ERR_USERONCHANNEL
            if (channel->hasClient(client)) {
                std::string errMsg = ":" + SERVER_NAME + " 443 " + client->getNickname() 
                                     + " " + channelName + " :is already on channel\r\n";
                client->receiveMessage(errMsg);
                continue;
            }

            // 473 ERR_INVITEONLYCHAN
            if (channel->hasMode(IRC::MODE_I) && !channel->isInvitedClient(client)) {
                std::string errMsg = ":" + SERVER_NAME + " 473 " + client->getNickname()
                                     + " " + channelName + " :Cannot join channel (+i)\r\n";
                client->receiveMessage(errMsg);
                continue;
            }

            // If +k is set, verify the correct key
            if (channel->hasMode(IRC::MODE_K)) {
                if (providedKey.empty() || providedKey != channel->getPassword()) {
                    std::string errMsg = ":" + SERVER_NAME + " 475 " + client->getNickname()
                                         + " " + channelName + " :Cannot join channel (+k)\r\n";
                    client->receiveMessage(errMsg);
                    continue;
                }
            }

            // 471 ERR_CHANNELISFULL
            if (!channel->addClient(client)) {
                std::string errMsg = ":" + SERVER_NAME + " 471 " + client->getNickname()
                                     + " " + channelName + " :Cannot join channel (+l)\r\n";
                client->receiveMessage(errMsg);
                continue;
            }

            // Successful join
            std::string joinMsg = ":" + client->getNickname() + "!"
                + client->getUsername() + "@" + SERVER_NAME
                + " JOIN " + channelName + "\r\n";
            client->receiveMessage(joinMsg);
            channel->broadcastMessage(joinMsg, client);

            // If the channel has a topic, send 332; otherwise 331
            if (!channel->getTopic().empty()) {
                std::string topicReply = ":" + SERVER_NAME + " 332 " + client->getNickname() +
                                         " " + channelName + " :" + channel->getTopic() + "\r\n";
                client->receiveMessage(topicReply);
            } else {
                std::string noTopic = ":" + SERVER_NAME + " 331 " + client->getNickname() +
                                      " " + channelName + " :No topic is set\r\n";
                client->receiveMessage(noTopic);
            }

            channel->sendNames(client);
            client->joinChannel(channel);
        }
    }
}


void Server::handlePrivmsgCommand(Message &message) {
	Client *sender = _clients[message.getSenderId()];

	if (message.getParams().empty()) {
        // 411 ERR_NORECIPIENT
		std::string response = ":" + SERVER_NAME + " 411 " + sender->getNickname() + " :No recipient given (PRIVMSG)\r\n";
		sender->receiveMessage(response);
		return;
	}

	if (message.getParams().size() < 2 || message.getParams()[1].empty()) {
        // 412 ERR_NOTEXTTOSEND
		std::string response = ":" + SERVER_NAME + " 412 " + sender->getNickname() + " :No text to send\r\n";
		sender->receiveMessage(response);
		return;
	}

	std::string target = message.getParams()[0];
    std::string msg = message.getParams()[1];

	if (target[0] == '#') { // Target is a channel
		if (_channels.find(target) == _channels.end()) {
            // 403 ERR_NOSUCHCHANNEL
			std::string response = ":" + SERVER_NAME + " 403 " + sender->getNickname() + " " + target + " :No such channel\r\n";
			sender->receiveMessage(response);
			return;
		}
		Channel *channel = _channels[target];
		if (!channel->hasClient(sender)) {
            // 404 ERR_CANNOTSENDTOCHAN
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
        // 401 ERR_NOSUCHNICK
		std::string response = ":" + SERVER_NAME + " 401 " + sender->getNickname() + " " + target + " :No such nick\r\n";
		sender->receiveMessage(response);

	}
}

void Server::handleInviteCommand(Message &message) {
	Client *inviter = _clients[message.getSenderId()];
	
	if (message.getParams().size() < 2) {
        // 461 ERR_NEEDMOREPARAMS
		std::string response = ":" + SERVER_NAME + " 461 " + inviter->getNickname() + " INVITE :Not enough parameters\r\n";
		inviter->receiveMessage(response);
		return;
	}

	std::string nickname = message.getParams()[0];
	std::string channelName = message.getParams()[1];

	if (_channels.find(channelName) == _channels.end()) {
        // 403 ERR_NOSUCHCHANNEL
		std::string response = ":" + SERVER_NAME + " 403 " + inviter->getNickname() + " " + channelName + " :No such channel\r\n";
        inviter->receiveMessage(response);
		return;
	}

	Channel *channel = _channels[channelName];
	if (!channel->hasClient(inviter)) {
        // 442 ERR_NOTONCHANNEL
		std::string response = ":" + SERVER_NAME + " 442 " + inviter->getNickname() + " " + channelName + " :You're not on that channel\r\n";
        inviter->receiveMessage(response);
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
        // 401 ERR_NOSUCHNICK
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

    // 341 RPL_INVITING
	std::string inviterResponse = ":" + SERVER_NAME + " 341 " + inviter->getNickname() + " " + nickname + " " + channelName + "\r\n";
	inviter->receiveMessage(inviterResponse);
	
	channel->addInvitedClient(invitee);
}

void Server::handleTopicCommand(Message &message) {
	Client *client = _clients[message.getSenderId()];

	if (message.getParams().empty()) {
        // 461 ERR_NEEDMOREPARAMS
		std::string response = ":" + SERVER_NAME + " 461 TOPIC :Not enough parameters\r\n";
		client->receiveMessage(response);
		return;
	}
	
	std::string channelName = message.getParams()[0];

	if (_channels.find(channelName) == _channels.end()) {
        // 403 ERR_NOSUCHCHANNEL
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
        client->receiveMessage(response);
		return;
	}

	std::string topic_response = ":" + client->getNickname() + "!" + client->getUsername() + "@" + SERVER_NAME + " TOPIC " + channelName + " :" + newTopic + "\r\n";
	channel->broadcastMessage(topic_response, NULL);
}

void Server::handleKickCommand(Message &message) {
	Client *kicker = _clients[message.getSenderId()];

	if (message.getParams().size() < 2) {
        // 461 ERR_NEEDMOREPARAMS
		std::string response = ":" + SERVER_NAME + " 461 KICK :Not enough parameters\r\n";
		kicker->receiveMessage(response);
		return;
	}

	std::string channelName = message.getParams()[0];
	std::string targetName = message.getParams()[1];
	std::string reason = (message.getParams().size() > 2) ? message.getParams()[2] : "No reason";

	if (_channels.find(channelName) == _channels.end()) {
        // 403 ERR_NOSUCHCHANNEL
		std::string response = ":" + SERVER_NAME + " 403 " + channelName + " :No such channel\r\n";
		kicker->receiveMessage(response);
		return;
	}

	Channel *channel = _channels[channelName];
	if (!channel->hasClient(kicker)) {
        // 442 ERR_NOTONCHANNEL
		std::string response = ":" + SERVER_NAME + " 442 " + kicker->getNickname() + " " + channelName + " :You're not on that channel\r\n";
		kicker->receiveMessage(response);
		return;
	}
	if (!channel->isOperator(kicker)) {
		std::string response = ":" + SERVER_NAME + " 482 " + channelName + " :You're not a channel operator\r\n";
        kicker->receiveMessage(response);
        return;
	}

    std::cout << "|" << targetName << "|" << std::endl;
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
        // 401 ERR_NOSUCHNICK
		std::string response = ":" + SERVER_NAME + " 401 " + targetName + " :No such nick\r\n";
        kicker->receiveMessage(response);
        return;
	}

	std::string kickResponse = ":" + kicker->getNickname() + "!" + kicker->getUsername() + "@" + SERVER_NAME + " KICK " + channelName + " " + targetName + " :" + reason + "\r\n";
	channel->broadcastMessage(kickResponse, NULL);
	target->receiveMessage(kickResponse);
	channel->removeClient(target); 
	target->leaveChannel(channel);
	
}

void Server::handleQuitCommand(Message &message) {
    std::map<unsigned int, Client*>::iterator it_client = _clients.find(message.getSenderId());
    if (it_client == _clients.end())
        return;

    Client *client = it_client->second;
    std::string reason = (message.getParams().empty()) ? "Client quit" : message.getParams()[0];
    std::string quitResponse =
        ":" + client->getNickname() + "!" + client->getUsername() +
        "@" + SERVER_NAME + " QUIT :" + reason + "\r\n";

    // Notify all channels
    for (std::map<const std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        Channel *channel = it->second;
        if (channel->hasClient(client)) {
            channel->broadcastMessage(quitResponse, client);
        }
    }

    // Send the QUIT message back to the quitting client
    client->receiveMessage(quitResponse);
}
