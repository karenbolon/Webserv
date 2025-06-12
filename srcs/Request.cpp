/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/19 07:11:37 by kellen            #+#    #+#             */
/*   Updated: 2025/06/12 03:30:53 by kellen           ###   ########.fr       */
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

/*
* This function parses the HTTP request line and extracts method + path.
*/
/*void Request::parse(const std::string& raw) {
	std::istringstream ss(raw);
	ss >> _method >> _path;
	if (_path == "/")
		_path = "/index.html";
	//extract just the header section
	size_t headerStart = raw.find("\r\n") + 2;
	size_t headerEnd = raw.find("\r\n\r\n");
	if (headerEnd != std::string::npos) {
		std::string headerBlock = raw.substr(headerStart, headerEnd - headerStart);
		parseHeaders(headerBlock);
	}
}*/

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

	// ✅ Extract headers
	std::string headerBlock;
	std::string headerLine;
	while (std::getline(ss, headerLine) && headerLine != "\r") {
		if (!headerLine.empty() && headerLine[headerLine.size() - 1] == '\r')
			headerLine.erase(headerLine.size() - 1);
		headerBlock += headerLine + "\n";
	}
	parseHeaders(headerBlock);
}


/*void Request::parseHeaders(const std::string& headerBlock) {
	std::istringstream stream(headerBlock);
	std::string line;

	// ✅ Parse the first line: method, target, version
	while (std::getline(stream, line)) {
		//trim \r from the end
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		std::istringstream lineStream(line);
		lineStream >> _method >> _target >> _version;
	}
	//parse rest of the headaers
	while (std::getline(stream, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		size_t colon = line.find(':');
		if (colon == std::string::npos)
			continue;
		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);

		//trim leading white space in value
		size_t start = value.find_first_not_of(" \t");
		if (start != std::string::npos)
			value = value.substr(start);

		//normalize key to lowercase
		for (size_t i = 0; i < key.size(); ++i)
			key[i] = std::tolower(key[i]);
		_headers[key] = value;
	}
	//extract path and query for CGI
	size_t token = _target.find('?');
	if (token != std::string::npos) {
		_path = _target.substr(0, token);
		_query = _target.substr(token + 1);
	}
	else {
		_path = _target;
		_query = "";
	}
}
*/

void Request::parseHeaders(const std::string& headerBlock) {
	std::istringstream stream(headerBlock);
	std::string line;

	while (std::getline(stream, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		size_t colon = line.find(':');
		if (colon == std::string::npos)
			continue;
		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);

		size_t start = value.find_first_not_of(" \t");
		if (start != std::string::npos)
			value = value.substr(start);

		for (size_t i = 0; i < key.size(); ++i)
			key[i] = std::tolower(key[i]);

		_headers[key] = value;
	}
}

const std::map<std::string, std::string>& Request::getHeaders() const {
	return _headers;
}

std::string Request::getRawRequest() const {
	return _raw;
}