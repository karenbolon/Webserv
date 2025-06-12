/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helperFunction.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 17:19:59 by kbolon            #+#    #+#             */
/*   Updated: 2025/06/12 00:37:17 by kellen           ###   ########.fr       */
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
		return 1;
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

void serveStaticFile(std::string path, int client_fd, const ServerConfig &config) {
	if (path.empty() || path == "/")
		path = "/" + config.index;
	std::string fullPath = config.root + path;
	std::ifstream file(fullPath.c_str());
	if (!file.is_open()) {
		std::cerr << "❌ Static file not found: " << fullPath << std::endl;
		std::string errorBody = getErrorPageBody(404, config);
		sendHtmlResponse(client_fd, 404, errorBody);
		return;
	}

	std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	// sendHtmlResponse(client_fd, 200, body);
	Response resp;
	std::string contentType = resp.getContentType(fullPath);
	std::string response = Response::build(200, body, contentType);
	ssize_t sent = send(client_fd, response.c_str(), response.size(), 0);
	if (sent != (ssize_t)response.size()) {
			std::cerr << "❌ Failed to send response for status code: " << sent << " of " << response.size() << " bytes\n";
	}
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
void handleUpload(const std::string &request, int client_fd, const ServerConfig &config) {
	// Find the filename first
	size_t filenamePos = request.find("filename=\"");
	if (filenamePos == std::string::npos) {
		sendHtmlResponse(client_fd, 500, getErrorPageBody(500, config));
		std::cerr << "❌ No filename found in request.\n";
		return;
	}

	size_t filenameStart = filenamePos + 10; // Length of "filename=\""
	size_t filenameEnd = request.find("\"", filenameStart);
	if (filenameEnd == std::string::npos) {
		sendHtmlResponse(client_fd, 500, getErrorPageBody(500, config));
		std::cerr << "❌ Invalid filename format.\n";
		return;
	}

	std::string filename = request.substr(filenameStart, filenameEnd - filenameStart);
	if (filename.empty()) {
		sendHtmlResponse(client_fd, 500, getErrorPageBody(500, config));
		std::cerr << "❌ Empty filename.\n";
		return;
	}
	std::cout << "Filename: " << filename << std::endl;

	// Find the double CRLF that marks the start of the file data
	// This is the most reliable marker - find the Content-Type header first,
	// then find the blank line that follows it
	size_t contentTypePos = request.find("Content-Type:", filenameEnd);
	if (contentTypePos == std::string::npos) {
		sendHtmlResponse(client_fd, 500, getErrorPageBody(500, config));
		std::cerr << "❌ No Content-Type header found for file part.\n";
		return;
	}

	size_t contentStartMarker = request.find("\r\n\r\n", contentTypePos);
	if (contentStartMarker == std::string::npos) {
		sendHtmlResponse(client_fd, 500, getErrorPageBody(500, config));
		std::cerr << "❌ Could not find blank line after Content-Type.\n";
		return;
	}

	// The actual file content starts immediately after the \r\n\r\n
	size_t contentStart = contentStartMarker + 4;

	// Extract the boundary string, handling both quoted and unquoted formats
	std::string rawBoundary = extractBoundary(request);
	if (rawBoundary.empty()) {
		std::cerr << "❌ Empty boundary string.\n";
		sendHtmlResponse(client_fd, 500, getErrorPageBody(500, config));
		return;
	}

	// The boundary in the content will be prefixed with "--"
	std::string boundary = "--" + rawBoundary;
	std::cout << "Boundary: " << boundary << std::endl;

	// Find the first occurrence of the boundary after the content start
	size_t contentEnd = request.find(boundary, contentStart);
	if (contentEnd == std::string::npos) {
		sendHtmlResponse(client_fd, 500, getErrorPageBody(500, config));
		std::cerr << "❌ Could not find closing boundary.\n";
		return;
	}

	// Check if there's a CRLF before the boundary and adjust for it
	if (contentEnd >= 2 && request[contentEnd - 1] == '\n' && request[contentEnd - 2] == '\r') {
			contentEnd -= 2; // Adjust to remove CRLF before boundary
	}

	// Extract the file content
	if (contentEnd <= contentStart) {
		sendHtmlResponse(client_fd, 500, getErrorPageBody(500, config));
		std::cerr << "❌ Invalid content boundaries (end <= start).\n";
		return;
	}

	// Use c_str() and careful binary handling to preserve the exact bytes
	size_t contentLength = contentEnd - contentStart;
	std::cout << "Content length: " << contentLength << " bytes" << std::endl;

	const char* fileData = request.c_str() + contentStart;

	// Create a temporary file for debugging/comparison
/*	std::string tempPath = "/tmp/upload_debug_" + filename;
	std::ofstream tempFile(tempPath.c_str(), std::ios::binary);
	if (tempFile.is_open()) {
		tempFile.write(fileData, contentLength);
		tempFile.close();
		std::cout << "Debug file saved to " << tempPath << std::endl;
	}*/

	// Save to final destination
	std::string filePath = config.root + "/upload/" + filename;
	std::ofstream outFile(filePath.c_str(), std::ios::binary);
	if (!outFile.is_open()) {
		sendHtmlResponse(client_fd, 500, getErrorPageBody(500, config));
		std::cerr << "❌ Could not open destination file: " << filePath << "\n";
		return;
	}

	outFile.write(fileData, contentLength);
	outFile.close();

	if (outFile.fail()) {
		sendHtmlResponse(client_fd, 500, getErrorPageBody(500, config));
		std::cerr << "❌ Error writing file.\n";
		return;
	}

	std::cout << "✅ File successfully saved: " << filePath << " (" << contentLength << " bytes)" << std::endl;

	// Send success response with proper charset
	// Create a response with a complete HTML5 document
	std::string successPath = config.root + "/templates/success.html";
	std::ifstream successFile(successPath.c_str());
	std::string body;

	if (successFile.is_open()) {
		body.assign((std::istreambuf_iterator<char>(successFile)),
					std::istreambuf_iterator<char>());
		successFile.close();
	}
	else
		body = "<html><body><h1>Upload successful</h1></body></html>";
	sendHtmlResponse(client_fd,  200, body);
}

void sendHtmlResponse(int fd, int code, const std::string& body) {

	std::string response = Response::build(code, body, "text/html");
	ssize_t sent = send(fd, response.c_str(), response.size(), 0);
	if (sent != (ssize_t)response.size()) {
		std::cerr << "❌ Failed to send response for status code: " << sent << " of " << response.size() << " bytes\n";
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
	// Remove from clients map
	std::map<int, ClientConnection*>::iterator it = clients.find(fd);
	if (it != clients.end()) {
		delete it->second;
		clients.erase(it);
	}

	// Close socket
	close(fd);

	// Remove from poll fds
	fds.erase(fds.begin() + i);
	--i;
}
