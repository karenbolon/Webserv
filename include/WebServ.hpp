/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 17:27:43 by keramos-          #+#    #+#             */
/*   Updated: 2025/05/09 13:52:56 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <cstring>
# include <iostream>
# include <fcntl.h>
# include <vector>
# include <poll.h>
# include <map>
# include <fstream>
# include <sstream>
# include <string>
# include <cstdlib>
# include <csignal>
# include "ClientConnection.hpp"
# include "ConfigParser.hpp"
# include "LocationConfig.hpp"
# include "ServerConfig.hpp"
# include "ServerSocket.hpp"

//# define PORT 8081
# define BUFFER_SIZE 4096
#define _XOPEN_SOURCE_EXTENDED 1

std::string	trim(std::string& s);
std::string getContentType(const std::string& path);
int			safe_socket(int domain, int type, int protocol);
bool		safe_bind(int fd, sockaddr_in & addr);
bool		safe_listen(int socket, int backlog);
//bool		parseKeyValue(const std::string& line, std::string& key, std::string& value);
//void		parseBlock(std::ifstream& file, const std::string& type, ServerConfig& server);
