/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/12 03:08:22 by kellen            #+#    #+#             */
/*   Updated: 2025/06/12 17:23:01 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

/**
 * Extract filename from multipart form data
 */
bool extractFilenameFromRequest(const std::string& request, std::string& filename) {
	size_t filenamePos = request.find("filename=\"");
	if (filenamePos == std::string::npos) {
		std::cerr << "❌ No filename found in request" << std::endl;
		return false;
	}

	size_t filenameStart = filenamePos + 10; // Length of "filename=\""
	size_t filenameEnd = request.find("\"", filenameStart);
	if (filenameEnd == std::string::npos) {
		std::cerr << "❌ Invalid filename format" << std::endl;
		return false;
	}

	filename = request.substr(filenameStart, filenameEnd - filenameStart);
	if (filename.empty()) {
		std::cerr << "❌ Empty filename" << std::endl;
		return false;
	}

	return true;
}

/**
 * Find the start and end positions of file content in the request
 */
bool findFileContentBoundaries(const std::string& request, const std::string& filename,
							size_t& contentStart, size_t& contentEnd) {

	// Find Content-Type header after filename
	size_t filenamePos = request.find("filename=\"" + filename + "\"");
	size_t contentTypePos = request.find("Content-Type:", filenamePos);
	if (contentTypePos == std::string::npos) {
		std::cerr << "❌ No Content-Type header found" << std::endl;
		return false;
	}

	// Find start of file content (after double CRLF)
	size_t contentStartMarker = request.find("\r\n\r\n", contentTypePos);
	if (contentStartMarker == std::string::npos) {
		std::cerr << "❌ Could not find content start marker" << std::endl;
		return false;
	}
	contentStart = contentStartMarker + 4;

	// Extract boundary to find content end
	std::string rawBoundary = extractBoundary(request);
	if (rawBoundary.empty()) {
		std::cerr << "❌ Empty boundary string" << std::endl;
		return false;
	}
	std::string boundary = "--" + rawBoundary;

	// Find end of file content
	contentEnd = request.find(boundary, contentStart);
	if (contentEnd == std::string::npos) {
		std::cerr << "❌ Could not find closing boundary" << std::endl;
		return false;
	}

	// Adjust for CRLF before boundary
	if (contentEnd >= 2 &&
		request[contentEnd - 1] == '\n' &&
		request[contentEnd - 2] == '\r') {
		contentEnd -= 2;
	}

	// Validate boundaries
	if (contentEnd <= contentStart) {
		std::cerr << "❌ Invalid content boundaries (end <= start)" << std::endl;
		return false;
	}

	return true;
}

/**
 * Check if file size is within server limits
 */
bool validateUploadFileSize(size_t fileSize, const ServerConfig& config) {
	size_t maxSize = config.client_max_body_size;
	if (maxSize == 0) {
		// Choose one of these defaults:

		// Option 1: 50MB default (good for photos and documents)
		//maxSize = 50 * 1024 * 1024; // 50MB

		// Option 2: 100MB default (good for videos and large files)
		maxSize = 100 * 1024 * 1024; // 100MB
	}

	if (fileSize > maxSize) {
		std::cout << "⚠️ File size " << fileSize << " exceeds limit " << maxSize << std::endl;
		std::cout << "📊 File size: " << (fileSize / 1024 / 1024) << "MB, Limit: " << (maxSize / 1024 / 1024) << "MB" << std::endl;
		return false;
	}

	return true;
}

/**
 * Write file data to server filesystem
 */
bool writeFileToServer(const std::string& request, size_t contentStart, size_t contentLength,
					const std::string& filePath) {

	// Create upload directory if needed
	std::string uploadDir = filePath.substr(0, filePath.find_last_of('/'));
	createDirectoryIfNotExists(uploadDir);

	// Open file for binary writing
	std::ofstream outFile(filePath.c_str(), std::ios::binary);
	if (!outFile.is_open()) {
		std::cerr << "❌ Could not open destination file: " << filePath << std::endl;
		return false;
	}

	// Get pointer to file data and write it
	const char* fileData = request.c_str() + contentStart;
	outFile.write(fileData, contentLength);
	outFile.close();

	// Check for write errors
	if (outFile.fail()) {
		std::cerr << "❌ Error writing file data" << std::endl;
		return false;
	}

	return true;
}

