/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/26 10:44:00 by juramos           #+#    #+#             */
/*   Updated: 2025/03/19 12:18:25 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

class Client;

#include "IRC.hpp"
#include "IRCTypes.hpp"

class Client;

class Channel {
private:
    // *** Attributes ***
    std::string                     _name;                                  
    std::string                     _topic;                                 
    std::string                     _password;          // Password (+k)
    std::vector<unsigned int>       _modes;                   
    std::map<unsigned int, Client*> _clients;           
    std::map<unsigned int, Client*> _operators;         
    std::map<unsigned int, Client*> _invitedClients;    
    size_t                          _userLimit;         // User limit (+l)
	
	// *** Methods ***
	bool	checkPassword(const std::string& pass) const;
	bool    canModifyTopic(Client* client) const;
	void    inviteClient(Client* operator_client, Client* target);
	void    removeInvitedClient(Client *client);
	void    notifyModeChange(Client* changer, char mode, bool enabled, const std::string& param = "");

    public:
    // Constructor & Destructor
    Channel(const std::string& name, Client* creator);
    ~Channel();
    
    // Getters & Setters
    const std::string&              getName() const;
    const std::string&              getTopic() const;
    const std::string&              getPassword() const;
    size_t                          getUserCount() const;
    size_t                          getUserLimit() const;
    std::string                     getModes() const;
    std::map<unsigned int, Client*> getClients() const;
    bool                            setTopic(Client* client, const std::string& newTopic);
    void                            setPassword(const std::string& pass);
    void                            setUserLimit(size_t limit);
    // - Mode Setters
    void                            setMode(IRC::ChannelMode mode, bool enabled = true);
    bool                            setModesFromString(const std::string& modeString, const std::vector<std::string>& params);
    
    // Checks
    bool    isOperator(Client* client) const;
    bool    hasMode(IRC::ChannelMode mode) const;
    bool    isInviteOnly() const;
    
    // Client Operations
    bool    addClient(Client* client);
    void    removeClient(Client* client);
    bool    hasClient(Client* client) const;
    
    // Invited Client Operations
    void    addInvitedClient(Client* client);
    bool    isInvitedClient(Client* client) const;
    
    // Operator Operations
    void    addOperator(Client* client);
    void    removeOperator(Client* client);
    
    // Communication
    void    broadcastMessage(const std::string& message, Client* exclude = NULL);
    void    sendNames(Client* client) const;

};

#endif