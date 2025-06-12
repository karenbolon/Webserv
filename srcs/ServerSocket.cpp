/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 17:19:37 by kbolon            #+#    #+#             */
/*   Updated: 2025/06/12 00:36:34 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

ServerSocket::ServerSocket() : _fd(-1) {}

ServerSocket::~ServerSocket() {
	closeSocket();
}

/*
Checks if host is 0.0.0.0 (public) or 127.0.0.1 (localhost) or something else
struct hostent is an old C-struct part of BSD sockets API
*/
bool checkHost(const std::string& host, in_addr& addr) {
	//checks if valid IPv4 address (like 127.0.0.1 or 0.0.0.0)
	if (inet_pton(AF_INET, host.c_str(), &addr) == 1)
		return true;

	//if not a valid IPv4, gets the local name and checks if empty or AF_INET
	struct hostent* he = gethostbyname(host.c_str());
	if (!he || he->h_addrtype != AF_INET)
		return false;

	//if everything ok, it copies the address
	std::memcpy(&addr, he->h_addr_list[0], sizeof(struct in_addr));
	return true;
}

/*
htons converts host byte order to network byte order (for port).
fcntl sets the socket to non-blocking mode.
We bind, listen, and accept incoming connections on this socket.
*/
bool	ServerSocket::init(int port, const std::string& host) {
	//creates a TCP socket for IPv4 and stores the FD
	_fd = safe_socket(AF_INET, SOCK_STREAM, 0);
	if (_fd == -1) return false;

	int yes = 1; //in use
	//allows reuse of socket, avoids "aready in use" errors
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
		close(_fd);
		return false;
	}
	//make the socket non-blocking
	if (fcntl(_fd, F_SETFL, O_NONBLOCK) == -1) {
		std::cerr << "Failed to set FD to non-blocking: " << std::strerror(errno) << std::endl;
		close(_fd);
		return false;
	}
	//fills socket address struct
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	in_addr	addr; //if valid, it is filled thru checkHost
	if (!checkHost(host, addr)) {
		std::cerr << "❌ Invalid host address: " << host << std::endl;
		closeSocket();
		return false;
	}
	serverAddress.sin_addr = addr;
	if (!safe_bind(_fd, serverAddress)) {
		closeSocket();
		return false;
	}
	if (!safe_listen(_fd, 20)) {
		closeSocket();
		return false;
	}
	return true;
}

void  ServerSocket::setConfig(const ServerConfig& config) {
	_config = config;
}

const ServerConfig& ServerSocket::getConfig() const {
	return _config;
}

int		ServerSocket::acceptClient() {
	int	client_fd = accept(_fd, NULL, NULL);
	if (client_fd == -1) {
		std::cerr << "Failed to accept: " << std::strerror(errno) << std::endl;
		return -1;
	}
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) {
		std::cerr << "Failed to set FD to non-blocking: " << std::strerror(errno) << std::endl;
		close(client_fd);
		return -1;
	}
	std::cout << "Accepted connection on socket: " << client_fd << std::endl;
	return client_fd;
}

void	ServerSocket::closeSocket() {
	if (_fd != -1) {
		close(_fd);
		_fd = -1;
	}
}

int		ServerSocket::getFD() {
	return _fd;
}
