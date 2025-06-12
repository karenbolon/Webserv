/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   initSocket.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 13:58:50 by kbolon            #+#    #+#             */
/*   Updated: 2025/06/12 04:04:48 by kellen           ###   ########.fr       */
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
			int	fd = server->getFD();
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
			std::cerr << "❌ Poll() error: " << strerror(errno) << std::endl;
			break;
		}
		//handle ready FD's (if there is data to read)
		for (size_t i = 0; i < fds.size(); ++i) {
			//revents field is declared as a short
			short tempRevent = fds[i].revents;
			int fd = fds[i].fd;

			if (tempRevent & (POLLERR | POLLHUP | POLLNVAL)) {
				std::cerr << "❌ Error or hangup on client side\n" << fd << std::endl;
				close(fd);
				fds.erase(fds.begin() + i);
				--i;
				continue;
			}
			if (tempRevent & POLLIN) {
				if (fdToSocket.count(fd))
					handleNewClient(fdToSocket[fd], fds, clients, clientToServer);
				else if (clients.count(fd))
					handleExistingClient(fd, fds, clients, i, clientToServer[fd]->getConfig());
				else {
					std::cerr << "⚠️ POLLIN on unknown fd " << fd << std::endl;
					close(fd);
					fds.erase(fds.begin() + i);
					--i;
				}
			}
		}
	}
}

/*
Accepts a new client connection, creates a new ClientConnection object to manage communication,
sets up a pollfd struct for the client, and adds it to the poll list.
*/
void	handleNewClient(ServerSocket* server, std::vector<pollfd> &fds, std::map<int, ClientConnection*>& clients, std::map<int, ServerSocket*>& clientToServer) {
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

	std::map<int, ClientConnection*>::iterator it = clients.find(fd);
	if (it == clients.end()) {
		std::cerr << "❌ Unknown client fd: " << fd << std::endl;
		return;
	}

	ClientConnection* client = it->second;

	try {
		// Read data from client
		client->recvFullRequest(fd, config);

		// Check if request is complete
		if (!client->isRequestComplete()) {
			return; // Wait for more data
		}

		std::string request = client->getRawRequest();
		if (request.empty()) {
			std::cout << "⚠️ Empty request received" << std::endl;
			handleClientCleanup(fd, fds, clients, i);
			return;
		}

		// Parse request
		Request req(request);
		std::string method = req.getMethod();
		std::string path = req.getPath();

		std::cout << "📨 " << method << " " << path << std::endl;

		// URL rewriting for clean URLs - BUT NOT FOR POST UPLOADS
		std::string actualPath = path;
		if (method == "GET") {
			actualPath = rewriteURL(path, config, method);
			if (actualPath != path) {
				std::cout << "🔄 URL rewrite: " << path << " → " << actualPath << std::endl;
				path = actualPath;
			}
		} else {
			// For POST, PUT, DELETE - keep original path
			std::cout << "📌 Keeping original path for " << method << ": " << path << std::endl;
		}
		// std::string actualPath = rewriteURL(path, config);
		// if (actualPath != path) {
		// 	std::cout << "🔄 URL rewrite: " << path << " → " << actualPath << std::endl;
		// 	path = actualPath;
		// }

		// Find matching location
		LocationConfig location = matchLocation(path, config);

		// Check if method is allowed in this location
		bool methodAllowed = false;
		for (size_t j = 0; j < location.methods.size(); ++j) {
			if (location.methods[j] == method) {
				methodAllowed = true;
				break;
			}
		}

		if (!methodAllowed) {
			std::cout << "❌ Method " << method << " not allowed for " << path << std::endl;
			std::string body = getErrorPageBody(405, config); // Method Not Allowed
			sendHtmlResponse(fd, 405, body);
			handleClientCleanup(fd, fds, clients, i);
			return;
		}

		// Handle different HTTP methods with CORRECT parameter order
		if (method == "GET") {
			// handleGET(fd, path, location, config)
			handleGet(fd, path, location, config);
		} else if (method == "POST") {
			// handlePOST(fd, req, path, location, config)
			handlePost(fd, req, path, location, config);
		} else if (method == "PUT") {
			// handlePUT(fd, req, path, location, config)
			handlePut(fd, req, path, location, config);
		} else if (method == "DELETE") {
			// handleDELETE(fd, path, location, config)
			handleDelete(fd, path, location, config);
		} else if (method == "HEAD") {
			// handleHEAD(fd, path, location, config)
			handleHead(fd, path, location, config);
		} else {
			std::cout << "❌ Method " << method << " not implemented" << std::endl;
			std::string body = getErrorPageBody(501, config); // Not Implemented
			sendHtmlResponse(fd, 501, body);
		}

	} catch (const std::exception& e) {
		std::cerr << "❌ Exception handling client " << fd << ": " << e.what() << std::endl;
		std::string errorBody = getErrorPageBody(500, config);
		sendHtmlResponse(fd, 500, errorBody);
	}

	// Clean up client connection
	handleClientCleanup(fd, fds, clients, i);
}