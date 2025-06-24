/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/12 03:08:22 by kellen            #+#    #+#             */
/*   Updated: 2025/06/24 00:47:10 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

int g_signal = -1;

// Signal handler for SIGINT and SIGTERM; sets global flag to initiate server shutdown.
void handleSignal(int signal) {
	if (signal == SIGINT || signal == SIGTERM) {
		std::cout << "\n🛑 Ctrl+C detected! Shutting down server! Cleaning up...\n";

		if (g_signal != -1) {
			//close(g_signal); // close the server socket!
			std::cout << "🔒 Server socket closed\n";
		}
		g_signal = 0;
		std::cout << "👋 Bye bye!\n";
		exit(0); // exit the program
	}
}

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

	std::cout << "💾 Fast file writing to: " << filePath << std::endl;

	// Create upload directory if needed
	std::string uploadDir = filePath.substr(0, filePath.find_last_of('/'));
	createDirectoryIfNotExists(uploadDir);

	// Open file for binary writing
	std::ofstream outFile(filePath.c_str(), std::ios::binary);
	if (!outFile.is_open()) {
		std::cerr << "❌ Could not open destination file: " << filePath << std::endl;
		return false;
	}

	// OPTIMIZED: Write data in chunks instead of all at once
	const size_t WRITE_CHUNK_SIZE = 64 * 1024; // 64KB chunks
	const char* fileData = request.c_str() + contentStart;
	size_t remainingBytes = contentLength;
	size_t bytesWritten = 0;

	std::cout << "📝 Writing file in " << WRITE_CHUNK_SIZE << " byte chunks..." << std::endl;

	while (remainingBytes > 0) {
		size_t chunkSize = std::min(WRITE_CHUNK_SIZE, remainingBytes);

		outFile.write(fileData + bytesWritten, chunkSize);
		if (outFile.fail()) {
			std::cerr << "❌ Error writing chunk at offset " << bytesWritten << std::endl;
			outFile.close();
			return false;
		}

		bytesWritten += chunkSize;
		remainingBytes -= chunkSize;

		// Optional: Show progress for very large files
		if (contentLength > 10 * 1024 * 1024) { // > 10MB
			double progress = (double)bytesWritten / contentLength * 100.0;
			// if ((int)progress % 10 == 0) { // Every 10%
			// 	std::cout << "📊 Write progress: " << (int)progress << "%" << std::endl;
			// }
			static int lastProgress = -1;
			int currentProgress = (int)progress / 10 * 10; // Round to nearest 10%
			if (currentProgress != lastProgress && currentProgress % 20 == 0) {
				std::cout << "📊 Write progress: " << currentProgress << "%" << std::endl;
				lastProgress = currentProgress;
			}
		}
	}

	outFile.close();
	// Get pointer to file data and write it
	// const char* fileData = request.c_str() + contentStart;
	// outFile.write(fileData, contentLength);
	// outFile.close();

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

void setupSignal() {
	struct sigaction sa;
	std::memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handleSignal;
	sa.sa_flags   = SA_RESTART;

	sigaction(SIGINT,  &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	signal(SIGPIPE, SIG_IGN);
}

void showUsage() {
	std::cout <<
	"\t__        __   _                            _       \n"
	"\t\\ \\      / /__| | ___ ___  _ __ ___   ___  | |_ ___ \n"
	"\t \\ \\ /\\ / / _ \\ |/ __/ _ \\| '_ ` _ \\ / _ \\ | __/ _ \\ \n"
	"\t  \\ V  V /  __/ | (_| (_) | | | | | |  __/ | || (_) |\n"
	"\t   \\_/\\_/ \\___|_|\\___\\___/|_| |_| |_|\\___|  \\__\\___/ \n"
	"\n"
	"			 Welcome to Our Webserv 🌐 " << std::endl;
	std::cout << "\t-----------------------------------------------------\n " << std::endl;

	std::cout << "\nUsage: ./webserv [config_file]\n" << std::endl;
	std::cout << "If no file is provided, we will use a default file.\n" << std::endl;

	std::cout << "Options:\n"
				<< "  -h, --help            Show this help message and exit\n" << std::endl;

	std::cout << "Accepted config file extensions:\n"
				<< "  .conf, .cfg, .config\n" << std::endl;

	std::cout << "Notes:\n"
				<< "  - The configuration file defines server blocks, ports, routes, etc.\n"
				<< "  - You can start the server with different configs to test behavior.\n"
				<< "  - To stop the server, use Ctrl+C (SIGINT).\n" << std::endl;

	std::cout << "Examples:\n"
				<< "  ./webserv                       # Start with default config\n"
				<< "  ./webserv my_config.conf        # Start with custom config\n"
				<< "  ./webserv --help                # Show this help message" << std::endl;
}

bool hasAllowedExtension(const std::string& filename) {
	const std::string extensions[] = {".conf", ".cfg", ".config"};
	size_t dot = filename.rfind('.');
	if (dot == std::string::npos)
		return (false);

	std::string ext = filename.substr(dot);
	for (size_t i = 0; i < sizeof(extensions)/sizeof(extensions[0]); ++i) {
		if (ext == extensions[i])
			return (true);
	}
	return (false);
}

bool parseArguments(int argc, char **argv, std::string &configPath) {

	if (argc == 1) {
		configPath = "conf/default.conf";
		std::cout << "\nNo config file provided, using default: " << configPath << std::endl;
		return (true);
	}

	if (argc == 2) {
		std::string rawArg = argv[1];
		std::string arg = trim(rawArg);
		if (arg == "-h" || arg == "--help") {
			showUsage();
			return (false);
		}

		if (arg.empty()) {
			std::cerr << "❌ Error: Config path cannot be empty or just spaces.\n";
			return false;
		}

		if (!hasAllowedExtension(arg)) {
			std::cerr << "⚠️  Warning: '" << arg << "' does not have a valid config extension (.conf, .cfg, .config)." << std::endl;
			return (false);
		}

		if (!fileExists(arg)) {
			std::cerr << "Error: File '" << arg << "' does not exist." << std::endl;
			return (false);
		}

		configPath = arg;
		return (true);
	}

	std::cerr << "Too many arguments! \nType -h or --help to see your options" << std::endl;
	// showUsage();
	return (false);
}

void artwelcom() {

	std::cout << "\n\n";
	std::cout <<
	"oooooo   oooooo     oooo          .o8        .oooooo..o\n"
	"`888.    `888.     .8'          \"888       d8P'    `Y8\n"
	" `888.   .8888.   .8'  .ooooo.   888oooo.  Y88bo.       .ooooo.  oooo d8b oooo    ooo\n"
	"  `888  .8'`888. .8'  d88' `88b  d88' `88b  `\"Y8888o.  d88' `88b `888\"\"8P  `88.  .8'\n"
	"   `888.8'  `888.8'   888ooo888  888   888      `\"Y88b 888ooo888  888       `88..8'\n"
	"    `888'    `888'    888    .o  888   888 oo     .d8P 888    .o  888        `888'\n"
	"     `8'      `8'     `Y8bod8P'  `Y8bod8P' 8\"\"88888P'  `Y8bod8P' d888b        `8'\n" << std::endl;

	std::cout << "\t\t	      Welcome to our Webserv in C++98 🚀\n"
				"\t\t	    Made with ❤️  by kbolon and keramos- 🌸\n" << std::endl;
	std::cout << "-----------------------------------------------------------------------------------------\n " << std::endl;
}