/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:22 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/10 21:53:24 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <string>
#include <map>
#include <vector>

struct	LocationConfig {
	std::string	path;//e.g. "/upload" or "/cgi-bin"
	std::string	root; //e.g. "/var/www/uploads"
	std::vector<std::string> methods; //parsed from methods/allowed_methods: GET POST
	std::string	index; // index.html
	std::string	redirect;
	std::string	upload_path;
	std::map<std::string, std::string> cgi_paths; //py or php paths
	bool	autoindex;

	LocationConfig();
	void	print() const;
};
