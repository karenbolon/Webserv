/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   initSocket.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 13:58:50 by kbolon            #+#    #+#             */
/*   Updated: 2025/07/08 02:37:30 by kbolon           ###   ########.fr       */
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

void handleCgiPipeFd(int fd, short revents, std::vector<pollfd>& fds) {
	if (revents & (POLLERR | POLLHUP)) {
		std::cerr << "❌ CGI pipe error/hangup on fd: " << fd << std::endl;
		removePollFd(fds, fd);
	}
}

/*
this is the main I/O loop with poll()
-fd is the server socket, handleNewClient accepts new connection and creates a ClientConnection and adds it to maps
and pollfd vector.
-existing clients, we find using findClientByFd function, get the ClientConnection object, if it is CGI pipe, handle CGI i/o
-if it is a regular client socket, POLLIN is set and then we receive the request and if complete, we process it
-if POLLOUT is set and client wants to write, we send the buffered data
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
			
			if (tempRevent == 0)
				continue;
			
			//CGI PIPE FD (not in clients map)
			if (clients.find(fd) == clients.end()) {
				//could be CGI fd, not client, we need to check
				handleCgiPipeFd(fd, tempRevent, fds);
				continue;
			}

			if (tempRevent & (POLLERR | POLLHUP | POLLNVAL)) {
				std::cerr << "❌ Error or hangup on client side: " << fd << std::endl;
				handleClientCleanup(fd, fds, clients, i);
				continue;
			}
			//accept new client if this is a server socket
			if (fdToSocket.count(fd)) {
				//server socket is ready to accept new client
				handleNewClient(fdToSocket[fd], fds, clients, clientToServer);
				++i;
				continue;
			}
			
			ClientConnection* client = findClientByFd(clients,fd);
			if (!client) {
				++i;
				continue;
			}
			
			//handle CGI stdout (ready to read)
			if ((tempRevent & POLLIN) && fd == client->getCgiOutputFd()) {
				char buf[4096];
				ssize_t n = read(fd, buf, sizeof(buf));
				if (n > 0)
					client->appendToCgiOutput(buf, n);
				else if (n == 0) {
					close(fd);
					std::cerr << "in initSocket function close\n";
					//mark output closed
					client->setCgiFds(client->getCgiInputFd(), -1);
				}
				++i;
				continue;
			}
			
			//handle CGU stdin (ready to write)	
			if ((tempRevent & POLLOUT) && fd == client->getCgiInputFd()) {
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
					std::cerr << "in initSocket function close 2\n";
					//mark input closed
					client->setCgiFds(-1, client->getCgiOutputFd());
				}
				++i;
				continue;
			}
			time_t now = time(NULL);
			if (client->isCgiRunning() && now - client->getCgiStartTime() > 5) {
				std::cerr << "⏰ CGI timeout on fd " << fd << ", killing 🔪🩸😵\n\n";
				kill(client->getCgiPid(), SIGKILL);
				//reap the child process to prevent zombies
				int status;
				waitpid(client->getCgiPid(), &status, WNOHANG);
				client->markCgiDone();
			}

			//CGI is finished, we need to send the output
			if (client->isCgiDone() && client->getCgiOutputFd() == -1) {
				//int status;
				//waitpid(client->getCgiPid(), &status, WNOHANG);
				std::string response = formatCGIResponse(client->getCgiOutputBuffer());
				client->setPendingResponse(response);
				int clientSocket = client->getFd();
				for (size_t j = 0; j < fds.size(); ++j) {
					if (fds[j].fd == clientSocket)
						fds[j].events |= POLLOUT;
				}

			}
			//back to dealing with normal sockets (non-CGI)
			if (tempRevent & POLLIN && fd == client->getFd()) {
				const	ServerConfig& serverConfig = clientToServer[fd]->getConfig();
				
				int recvResult = client->recvFullRequest(fd, serverConfig, client, fds);
				if (recvResult <= 0) {
					handleClientCleanup(fd, fds, clients, i);
					continue;
				}
				if (client->isRequestComplete()) {
					processClientRequest(fd, fds, clients, clientToServer);
				}
				++i;
				continue;
			}
			if (tempRevent & POLLOUT && fd == client->getFd() && client->wantsToWrite()) {
				std::string& buf = client->getPendingSendBuffer();
				size_t& sent = client->getBytesSentSoFar();
				ssize_t n = send(fd, buf.c_str() + sent, buf.size() - sent, 0);
				if (n == -1) {
					handleClientCleanup(fd, fds, clients, i);
					continue;
				}
				if (n == 0)
					continue;
				sent += n;
				if (sent == buf.size()) {
					client->clearSendState();
					handleClientCleanup(fd, fds, clients, i);
				}
				++i;
				continue;
			}
			++i;
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
	std::map<int, ServerSocket*>& clientToServer)
{
	ClientConnection* client = clients[fd];
	std::string raw = client->getRawRequest();

	Request req(raw);
	if (req.getMethod().empty() || req.getPath().empty()) {
    	// Malformed request, send 400 Bad Request
    	std::string body = "<h1>400 Bad Request</h1>";
    	std::string response = Response::build(400, body, "text/html");
    	client->setPendingResponse(response);
    	// Set POLLOUT for this fd so the response is sent
    	for (size_t j = 0; j < fds.size(); ++j) {
      	  if (fds[j].fd == client->getFd())
      	      fds[j].events |= POLLOUT;
		}
   		return;
	}
	std::string method = req.getMethod();
	std::string path = req.getPath();

	const ServerConfig& config = clientToServer[fd]->getConfig();
	LocationConfig location = matchLocation(path, config);

	// Check if method is allowed
	if (!methodAllowed(method, location.methods)) {
		std::string body = getErrorPageBody(405, config);
		std::string response = Response::build(405, body, "text/html");
		client->setPendingResponse(response);
		//set POLLOUT for this fd
		for (size_t j = 0; j < fds.size(); ++j) {
			if (fds[j].fd == fd) {
				//this sets the POLLOUT flag which tells poll() that this FD is ready to write
				fds[j].events |= POLLOUT;
			}
		}
		return;
	}

	// Check redirect
	if (!location.redirect.empty()) {
		//std::cout << "🔄 Found redirect for " << path << " -> " << location.redirect << std::endl;

		int statusCode = 302; // Default temporary redirect

		if (location.returnStatusCode >= 300 && location.returnStatusCode < 400)
			statusCode = location.returnStatusCode;

		std::ostringstream response;
		response << "HTTP/1.1 " << statusCode << " " << HttpStatus::getStatusMessages(statusCode) << "\r\n";
		response << "Location: " << location.redirect << "\r\n";
		response << "Connection: close\r\n";
		response << "\r\n";

		client->setPendingResponse(response.str());
		for (size_t j = 0; j < fds.size(); ++j) {
			if (fds[j].fd == fd) {
				//this sets the POLLOUT flag which tells poll() that this FD is ready to write
				fds[j].events |= POLLOUT;
			}
		}
	}

	// Route to correct handler
	if (method == "GET") {
		handleGet(client, fds, req, path, location, config);
	} else if (method == "POST") {
		// This may become setupCGI() call
		handlePost(client, fds, req, path, location, config);
	} else if (method == "DELETE") {
		handleDelete(path, location, config, client, fds);
	} else {
		std::string body = getErrorPageBody(501, config);
		std::string response = Response::build(501, body, "text/html");
		client->setPendingResponse(response);
		for (size_t j = 0; j < fds.size(); ++j) {
			if (fds[j].fd == fd) {
				//this sets the POLLOUT flag which tells poll() that this FD is ready to write
				fds[j].events |= POLLOUT;
			}
		}
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
	const ServerConfig& config,  std::map<int, ServerSocket*>& clientToServer)
{

	ClientConnection* client = clients[fd];

	try {
		// Read data from client
		int bytes = client->recvFullRequest(fd, config, client, fds);
		if (bytes <= 0) {
			handleClientCleanup(fd, fds, clients, i);
			return;
		}

		// Check if request is complete
		if (!client->isRequestComplete()) {
			return; // Wait for more data
		}
		processClientRequest(fd, fds, clients, clientToServer);
	}
	catch (const std::exception& e) {
		std::cerr << "❌ Exception in handle existing client: " << e.what() << std::endl;
		handleClientCleanup(fd, fds, clients, i);
	}
}