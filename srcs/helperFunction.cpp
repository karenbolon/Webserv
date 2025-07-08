/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helperFunction.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 17:19:59 by kbolon            #+#    #+#             */
/*   Updated: 2025/07/08 02:24:43 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

std::string	intToStr(int n) {
	std::ostringstream oss;
	oss << n;
	return oss.str();
}

int	safe_socket(int domain, int type, int protocol) {
	int	fd = socket(domain, type, protocol);
	if (fd == -1) {
		std::cerr << "Failed to create socket: " << std::strerror(errno) << std::endl;
		return -1;
	}
	return fd;
}

/*
int bind(int socket, const struct sockaddr *address, socklen_t address_len)

The sin_port field is set to the port to which the application must bind.
It must be specified in network byte order. If sin_port is set to 0, the
caller leaves it to the system to assign an available port.
The application can call getsockname() to discover the port number assigned.
*/
bool	safe_bind(int fd, sockaddr_in & addr) {
	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		std::cerr << "Failed to bind: " << std::strerror(errno) << std::endl;
		return false;
	}
	return true;
}

/*
listen only applies to stream sockets, it creates a connection request queue
size of backlog
*/
bool	safe_listen(int socket, int backlog) {
	if (listen(socket, backlog) == -1) {
		std::cerr << "Failed to listen: " << std::strerror(errno) << std::endl;
		return false;
	}
	return true;
}

std::string	trim(std::string& s) {
	size_t	start = s.find_first_not_of(" \t\r\n");
	size_t end = s.find_last_not_of(" \t\r\n");
	if (start == std::string::npos || end == std::string::npos) {
		s.clear();//all whitespace
		return "";
	}
	else {
		s = s.substr(start, end - start + 1);
		return s;
	}
}

std::string	cleanValue(std::string s) {
	//look for '//' preceded by ' ' or ';'
	size_t commentPos = std::string::npos;
	for (size_t i = 2; i < s.length(); ++i) {
		if (s[i] == '/' && s[i - 1] == '/' && (s[i - 2] == ' ' || s[i - 2] == ';')) {
			commentPos = i - 1;
			break;
		}
	}
	//use '#' if no comment '//' found
	size_t hashPos = s.find('#');
	if (hashPos != std::string::npos && (commentPos == std::string::npos || hashPos < commentPos))
		commentPos = hashPos;
	//remove comment
	if (commentPos != std::string::npos)
		s = s.substr(0, commentPos);
	//remove last semicolon
	size_t	semicolon = s.find_last_of(";");
	if (semicolon != std::string::npos)
		s = s.substr(0, semicolon);
	return trim(s);
}

void	shutDownWebserv(std::vector<ServerSocket*>& serverSockets, std::map<int, ClientConnection*>& clients) {
	for (size_t i = 0; i < serverSockets.size(); i++)
		serverSockets[i]->closeSocket();
	for (std::map<int, ClientConnection*>::iterator it = clients.begin(); it != clients.end(); ++it)
		delete it->second;
	for (size_t i = 0; i < serverSockets.size(); ++i)
		delete serverSockets[i];
	std::cout << "🧼 Webserv shut down cleanly.\n";
}

// Add this debug function to helperFunction.cpp
void debugImageServing(const std::string& path, const std::string& fullPath) {
	std::cout << "🖼️ Image Debug Info:" << std::endl;
	std::cout << "   Request path: " << path << std::endl;
	std::cout << "   Full path: " << fullPath << std::endl;

	// Check if file exists
	if (!fileExists(fullPath)) {
		std::cout << "   ❌ File does not exist!" << std::endl;
		return;
	}

	// Get file size
	std::ifstream file(fullPath.c_str(), std::ios::binary | std::ios::ate);
	if (file.is_open()) {
		size_t size = file.tellg();
		std::cout << "   📏 File size: " << size << " bytes" << std::endl;
		std::cerr << "in helper function close\n";
		file.close();
	}

	// Get content type
	Response resp;
	std::string contentType = resp.getContentType(fullPath);
//	std::cout << "   📝 Content-Type: " << contentType << std::endl;

	// Check if it's a valid image type
	if (contentType.find("image/") == 0) {
		std::cout << "   ✅ Valid image content type" << std::endl;
	} else {
		std::cout << "   ⚠️ Not recognized as image!" << std::endl;
	}
}

