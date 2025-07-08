/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 17:27:43 by keramos-          #+#    #+#             */
/*   Updated: 2025/07/08 00:10:28 by kbolon           ###   ########.fr       */
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
bool 		sendAll(int fd, const char* buffer, size_t length);
void		shutDownWebserv(std::vector<ServerSocket*>& serverSockets, std::map<int, ClientConnection*>& clients);
void 		handleUpload(const std::string &request, int client_fd, const ServerConfig &config);
void 		serveStaticFile(std::string path, const ServerConfig &config, ClientConnection* client, std::vector<struct pollfd>& fds);
std::string	getInterpreter(const std::string& path, const ServerConfig& config);
void 		handleCgi(const Request req, int fd, const ServerConfig& config, std::string interpreter);
std::string	getErrorPageBody(int code, const ServerConfig& config);
void 		sendHtmlResponse(int code, const std::string& body, ClientConnection* client, std::vector<struct pollfd>& fds);
std::string	buildHtmlResponse(int code, const std::string& body);
bool		validatePort(const std::string& portString);
void		convertListenEntriesToPortsAndHost(ServerConfig& server);
void 		checkDuplicateHostPortPairs(const std::vector<ServerConfig>& servers);
void		runEventLoop(std::vector<struct pollfd>& fds, std::map<int, ServerSocket*>& fdToSocket, std::map<int, ClientConnection*>& clients,
				std::map<int, ServerSocket*>& clientToServer);
bool 		initialiseSockets(const std::vector<ServerConfig>& servers, std::vector<ServerSocket*>& serverSockets,
				std::vector<struct pollfd>& fds, std::map<int, ServerSocket*>& fdToSocket);
void		handleExistingClient(int fd, std::vector<pollfd> &fds, std::map<int, ClientConnection*>& clients, size_t& i,
				const ServerConfig& config);
void		handleNewClient(ServerSocket* server, std::vector<pollfd> &fds, std::map<int, ClientConnection*>& clients,
				std::map<int, ServerSocket*>& clientToServer);
LocationConfig	matchLocation(const std::string& path, const ServerConfig& config);
// Add function declarations to WebServ.hpp
bool 		handleGet(ClientConnection* client, std::vector<struct pollfd>& fds, const Request& req, const std::string& path, const LocationConfig& location, const ServerConfig& config);
bool		handlePost(ClientConnection* client, std::vector<struct pollfd>& fds, const Request& req, const std::string& path, const LocationConfig& location, const ServerConfig& config);
void		handlePut(const Request& req, const std::string& path, const LocationConfig& location, const ServerConfig& config,
				ClientConnection* client, std::vector<struct pollfd>& fds);
void 		handleDelete(const std::string& path, const LocationConfig& location, const ServerConfig& config,
				ClientConnection* client, std::vector<struct pollfd>& fds);
bool		handleHead(int fd, const std::string& path, const LocationConfig& location, const ServerConfig& config, ClientConnection* client, std::vector<struct pollfd>& fds);
bool		handleRedirect(int fd, const LocationConfig& location, const ServerConfig& config);

// Helper Functions
void 		handleClientCleanup(int fd, std::vector<pollfd>& fds,
				std::map<int, ClientConnection*>& clients, size_t& i);
bool		fileExists(const std::string& path);
bool		isDirectory(const std::string& path);
void		createDirectoryIfNotExists(const std::string& path);
std::string	getContentType(const std::string& path);
std::string	generateSimpleDirectoryListing(const std::string& dirPath, const std::string& urlPath);
void		handleSimpleUpload(const std::string& request, const ServerConfig& config, ClientConnection* client, std::vector<struct pollfd>& fds);
bool 		handleSimpleCGI(ClientConnection* client, std::vector<struct pollfd>& fds, const Request& req, const std::string& path, const ServerConfig& config);

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
std::string	generateJsonDirectoryListing(const std::string& dirPath);

void		handleSignal(int signal);
void		setupSignal();
void		showUsage();
bool		fileExists(const std::string& path);
bool		hasAllowedExtension(const std::string& filename);
bool		parseArguments(int argc, char **argv, std::string &configPath);
void		artwelcom();
std::string	formatCGIResponse(const std::string& scriptOutput);

// Chunked transfer functions (no class needed!)
bool		useChunkedTransfer(const std::string& fullPath);
bool		sendFileChunked(int fd, const std::string& fullPath, const std::string& contentType);

// Enhanced PUT handling functions
void		handleFileRename(const std::string& path, const std::string& newName,
				const LocationConfig& location, const ServerConfig& config,
				ClientConnection* client, std::vector<struct pollfd>& fds);
void 		handleFileUpload(const Request& req, const std::string& path,
				const LocationConfig& location, const ServerConfig& config,
				ClientConnection* client, std::vector<struct pollfd>& fds);

// Enhanced multiple file upload handler
size_t		findNextFileSection(const std::string& request, const std::string& boundary, size_t startPos);
void		handleEnhancedUpload(const std::string& request, int client_fd, const ServerConfig& config);
bool		extractFilenameFromSection(const std::string& request, size_t sectionStart,
				size_t sectionEnd, std::string& filename);
bool		findFileContentInSection(const std::string& request, size_t sectionStart,
				size_t sectionEnd, size_t& contentStart, size_t& contentLength);
std::string	generateSimpleUploadResponse(const std::vector<std::string>& successFiles,
				const std::vector<std::string>& failedFiles);

void	 	handleExistingClient(int fd, std::vector<pollfd> &fds,
				std::map<int, ClientConnection*>& clients, size_t& i,
				const ServerConfig& config,  std::map<int, ServerSocket*>& clientToServer);
bool		methodAllowed(const std::string& method, const std::vector<std::string>& allowed);
void		removePollFd(std::vector<pollfd>& fds, int fd);
bool		setUpCgi(ClientConnection* client, std::vector<struct pollfd>& fds, const std::string& interpreter, const std::string& scriptPath,
				const Request& req);
void 		processClientRequest(int fd, std::vector<struct pollfd>& fds, std::map<int, ClientConnection*>& clients,
				std::map<int, ServerSocket*>& clientToServer);
				
#endif // WEBSERV_HPP