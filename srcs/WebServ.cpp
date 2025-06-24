/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 17:19:29 by kbolon            #+#    #+#             */
/*   Updated: 2025/06/16 16:57:57 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

/*
Initializes the server using the config file, sets up server sockets and poll monitoring,
and runs the main poll loop to handle client connections and requests.
*/
int	init_webserv(std::string configPath) {
	ConfigParser	parser;

	try {
		parser.parseFile(configPath);
	}
	catch (const std::exception& e) {
		std::cerr << "❌ Error while parsing config: " << e.what() << std::endl;
		return 1;
	}
	parser.print();

	const	std::vector<ServerConfig>& servers = parser.getServers();
	if (servers.empty()) {
		std::cerr << "❌ Error: No servers found in config file.\n";
		return 1;
	}
	//check for Duplicate Host Port Pairs
	checkDuplicateHostPortPairs(servers);

	std::vector<ServerSocket*> serverSockets;
	std::vector<struct pollfd> fds;
	std::map<int, ServerSocket*> fdToSocket;
	if (!initialiseSockets(servers, serverSockets, fds, fdToSocket))
		return 1;

	std::map<int, ClientConnection*> clients;
	std::map<int, ServerSocket*> clientToServer;

	runEventLoop(fds, fdToSocket, clients, clientToServer);

	shutDownWebserv(serverSockets, clients);
	std::cout << "👋 Bye bye!\n";
	return 0;
}

/*
Main entry point: sets up signal handlers, parses CLI arguments for the config file path,
and starts the server using init_webserv().
*/
int main(int ac, char **av) {

	setupSignal();
	std::string configPath;

	if (!parseArguments(ac, av, configPath))
		return EXIT_FAILURE;

	artwelcom();
	std::cout << "Using configuration file: " << configPath << std::endl;

	return (init_webserv(configPath));
}
