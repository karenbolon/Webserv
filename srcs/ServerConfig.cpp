/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 17:19:52 by kbolon            #+#    #+#             */
/*   Updated: 2025/06/12 00:23:29 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

ServerConfig::ServerConfig() : ports(0), client_max_body_size(0) {}

void	ServerConfig::print() const {
	std::cout << "\n======================" << std::endl;
	std::cout << "listen: ";
	for (size_t i = 0; i < ports.size(); ++i)
		std::cout << ports[i] << " ";
	std::cout << std::endl;
	std::cout << "host: " << host << std::endl;
	std::cout << "server_name: " << server_name << std::endl;
	std::cout << "root: " << root << std::endl;
	std::cout << "index: " << index << std::endl;
	std::cout << "client_max_body_size: " << client_max_body_size << std::endl;

	for (std::map<int, std::string>::const_iterator it = error_pages.begin(); it != error_pages.end(); ++it)
		std::cout << "error_page " << it->first << " => " << it->second << std::endl;

	for (size_t i = 0; i < locations.size(); ++i)
		locations[i].print();

	if (!raw.empty()) {
		std::cout << "\nSERVER RAW DIRECTIVES:\n";
		for (std::map<std::string, std::string>::const_iterator it = raw.begin(); it != raw.end(); ++it)
			std::cout << "    " << it->first << ": " << it->second << std::endl;
	}
}

void	applyDefaults(ServerConfig& server) {
	if (server.listen_entries.empty()) {
		server.listen_entries.push_back("8080");
		convertListenEntriesToPortsAndHost(server);
	}
	if (server.host.empty())
		server.host = "127.0.0.1";
	if (server.root.empty())
		server.root = "www";
	if (server.index.empty())
		server.index = "index.html";
	if (server.server_name.empty())
		server.server_name = "default_server";
	if (!server.client_max_body_size)
		server.client_max_body_size = 1000000;
}
