#include "../include/ServerSocket.hpp"
#include <iostream>
#include <unistd.h> //close
#include <cstring> //strerror
#include <cerrno> //errno
#include <stdexcept> //std::runtime_error
#include <netinet/in.h>
#include <sys/socket.h> //internet protocol family


ServerSocket::ServerSocket() : _fd(-1) {}

/*void	initialisePollFDs() {
	fds.reserve(1024);
}*/

ServerSocket::~ServerSocket() {
	closeSocket();
}

/*
htons (Host To Network Short): converts machine byte to network byte order
Basically, you’ll want to convert the numbers to Network Byte Order before
they go out on the wire, and convert them to Host Byte Order as they come in off the wire.

INADDR_ANY is used when we don't want to bind our socket to any particular
IP and instead make it listen to all IPs available

Bind the socket, tell it to listen to the socket referred to by serverSocket_fd.
Then we accept the connection request that is received on the socket
the app is listening to
*/
bool	ServerSocket::init(int port) {
	_fd = safe_socket(AF_INET, SOCK_STREAM, 0);
	if (_fd == -1) return false;

	//make it non-block (fcntl sets fd to nonblock)
	if (fcntl(_fd, F_SETFL, O_NONBLOCK) == -1) {
		std::cerr << "Failed to set FD to non-blocking: " << std::strerror(errno) << std::endl;
		close(_fd);
		return -1;
	}
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (!safe_bind(_fd, serverAddress)) {
		closeSocket();
		return false;
	}

	if (!safe_listen(_fd, 5)) {
		closeSocket();
		return false;
	}
	return true;
}

int		ServerSocket::acceptClient() {
	int	client_fd = accept(_fd, NULL, NULL);
	if (client_fd == -1) {
		std::cerr << "Failed to accept: " << std::strerror(errno) << std::endl;
//		closeSocket();
		return -1;
	}
	//Make client fd non-blocking
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) {
		std::cerr << "Failed to set FD to non-blocking: " << std::strerror(errno) << std::endl;
		close(client_fd);
		return -1;
	}
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

int		ServerSocket::getPort() {
	return _port;
}
