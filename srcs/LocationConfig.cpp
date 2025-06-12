/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 14:13:02 by kbolon            #+#    #+#             */
/*   Updated: 2025/06/12 00:37:39 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

LocationConfig::LocationConfig() : returnStatusCode(0), autoindex(false), root_set(false), index_set(false) {}

void	LocationConfig::print() const {
	std::cout << "\nLOCATION:\n";
	std::cout << "path: " << path << std::endl;
	std::cout << "root: " << root << std::endl;
	std::cout << "index: " << index << std::endl;
	std::cout << "return Status Code: " << returnStatusCode << std::endl;
	std::cout << "redirect: " << redirect << std::endl;
	std::cout << "autoindex: " << autoindex << std::endl;
	std::cout << "methods: ";
	for (size_t i = 0; i < methods.size(); i++)
		std::cout << methods[i] << " ";
	std::cout << std::endl;
	std::cout << "upload_path: " << upload_path << std::endl;

	for (std::map<std::string, std::string>::const_iterator it = cgi_paths.begin(); it != cgi_paths.end(); ++it) {
		std::cout << "cgi[" << it->first << "] = " << it->second << std::endl;
	}

	if (!raw.empty()) {
		std::cout << "\n  RAW DIRECTIVES:\n";
		for (std::map<std::string, std::string>::const_iterator it = raw.begin(); it != raw.end(); ++it)
		std::cout << "    " << it->first << ": " << it->second << std::endl;
	}
	std::cout << std::endl;
}
