/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRC.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/25 10:38:54 by juramos           #+#    #+#             */
/*   Updated: 2025/03/10 15:14:48 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRC_HPP
#define IRC_HPP

#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Message.hpp"

#define RESET   "\033[0m" 
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define BLUE    "\033[34m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"

class Channel;
class Client;
class Server;
class Message;

const static int BUFFER_SIZE = 1024;
static std::string SERVER_NAME = "irc.localhost";
static volatile sig_atomic_t g_running = 1;

// Utils
int		stringToInt(const std::string& str);
bool	isValidMode(char mode, bool isChannelMode);
void 	error(const std::string& errorMsg, bool throws, bool usesErrno);
bool 	isValidNick(const std::string &nick);
void 	signalHandler(int signal);

#endif