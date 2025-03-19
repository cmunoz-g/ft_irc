/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCTypes.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/28 13:23:33 by juramos           #+#    #+#             */
/*   Updated: 2025/03/19 10:09:57 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRC_TYPES_HPP
#define IRC_TYPES_HPP

namespace IRC {
	enum CommandType {
		// *** Connection validation between client and server ***
		CMD_CAP,
		
		// *** User authentication and configuration *** 
		CMD_PASS,    // Authenticate with password
		CMD_NICK,    // Set/change nickname
		CMD_USER,    // Set username
		
		// *** Channel commands *** 
		CMD_JOIN,    // Join a channel
		CMD_PRIVMSG, // Send private/channel message
		
		// *** Operator commands *** 
		CMD_KICK,    // Kick user from the channel
		CMD_INVITE,  // Invite user to the channel
		CMD_TOPIC,   // Change/view channel topic
		CMD_MODE,    // Change channel mode
		
		// *** Additional necessary commands *** 
		CMD_PING,    // Keep connection alive
		CMD_PONG,    // Response to PING
		CMD_QUIT,    // Disconnect from the server
		
		CMD_UNKNOWN 
	};

	enum ChannelMode {
		MODE_NONE,
		MODE_I,    // i - Invite-only channel
		MODE_T,    // t - Only ops can change the topic
		MODE_K,    // k - Channel has a password
		MODE_O,    // o - Operator privileges
		MODE_L     // l - User limit
	};

	enum ClientMode {
		C_MODE_NONE,
		C_MODE_O,    // o - Operator privileges
		C_MODE_I     // i - Invisible
	};
}

#endif