void serveStaticFile(std::string path, const ServerConfig &config,
	ClientConnection* client, std::vector<struct pollfd>& fds) {
	// Check if the path is empty or just a slash, then use the index file
//	std::cout << "🗂️ Serving static file: fullPath = '" << path << "'" << std::endl;

	if (path.empty() || path == "/")
		path = "/" + config.index;
	std::string fullPath = config.root + path;

	// Security check
	if (path.find("..") != std::string::npos) {
		std::cout << "🚨 Security violation: path traversal detected!" << std::endl;
		std::string errorBody = getErrorPageBody(403, config);
		sendHtmlResponse(403, errorBody, client, fds);
		return;
	}

	// Check if file exists
	if (!fileExists(fullPath)) {
		std::cout << "DEBUG: fullPath in HELPER= '" << fullPath << "'" << std::endl;
		std::cerr << "❌ Static file not found: " << fullPath << std::endl;
		std::string errorBody = getErrorPageBody(404, config);
		sendHtmlResponse(404, errorBody, client, fds);
		return;
	}

	// Get content type
	Response resp;
	std::string contentType = resp.getContentType(fullPath);
//	std::cout << "📝 Content-Type: " << contentType << std::endl;

	// Check if we should use chunked transfer
	if (useChunkedTransfer(fullPath)) {
//		std::cout << "🚀 Using CHUNKED TRANSFER for large file" << std::endl;

		if (sendFileChunked(client->getFd(), fullPath, contentType)) {
			std::cout << "✅ Chunked transfer completed successfully" << std::endl;
		}
		else {
			std::cout << "❌ Chunked transfer failed, sending error" << std::endl;
			std::string errorBody = getErrorPageBody(500, config);
			sendHtmlResponse(500, errorBody, client, fds);
		}
		return;
	}

	// Use regular transfer for smaller files
	//std::cout << "📄 Using REGULAR TRANSFER for normal-sized file" << std::endl;

	std::ifstream file(fullPath.c_str(), std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "❌ Cannot open file: " << fullPath << std::endl;
		std::string errorBody = getErrorPageBody(500, config);
		sendHtmlResponse(500, errorBody, client, fds);
		return;
	}

	std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	std::cerr << "in helper function close2\n";
	std::string response = Response::build(200, body, contentType);
	client->setPendingResponse(response);
	for (size_t j = 0; j < fds.size(); ++j) {
		if (fds[j].fd == client->getFd())
			fds[j].events |= POLLOUT;
	}
	return;

}

std::string extractBoundary(const std::string& request) {
	// We must find the end of the file data, which is marked by a boundary
	// First, extract the boundary from the Content-Type header
	size_t boundaryPos = request.find("boundary=");
	if (boundaryPos == std::string::npos)
		return "";

	// Extract the boundary string, handling both quoted and unquoted formats
	size_t boundaryStart = boundaryPos + 9; // "boundary=" length

	if (request[boundaryStart] == '"') {
		// Quoted boundary
		boundaryStart++;
		size_t boundaryEnd = request.find("\"", boundaryStart);
		if (boundaryEnd == std::string::npos)
			return "";
		return request.substr(boundaryStart, boundaryEnd - boundaryStart);
	}
	else {
		// Unquoted boundary
		size_t boundaryEnd = request.find_first_of(" \r\n;", boundaryStart);
		if (boundaryEnd == std::string::npos) {
			// If no terminator, use the rest of the line
			return request.substr(boundaryStart);
		} else {
			return request.substr(boundaryStart, boundaryEnd - boundaryStart);
		}
	}
}

