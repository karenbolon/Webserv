#include "../include/WebServ.hpp"
#include <sys/stat.h>
#include <iostream>
#include <unistd.h> //close
#include <cstring> //strerror
#include <sys/socket.h> //internet protocol family
#include <string>
#include <stdint.h>

ClientConnection::ClientConnection(int fd) : _fd(fd) {}

ClientConnection::~ClientConnection() {
	closeConnection();
}

/*
problem with recv() as the client can send more than the buffer, must
create a loop to listen until data is done or connection is closed
*/
bool	ClientConnection::receiveMessage() {
	const size_t	bufferSize = 1024;
	char buffer[bufferSize];
	mkdir("test/output", 0777); //ignores if already exists

	while (true) { //replace with poll()

		//read filename length
		uint8_t	nameLen = 0;
		ssize_t bytes = recv(_fd, &nameLen, 1, 0);
		if (bytes == 0) break; //client closed the connection
		if (bytes != 1) {
			std::cerr << "Failed to read filename length\n";
			return false;
		}

		//read filename
		char	nameBuffer[256];
		bytes = recv(_fd, nameBuffer, nameLen, 0);
		if (bytes != nameLen) {
			std::cerr << "Failed to read filname\n";
			return false;
		}
		std::string	fileName(nameBuffer, nameLen);

		//Read file size
		uint32_t	fileSize = 0;
		bytes = recv(_fd, &fileSize, sizeof(fileSize), 0);
		if (bytes != sizeof(fileSize)) {
			std::cerr << "Failed to read file size\n";
			return false;
		}
		std::string fullPath =  "output/" + fileName;
		std::ofstream	output(fullPath.c_str(), std::ios::binary);
		if (!output) {
			std::cerr << "Failed to open file for writing\n";
			return false;
		}

		//read exactly fileSize bytes
		uint32_t remaining = fileSize;
		while (remaining > 0) {
			ssize_t toRead = (remaining > bufferSize) ? bufferSize : remaining;
			ssize_t	received = recv(_fd, buffer, toRead, 0);
			if (received <= 0) {
				std::cerr << "Receive failed or connection was interrupted\n";
				return false;
			}
			output.write(buffer, received);
			if (!output){
				std::cerr << "Failed to write to file\n";
				return false;
			}
			remaining -= received;
		}
		output.close();
		std::cout << "Saved: " << fileName << std::endl;
	}
	return true;
}

// bool	ClientConnection::sendMessage() {
// 	const	std::string	response =

// }

std::string	ClientConnection::getMessage() const {
	return _message;
}

int	ClientConnection::getFd() const {
	return _fd;
}

void	ClientConnection::closeConnection() {
	if (_fd != -1)
		close(_fd);
}
