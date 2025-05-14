/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 17:26:45 by keramos-          #+#    #+#             */
/*   Updated: 2025/05/14 12:44:14 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/WebServ.hpp"

int g_signal = -1;

void handleSignal(int signal) {
	if (signal == SIGINT) {
		std::cout << "\n🛑 Ctrl+C detected! Shutting down server...\n";

		if (g_signal != -1) {
			std::cout << "🔒 Server socket closed\n";
		}
		g_signal = 0;
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
	std::vector<struct pollfd> fds;
	std::vector<ServerSocket*> serverSockets;
	std::map<int, ServerSocket*> fdToSocket;
	
	for (size_t i = 0; i < servers.size(); ++i) {
		const std::string& host = servers[i].host;
		int port = servers[i].port;

		ServerSocket*	server = new ServerSocket();
		if (!server->init(port, host)) {
			delete server;
			std::cerr << "❌ Failed to initialise server on port: " << port << std::endl;
			continue;
		}
		
		int	fd = server->getFD();
		struct pollfd pfd;
		pfd.fd = fd;
		pfd.events = POLLIN;
		pfd.revents = 0;
		fds.push_back(pfd);
		
		std::cout << "✅ Server is up and running on: " << host << ":" << port << std::endl;
		serverSockets.push_back(server);
		fdToSocket[fd] = server;
	}

	std::map<int, ClientConnection*> clients;

	while (g_signal != 0) {
		int openAndReadyFDs = poll(&fds[0], fds.size(), -1);
		if (openAndReadyFDs == -1) {
			std::cerr << "❌ Failed in poll() FD checks\n";
			break;
		}
		for (size_t i = 0; i < fds.size(); ++i) {
			if (fds[i].revents & POLLIN) {
				//New client?
				if (fdToSocket.count(fds[i].fd)) {
					int client_fd = fdToSocket[fds[i].fd]->acceptClient();
					if (client_fd == -1) {
						std::cerr << "❌ Failed to accept client\n";
						continue;
					}
					ClientConnection* client = new ClientConnection(client_fd);
					struct pollfd client_pfd;
					client_pfd.fd = client_fd;
					client_pfd.events = POLLIN;
					client_pfd.revents = 0;
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
					std::cerr << "Client has disconnected from server 👋\n";
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
	shutDownWebserv(serverSockets, clients);
	std::cout << "👋 Bye bye!\n";
	return 0;
}

int main(int ac, char **av) {

	signal(SIGINT, handleSignal); //handle Contrl + C
	signal(SIGTERM, handleSignal); //handle kill <pid>
	std::string configPath;
	std::cout << "		My Webserv in C++98" << std::endl;
	std::cout << "--------------------------------------------------\n " << std::endl;

	if (ac == 1)
		configPath = "conf/default.conf";
	else if (ac == 2)
		configPath = av[1];
	else{
		std::cout << "Too many arguments!!\nUsage: ./webserv [config_file]" << std::endl;
		return (EXIT_FAILURE);
	}
	std::cout << "Using configuration file: " << configPath << std::endl;

	return (init_webserv(configPath));
}
