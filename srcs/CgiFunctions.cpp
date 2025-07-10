/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiFunctions.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 15:38:46 by kbolon            #+#    #+#             */
/*   Updated: 2025/07/10 14:45:25 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

/*
find the script/language interpreter by lopping through all config.locations
check that the script extension matches

To Run CGI for testing, once ./webserv is running, in another terminal:
curl http://localhost:8081/cgi-bin/hello.py
*/
std::string getInterpreter(const std::string& path, const ServerConfig& config) {
	for (size_t i = 0; i < config.locations.size(); ++i) {
		const std::map<std::string, std::string>& cgiMap = config.locations[i].cgi_paths;
		for (std::map<std::string, std::string>::const_iterator it = cgiMap.begin(); it != cgiMap.end(); ++it) {
			// only match file extension in extension (so no py.backup is used as an example)
			// checks if path is shorter than ext, then checks if end of path string matches the extension exactly (path.compare(startIndex, lengthToCompare, stringToMatch))
			if (path.length() >= it->first.length() && path.compare(path.length() - it->first.length(), it->first.length(), it->first) == 0)
				return it->second;
		}
	}
	return "";
}

const LocationConfig* findMatchingLocation(const std::string& path, const ServerConfig& config) {
	for (size_t i = 0; i < config.locations.size(); ++i) {
		if (path.find(config.locations[i].path) == 0) {
			return &config.locations[i];
		}
	}
	return NULL;
}


bool handleSimpleCGI(ClientConnection* client, std::vector<struct pollfd>& fds, const Request& req, const std::string& path, const ServerConfig& config) {
//	std::cout << "🚀 Starting Simple CGI execution for: " << path << std::endl;

	// Step 1: Find the interpreter for this script
	std::string interpreter = getInterpreter(path, config);
	if (interpreter.empty()) {
		std::cerr << "❌ No interpreter found for " << path << std::endl;
		std::string errorBody = getErrorPageBody(500, config);
		sendHtmlResponse(500, errorBody, client, fds);
		return false;
	}

	// Step 2: Build the full path to the script
	std::string scriptPath = config.root + path;

	// Remove query string from script path if present
	size_t queryPos = scriptPath.find('?');
	if (queryPos != std::string::npos) {
		scriptPath = scriptPath.substr(0, queryPos);
	}

	// Step 3: Check if the script file exists
	if (!fileExists(scriptPath)) {
		std::cerr << "❌ Script file not found: " << scriptPath << std::endl;
		std::string errorBody = getErrorPageBody(404, config);
		sendHtmlResponse(404, errorBody, client, fds);
		return false;
	}

	if (access(scriptPath.c_str(), X_OK) != 0) {
		std::cerr << "⚠️ Script may not be executable, but continuing..." << std::endl;
	}

	// Step 4: Execute the script and capture output
	if (!setUpCgi(client, fds, interpreter, scriptPath, req)) {
		std::cerr << "❌ Failed to set up CGI execution for: " << scriptPath << std:: endl;
		std::string errorBody = getErrorPageBody(500, config);
		client->setPendingResponse(errorBody);
		for (size_t j = 0; j < fds.size(); ++j) {
			if (fds[j].fd == client->getFd())
				fds[j].events |= POLLOUT;
		}
		return false;
	}

	return true;
}