/**
 * Load success template and process it with filename
 */
std::string loadAndProcessSuccessTemplate(const ServerConfig& config, const std::string& filename) {
	std::string successPath = config.root + "/templates/success.html";
	std::ifstream successFile(successPath.c_str());
	std::string templateContent;

	if (successFile.is_open()) {
		std::cout << "✅ Loading success template from: " << successPath << std::endl;

		// Read entire template file
		templateContent.assign((std::istreambuf_iterator<char>(successFile)),
							std::istreambuf_iterator<char>());
		successFile.close();

		// Process template with "uploaded" action
		replaceTemplateVariables(templateContent, filename, "uploaded");

	} else {
		std::cout << "⚠️ Success template not found at: " << successPath << std::endl;

		// Create simple fallback response
		std::ostringstream fallback;
		fallback << "<!DOCTYPE html>\n";
		fallback << "<html><head><title>Upload Successful</title>";
		fallback << "<link rel=\"stylesheet\" href=\"/style.css\"></head>\n";
		fallback << "<body><div class=\"container text-center\">\n";
		fallback << "<h1>✅ Upload Successful!</h1>\n";
		fallback << "<p>File <strong>" << filename << "</strong> uploaded successfully.</p>\n";
		fallback << "<div class=\"nav-buttons\">";
		fallback << "<a href=\"/upload\" class=\"nav-btn\">📸 Upload Another</a> ";
		fallback << "<a href=\"/\" class=\"nav-btn\">🏠 Go Home</a>";
		fallback << "</div></div></body></html>\n";
		templateContent = fallback.str();
	}

	return templateContent;
}

// For deletions (add this new function):
std::string loadAndProcessDeleteTemplate(const ServerConfig& config, const std::string& filename) {
	std::string successPath = config.root + "/templates/success.html";
	std::ifstream successFile(successPath.c_str());
	std::string templateContent;

	if (successFile.is_open()) {
		std::cout << "✅ Loading success template for deletion from: " << successPath << std::endl;

		templateContent.assign((std::istreambuf_iterator<char>(successFile)),
							std::istreambuf_iterator<char>());
		successFile.close();

		// Process template with "deleted" action
		replaceTemplateVariables(templateContent, filename, "deleted");

	} else {
		std::cout << "⚠️ Success template not found, using fallback for deletion" << std::endl;

		std::ostringstream fallback;
		fallback << "<!DOCTYPE html>\n";
		fallback << "<html><head><title>File Deleted</title>";
		fallback << "<link rel=\"stylesheet\" href=\"/style.css\"></head>\n";
		fallback << "<body><div class=\"container text-center\">\n";
		fallback << "<h1>🗑️ File Deleted Successfully!</h1>\n";
		fallback << "<p>File <strong>" << filename << "</strong> has been deleted.</p>\n";
		fallback << "<div class=\"nav-buttons\">";
		fallback << "<a href=\"/upload\" class=\"nav-btn\">📸 Upload New File</a> ";
		fallback << "<a href=\"/\" class=\"nav-btn\">🏠 Go Home</a>";
		fallback << "</div></div></body></html>\n";
		templateContent = fallback.str();
	}

	return templateContent;
}

/**
 * Replace template variables like {{action}} and {{filename}}
 */
void replaceTemplateVariables(std::string& templateContent, const std::string& filename, const std::string& action) {
	// Replace {{action}} with "uploaded"
	size_t pos = 0;
	while ((pos = templateContent.find("{{action}}", pos)) != std::string::npos) {
		templateContent.replace(pos, 10, action);
		pos += 8; // length of "uploaded"
	}

	// Replace {{filename}} with actual filename
	pos = 0;
	while ((pos = templateContent.find("{{filename}}", pos)) != std::string::npos) {
		templateContent.replace(pos, 12, filename);
		pos += filename.length();
	}

	std::cout << "✅ Template variables processed" << std::endl;
}