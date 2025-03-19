#include "IRC.hpp"

void Server::broadcastNickChange(Client* client, const std::string &oldNick, const std::string &newNick) {
    if (oldNick.empty() || oldNick == newNick)
        return;

    std::string nickMsg = ":" + oldNick + "!" + client->getUsername() + "@" + SERVER_NAME + " NICK :" + newNick + "\r\n";

    std::map<const std::string, Channel*> channels = client->getChannels(); 
    
    std::map<const std::string, Channel*>::iterator it = channels.begin();
    std::map<unsigned int, Client*> clientsToReceive;
    for (; it != channels.end(); ++it) {
        Channel* channel = it->second;
        if (channel) {
            std::map<unsigned int, Client*> clientsInChannel = channel->getClients();
            for (std::map<unsigned int, Client*>::iterator it = clientsInChannel.begin(); it != clientsInChannel.end(); ++it) {
                clientsToReceive[it->first] = it->second;
            }
        }
    }

    for (std::map<unsigned int, Client*>::iterator it = clientsToReceive.begin(); it != clientsToReceive.end(); ++it) {
        it->second->receiveMessage(nickMsg);
    }
}

bool Server::checkUniqueNick(std::string nick) {
	std::map<unsigned int, Client*>::iterator it = _clients.begin();

	while (it != _clients.end()) {
		if (it->second->getNickname() == nick)
			return (false); // Not unique
		++it;
	}
	return (true);
}

void Server::tryRegister(Client* client) {
    if (client->isRegistered()) {
        return;
    }

    if (!_password.empty() && !client->isAuthenticated()) {
        return; 
    }

    if (client->getUsername().empty()) {
        return;
    }

    if (client->getNickname().empty()) {
        return;
    }

    client->setRegistered(true);
    std::string response = ":" + SERVER_NAME + " 001 " + client->getNickname() + " :Welcome to the IRC Server!\r\n";
    client->receiveMessage(response);
}

unsigned int Server::fetchClientIdFromPid(int fd) {
    for (std::map<unsigned int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->getSocket() == fd) {
            return it->first;
        }
    }
    std::stringstream ss;
	ss << "Client with file descriptor " << fd << " not found" << std::endl;
    error(ss.str(), true, false);
    return 0; 
}