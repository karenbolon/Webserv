/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/19 07:11:37 by kellen            #+#    #+#             */
/*   Updated: 2025/06/25 00:31:34 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

/*
* Constructor that parses the raw HTTP request.
*/
Request::Request(const std::string& raw) : _raw(raw) {
	parse(raw);
}

/*
* This function returns the HTTP method (e.g., GET, POST).
*/
std::string Request::getMethod() const {
	return _method;
}

/*
* This function returns the requested URL path (e.g., /index.html).
*/
std::string Request::getPath() const {
	return _path;
}

/*
 * This function extracts the HTTP request body (after headers).
 */
std::string Request::getBody() const {
	size_t pos = _raw.find("\r\n\r\n");
	if (pos == std::string::npos)
		return "";
	return _raw.substr(pos + 4);
}


std::string Request::getQuery() const {
	return _query;
}

void Request::parse(const std::string& raw) {
	std::istringstream ss(raw);
	std::string line;

	// ✅ Parse the request line
	if (std::getline(ss, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		std::istringstream lineStream(line);
		lineStream >> _method >> _target >> _version;

		size_t token = _target.find('?');
		if (token != std::string::npos) {
			_path = _target.substr(0, token);
			_query = _target.substr(token + 1);
		} else {
			_path = _target;
			_query = "";
		}
	}

	// ✅ Extract headers - FIXED VERSION
	std::string headerBlock;
	std::string headerLine;
	while (std::getline(ss, headerLine)) {
		// Remove trailing \r first
		if (!headerLine.empty() && headerLine[headerLine.size() - 1] == '\r')
			headerLine.erase(headerLine.size() - 1);

		// NOW check if line is empty (this marks end of headers)
		if (headerLine.empty()) {
			break;  // Empty line means end of headers
		}

		headerBlock += headerLine + "\n";
	}
	parseHeaders(headerBlock);
}

// Also add debug to parseHeaders method:
void Request::parseHeaders(const std::string& headerBlock) {

	std::istringstream stream(headerBlock);
	std::string line;

	while (std::getline(stream, line)) {

		if (!line.empty() && line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1);
		}

		size_t colon = line.find(':');
		if (colon == std::string::npos) {
			continue;
		}

		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);

		size_t start = value.find_first_not_of(" \t");
		if (start != std::string::npos)
			value = value.substr(start);

		for (size_t i = 0; i < key.size(); ++i)
			key[i] = std::tolower(static_cast<unsigned char>(key[i]));

		_headers[key] = value;
	}

	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
		it != _headers.end(); ++it) {
	}
}

const std::map<std::string, std::string>& Request::getHeaders() const {
	return _headers;
}

std::string Request::getRawRequest() const {
	return _raw;
}