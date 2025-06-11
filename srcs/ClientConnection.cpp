/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: keramos- <keramos-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 02:34:47 by kbolon            #+#    #+#             */
/*   Updated: 2025/06/11 17:36:32 by keramos-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ClientConnection.hpp"
#include "../include/WebServ.hpp"
#include <poll.h>
#include <sys/stat.h>
#include <iostream>
#include <unistd.h> //close
#include <cstring> //strerror
#include <sys/socket.h> //internet protocol family
#include <string>
#include <vector>
#include <stdint.h>
#include <fstream>
#include <fcntl.h>

ClientConnection::ClientConnection(int fd) : _fd(fd) {
	int flags = fcntl(_fd, F_GETFL, 0);
	if (!(flags & O_NONBLOCK))
		fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
}

int	ClientConnection::getFd() const {
	return _fd;
}

void	ClientConnection::closeConnection() {
	if (_fd != -1)
		close(_fd);
}

ClientConnection::~ClientConnection() {
	closeConnection();
}

/*
-We use non-blockeing recv() safely
-server reads one full file at a time per client connection
-gracefully handles slow clients and disconnects
-exits cleanly after each transfer
*/
int ClientConnection::recvFullRequest(int client_fd, const ServerConfig& config) {
	//switched to vector to handle images and pdfs
	char buffer[8192];//8 kb buffer size
	int bytes = recv(client_fd, buffer, sizeof(buffer), 0);

	if (bytes <= 0) {
		if (bytes == 0)
			std::cerr << "Client disconnected cleanly\n";
		else {
			std::cerr << "⚠️ Connection closed or recv failed during recvFullRequest\n";
			std::string body = getErrorPageBody(500, config);
			sendHtmlResponse(client_fd, 500, body);
		}
		return bytes;
	}
	this->_buffer.insert(this->_buffer.end(), buffer, buffer + bytes);
	std::string reqStr(_buffer.begin(), _buffer.end());
	size_t headerEnd = reqStr.find("\r\n\r\n");
	if (headerEnd == std::string::npos) {
		std::cerr << "❌ Incomplete headers\n";
		std::string body = getErrorPageBody(400, config);
		sendHtmlResponse(client_fd, 400, body);
		return bytes;
	}
	// Found end of headers, check for Content-Length
	size_t contentLengthPos = reqStr.find("Content-Length:");
	if (contentLengthPos != std::string::npos) {
		size_t valueStart = contentLengthPos + 15; // Length of "Content-Length:"
		size_t valueEnd = reqStr.find("\r\n", valueStart);
		if (valueEnd != std::string::npos) {
			std::string lengthStr = reqStr.substr(valueStart, valueEnd - valueStart);
			size_t totalContentLength = atoi(lengthStr.c_str());
			size_t bodyStart = headerEnd + 4;
			if (_buffer.size() - bodyStart < totalContentLength)
				return bytes;
		}
	}
	return bytes;
}

bool ClientConnection::isRequestComplete() const {
	std::string reqStr(_buffer.begin(), _buffer.end());
	size_t headerEnd = reqStr.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return false;
	size_t contentLengthPos = reqStr.find("Content-Length:");
	if (contentLengthPos == std::string::npos)
		return true;
	size_t valueStart = contentLengthPos + 15; // Length of "Content-Length:"
	size_t valueEnd = reqStr.find("\r\n", valueStart);
	std::string lengthStr = reqStr.substr(valueStart, valueEnd - valueStart);
	size_t totalLength = atoi(lengthStr.c_str());
	size_t bodyStart = headerEnd + 4;
	return (_buffer.size() - bodyStart >= totalLength);
}

std::string ClientConnection::getRawRequest() const {
	return std::string(_buffer.begin(), _buffer.end());
}