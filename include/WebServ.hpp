/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 17:27:43 by keramos-          #+#    #+#             */
/*   Updated: 2025/06/12 17:16:08 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "ServerSocket.hpp"
# include "ClientConnection.hpp"
# include "ConfigParser.hpp"
# include "HttpStatus.hpp"

# include "Request.hpp"
# include "Response.hpp"
# include "LocationConfig.hpp"
# include "ServerConfig.hpp"

# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <cstring>
# include <iostream>
# include <fcntl.h>
# include <vector>
# include <poll.h>
# include <set>
# include <map>
# include <fstream>
# include <sstream>
# include <cstring>
# include <cstdlib>
# include <csignal>
# include <ctime>
# include <errno.h>      // errno
# include <sys/types.h>  // ssize_t
# include <sys/stat.h>   // stat
# include <dirent.h>     // opendir, readdir, closedir
# include <string>
# include <algorithm>    // std::sort
# include <sys/wait.h>   // waitpid
# include <sys/time.h>   // gettimeofday
# include <sys/sendfile.h> // sendfile
# include <arpa/inet.h>  // for inet_pton
# include <netdb.h>      // for gethostbyname and struct hostent
# include <limits.h>     // for PATH_MAX

#define _XOPEN_SOURCE_EXTENDED 1
extern int g_signal;

class ServerSocket;
class ClientConnection;
class Response;


std::string	intToStr(int n);
std::string	trim(std::string& s);
std::string	cleanValue(std::string s);
std::string	getContentType(const std::string& path);
int			safe_socket(int domain, int type, int protocol);
bool		safe_bind(int fd, sockaddr_in & addr);
bool		safe_listen(int socket, int backlog);
void		shutDownWebserv(std::vector<ServerSocket*>& serverSockets, std::map<int, ClientConnection*>& clients);
void 		handleUpload(const std::string &request, int client_fd, const ServerConfig &config);
void 		serveStaticFile(std::string path, int client_fd, const ServerConfig &config);
std::string	getInterpreter(const std::string& path, const ServerConfig& config);
void 		handleCgi(const Request req, int fd, const ServerConfig& config, std::string interpreter);
std::string	getErrorPageBody(int code, const ServerConfig& config);
void 		sendHtmlResponse(int fd, int code, const std::string& body);
std::string	buildHtmlResponse(int code, const std::string& body);
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
LocationConfig	matchLocation(const std::string& path, const ServerConfig& config);
// Add function declarations to WebServ.hpp
void		handleGet(int fd, const std::string& path, const LocationConfig& location, const ServerConfig& config);
void		handlePost(int fd, const Request& req, const std::string& path, const LocationConfig& location, const ServerConfig& config);
void		handlePut(int fd, const Request& req, const std::string& path, const LocationConfig& location, const ServerConfig& config);
void		handleDelete(int fd, const std::string& path, const LocationConfig& location, const ServerConfig& config);
void		handleHead(int fd, const std::string& path, const LocationConfig& location, const ServerConfig& config);

// Helper Functions
void		handleClientCleanup(int fd, std::vector<pollfd>& fds, std::map<int, ClientConnection*>& clients, size_t& i);
bool		fileExists(const std::string& path);
bool		isDirectory(const std::string& path);
void		createDirectoryIfNotExists(const std::string& path);
std::string	getContentType(const std::string& path);
std::string	generateSimpleDirectoryListing(const std::string& dirPath, const std::string& urlPath);
void		handleSimpleUpload(const std::string& request, int client_fd, const ServerConfig& config);
void		handleSimpleCGI(int fd, const Request& req, const std::string& path, const ServerConfig& config);


// URL Rewriting (if you haven't added this yet)
std::string	rewriteURL(const std::string& path, const ServerConfig& config, const std::string& method);

// Helper function prototypes (add these to your header file)
bool		extractFilenameFromRequest(const std::string& request, std::string& filename);
bool		findFileContentBoundaries(const std::string& request, const std::string& filename,
							size_t& contentStart, size_t& contentEnd);
bool		validateUploadFileSize(size_t fileSize, const ServerConfig& config);
bool		writeFileToServer(const std::string& request, size_t contentStart, size_t contentLength,
					const std::string& filePath);
std::string	loadAndProcessSuccessTemplate(const ServerConfig& config, const std::string& filename);
std::string	loadAndProcessDeleteTemplate(const ServerConfig& config, const std::string& filename);
void		replaceTemplateVariables(std::string& templateContent, const std::string& filename, const std::string& action);
std::string	extractBoundary(const std::string& request);

#endif // WEBSERV_HPP