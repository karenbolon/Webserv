/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   inputValidation.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 11:14:38 by kbolon            #+#    #+#             */
/*   Updated: 2025/06/12 00:37:34 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

/*
This function validates the raw port string, checks if it is
a digit, converts to an int and also checks if it is a valid port.

Port ranges:
0-1023: Forbidded/reserved ports (root required)
1024-65535: Valid
8000-9000: Valid but could conflict with other dev tools
*/
bool validatePort(const std::string& portString) {
	if (portString.empty())
		return false;
	for (size_t i = 0; i < portString.length(); ++i) {
		if (!isdigit(portString[i]))
			return false;
	}
	int port = std::atoi(portString.c_str());
	if (port >= 0 && port < 1024) {
		std::cerr << "Ports 0 - 1023 are reserved for the system services, please fix\n";
		return false;
	}
	if (port >= 8000 && port <= 9000) {
		std::cerr << "⚠️ Port " << port << " is commonly used.  Might conflict.\n";
	}
	return (port > 0 && port < 65536);
}

/*
Parses each 'listen' entry from the server block and extracts host and port values.
If the entry is in the form 'host:port', it sets the server's host and port accordingly.
If no host is specified, it assumes the entry is just a port.
Validates each port, and if valid, adds it to the server's ports vector.
*/
void	convertListenEntriesToPortsAndHost(ServerConfig& server) {
	for (size_t i = 0; i < server.listen_entries.size(); ++i) {
		const std::string& entry = server.listen_entries[i];
		size_t colon = entry.find(':');
		std::string portString;
		if (colon == std::string::npos) //no colon found
			portString = entry;
		else
		{
			server.host = entry.substr(0, colon);
			portString = entry.substr(colon + 1);
		}
		if (!validatePort(portString)) {
			std::cerr << "⚠️ Invalid port in listen directive: " << entry << std::endl;
			continue;
		}
		server.ports.push_back(std::atoi(portString.c_str()));
	}
}

/*
We must check that there are no duplicate host:port pairs as this will
cause a bind() error.

Valid (different server blocks):
listen 127.0.0.1:8080 and listen 127.0.0.1:8081,
listen 127.0.0.1:8081 and listen 192.168.1.1:8081

Not valid (different server blocks):
listen 127.0.0.1:8080 and listen 127.0.0.1:8080

This function parses through all of the servers listed to check for duplicate pairs
*/
void checkDuplicateHostPortPairs(const std::vector<ServerConfig>& servers) {
	std::set<std::string> seen;

	for (size_t i = 0; i < servers.size(); ++i) {
		const std::string& host = servers[i].host;
		for (size_t j = 0; j < servers[i].ports.size(); ++j) {
			int port = servers[i].ports[j];
			std::ostringstream oss;
			oss << host << ":" << port;
			std::string key = oss.str();

			if (seen.count(key)) {
				throw std::runtime_error("❌ Duplicate Host Port pair found\n");
			}
			seen.insert(key);
		}
	}
}