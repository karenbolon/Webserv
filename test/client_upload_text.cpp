/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client_upload_text.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/13 16:14:38 by kbolon            #+#    #+#             */
/*   Updated: 2025/05/13 18:03:18 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <sstream>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h> 
#include <cctype> //for std::tolower
#include <dirent.h> //to allow access to file directory
//#include "../include/WebServ.hpp"
#include "../include/ConfigParser.hpp"
//#include "../include/ServerSocket.hpp"
#include <arpa/inet.h>

std::string toLower(const std::string& str) {
	std::string result;
	for (size_t i = 0; i < str.size(); ++i)
		result += std::tolower(str[i]);
	return result;
}

bool isFile(const std::string& name) {
	std::string	lowerName = toLower(name);
	const char* extensions[] = {".jpg", ".jpeg", ".png", ".heic", ".txt", ".pdf", "cpp"};
	int count = sizeof(extensions) / sizeof(extensions[0]);
	for (int i = 0; i < count; i++) {
		const char* ext = extensions[i];
		size_t	extLen = std::strlen(ext);
		if (lowerName.size() >= extLen && lowerName.compare(lowerName.size() - extLen, extLen, ext) == 0)
			return true;
	}
	return false;
}

std::string createUniqueFileName(const std::string& base, std::set<std::string>& used) {
	std::string name = base;
	int counter = 1;
	while (used.find(name) != used.end()) {
		size_t dot = name.find_last_of(".");
		std::string nameOnly = base.substr(0, dot);
		std::string ext = base.substr(dot);
		std::ostringstream oss;
		oss << nameOnly << "_" << counter << ext;
		name = oss.str();
		counter++;
	}
	used.insert(name);
	return name;
}

int main() {

	ConfigParser	parser;

	parser.parseFile("conf/default.conf");
	const std::vector<ServerConfig>& servers = parser.getServers();
	if (servers.empty()) {
		std::cerr << "❌ Problem retrieving servers in test client\n";
		return 1;
	}

	//initialise the first server
	int port = servers[0].port;

	std::cout << "Starting server on port: " << port << std::endl;
	//creating socket
	int	clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	//specify address
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

	//send connection request
	if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		std::cerr << "❌ Client failed to connect\n";
		return 1;
	}

	//open directory of test files
	DIR* dir = opendir("./test/test_samples");
	if (!dir) {
		std::cerr << "Failed to open directory\n";
		return 1;
	}
	std::set<std::string> usedNames;
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string original = entry->d_name;
		if (original == "." || original == "..")
			continue;
		//f		std::cout << "Checking file: " << fileName << std::endl;
		if (!isFile(original))
			continue;
		std::string filePath = "./test/test_samples/" + original;
		
		std::string fileName = createUniqueFileName(original, usedNames);
		//send filename length
		uint8_t	nameLen = fileName.size();
		if (send(clientSocket, &nameLen, 1, 0) == -1){
			std::cerr << "Failed to send filename length\n";
			return 1;
		}

		//send filename
		if (send(clientSocket, fileName.c_str(), nameLen, 0) == -1) {
			std::cerr << "Failed to send filename\n";
			return 1;
		}

		std::ifstream	file(filePath.c_str(), std::ios::binary);
		if (!file) {
			std::cerr << "Cannot open input\n";
			return 1;
		}
		file.seekg(0, std::ios::end);
		//tellg() Returns the position of the current character in the input stream.
		uint32_t	fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		if (send(clientSocket, &fileSize, sizeof(fileSize), 0) == -1) {
			std::cerr << "Failed to send file size\n";
			return 1;
		}

		char buffer[1024];
		while (file.read(buffer, sizeof(buffer))) {
			send(clientSocket, buffer, sizeof(buffer), 0);
		}
		//gcount() Returns the number of characters extracted by the last 
		//unformatted input operation performed on the object.
		send(clientSocket, buffer, file.gcount(), 0);
		file.close();
		std::cout << "Sent: " << fileName << std::endl;
	}
	closedir(dir);
	close(clientSocket);
	return 0;
}