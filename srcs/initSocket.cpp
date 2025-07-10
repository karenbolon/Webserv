/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   initSocket.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 13:58:50 by kbolon            #+#    #+#             */
/*   Updated: 2025/07/10 07:11:48 by kbolon           ###   ########.fr       */
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

void handleCgiPipeFd(int fd, short revents, std::vector<pollfd>& fds, std::map<int, ClientConnection*>& clients) {
	if (revents & (POLLERR | POLLHUP)) {
		ClientConnection* client = findClientByFd(clients,fd);
		if (!client) {
			std::cerr << "⚠️ Orphaned CGI pipe error/hangup on fd: " << fd << std::endl;
			removePollFd(fds, fd);
			return;
		
		}
		//only remove CGI pipe if it's marked done and we are not waiting on it anymore
		if (fd == client->getCgiInputFd()) {
			std::cerr << "❌ CGI stdin hangup on fd: " << fd << std::endl;
			close(fd);
			client->setCgiFds(-1, client->getCgiOutputFd());
			removePollFd(fds, fd);
		}
		else if (fd == client->getCgiOutputFd()) {
			std::cerr << "❌ CGI stdout hangup on fd: " << fd <<std::endl;
		}
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
void runEventLoop(std::vector<struct pollfd>& fds,
                  std::map<int, ServerSocket*>& fdToSocket,
                  std::map<int, ClientConnection*>& clients,
                  std::map<int, ServerSocket*>& clientToServer) 
{
	while (g_signal != 0) {
		//the -1 tells the poll to wait indefinately until some fd becomes ready
		//int ready = poll(&fds[0], fds.size(), -1); 
		//500 ms tells the poll to "wake up" every 500 ms and check the status (every 0.5 seconds)
		int ready = poll(&fds[0], fds.size(), 500); //500 ms 
		if (ready < 0) {
			if (errno == EINTR) continue;
			std::cerr << "❌ Poll() error\n";
			break;
		}
		//check for CGI timeouts once per cycle
		for (std::map<int, ClientConnection*>::iterator it = clients.begin(); it != clients.end(); ++it)
			checkCgiTimeout(it->second, fds);

		//check for general client timeouts
		for (std::map<int, ClientConnection*>::iterator it = clients.begin(); it != clients.end(); ++it)
			checkGeneralTimeout(it->second, fds);
			
		for (size_t i = 0; i < fds.size(); ) {
			int fd = fds[i].fd;
			short revents = fds[i].revents;

			if (revents == 0) { 
				++i; 
				continue; 
			}

			if (fdToSocket.count(fd)) {
				handleNewClient(fdToSocket[fd], fds, clients, clientToServer);
				++i; 
				continue;
			}

			ClientConnection* client = findClientByFd(clients, fd);
			if (!client) {
				handleCgiPipeFd(fd, revents, fds, clients);
				++i; 
				continue;
			}
			//Check for CGI script crashes BEFORE timeout check
			if (client->isCgiRunning()) {
				int status;
				pid_t result = waitpid(client->getCgiPid(), &status, WNOHANG);
				if (result > 0) {
					// CGI process has exited
					if (WIFEXITED(status)) {
						int exit_code = WEXITSTATUS(status);
						std::cout << "🔍 CGI process exited with code: " << exit_code << std::endl;

						if (exit_code != 0) {
							// ✅ CGI script crashed - serve configured 500 error page
							std::cout << "💥 CGI script crashed, serving 500 error page" << std::endl;

							// Clean up CGI pipes
							if (client->getCgiOutputFd() != -1) {
								close(client->getCgiOutputFd());
								removePollFd(fds, client->getCgiOutputFd());
							}
							if (client->getCgiInputFd() != -1) {
								close(client->getCgiInputFd());
								removePollFd(fds, client->getCgiInputFd());
							}

							// ✅ Use clientToServer to get config and serve error page
							const ServerConfig& config = clientToServer[client->getFd()]->getConfig();
							std::string errorBody = getErrorPageBody(500, config);
							std::string errorResponse = Response::build(500, errorBody, "text/html");

							client->setPendingResponse(errorResponse);
							client->setCgiFds(-1, -1);
							client->markCgiDone();

							// Mark for sending
							for (size_t j = 0; j < fds.size(); ++j)
								if (fds[j].fd == client->getFd())
									fds[j].events |= POLLOUT;

							++i;
							continue;
						}
						// If exit_code == 0, script finished normally, let it continue processing output
					}
				}
			}

			// === CGI STDOUT ===
			if ((revents & POLLIN) && fd == client->getCgiOutputFd()) {
				char buf[4096];
				ssize_t n = read(fd, buf, sizeof(buf));
				if (n > 0)
					client->appendToCgiOutput(buf, n);
				
				if (n <= 0) {
					if (n == 0) {
						std::cerr << "📭 CGI stdout EOF on fd " << fd << "\n";
					} else {
						std::cerr << "⚠️  CGI stdout is currently unavailable on fd " << fd << "\n";
					}
					//std::cerr << "📭 CGI stdout EOF, closing fd " << fd << "\n";
					removePollFd(fds, fd);
					close(fd);
					client->setCgiFds(client->getCgiInputFd(), -1);
					if (client->getCgiInputFd() == -1)
						client->markCgiDone();

					if (client->isCgiDone()) {
						std::string response = formatCGIResponse(client->getCgiOutputBuffer());
						client->setPendingResponse(response);
						//std::cerr << "📤 CGI response ready on client fd " << client->getFd() << "\n";

						for (size_t j = 0; j < fds.size(); ++j)
							if (fds[j].fd == client->getFd())
								fds[j].events |= POLLOUT;
					}
				}
				--i; 
				continue;
			}

			// === CGI STDIN ===
			if ((revents & POLLOUT) && fd == client->getCgiInputFd()) {
				std::string& body = client->getCgiInputBuffer();
				// FIX: Don't write if CGI process is dead
				if (!client->isCgiRunning()) {
					shutdown(fd, SHUT_WR);
					removePollFd(fds, fd);
					client->setCgiFds(-1, client->getCgiOutputFd());
					if (client->getCgiOutputFd() == -1)
						client->markCgiDone();
					++i; 
					continue;
				}
				if (body.empty()) {
					shutdown(fd, SHUT_WR);
					removePollFd(fds, fd);
					client->setCgiFds(-1, client->getCgiOutputFd());
					if (client->getCgiOutputFd() == -1)
						client->markCgiDone();
					++i; 
					continue;
				}
				ssize_t sent = write(fd, body.c_str(), body.size());
				if (sent <= 0) {
					if (sent == 0)
						std::cerr << "⚠️ CGI stdin write failed on fd " << fd << "\n";
					else
						std::cerr << "⚠️ CGI stdin is currently unavailable on fd " << fd << "\n";
					close(fd);
					removePollFd(fds, fd);
					client->setCgiFds(-1, client->getCgiOutputFd());
					if (client->getCgiOutputFd() == -1)
						client->markCgiDone();
					--i; 
					continue;
				}
				client->consumeCgiInput(sent);
				if (client->cgiInputBufferEmpty()) {
					shutdown(fd, SHUT_WR);
					removePollFd(fds, fd);
					client->setCgiFds(-1, client->getCgiOutputFd());
					if (client->getCgiOutputFd() == -1)
						client->markCgiDone();
				}
				++i; 
				continue;
			}

			// === Client POLLIN ===
			if ((revents & POLLIN) && fd == client->getFd()) {
				int recvResult = client->recvFullRequest(fd, clientToServer[fd]->getConfig(), client, fds);
				if (recvResult <= 0) {
					if (recvResult == 0) {
						std::cerr << "💔 Client disconnected on fd " << fd << "\n";
					} else {
						std::cerr << "📡 No data received or client closed socket on client fd " << fd << "\n";
					}
					handleClientCleanup(fd, fds, clients, i);
					continue;
				}
				if (client->isRequestComplete())
					processClientRequest(fd, fds, clients, clientToServer);
				++i; 
				continue;
			}

			// === Client POLLOUT (static or chunked) ===
			if ((revents & POLLOUT) && fd == client->getFd() && client->wantsToWrite()) {
				std::string& buf = client->getPendingSendBuffer();
				size_t& sent = client->getBytesSentSoFar();
				ssize_t n = send(fd, buf.c_str() + sent, buf.size() - sent, 0);
										
				if (n <= 0) {
					if (n == 0) {
						std::cerr << "💔 Client disconnected on fd " << fd << "\n";
					} else {
						std::cerr << "❌ Send failed on client fd " << fd << "\n";
					}
					//std::cerr << "❌ Send failed on client fd " << fd << "\n";
					client->setChunkState(ERROR);
					client->clearSendState();
					handleClientCleanup(fd, fds, clients, i);
					continue;
				}

				sent += n;
				//client->updateLastActivity(); //reset on write
				if (sent == buf.size()) {
					client->clearSendState();
					if (client->getChunkState() == ERROR) {
						handleClientCleanup(fd, fds, clients, i);
						continue;
					}
					if (client->getChunkState() == IDLE && client->getChunkFilePath().empty()
    					&& client->getCgiPid() == -1 && client->isCgiDone()) {
						// response was from CGI and error is sent — cleanup
						handleClientCleanup(fd, fds, clients, i);
						continue;
					}

					if (client->isChunkedDone()) {
						//std::cerr << "✅ Final chunk sent to client fd " << fd << "\n";
						handleClientCleanup(fd, fds, clients, i);
						continue;
					}
					if (client->getChunkState() == IN_PROGRESS && !client->getChunkFilePath().empty()) {
						if (!sendNextChunk(client)) {
							std::cerr << "❌ Failed to send next chunk\n";
							client->setChunkState(ERROR);
							client->clearSendState();
							handleClientCleanup(fd, fds, clients, i);
							continue;
						}
					}
					if (client->isChunkedError()) {
						std::string response = getErrorPageBody(500, clientToServer[fd]->getConfig());
						client->setPendingResponse(response);
						client->resetChunkedFlags();
						for (size_t j = 0; j < fds.size(); ++j)
							if (fds[j].fd == client->getFd())
								fds[j].events |= POLLOUT;
						++i;
						continue;
					}
				}
				++i;
				continue;
			}
			++i;
		}
	}
}

void checkCgiTimeout(ClientConnection* client, std::vector<struct pollfd>& fds) {
	if (!client->isCgiRunning())
		return;
	
	time_t now = time(NULL);
	
	if (now - client->getCgiStartTime() > 10) {
		std::cerr << "⏰ CGI timeout on fd " << client->getFd() << ", killing 🔪🩸😵\n\n";
		
		kill(client->getCgiPid(), SIGKILL);
		
		//reap the child process to prevent zombies
		int status;
		waitpid(client->getCgiPid(), &status, WNOHANG);

		//close pipe FDs
		if (client->getCgiOutputFd() != -1) {
			close(client->getCgiOutputFd());
			removePollFd(fds, client->getCgiOutputFd());
		}
		if (client->getCgiInputFd() != -1) {
			close(client->getCgiInputFd());
			removePollFd(fds, client->getCgiInputFd());
		}
		
		//set both FDs ot -11 BEFORE markCgiDone
		client->setCgiFds(-1, -1);
		client->setCgiRunning(false);
		client->markCgiDone();
		
		//Send 504 response!
		std::string body = "<html><body><h1>504 Gateway Timeout</h1></body></html>";
		std::string response = Response::build(504, body, "text/html");
		client->setPendingResponse(response);
		
		//Ensure it gets sent
		for (size_t i = 0; i < fds.size(); ++i) {
			if (fds[i].fd == client->getFd()) {
				fds[i].events |= POLLOUT;
				break;
			}
		}
	}
}


void checkGeneralTimeout(ClientConnection* client, std::vector<struct pollfd>& fds) {

	if (!client) {
		return;
	}
	time_t now = time(NULL);
	if (now - client->getActivityTime() > 30) { //30 second timeout
		int fd = client->getFd();
		std::cerr << "⏰ General timeout on fd " << fd << ", cleaning up 🔪🩸😵\n";
		
		//Send 504 response!
		std::string body = "<html><body><h1>504 Gateway Timeout</h1></body></html>";
		std::string response = Response::build(504, body, "text/html");
		client->setPendingResponse(response);
		
		//Ensure it gets sent
		for (size_t i = 0; i < fds.size(); ++i) {
			if (fds[i].fd == client->getFd()) {
				fds[i].events |= POLLOUT;
				break;
			}
		}
		//Mark this client so you can clean them up after the response is sent
		client->setChunkState(ERROR);  // reuse a flag like IDLE/DONE/ERROR
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
                std::map<int, ServerSocket*>& clientToServer) {
        			
    ClientConnection* client = clients[fd];
    std::string raw = client->getRawRequest();

    Request req(raw);
    if (req.getMethod().empty() || req.getPath().empty()) {
        std::string body = "<h1>400 Bad Request</h1>";
        std::string response = Response::build(400, body, "text/html");
        client->setPendingResponse(response);
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
        for (size_t j = 0; j < fds.size(); ++j) {
            if (fds[j].fd == fd) {
                fds[j].events |= POLLOUT;
            }
        }
        return;
    }

    // Check redirect
    if (!location.redirect.empty()) {
        int statusCode = 302;
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
                fds[j].events |= POLLOUT;
            }
        }
        return;
    }

    // Route to correct handler
    if (method == "GET") {
        handleGet(client, fds, req, path, location, config);
    } else if (method == "POST") {
        handlePost(client, fds, req, path, location, config);
    } else if (method == "DELETE") {
        handleDelete(path, location, config, client, fds);
	} else if (method == "HEAD") {
		handleHead(fd, path, location, client, fds);
	} else if (method == "PUT") {
		handlePut(req, path, location, config, client, fds);
	} else {
        std::string body = getErrorPageBody(501, config);
        std::string response = Response::build(501, body, "text/html");
        client->setPendingResponse(response);
        for (size_t j = 0; j < fds.size(); ++j) {
            if (fds[j].fd == fd) {
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
	std::cout << "🤝 A new client has been connected: " << client_fd << std::endl;
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