bool setUpCgi(ClientConnection* client, std::vector<struct pollfd>& fds,
			  const std::string& interpreter, const std::string& scriptPath,
			  const Request& req) {

	int outputPipe[2];
	int inputPipe[2];

	if (pipe(outputPipe) == -1 || pipe(inputPipe) == -1) {
		std::cerr << "❌ Failed to create pipes" << std::endl;
		return false;
	}

	pid_t pid = fork();
	if (pid < 0) {
		std::cerr << "❌ Fork failed" << std::endl;
		close(outputPipe[0]); close(outputPipe[1]);
		close(inputPipe[0]); close(inputPipe[1]);
		return false;
	}

	if (pid == 0) {
		// Child process

		dup2(inputPipe[0], STDIN_FILENO);
		dup2(outputPipe[1], STDOUT_FILENO);

		close(outputPipe[0]); close(outputPipe[1]);
		close(inputPipe[0]); close(inputPipe[1]);

		std::vector<std::string> envStrings;
		envStrings.push_back("REQUEST_METHOD=" + req.getMethod());
		envStrings.push_back("QUERY_STRING=" + req.getQuery());
		envStrings.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
		envStrings.push_back("CONTENT_LENGTH=" + intToStr(req.getBody().length()));
		envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");
		envStrings.push_back("SERVER_PROTOCOL=HTTP/1.1");
		envStrings.push_back("SCRIPT_NAME=" + scriptPath);
		if (scriptPath.find(".php") != std::string::npos) {
			envStrings.push_back("SCRIPT_FILENAME=" + scriptPath);
			envStrings.push_back("REDIRECT_STATUS=200");
		}

		const std::map<std::string, std::string>& headers = req.getHeaders();
		for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
			std::string httpVar = "HTTP_";
			for (size_t i = 0; i < it->first.length(); ++i) {
				char c = it->first[i];
				httpVar += (c == '-') ? '_' : std::toupper(static_cast<unsigned char>(c));
			}
			envStrings.push_back(httpVar + "=" + it->second);
		}

		std::vector<char*> envp;
		for (size_t i = 0; i < envStrings.size(); ++i) {
			envp.push_back(const_cast<char*>(envStrings[i].c_str()));
		}
		envp.push_back(NULL);

		char* args[] = {
			const_cast<char*>(interpreter.c_str()),
			const_cast<char*>(scriptPath.c_str()),
			NULL
		};

		execve(interpreter.c_str(), args, &envp[0]);
		_exit(1);
	} else {
		// Parent process
		close(inputPipe[0]);
		close(outputPipe[1]);

		fcntl(inputPipe[1], F_SETFL, O_NONBLOCK);
		fcntl(outputPipe[0], F_SETFL, O_NONBLOCK);

		client->setCgiFds(inputPipe[1], outputPipe[0]);
		client->setCgiPid(pid);
		client->markCgiRunning(); //start CGI timer
		client->setCgiStartTime(time(NULL));

		struct pollfd outPoll = {outputPipe[0], POLLIN, 0};
		fds.push_back(outPoll);
		
		//only write to CGI input for POST
		if (req.getMethod() == "POST") {
			client->setCgiInputBuffer(req.getBody());
			client->setCgiFds(inputPipe[1], outputPipe[0]);
			struct pollfd inPoll = {inputPipe[1], POLLOUT, 0};
			fds.push_back(inPoll);
		}
		else {
			//we won't write anything and close the inputPipe[1]
			//struct pollfd inPoll = {inputPipe[1], POLLOUT, 0};
			//fds.push_back(inPoll);
			shutdown(inputPipe[1], SHUT_WR); //instead of close() as this causes the FD to close before child is done
			close(inputPipe[1]);
			client->setCgiFds(-1, outputPipe[0]);
		}
	}
	return true;
}

// Helper function to format CGI output as HTTP response
std::string formatCGIResponse(const std::string& scriptOutput) {
	if (scriptOutput.empty()) {
		return "HTTP/1.1 500 Internal Server Error\r\n"
			   "Content-Type: text/html\r\n"
			   "Content-Length: 49\r\n"
			   "Connection: close\r\n"
			   "\r\n"
			   "<html><body><h1>500 Internal Server Error</h1></body></html>";
	}

//	std::cout << "📋 Formatting CGI response (" << scriptOutput.size() << " bytes)" << std::endl;

	// Check if the script already included HTTP headers
	size_t headerEnd = scriptOutput.find("\r\n\r\n");
	if (headerEnd != std::string::npos) {
		// Check if script provided its own status
		size_t statusPos = scriptOutput.find("Status:");
		if (statusPos != std::string::npos && statusPos < headerEnd) {
			// Extract status code from "Status: 418 I'm a teapot"
			size_t statusStart = statusPos + 7; // "Status:" length
			size_t statusEnd = scriptOutput.find("\r\n", statusStart);
			if (statusEnd != std::string::npos) {
				std::string statusLine = scriptOutput.substr(statusStart, statusEnd - statusStart);
				// Remove the Status: line from output and add proper HTTP status
				std::string cleanedOutput = scriptOutput;
				cleanedOutput.erase(statusPos, statusEnd - statusPos + 2); // +2 for \r\n

				return "HTTP/1.1 " + statusLine + "\r\n" + cleanedOutput;
			}
		}

		// Script provided headers but no status - default to 200
		if (scriptOutput.find("Content-Type:") < headerEnd) {
			return "HTTP/1.1 200 OK\r\n" + scriptOutput;
		}
	}

	// Script didn't provide headers, add them
	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Type: text/html\r\n";
	response << "Content-Length: " << scriptOutput.size() << "\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	response << scriptOutput;
	return response.str();
}
