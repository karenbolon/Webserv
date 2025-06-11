/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: keramos- <keramos-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 17:27:43 by keramos-          #+#    #+#             */
/*   Updated: 2025/06/11 15:33:55 by keramos-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <fcntl.h>
#include <vector>
#include <poll.h>
#include <map>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <csignal>
#include "ClientConnection.hpp"
#include "ServerSocket.hpp"
#include "ServerConfig.hpp"
#include "ConfigParser.hpp"
#include "Request.hpp"
#include "Response.hpp"

#define _XOPEN_SOURCE_EXTENDED 1
extern int g_signal;

class ServerSocket;
class ClientConnection;

std::string	intToStr(int n);
std::string	trim(std::string& s);
std::string	cleanValue(std::string s);
std::string getContentType(const std::string& path);
int			safe_socket(int domain, int type, int protocol);
bool		safe_bind(int fd, sockaddr_in & addr);
bool		safe_listen(int socket, int backlog);
void		shutDownWebserv(std::vector<ServerSocket*>& serverSockets, std::map<int, ClientConnection*>& clients);
void 		handleUpload(const std::string &request, int client_fd, const ServerConfig &config);
void 		serveStaticFile(std::string path, int client_fd, const ServerConfig &config);
std::string getInterpreter(const std::string& path, const ServerConfig& config);
void 		handleCgi(const Request req, int fd, const ServerConfig& config, std::string interpreter);
std::string	getErrorPageBody(int code, const ServerConfig& config);
void 		sendHtmlResponse(int fd, int code, const std::string& body);
std::string buildHtmlResponse(int code, const std::string& body);
bool		validatePort(const std::string& portString);
void		convertListenEntriesToPortsAndHost(ServerConfig& server);
void 		checkDuplicateHostPortPairs(const std::vector<ServerConfig>& servers);
void		runEventLoop(std::vector<struct pollfd>& fds, std::map<int, ServerSocket*>& fdToSocket,
				std::map<int, ClientConnection*>& clients, std::map<int, ServerSocket*>& clientToServer);
bool 		initialiseSockets(const std::vector<ServerConfig>& servers, std::vector<ServerSocket*>& serverSockets,
				std::vector<struct pollfd>& fds, std::map<int, ServerSocket*>& fdToSocket);
void		handleExistingClient(int fd, std::vector<pollfd> &fds, std::map<int, ClientConnection*>& clients, size_t& i,
				const ServerConfig& config);
void		handleNewClient(ServerSocket* server, std::vector<pollfd> &fds, std::map<int, ClientConnection*>& clients,
				std::map<int, ServerSocket*>& clientToServer);
LocationConfig matchLocation(const std::string& path, const ServerConfig& config);