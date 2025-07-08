/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Method.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/11 23:13:55 by kellen            #+#    #+#             */
/*   Updated: 2025/07/08 01:32:55 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

bool handleGet(ClientConnection* client, std::vector<struct pollfd>& fds, const Request& req, 
	const std::string& path, const LocationConfig& location, const ServerConfig& config) {
	
	if (path == "/appspecific/com.chrome.devtools.json" || path == "/.well-known/appspecific/com.chrome.devtools.json") {
		//ignore the request
		std::string response = "HTTP/1.1 204 No Content\r\nConnection: close\r\n\r\n";;
		client->setPendingResponse(response);
		for (size_t j = 0; j < fds.size(); ++j) {
			if (fds[j].fd == client->getFd())
				fds[j].events |= POLLOUT;
		}
		return true;
	}
	std::cout << "📥 Handling GET request for " << path << std::endl;
	
	// First: Check if this is a CGI request FIRST (highest priority)
	if (path.find("/cgi-bin/") == 0) {

		// Call our improved handleSimpleCGI function
		if (!handleSimpleCGI(client, fds, req, path, config)) {
			return false;
		}
		return true;
	}
	
	// NEW: Handle API endpoint for listing uploaded files
	if (path == "/api/photos" || path == "/api/files") {
		std::string uploadDir = location.upload_path.empty() ? "www/upload" : location.upload_path;
		std::string jsonResponse = generateJsonDirectoryListing(uploadDir);

		// Send JSON response
		std::string response = Response::build(200, jsonResponse, "application/json");
		client->setPendingResponse(response);
		for (size_t j = 0; j < fds.size(); ++j) {
			if (fds[j].fd == client->getFd())
				fds[j].events |= POLLOUT;
		}
		return true;
	}
	// Add this at the start of handleGet
	std::string effectivePath = path;
	if (effectivePath == "/")
   		effectivePath = "/" + config.index; // config.index should be "index.html"
	// Check if path is a directory and autoindex is enabled
	std::string fullPath = location.root + effectivePath;
	
	if (fileExists(fullPath)) {
   		serveStaticFile(effectivePath, config, client, fds);
   		return true;
	}
	if (isDirectory(fullPath)) {
		std::cout << "📁 Path is directory" << std::endl;
		if (location.autoindex) {
			//std::cout << "📁 Serving directory listing for " << path << std::endl;
			// For now, send a simple directory listing instead of complex function
			std::string body = generateSimpleDirectoryListing(fullPath, path);
			std::string response = Response::build(200, body, "text/html");
			client->setPendingResponse(response);
			for (size_t j = 0; j < fds.size(); ++j) {
				if (fds[j].fd == client->getFd())
					fds[j].events |= POLLOUT;
			}
			return true;

		} else {
			// Try to serve index file
			std::string indexPath = fullPath + "/" + config.index;
			if (fileExists(indexPath)) {
				serveStaticFile(indexPath, config, client, fds);
				return true;
			} else {
				//std::cout << "❌ Directory access forbidden: " << path << std::endl;
				std::string body = getErrorPageBody(403, config);
				sendHtmlResponse(403, body, client, fds);
				return false;
			}
		}
	}
	//not found
	std::string body = getErrorPageBody(404, config);
	sendHtmlResponse(404, body, client, fds);
	return false;
}

bool handlePost(ClientConnection* client, std::vector<struct pollfd>& fds, const Request& req, const std::string& path, const LocationConfig& location, const ServerConfig& config) {
	std::cout << "📤 Handling POST request for " << path << std::endl;

	(void)location; // Suppress unused warning, as location is not used in this example
	if (req.getBody().size() > static_cast<size_t>(config.client_max_body_size)) {
		std::cerr << "❌ Request body exceeds client_max_body_size\n";
		std::string errorBody = getErrorPageBody(413, config); // 413 Payload Too Large
		sendHtmlResponse(413, errorBody, client, fds);
		return false;
	}
	// Check if this is a file upload
	if (path == "/upload" || path.find("/upload") == 0) {
		std::cout << "📁 This is a file upload request" << std::endl;
		// Simple file upload handling - you can expand this
		std::string rawRequest = req.getRawRequest();
		handleSimpleUpload(rawRequest, config, client, fds);
		return true;
	}

	// Check if this is a CGI script
	if (path.find("/cgi-bin/") == 0) {
		std::cout << "🔧 This is a CGI POST request, calling handleSimpleCGI" << std::endl;
		// Simple CGI execution - you can expand this
		if (!handleSimpleCGI(client, fds, req, path, config)){
			return false;
		}
	}

	// Default POST handling
	// std::string body = "POST request received for: " + path;

	std::cout << "📝 Default POST handling for: " << path << std::endl;
	std::string body = "<html><body><h1>POST Request Received</h1>";
	body += "<p>Path: " + path + "</p>";
	body += "<p>Method: POST</p>";

	// Show some request info
	std::string requestBody = req.getBody();
	if (!requestBody.empty()) {
		body += "<p>Body size: " + intToStr(requestBody.size()) + " bytes</p>";
	}

	body += "<p><a href='/'>← Back to Home</a></p>";
	body += "</body></html>";
	sendHtmlResponse(200, body, client, fds);
	return true;
}

void handlePut(const Request& req, const std::string& path, const LocationConfig& location, const ServerConfig& config,
			ClientConnection* client, std::vector<struct pollfd>& fds) {
	std::cout << "📝 Handling PUT request for " << path << std::endl;

	// Check if this is a rename operation (look for X-Rename-To header)
	const std::map<std::string, std::string>& headers = req.getHeaders();
	std::map<std::string, std::string>::const_iterator renameIt = headers.find("x-rename-to");

	if (renameIt != headers.end() && !renameIt->second.empty()) {
		// This is a RENAME operation
		handleFileRename(path, renameIt->second, location, config, client, fds);
		return;
	}

	// This is a regular UPLOAD operation
	handleFileUpload(req, path, location, config, client, fds);
}

// NEW FUNCTION: Handle file renaming
void handleFileRename(const std::string& path, const std::string& newName,
					const LocationConfig& location, const ServerConfig& config,
					ClientConnection* client, std::vector<struct pollfd>& fds) {
	std::cout << "🏷️ Handling file rename operation" << std::endl;

	// Extract current filename from path
	std::string currentFilename = path;
	if (currentFilename.find_last_of('/') != std::string::npos) {
		currentFilename = currentFilename.substr(currentFilename.find_last_of('/') + 1);
	}

	// Validate new filename
	if (newName.empty()) {
		std::cout << "❌ New filename is empty" << std::endl;
		std::string body = getErrorPageBody(400, config);
		sendHtmlResponse(400, body, client, fds);
		return;
	}

	// Security validation for new filename
	if (newName.find("..") != std::string::npos ||
		newName.find("/") != std::string::npos ||
		newName.find("\\") != std::string::npos) {
		std::cout << "❌ Invalid characters in new filename: " << newName << std::endl;
		std::string body = getErrorPageBody(400, config);
		sendHtmlResponse(400, body, client, fds);
		return;
	}

	// Construct file paths
	std::string uploadPath = location.upload_path;
	if (uploadPath.empty()) {
		uploadPath = "www/upload";
	}

	std::string currentPath = uploadPath + "/" + currentFilename;
	std::string newPath = uploadPath + "/" + newName;

	// Check if current file exists
	if (!fileExists(currentPath)) {
		std::cout << "❌ File not found for rename: " << currentPath << std::endl;
		std::string body = getErrorPageBody(404, config);
		sendHtmlResponse(404, body, client, fds);
		return;
	}

	// Check if new filename already exists (prevent overwriting)
	if (fileExists(newPath) && newPath != currentPath) {
		std::cout << "❌ File already exists with new name: " << newPath << std::endl;
		std::string body = "Error: A file with that name already exists";
		sendHtmlResponse(409, body, client, fds); // 409 Conflict
		return;
	}

	// Perform the rename operation
	if (std::rename(currentPath.c_str(), newPath.c_str()) == 0) {
		std::cout << "✅ File renamed successfully: " << currentFilename << " → " << newName << std::endl;

		// Send success response
		std::string responseBody = "File renamed successfully: " + currentFilename + " → " + newName;
		sendHtmlResponse(200, responseBody, client, fds);
	} else {
		std::cout << "❌ Failed to rename file"<< std::endl;
		std::string body = getErrorPageBody(500, config);
		sendHtmlResponse(500, body, client, fds);
	}
}

// NEW FUNCTION: Handle regular file upload (your existing logic)
void handleFileUpload(const Request& req, const std::string& path,
		const LocationConfig& location, const ServerConfig& config,
		ClientConnection* client, std::vector<struct pollfd>& fds) {
	std::cout << "📁 Handling file upload operation" << std::endl;

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
		sendHtmlResponse(400, body, client, fds);
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
		sendHtmlResponse(500, errorBody, client, fds);
		return;
	}

	file.write(body.c_str(), body.size());
	if (file.fail()) {
		std::cerr << "❌ Error writing handleFileUpload" << std::endl;
		file.close();
		std::cerr << "in MEthod function close\n";
		return;
	}
	file.close();
	std::cerr << "in method function close 2\n";

	// Check if file was written successfully
	if (!fileExists(fullPath)) {
		std::cout << "❌ File was not created successfully: " << fullPath << std::endl;
		std::string errorBody = getErrorPageBody(500, config);
		sendHtmlResponse(500, errorBody, client, fds);
		return;
	}

	std::cout << "✅ File uploaded via PUT: " << fullPath << " (" << body.size() << " bytes)" << std::endl;

	// Send success response
	std::string responseBody = "File uploaded successfully: " + filename;
	sendHtmlResponse(201, responseBody, client, fds); // 201 Created
}

void handleDelete(const std::string& path, const LocationConfig& location, const ServerConfig& config,
	ClientConnection* client, std::vector<struct pollfd>& fds) {
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
		sendHtmlResponse(400, body, client, fds);
		return;
	}

	// Check if file exists
	if (!fileExists(fullPath)) {
		std::cout << "❌ File not found for deletion: " << fullPath << std::endl;
		std::string body = getErrorPageBody(404, config);
		sendHtmlResponse(404, body, client, fds);
		return;
	}

	// Delete the file
	if (std::remove(fullPath.c_str()) != 0) {
		std::cout << "❌ Failed to delete file: " << fullPath << std::endl;
		std::string body = getErrorPageBody(500, config);
		sendHtmlResponse(500, body, client, fds);
		return;
	}

	std::cout << "✅ File deleted: " << fullPath << std::endl;

	// Send success response
	std::string responseBody = "File deleted successfully: " + filename;
	sendHtmlResponse(200, responseBody, client, fds); // 200 OK
}

bool handleHead(int fd, const std::string& path, const LocationConfig& location, const ServerConfig& config, ClientConnection* client, std::vector<struct pollfd>& fds) {
	std::cout << "📋 Handling HEAD request for " << path << std::endl;
	//for now
	(void)config;  // Add this line to suppress warning

	// HEAD is like GET but without the response body
	std::string fullPath = location.root + path;
	std::cout << "DEBUG: fullPath = '" << fullPath << "'" << std::endl;

	if (!fileExists(fullPath)) {
		// Send 404 headers only (no body)
		std::string headers = Response::buildHeader(404, 0, "text/html");
		client->setPendingResponse(headers);
		for (size_t j = 0; j < fds.size(); ++j) {
			if (fds[j].fd == fd)
				fds[j].events |= POLLOUT;
		}

		return true;
	}

	// Get file info
	std::ifstream file(fullPath.c_str(), std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		std::string headers = Response::buildHeader(500, 0, "text/html");
		client->setPendingResponse(headers);
		for (size_t j = 0; j < fds.size(); ++j) {
			if (fds[j].fd == fd)
				fds[j].events |= POLLOUT;
		}
		return true;
	}

	size_t fileSize = file.tellg();
	file.close();
	std::cerr << "in method function close 3\n";

	// Determine content type
	std::string contentType = Response::getContentType(fullPath);

	// Send headers only (no body for HEAD request)
	std::string headers = Response::buildHeader(200, fileSize, contentType);
	client->setPendingResponse(headers);
	for (size_t j = 0; j < fds.size(); ++j) {
		if (fds[j].fd == fd)
			fds[j].events |= POLLOUT;
	}
	return true;
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
	if (stat(path.c_str(), &st) == -1) {
		// Directory doesn't exist, create it
		if (mkdir(path.c_str(), 0755) == 0) {
			std::cout << "📁 Created directory: " << path << std::endl;
		} else {
			std::cout << "❌ Failed to create directory: " << path << std::endl;
		}
	}
}

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

void handleSimpleUpload(const std::string& request, const ServerConfig& config, ClientConnection* client, std::vector<struct pollfd>& fds) {

	// Step 1: Extract boundary
	std::string boundary = extractBoundary(request);
	if (boundary.empty()) {
		std::cerr << "❌ No boundary found in request" << std::endl;
		sendHtmlResponse(400, getErrorPageBody(400, config), client, fds);
		return;
	}

	// Step 2: Process all files using YOUR existing functions
	std::vector<std::string> successfulUploads;
	std::vector<std::string> failedUploads;

	size_t currentPos = 0;
	int fileCount = 0;

	while (true) {
		//find next file section
		size_t sectionStart = findNextFileSection(request, boundary, currentPos);
		if (sectionStart == std::string::npos) {
			break; // No more files found
		}

		fileCount++;

		// Find the end of this section (next boundary)
		std::string fullBoundary = "--" + boundary;
		size_t sectionEnd = request.find(fullBoundary, sectionStart);
		if (sectionEnd == std::string::npos) {
			sectionEnd = request.length();
		}

		//extract filename
		std::string filename;
		if (!extractFilenameFromSection(request, sectionStart, sectionEnd, filename)) {
			std::cerr << "❌ Could not extract filename from section " << fileCount << std::endl;
			currentPos = sectionEnd;
			continue;
		}

		//find content boundaries
		size_t contentStart, contentLength;
		if (!findFileContentInSection(request, sectionStart, sectionEnd, contentStart, contentLength)) {
			std::cerr << "❌ Could not find content boundaries for: " << filename << std::endl;
			failedUploads.push_back(filename);
			currentPos = sectionEnd;
			continue;
		}

		// Step 3: Validate file size
		if (!validateUploadFileSize(contentLength, config)) {
			std::cerr << "❌ File too large: " << filename << " (" << contentLength << " bytes)" << std::endl;
			failedUploads.push_back(filename);
			currentPos = sectionEnd;
			continue;
		}

		// Step 4: Save the file
		std::string filePath = config.root + "/upload/" + filename;
		if (writeFileToServer(request, contentStart, contentLength, filePath)) {
			std::cout << "✅ Successfully uploaded: " << filename << std::endl;
			successfulUploads.push_back(filename);
		} else {
			std::cerr << "❌ Failed to save: " << filename << std::endl;
			failedUploads.push_back(filename);
		}

		// Move to next section
		currentPos = sectionEnd;
	}

	// Step 5: Response system
	if (successfulUploads.empty() && failedUploads.empty()) {
		std::cerr << "❌ No files found in request" << std::endl;
		sendHtmlResponse(400, getErrorPageBody(400, config), client, fds);
		return;
	}

	if (failedUploads.empty()) {
		// All files uploaded successfully
		std::string filename = (successfulUploads.size() == 1) ?
							successfulUploads[0] :
							(intToStr(successfulUploads.size()) + " files");

		std::string successResponse = loadAndProcessSuccessTemplate(config, filename);
		sendHtmlResponse(200, successResponse, client, fds);
		std::cout << "📤 All " << successfulUploads.size() << " files uploaded successfully!" << std::endl;
	} else {
		// Some or all files failed - error system
		std::string errorBody = getErrorPageBody(400, config);
		sendHtmlResponse(400, errorBody, client, fds);
		std::cout << "❌ Upload failed for " << failedUploads.size() << " files" << std::endl;
	}
}
