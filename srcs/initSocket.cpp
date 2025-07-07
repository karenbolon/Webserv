/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   initSocket.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 13:58:50 by kbolon            #+#    #+#             */
/*   Updated: 2025/07/07 17:16:57 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

/*
This function takes the groups of Servers, Sockets etc
and tries to initilise them
*/
bool initialiseSockets(const std::vector<ServerConfig>& servers, std::vector<ServerSocket*>& serverSockets,
			std::vector<struct pollfd>& fds, std::map<int, ServerSocket*>& fdToSocket) {
	// Creates a ServerSocket, binds/listens on specified host/port, then configures the server.
	// Adds the server FD to the poll list to monitor for POLLIN events,
	// and tracks the ServerSocket for later access and cleanup.
	for (size_t i = 0; i < servers.size(); ++i) {
		for (size_t j = 0; j < servers[i].ports.size(); ++j) {
			ServerSocket*	server = new ServerSocket();
			if (!server->init(servers[i].ports[j], servers[i].host)) {
				delete server;
				std::cerr << "❌ Failed to initialise server on port: " << servers[i].ports[j] << std::endl;
				continue;
			}
			server->setConfig(servers[i]);
			int	fd = server->getFd();
			struct pollfd pfd;
			pfd.fd = fd;
			pfd.events = POLLIN;
			pfd.revents = 0;
			fds.push_back(pfd);
			serverSockets.push_back(server);
			fdToSocket[fd] = server;
			std::cout << "✅ Server is up at http://" << servers[i].host << ":" << servers[i].ports[j] << std::endl;
		}
	}
	if (serverSockets.empty())
		return false;
	return true;
}

ClientConnection* findClientByFd(std::map<int, ClientConnection*>& clients, int fd) {
	for (std::map<int, ClientConnection*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->getFd() == fd ||
			it->second->getCgiInputFd() == fd ||
			it->second->getCgiOutputFd() == fd)
			return it->second;
	}
	return NULL;
}

/*
this is the main I/O loop with poll()
*/
void	runEventLoop(	std::vector<struct pollfd>& fds,
						std::map<int, ServerSocket*>& fdToSocket,
						std::map<int, ClientConnection*>& clients,
						std::map<int, ServerSocket*>& clientToServer) {

	while (g_signal != 0) {
		//safe to call poll()
		// revents will be automatically set by poll(), no need to reset manually
		int ready = poll(&fds[0], fds.size(), -1);
		if (ready < 0) {
			if (errno == EINTR)
				continue;
			std::cerr << "❌ Poll() error" << std::endl;
			break;
		}
		//handle ready FD's (if there is data to read)
		for (size_t i = 0; i < fds.size(); ++i) {
			//revents field is declared as a short
			short tempRevent = fds[i].revents;
			int fd = fds[i].fd;

			if (tempRevent & (POLLERR | POLLHUP | POLLNVAL)) {
				std::cerr << "❌ Error or hangup on client side: " << fd << std::endl;
				handleClientCleanup(fd, fds, clients, i);
				continue;
			}
			if (fdToSocket.count(fd)) {
				//server socket is ready to accept new client
				handleNewClient(fdToSocket[fd], fds, clients, clientToServer);
				continue;
			}
			
			ClientConnection* client = findClientByFd(clients,fd);
			if (!client)
				continue;
			
			//handle CGI stdout (ready to read)
			if (tempRevent & POLLIN && fd == client->getCgiOutputFd()) {
				char buf[4096];
				ssize_t n = read(fd, buf, sizeof(buf));
				if (n > 0)
					client->appendToCgiOutput(buf, n);
				else if (n == 0) {
					close(fd);
					//mark output closed
					client->setCgiFds(client->getCgiInputFd(), -1);
				}
			}
			//handle CGU stdin (ready to write)	
			if (tempRevent & POLLOUT && fd == client->getCgiInputFd()) {
				std::string& body = client->getCgiInputBuffer();
				ssize_t sent = write(fd, body.c_str(), body.size());
				if (sent == 0)
					continue;
				if (sent == -1) {
					handleClientCleanup(fd, fds, clients, i);
					continue;
				}
				client->consumeCgiInput(sent);
				if (client->cgiInputBufferEmpty()) {
					close(fd);
					//mark input closed
					client->setCgiFds(-1, client->getCgiOutputFd());
				}

				//handle CGI timeout and process end
				if (client->isCgiRunning()) {
					int status;
					pid_t	result = waitpid(client->getCgiPid(), &status, WNOHANG);
					if (result == client->getCgiPid())
						client->markCgiDone();
					if (time(NULL) - client->getCgiStartTime() > 5) {
						std::cerr << "⏰ CGI timeout on fd " << fd << ", killing 🔪🩸😵\n\n";
						kill(client->getCgiPid(), SIGKILL);
						client->markCgiDone();
					}
				}
				//CGI is finished, we need to send the output
				if (client->isCgiDone() && client->getCgiOutputFd() == -1) {
					std::string response = formatCGIResponse(client->getCgiOutputBuffer());
					safeSend(client->getFd(), response);
					handleClientCleanup(fd, fds, clients, i);
					continue;
				}
				//back to dealing with normal sockets (non-CGI)
				if (tempRevent & POLLIN && fd == client->getFd()) {
					const	ServerConfig& serverConfig = clientToServer[fd]->getConfig();
				
					int recvResult = client->recvFullRequest(fd, serverConfig);
					if (recvResult <= 0) {
						handleClientCleanup(fd, fds, clients, i);
						continue;
					}
					if (client->isRequestComplete()) {
						processClientRequest(fd, fds, clients, clientToServer, i);
					}
				}
			}
		}
	}
}