/*
* This is a complete rewrite of the upload handler using a direct byte-by-byte approach
* designed specifically to handle binary file uploads correctly.
*/
//
void sendHtmlResponse(int code, const std::string& body, ClientConnection* client, std::vector<struct pollfd>& fds) {

	std::string response = Response::build(code, body, "text/html");
	client->setPendingResponse(response);
	for (size_t j = 0; j < fds.size(); ++j) {
		if (fds[j].fd == client->getFd())
			//this flags the FD as POLLOUT so the poll() will find it, this FD wants to send a message
			fds[j].events |= POLLOUT;
	}

}

/*
This function looks up the error code per config map, if found,
it tries to open the file per the path provided.  If successful, it returns the
error page otherwise, it generates a simple fallback HTML error page with code and message.
*/
std::string	getErrorPageBody(int code, const ServerConfig& config) {
	std::map<int, std::string>::const_iterator it = config.error_pages.find(code);
	if (it != config.error_pages.end()) {
		std::string fullPath = config.root;
		if (!fullPath.empty() && fullPath[fullPath.length() - 1] != '/')
			fullPath += "/";
		fullPath += it->second;
		std::cerr << "Looking for: " << fullPath << std::endl;

		std::ifstream file(fullPath.c_str());
		if (file.is_open()) {
			std::string content((std::istreambuf_iterator<char>(file)),
								std::istreambuf_iterator<char>());
			file.close();
			std::cerr << "in helper function close 3\n";
			return content;
		}
	}
	std::ostringstream oss;
	oss << "<html><body><h1>" << code << " - ";
	oss << HttpStatus::getStatusMessages(code);
	oss << "</h1></body></html>";
	return oss.str();
}

/*
implemented location block selection using a longest prefix match. That allows more specific paths like:

location /  # very general
location /images  # more specific
location /images/cats  # even more specific (and longest match)

We do not use exact match as an exact match would miss: location /images/cats/cute.jpg
 */
LocationConfig matchLocation(const std::string& path, const ServerConfig& config) {
	const std::vector<LocationConfig>& locations = config.locations;

	LocationConfig bestMatch;
	size_t length = 0;

	for (size_t i = 0; i < locations.size(); ++i) {
		const std::string& locationPath = locations[i].path;
		if (path.find(locationPath) == 0 && locationPath.length() > length) {
			bestMatch = locations[i];
			length = locationPath.length();
		}
	}
	return bestMatch;
}

void handleClientCleanup(int fd, std::vector<pollfd>& fds,
		std::map<int, ClientConnection*>& clients, size_t& i) {

	 // 🚧 Guard: make sure this fd belongs to a real ClientConnection
    if (clients.find(fd) == clients.end() || clients[fd] == NULL) {
        std::cerr << "⚠️  Skipping cleanup: fd " << fd << " has no ClientConnection\n";
        return;
    }
	
	ClientConnection* client = clients[fd];
	
	// Track CGI fds before erasing client
	int cgiInputFd = client->getCgiInputFd();

	// Safely close CGI FDs
    if (client->getCgiInputFd() != -1) 
		close(client->getCgiInputFd());
    if (client->getCgiOutputFd() != -1) 
		close(client->getCgiOutputFd());
	if (!client) {
		std::cerr << "❌ handleClientCleanup: ClientConnection is null for fd " << fd << std::endl;
	return;
	}
	int cgiOutputFd = client->getCgiOutputFd();
	
	delete client;
	clients.erase(fd);
	close(fd);
	std::cerr << "in helper function close 4\n";
	// Remove from poll fds
	fds.erase(fds.begin() + i);
	--i;

	// Remove CGI fds from pollfds if present
	if (cgiInputFd != -1)
		removePollFd(fds, cgiInputFd);
	if (cgiOutputFd != -1)
		removePollFd(fds, cgiOutputFd);
}

void removePollFd(std::vector<pollfd>& fds, int fd) {
	for (size_t i = 0; i <fds.size(); ++i) {
		if (fds[i].fd == fd) {
			close(fd);
			std::cerr << "in helper function close 5\n";
			fds.erase(fds.begin() + i);
			break;
		}
	}
}

