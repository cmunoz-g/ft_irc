/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/25 12:01:43 by cmunoz-g          #+#    #+#             */
/*   Updated: 2025/03/19 12:21:37 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "IRC.hpp"
#include "IRCTypes.hpp"

struct CommandData {
	std::string					_prefix;
	std::string					_command;
	std::vector<std::string>	_params;
};

class Message {
private:
	// *** Attributes ***
    CommandData										_command;
	IRC::CommandType								_commandType;
	static std::map<std::string, IRC::CommandType>	_commandMap;
	int												_senderSocket;
	unsigned int									_senderId;
	std::string										_receiverChannel;
	
	// *** Methods ***
	void	parse(const std::string& buffer);
	void	setReceiver(void);
	void    setCommandType(void);
    
public:
	// Constructor, Destructor
	Message(const Client *client);
	~Message(void);
    
    // Getters
    const std::string&				getPrefix() const;
    const std::string&				getCommand() const;
    const std::vector<std::string>& getParams() const;
    IRC::CommandType				getCommandType() const;
	int								getSenderSocket() const;
	unsigned int					getSenderId() const;
	std::string						getReceiverChannel() const;

	// Print
	void							printMessageDebug(int client_id) const;
	
	// Static
	static void						initCommandMap(void);
};

#endif