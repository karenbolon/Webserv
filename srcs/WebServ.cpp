/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 17:26:45 by keramos-          #+#    #+#             */
/*   Updated: 2025/05/13 12:00:23 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/WebServ.hpp"

int g_signal = -1;

void handleSignal(int signal) {
	if (signal == SIGINT) {
		std::cout << "\n🛑 Ctrl+C detected! Shutting down server...\n";

		// Close the server socket if it's open
		// for now this is for the socket, but later
		// we will add a destructor to the server class
		// to close all the sockets
		if (g_signal != -1) {
			close(g_signal); // close the server socket!
			std::cout << "🔒 Server socket closed\n";
		}
		g_signal = 0;
		std::cout << "👋 Bye bye!\n";
		exit(0); // exit the program
	}
}

int	init_webserv(std::string configPath) {
	ConfigParser	parser;
	try {
		parser.parseFile(configPath);
	}
	catch (const std::exception& e) {
		std::cerr << "❌ Error while parsing config: " << e.what() << std::endl;
		return 1;
	}
	//	parser.print();

	const	std::vector<ServerConfig>& servers = parser.getServers();
	if (servers.empty()) {
		std::cerr << "❌ Error: No servers found in config file.\n";
		return 1;
	}

	//initialise the first server socket
	int port = servers[0].port;
	std::cout << "Starting server on port: " << port << std::endl;

	ServerSocket	server;
	if (!server.init(port)) {
		std::cerr << "❌ Failed to initialise server\n";
		return 1;
	}
	std::cout << "✅ Server is up and running\n";

	//handles only 1 client (non-blocking)
	//poll setup
	std::vector<struct pollfd> fds;
	std::map<int, ClientConnection*> clients;

	//add the open/listening server to poll list
	struct pollfd pfd;
	pfd.fd = server.getFD();
	pfd.events = POLLIN;
	fds.push_back(pfd);

	while (g_signal != 0) {
		int openAndReadyFDs = poll(&fds[0], fds.size(), -1);
		if (openAndReadyFDs == -1) {
			std::cerr << "❌ Failed in poll() FD checks\n";
			break;
		}
		for (size_t i = 0; i < fds.size(); ++i) {
			if (fds[i].revents & POLLIN) {
				//New client?
				if (fds[i].fd == server.getFD()) {
					int client_fd = server.acceptClient();
					if (client_fd == -1) {
						std::cerr << "❌ Failed to accept client\n";
						return 1;
					}
					ClientConnection*	client = new ClientConnection(client_fd);
					struct pollfd client_pfd = {client_fd, POLLIN, 0};
					fds.push_back(client_pfd);
					clients[client_fd] = client;
					std::cout << "A new client has been connected: " << client_fd << std::endl;
				}
			}
			//existing client
			else {
				int client_fd = fds[i].fd;
				ClientConnection* client = clients[client_fd];
				if (!client->receiveMessage()) {
					std::cerr << "❌ Failed to receive message from client\n";
					close(client_fd);
					delete client;
					clients.erase(client_fd);
					fds.erase(fds.begin() + i);
					--i;
				}
			}
		}
	}
	//cleanup
	for (size_t i = 0; i < fds.size(); i++) {
		close(fds[i].fd);
	}
	for (std::map<int, ClientConnection*>::iterator it = clients.begin(); it != clients.end(); ++it)
		delete it->second;
	return 0;
}

int main(int ac, char **av) {

	setupSignal();
	std::string configPath;

	if (!parseArguments(ac, av, configPath))
		return EXIT_FAILURE;

	artwelcom();
	std::cout << "Using configuration file: " << configPath << std::endl;
	return (init_webserv(configPath));
}