std::string generateJsonDirectoryListing(const std::string& dirPath) {
	std::ostringstream json;
	json << "[";

	DIR* dir = opendir(dirPath.c_str());
	if (!dir) {
		json << "]";
		return json.str();
	}

	std::vector<std::string> files;
	struct dirent* entry;

	// Collect all files (excluding . and ..)
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;
		if (name != "." && name != "..") {
			std::string fullPath = dirPath + "/" + name;
			struct stat statbuf;
			if (stat(fullPath.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
				files.push_back(name);
			}
		}
	}
	closedir(dir);

	// Sort files by name
	std::sort(files.begin(), files.end());

	// Generate JSON
	for (size_t i = 0; i < files.size(); ++i) {
		if (i > 0) json << ",";

		std::string fullPath = dirPath + "/" + files[i];
		struct stat statbuf;
		stat(fullPath.c_str(), &statbuf);

		// Determine file type
		std::string fileType = "document";
		std::string ext = files[i].substr(files[i].find_last_of('.') + 1);
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "gif" || ext == "webp") {
			fileType = "image";
		} else if (ext == "pdf") {
			fileType = "pdf";
		} else if (ext == "txt") {
			fileType = "txt";
		} else if (ext == "mp3" || ext == "wav" || ext == "ogg") {
			fileType = "audio";
		} else if (ext == "mp4" || ext == "avi" || ext == "mov") {
			fileType = "video";
		}

		json << "{";
		json << "\"name\":\"" << files[i] << "\",";
		json << "\"type\":\"" << fileType << "\",";
		json << "\"size\":" << statbuf.st_size << ",";
		json << "\"uploadTime\":" << (statbuf.st_mtime * 1000); // JavaScript timestamp
		json << "}";
	}

	json << "]";
	return json.str();
}

// Check if file should use chunked transfer
bool useChunkedTransfer(const std::string& fullPath) {
	std::ifstream file(fullPath.c_str(), std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		std::cout << "❌ Cannot open file for size check: " << fullPath << std::endl;
		return false;
	}

	size_t fileSize = file.tellg();
	file.close();
	std::cerr << "in helper function close 6\n";

	bool useChunked = fileSize > 1024 * 1024;

	return useChunked;
}

// Send file using chunked transfer encoding
bool sendFileChunked(int fd, const std::string& fullPath, const std::string& contentType) {

	std::ifstream file(fullPath.c_str(), std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "❌ Cannot open file for chunked transfer: " << fullPath << std::endl;
		return false;
	}

	// Send HTTP headers with chunked encoding
	std::ostringstream headers;
	headers << "HTTP/1.1 200 OK\r\n";
	headers << "Content-Type: " << contentType << "\r\n";
	headers << "Transfer-Encoding: chunked\r\n";
	headers << "Connection: close\r\n";
	headers << "\r\n";

	std::string headerStr = headers.str();
	if (!sendAll(fd, headerStr.c_str(), headerStr.size())) {
		std::cerr << "❌ Failed to send chunked headers" << std::endl;
		file.close();
		std::cerr << "in helper function close 7\n";
		return false;
	}

	// Send file in chunks
	char buffer[65536];
	size_t totalBytesSent = 0;
	size_t chunkCount = 0;

	while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
		if (file.bad()) {
			std::cerr << "❌ File read error during chunked transfer" << std::endl;
			file.close();
			std::cerr << "in helper function close 8\n";
			return false;
		}
		size_t bytesRead = file.gcount();
		chunkCount++;

		// Send chunk size in hexadecimal
		std::ostringstream chunkHeader;
		chunkHeader << std::hex << bytesRead << "\r\n";
		std::string chunkHeaderStr = chunkHeader.str();

		// Send chunk size
		if (!sendAll(fd, chunkHeaderStr.c_str(), chunkHeaderStr.size())) {
			file.close();
			std::cerr << "in helper function close 9\n";
			return false;
		}

		// Send chunk data
		if (!sendAll(fd, buffer, bytesRead)) {
			file.close();
			std::cerr << "in helper function close 10\n";
			return false;
		}

		// Send chunk trailing CRLF
		if (!sendAll(fd, "\r\n", 2)) {
			file.close();
			std::cerr << "in helper function close 11\n";
			return false;
		}

		totalBytesSent += bytesRead;
	}
	
	file.close();
	std::cerr << "in helper function close 12\n";

	// Send final chunk (size 0) to indicate end
	if (!sendAll(fd, "0\r\n\r\n", 5)) {
		std::cerr << "❌ Failed to send final chunk" << std::endl;
		return false;
	}

	std::cout << "✅ Chunked transfer complete!" << std::endl;
	std::cout << "   📊 Total chunks: " << chunkCount << std::endl;
	std::cout << "   📏 Total bytes: " << totalBytesSent << std::endl;

	return true;
}

