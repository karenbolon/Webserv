/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Method.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/11 23:13:55 by kellen            #+#    #+#             */
/*   Updated: 2025/06/12 17:09:02 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

void handleGet(int fd, const std::string& path, const LocationConfig& location, const ServerConfig& config) {
	std::cout << "📥 Handling GET request for " << path << std::endl;

	// Check if path is a directory and autoindex is enabled
	std::string fullPath = location.root + path;
	std::cout << "🔧 DEBUG: fullPath = '" << fullPath << "'" << std::endl;


	if (isDirectory(fullPath)) {
		std::cout << "📁 Path is directory" << std::endl;
		if (location.autoindex) {
			std::cout << "📁 Serving directory listing for " << path << std::endl;
			// For now, send a simple directory listing instead of complex function
			std::string body = generateSimpleDirectoryListing(fullPath, path);
			sendHtmlResponse(fd, 200, body);
		} else {
			// Try to serve index file
			std::string indexPath = fullPath + "/" + config.index;
			if (fileExists(indexPath)) {
				serveStaticFile(indexPath, fd, config);
			} else {
				std::cout << "❌ Directory access forbidden: " << path << std::endl;
				std::string body = getErrorPageBody(403, config);
				sendHtmlResponse(fd, 403, body);
			}
		}
	} else{
		// Serve static file
		std::cout << "📄 Calling serveStaticFile for: " << path << std::endl;
		serveStaticFile(path, fd, config);
		std::cout << "✅ serveStaticFile call completed" << std::endl;
	}
}

void handlePost(int fd, const Request& req, const std::string& path, const LocationConfig& location, const ServerConfig& config) {
	std::cout << "📤 Handling POST request for " << path << std::endl;

	(void)location; // Suppress unused warning, as location is not used in this example
	// Check if this is a file upload
	if (path == "/upload" || path.find("/upload") == 0) {
		// Simple file upload handling - you can expand this
		std::string rawRequest = req.getRawRequest();
		handleSimpleUpload(rawRequest, fd, config);
		return;
	}

	// Check if this is a CGI script
	if (path.find("/cgi-bin/") == 0) {
		// Simple CGI execution - you can expand this
		handleSimpleCGI(fd, req, path, config);
		return;
	}

	// Default POST handling
	std::string body = "POST request received for: " + path;
	sendHtmlResponse(fd, 200, body);
}

void handlePut(int fd, const Request& req, const std::string& path, const LocationConfig& location, const ServerConfig& config) {
	std::cout << "📝 Handling PUT request for " << path << std::endl;

	// Extract filename from path
	std::string filename = path;
	if (filename.find_last_of('/') != std::string::npos) {
		filename = filename.substr(filename.find_last_of('/') + 1);
	}

	// Construct full file path
	std::string uploadPath = location.upload_path;
	if (uploadPath.empty()) {
		uploadPath = "www/upload"; // Default upload directory
	}

	std::string fullPath = uploadPath + "/" + filename;

	// Validate file path (security check)
	if (filename.find("..") != std::string::npos || filename.find("/") != std::string::npos) {
		std::cout << "❌ Invalid filename in PUT request: " << filename << std::endl;
		std::string body = getErrorPageBody(400, config);
		sendHtmlResponse(fd, 400, body);
		return;
	}

	// Create upload directory if it doesn't exist
	createDirectoryIfNotExists(uploadPath);

	// Write file content
	std::string body = req.getBody();
	std::ofstream file(fullPath.c_str(), std::ios::binary);

	if (!file.is_open()) {
		std::cout << "❌ Cannot create file: " << fullPath << std::endl;
		std::string errorBody = getErrorPageBody(500, config);
		sendHtmlResponse(fd, 500, errorBody);
		return;
	}

	file.write(body.c_str(), body.size());
	file.close();

	std::cout << "✅ File uploaded via PUT: " << fullPath << " (" << body.size() << " bytes)" << std::endl;

	// Send success response
	std::string responseBody = "File uploaded successfully: " + filename;
	sendHtmlResponse(fd, 201, responseBody); // 201 Created
}

void handleDelete(int fd, const std::string& path, const LocationConfig& location, const ServerConfig& config) {
	std::cout << "🗑️ Handling DELETE request for " << path << std::endl;

	// Extract filename from path
	std::string filename = path;
	if (filename.find_last_of('/') != std::string::npos) {
		filename = filename.substr(filename.find_last_of('/') + 1);
	}

	// Construct full file path
	std::string uploadPath = location.upload_path;
	if (uploadPath.empty()) {
		uploadPath = "www/upload"; // Default upload directory
	}

	std::string fullPath = uploadPath + "/" + filename;

	// Validate file path (security check)
	if (filename.find("..") != std::string::npos || filename.find("/") != std::string::npos) {
		std::cout << "❌ Invalid filename in DELETE request: " << filename << std::endl;
		std::string body = getErrorPageBody(400, config);
		sendHtmlResponse(fd, 400, body);
		return;
	}

	// Check if file exists
	if (!fileExists(fullPath)) {
		std::cout << "❌ File not found for deletion: " << fullPath << std::endl;
		std::string body = getErrorPageBody(404, config);
		sendHtmlResponse(fd, 404, body);
		return;
	}

	// Delete the file
	if (std::remove(fullPath.c_str()) != 0) {
		std::cout << "❌ Failed to delete file: " << fullPath << std::endl;
		std::string body = getErrorPageBody(500, config);
		sendHtmlResponse(fd, 500, body);
		return;
	}

	std::cout << "✅ File deleted: " << fullPath << std::endl;

	// Send success response
	std::string responseBody = "File deleted successfully: " + filename;
	sendHtmlResponse(fd, 200, responseBody); // 200 OK
}

void handleHead(int fd, const std::string& path, const LocationConfig& location, const ServerConfig& config) {
	std::cout << "📋 Handling HEAD request for " << path << std::endl;
	//for now
	(void)config;  // Add this line to suppress warning

	// HEAD is like GET but without the response body
	std::string fullPath = location.root + path;

	if (!fileExists(fullPath)) {
		// Send 404 headers only (no body)
		std::string headers = Response::buildHeader(404, 0, "text/html");
		send(fd, headers.c_str(), headers.size(), 0);
		return;
	}

	// Get file info
	std::ifstream file(fullPath.c_str(), std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		std::string headers = Response::buildHeader(500, 0, "text/html");
		send(fd, headers.c_str(), headers.size(), 0);
		return;
	}

	size_t fileSize = file.tellg();
	file.close();

	// Determine content type
	std::string contentType = Response::getContentType(fullPath);

	// Send headers only (no body for HEAD request)
	std::string headers = Response::buildHeader(200, fileSize, contentType);
	send(fd, headers.c_str(), headers.size(), 0);

	std::cout << "✅ HEAD response sent for " << path << " (size: " << fileSize << ")" << std::endl;
}

// Helper functions

bool fileExists(const std::string& path) {
	std::ifstream file(path.c_str());
	return file.good();
}

bool isDirectory(const std::string& path) {
	struct stat statbuf;
	if (::stat(path.c_str(), &statbuf) != 0) {  // Use :: to call global stat function
		return false;
	}
	return S_ISDIR(statbuf.st_mode);
}

void createDirectoryIfNotExists(const std::string& path) {
	struct stat st;
	if (::stat(path.c_str(), &st) == -1) {  // Use :: to call global stat function
		::mkdir(path.c_str(), 0755);  // Use :: to call global mkdir function
	}
}

// // Simple header builder (replace with your existing one if you have it)
// std::string buildSimpleHeaders(int statusCode, size_t contentLength, const std::string& contentType) {
// 	std::ostringstream headers;
// 	headers << "HTTP/1.1 " << statusCode;

// 	switch (statusCode) {
// 		case 200: headers << " OK"; break;
// 		case 201: headers << " Created"; break;
// 		case 400: headers << " Bad Request"; break;
// 		case 403: headers << " Forbidden"; break;
// 		case 404: headers << " Not Found"; break;
// 		case 500: headers << " Internal Server Error"; break;
// 		default: headers << " Unknown"; break;
// 	}

// 	headers << "\r\n";
// 	headers << "Content-Length: " << contentLength << "\r\n";
// 	headers << "Content-Type: " << contentType << "\r\n";
// 	headers << "Connection: close\r\n";
// 	headers << "\r\n";

// 	return headers.str();
// }

// Simple directory listing
std::string generateSimpleDirectoryListing(const std::string& dirPath, const std::string& urlPath) {
	std::ostringstream html;

	html << "<!DOCTYPE html>\n";
	html << "<html><head><title>Directory: " << urlPath << "</title></head>\n";
	html << "<body><h1>Directory listing for " << urlPath << "</h1>\n";
	html << "<ul>\n";

	// Add parent directory link if not root
	if (urlPath != "/") {
		html << "<li><a href=\"../\">../</a></li>\n";
	}

	// List directory contents (simplified version)
	DIR* dir = opendir(dirPath.c_str());
	if (dir) {
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL) {
			std::string name = entry->d_name;
			if (name != "." && name != "..") {
				html << "<li><a href=\"" << name << "\">" << name << "</a></li>\n";
			}
		}
		closedir(dir);
	}

	html << "</ul></body></html>\n";
	return html.str();
}

std::string rewriteURL(const std::string& path, const ServerConfig& config, const std::string& method) {
	// Handle root path
	if (path == "/") {
		return "/" + config.index;
	}

	// For POST requests to /upload, don't rewrite
	if (method == "POST" && path == "/upload") {
		return path;
	}

	// URL mapping for clean URLs
	std::map<std::string, std::string> urlMap;
	urlMap["/home"] = "/index.html";
	urlMap["/gallery"] = "/gallery.html";
	urlMap["/upload"] = "/upload.html";
	urlMap["/interactive"] = "/interactive.html";
	urlMap["/cookies"] = "/cookie-demo.html";
	urlMap["/about"] = "/about.html";
	urlMap["/contact"] = "/contact.html";
	urlMap["/error"] = "/error/404.html";
	urlMap["/help"] = "/help.html";

	// Check if this is a clean URL that needs rewriting
	std::map<std::string, std::string>::const_iterator it = urlMap.find(path);
	if (it != urlMap.end()) {
		return it->second;
	}

	// Handle directory-style URLs (without trailing slash)
	std::string pathWithSlash = path + "/";
	it = urlMap.find(pathWithSlash);
	if (it != urlMap.end()) {
		return it->second;
	}

	// Check if file exists as-is (for files with extensions)
	std::string fullPath = config.root + path;
	if (fileExists(fullPath)) {
		return path; // File exists, use as-is
	}

	// Try adding .html extension
	std::string htmlPath = path + ".html";
	std::string fullHtmlPath = config.root + htmlPath;
	if (fileExists(fullHtmlPath)) {
		return htmlPath;
	}

	// Try as directory with index.html
	std::string indexPath = path + "/index.html";
	std::string fullIndexPath = config.root + indexPath;
	if (fileExists(fullIndexPath)) {
		return indexPath;
	}

	// Return original path if no rewrite rule found
	return path;
}

void handleSimpleUpload(const std::string& request, int client_fd, const ServerConfig& config) {
	std::cout << "🚀 Starting file upload process..." << std::endl;

	// Step 1: Extract filename
	std::string filename;
	if (!extractFilenameFromRequest(request, filename)) {
		std::cerr << "❌ Failed to extract filename" << std::endl;
		sendHtmlResponse(client_fd, 400, getErrorPageBody(400, config));
		return;
	}
	std::cout << "📁 Extracted filename: " << filename << std::endl;

	// Step 2: Find content boundaries
	size_t contentStart, contentEnd;
	if (!findFileContentBoundaries(request, filename, contentStart, contentEnd)) {
		std::cerr << "❌ Failed to find content boundaries" << std::endl;
		sendHtmlResponse(client_fd, 400, getErrorPageBody(400, config));
		return;
	}

	size_t contentLength = contentEnd - contentStart;
	std::cout << "📏 Content length: " << contentLength << " bytes" << std::endl;

	// Step 3: Validate file size
	if (!validateUploadFileSize(contentLength, config)) {
		std::cerr << "❌ File too large: " << contentLength << " bytes" << std::endl;
		sendHtmlResponse(client_fd, 413, getErrorPageBody(413, config));
		return;
	}
	std::cout << "✅ File size validation passed" << std::endl;

	// Step 4: Save file to server
	std::string filePath = config.root + "/upload/" + filename;
	if (!writeFileToServer(request, contentStart, contentLength, filePath)) {
		std::cerr << "❌ Failed to save file to: " << filePath << std::endl;
		sendHtmlResponse(client_fd, 500, getErrorPageBody(500, config));
		return;
	}
	std::cout << "✅ File saved successfully: " << filePath << std::endl;

	// Step 5: Send success response
	std::string successResponse = loadAndProcessSuccessTemplate(config, filename);
	sendHtmlResponse(client_fd, 200, successResponse);
	std::cout << "📤 Success response sent!" << std::endl;
}

void handleSimpleCGI(int fd, const Request& req, const std::string& path, const ServerConfig& config) {
	std::string method = req.getMethod();
	std::string query = req.getQuery();
	std::string body = req.getBody();

	// Simple CGI response
	std::ostringstream response;
	response << "<!DOCTYPE html>\n";
	response << "<html><head><title>CGI Response</title></head>\n";
	response << "<body>\n";
	response << "<h1>CGI Script Execution</h1>\n";
	response << "<p><strong>Script:</strong> " << path << "</p>\n";
	response << "<p><strong>Method:</strong> " << method << "</p>\n";

	if (!query.empty()) {
		response << "<p><strong>Query:</strong> " << query << "</p>\n";
	}

	if (!body.empty()) {
		response << "<p><strong>Body Size:</strong> " << body.size() << " bytes</p>\n";
	}

	response << "<p><strong>Server:</strong> " << config.server_name << "</p>\n";
	response << "<p><em>CGI functionality is simplified for this demo.</em></p>\n";
	response << "</body></html>\n";

	sendHtmlResponse(fd, 200, response.str());

	std::cout << "✅ CGI response sent for " << path << std::endl;
}