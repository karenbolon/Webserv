/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:17 by kbolon            #+#    #+#             */
/*   Updated: 2025/06/12 00:24:15 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>
#include <map>

class LocationConfig;
struct	ServerConfig {
		//raw is for testing, ensure we process everything (remove before finishing)
	std::map<std::string, std::string> raw; //stores unprocessed directives
	std::vector<int>			ports; //converted and valid ports pulled from listen_entries
	std::vector<std::string>	listen_entries; //read initial ports as string
	std::string					host; // IP or host name
	std::string					server_name;
	std::string					root; //root directory for requests
	std::string					index; //default directory if no URI provided
	long						client_max_body_size;
	std::map<int, std::string>	error_pages; //error code and path
	std::vector<LocationConfig>	locations; //location blocks

	ServerConfig();

	void	print() const;
	void	loadErrorPages();
};

void		applyDefaults(ServerConfig& server);

#endif // SERVERCONFIG_HPP