/**
 * Find the next file section in a multipart request starting from a given position
 * Returns the start position of the file section, or std::string::npos if no more files
 */
size_t findNextFileSection(const std::string& request, const std::string& boundary, size_t startPos) {
	std::string fullBoundary = "--" + boundary;

	size_t pos = startPos;
	while ((pos = request.find(fullBoundary, pos)) != std::string::npos) {
		size_t sectionStart = pos + fullBoundary.length();

		// Skip CRLF after boundary
		if (sectionStart + 2 < request.length() &&
			request.substr(sectionStart, 2) == "\r\n") {
			sectionStart += 2;
		}

		// Check if this section contains a filename (is a file upload)
		size_t nextBoundary = request.find(fullBoundary, sectionStart);
		if (nextBoundary == std::string::npos) {
			break; // No more sections
		}

		std::string section = request.substr(sectionStart, nextBoundary - sectionStart);
		if (section.find("filename=\"") != std::string::npos) {
			return sectionStart; // Found a file section
		}

		pos = nextBoundary;
	}

	return std::string::npos; // No more file sections found
}

/**
 * Extract filename from a specific section of the request
 */
bool extractFilenameFromSection(const std::string& request, size_t sectionStart,
							size_t sectionEnd, std::string& filename) {
	std::string section = request.substr(sectionStart, sectionEnd - sectionStart);

	size_t filenamePos = section.find("filename=\"");
	if (filenamePos == std::string::npos) {
		return false;
	}

	size_t filenameStart = filenamePos + 10; // Length of "filename=\""
	size_t filenameEnd = section.find("\"", filenameStart);
	if (filenameEnd == std::string::npos) {
		return false;
	}

	filename = section.substr(filenameStart, filenameEnd - filenameStart);
	return !filename.empty();
}

bool sendAll(int fd, const char* buffer, size_t length) {
	size_t totalSent = 0;
	int retryCount = 0;
	const int maxRetries = 1000;

	while (totalSent < length) {
		ssize_t sent = send(fd, buffer + totalSent, length - totalSent, 0);
		if (sent < 0) {
			if (++retryCount > maxRetries)
				return false;
			usleep(1000);
			continue;
		}
		if (sent == 0)
			return false;
		totalSent += sent;
		retryCount = 0; // Reset on successful send
	}
	return true;
}

/**
 * Find file content boundaries within a specific section
 */
bool findFileContentInSection(const std::string& request, size_t sectionStart,
							size_t sectionEnd, size_t& contentStart, size_t& contentLength) {
	// Find the end of headers within this section (marked by \r\n\r\n)
	size_t headerEnd = request.find("\r\n\r\n", sectionStart);
	if (headerEnd == std::string::npos || headerEnd >= sectionEnd) {
		return false;
	}

	contentStart = headerEnd + 4; // Skip "\r\n\r\n"

	// Content ends before the next boundary (minus trailing CRLF)
	size_t contentEnd = sectionEnd;
	if (contentEnd >= 2 &&
		request.substr(contentEnd - 2, 2) == "\r\n") {
		contentEnd -= 2;
	}

	if (contentEnd <= contentStart) {
		return false;
	}

	contentLength = contentEnd - contentStart;
	return true;
}
