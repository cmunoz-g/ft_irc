/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCTypes.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/28 13:23:33 by juramos           #+#    #+#             */
/*   Updated: 2025/03/13 13:54:23 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRC_TYPES_HPP
#define IRC_TYPES_HPP

namespace IRC {
    enum CommandType {
		// Validación conexión entre cliente y servidor
		CMD_CAP,
        // Autenticación y configuración de usuario
        CMD_PASS,    // Autenticación con password
        CMD_NICK,    // Establecer/cambiar nickname
        CMD_USER,    // Establecer username
        
        // Comandos de canal
        CMD_JOIN,    // Unirse a un canal
        CMD_PRIVMSG, // Enviar mensaje privado/canal
        
        // Comandos de operador
        CMD_KICK,    // Expulsar usuario del canal
        CMD_INVITE,  // Invitar usuario al canal
        CMD_TOPIC,   // Cambiar/ver topic del canal
        CMD_MODE,    // Cambiar modo del canal
        
        // Comandos adicionales necesarios
        CMD_PING,    // Mantener conexión viva
        CMD_PONG,    // Respuesta a PING
        CMD_QUIT,     // Desconexión del servidor
        
        CMD_UNKNOWN 
    };

    // Modos de canal requeridos según el subject
    enum ChannelMode {
        MODE_NONE,
        MODE_I,    // i - Canal solo por invitación
        MODE_T,    // t - Solo ops pueden cambiar topic
        MODE_K,    // k - Canal tiene password
        MODE_O,    // o - Privilegios de operador
        MODE_L     // l - Límite de usuarios
    };

    enum ClientMode {
        C_MODE_NONE,
        C_MODE_O,    // o - Privilegios de operador
        C_MODE_I    // i - Invisible
    };
}

#endif