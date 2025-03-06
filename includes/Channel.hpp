/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/26 10:44:00 by juramos           #+#    #+#             */
/*   Updated: 2025/03/06 21:45:16 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

class Client;

#include "IRC.hpp"
#include "IRCTypes.hpp"

class Client;

class Channel {
private:
    std::string _name;                                  // Nombre del canal
    std::string _topic;                                 // Topic del canal
    std::string _password;                              // Password (para modo k)
    std::vector<unsigned int> _modes;                   // Modos activos (usando ChannelMode)
    std::map<unsigned int, Client*> _clients;           // Clientes en el canal
    std::map<unsigned int, Client*> _operators;         // Operadores (true) y usuarios normales (false)
    std::map<unsigned int, Client*> _invitedClients;    // Clientes invitados
    size_t _userLimit;                                  // Límite de usuarios (para modo l)
    Channel(void);
    // Métodos auxiliares
    bool canModifyTopic(Client* client) const;
    bool isInviteOnly() const;
    void notifyModeChange(Client* changer, char mode, bool enabled, const std::string& param = "");
    
    public:
    // Constructor y destructor
    Channel(const std::string& name, Client* creator);
    ~Channel();
    Channel(const Channel& other);
    Channel& operator=(const Channel& other);

    // Getters básicos
    const std::string& getName() const;
    const std::string& getTopic() const;
    const std::string& getPassword() const;
    size_t getUserCount() const;
    size_t getUserLimit() const;
    bool isOperator(Client* client) const;

    // Manejo de modos (requeridos por el subject)
    void setMode(IRC::ChannelMode mode, bool enabled = true);
    bool setModesFromString(const std::string& modeString, const std::vector<std::string>& params);
    std::string getModes() const;
    bool hasMode(IRC::ChannelMode mode) const;
    void setPassword(const std::string& pass);
    void setUserLimit(size_t limit);
    bool checkPassword(const std::string& pass) const;

    // Manejo de topic (requerido por el subject)
    bool setTopic(Client* client, const std::string& newTopic);
    
    // Manejo de usuarios
    bool addClient(Client* client);
    void removeClient(Client* client);
    bool hasClient(Client* client) const;
    
    // Manejo de invitados
    void addInvitedClient(Client* client);
    bool isInvitedClient(Client* client) const;
    void removeInvitedClient(Client *client);

    // Manejo de operadores
    void addOperator(Client* client);
    void removeOperator(Client* client);
    
    // Comandos de operador (requeridos por el subject)
    void inviteClient(Client* operator_client, Client* target);

    // Mensajes
    void broadcastMessage(const std::string& message, Client* exclude = NULL);
    void sendNames(Client* client) const;

};

#endif