bool	methodAllowed(const std::string& method, const std::vector<std::string>& allowed) {
	for (size_t i = 0; i < allowed.size(); ++i) {
		if (allowed[i] == method)
			return true;
	}
	return false;
}

void processClientRequest(int fd,
	std::vector<struct pollfd>& fds,
	std::map<int, ClientConnection*>& clients,
	std::map<int, ServerSocket*>& clientToServer, size_t& i)
{
	ClientConnection* client = clients[fd];
	std::string raw = client->getRawRequest();

	Request req(raw);
	std::string method = req.getMethod();
	std::string path = req.getPath();

	const ServerConfig& config = clientToServer[fd]->getConfig();
	LocationConfig location = matchLocation(path, config);

	// Check if method is allowed
	if (!methodAllowed(method, location.methods)) {
		std::string body = getErrorPageBody(405, config);
		sendHtmlResponse(fd, 405, body); // ⚠️ make sure this checks POLLOUT
		handleClientCleanup(fd, fds, clients, i);
		return;
	}

	// Check redirect
	if (!location.redirect.empty()) {
		//std::cout << "🔄 Found redirect for " << path << " -> " << location.redirect << std::endl;

		int statusCode = 302; // Default temporary redirect

		if (location.returnStatusCode >= 300 && location.returnStatusCode < 400) {
			statusCode = location.returnStatusCode;
			std::cout << "   Using specified status code: " << statusCode << std::endl;
		} else {
			std::cout << "   Using default status code: " << statusCode << " (temporary redirect)" << std::endl;
		}

		std::ostringstream response;
		response << "HTTP/1.1 " << statusCode << " " << HttpStatus::getStatusMessages(statusCode) << "\r\n";
		response << "Location: " << location.redirect << "\r\n";
		response << "Connection: close\r\n";
		response << "\r\n";

		std::string responseStr = response.str();
		std::cout << "Sending redirect response:\n" << responseStr << std::endl;

		ssize_t sent = send(fd, responseStr.c_str(), responseStr.size(), 0);  // Or whatever your send function was
		if (sent == 0)
			return;
		if (sent == -1) {
			std::cerr << "❌ Failed to send redirect response on fd " << fd << std::endl;
			handleClientCleanup(fd, fds, clients, i);
			return;
		}
}

	// Route to correct handler
	if (method == "GET") {
		handleGet(client, fd, fds, req, path, location, config);
	} else if (method == "POST") {
		// This may become setupCGI() call
		handlePost(client, fd, fds, req, path, location, config);
	} else if (method == "DELETE") {
		handleDelete(fd, path, location, config);
	} else {
		std::string body = getErrorPageBody(501, config);
		sendHtmlResponse(fd, 501, body);
		handleClientCleanup(fd, fds, clients, i);
	}
}

/*
Accepts a new client connection, creates a new ClientConnection object to manage communication,
sets up a pollfd struct for the client, and adds it to the poll list.
*/
void	handleNewClient(ServerSocket* server, std::vector<pollfd> &fds, 
			std::map<int, ClientConnection*>& clients, std::map<int, ServerSocket*>& clientToServer) {

	int	client_fd = server->acceptClient();
	if (client_fd == -1) {
		std::cerr << "❌ Failed to accept client\n";
		return;
	}
	
	ClientConnection* client = new ClientConnection(client_fd);
	pollfd client_pfd;
	client_pfd.fd = client_fd;
	client_pfd.events = POLLIN;
	client_pfd.revents = 0;
	fds.push_back(client_pfd);
	clients[client_fd] = client;
	clientToServer[client_fd] = server;
	std::cout << "A new client has been connected: " << client_fd << std::endl;
}

/*
This function finds the corresponding file descriptor in the ClientConnection map,
parses and handles the HTTP request using the appropriate handler (static, CGI, or upload),
and then cleans up the client connection.
*/
void handleExistingClient(int fd, std::vector<pollfd> &fds,
	std::map<int, ClientConnection*>& clients, size_t& i,
	const ServerConfig& config)
{

	/*std::map<int, ClientConnection*>::iterator it = clients.find(fd);
	if (it == clients.end()) {
		std::cerr << "❌ Unknown client fd: " << fd << std::endl;
		return;
	}*/

	ClientConnection* client = clients[fd];

	try {
		// Read data from client
		int bytes = client->recvFullRequest(fd, config);
		if (bytes <= 0) {
			handleClientCleanup(fd, fds, clients, i);
			return;
		}

		// Check if request is complete
		if (!client->isRequestComplete()) {
			client->markRequestReady();
			//return; // Wait for more data
		}

	}
	catch (const std::exception& e) {
		std::cerr << "❌ Exception in handle existing client: " << e.what() << std::endl;
		handleClientCleanup(fd, fds, clients, i);
	}
}