/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStatus.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:24:24 by kbolon            #+#    #+#             */
/*   Updated: 2025/06/12 17:55:48 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

const std::map<int, std::string>& HttpStatus::statusMessages() {
	static std::map<int, std::string> statusList;
	if (statusList.empty()) {
		statusList[200] = "OK";
		statusList[201] = "Created";
		statusList[204] = "No Content";
		statusList[301] = "Moved Permanently";
		statusList[302] = "Found";
		statusList[400] = "Bad Request";
		statusList[401] = "Unauthorized";
		statusList[403] = "Forbidden";
		statusList[404] = "Not Found";
		statusList[405] = "Method Not Allowed";
		statusList[409] = "Conflict";
		statusList[413] = "Payload Too Large";
		statusList[500] = "Internal Server Error";
		statusList[501] = "Not Implemented";
		statusList[502] = "Bad Gateway";
		statusList[503] = "Service Unavailable";
	}
	return statusList;
}

std::string HttpStatus::getStatusMessages(int code) {
	//this line pulls/gets the list of messages (per above)
	const std::map<int, std::string>& messages = statusMessages();
	//iterate through the map to find the corresponding code and message

	std::map<int, std::string>::const_iterator it = messages.find(code);
	if (it != messages.end())
		return it->second.c_str();
	return "Unknown Status";
}