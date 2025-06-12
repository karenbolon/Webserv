/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiFunctions.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 15:38:46 by kbolon            #+#    #+#             */
/*   Updated: 2025/06/12 00:22:58 by kellen           ###   ########.fr       */
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

/*
launching an external interpreter (i.e. Python or PHP) to run a script
and send the output to the client

WebServ acts like a middleman
	-gets input (i.e. POST data) to the script
	-gets the output (i.e.HTML) back
	-forward it to the browser
*/
void handleCgi(const Request req, int fd, const ServerConfig& config, std::string interpreter) {
	const LocationConfig* location = findMatchingLocation(req.getPath(), config);
	if (!location) {
		std::cerr << "❌ Location for CGI request does not match\n";
		return;
	}
	std::string fullPath = req.getPath();
	size_t qm = fullPath.find('?');
	if (qm != std::string::npos)
		fullPath = fullPath.substr(0, qm);

	std::string relativePath = fullPath.substr(location->path.length());

	std::string scriptPath = location->root;
	if (!scriptPath.empty() && scriptPath[scriptPath.size() - 1] != '/')
		scriptPath += '/';
	scriptPath += relativePath;
//	std::string relativePath = req.getPath().substr(location->path.length()); // e.g. "/hello.py"
//	std::string scriptPath = location->root;
//	if (!scriptPath.empty() && scriptPath[scriptPath.size() - 1] != '/')
//		scriptPath += '/';
//	scriptPath += relativePath; // e.g. ./cgi-bin/hello.py
	std::cout << "👣 Running CGI script: " << scriptPath << " with " << interpreter << std::endl;
	int	inputPipe[2];
	int	outputPipe[2];
	if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1) {
		std::cerr << "❌ Failed to create pipes\n";
		return;
	}
	pid_t pid = fork();
	if (pid < 0) {
		std::cerr << "❌ Fork failed\n";
		return;
	}
	else if (pid == 0) {
		//child process
		dup2(inputPipe[0], STDIN_FILENO);
		dup2(outputPipe[1], STDOUT_FILENO);
		close(inputPipe[1]);
		close(outputPipe[0]);

		//prepare list of executable paths (/usr/bin/python3) & script (/www.cgi-bin/script.py)
		char* pathToInterpreterAndScript[] = {
			const_cast<char*>(interpreter.c_str()),
			const_cast<char*>(scriptPath.c_str()),
			NULL
		};

		//prepare small env
		std::string method = req.getMethod();
		std::string pathInfo = req.getPath();
		std::ostringstream oss;
		oss << req.getBody().length();
		std::string contentLengthStr = oss.str();

		//make "CGI headers but passed through execve() instead of HTTP stream"
		//pre-set values or ENV variables the CGI uses when running the script
		//must convert to string for execve
		std::vector<std::string> envStrings;
		envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");
		envStrings.push_back("SERVER_PROTOCOL=HTTP/1.1");
		envStrings.push_back("REQUEST_METHOD=" + method);
		envStrings.push_back("PATH_INFO=" + pathInfo);
		envStrings.push_back("SCRIPT_NAME=" + scriptPath);
		envStrings.push_back("CONTENT_LENGTH=" + contentLengthStr);
		envStrings.push_back("CONTENT_TYPE=text/plain");
		envStrings.push_back("QUERY_STRING=" + req.getQuery());
		envStrings.push_back("SCRIPT_NAME=" + relativePath);

		std::vector<char*> envp;
		for (size_t i = 0; i < envStrings.size(); ++i)
			envp.push_back(const_cast<char*>(envStrings[i].c_str()));
		envp.push_back(NULL);

		execve(pathToInterpreterAndScript[0], pathToInterpreterAndScript, &envp[0]);
		std::cerr << "❌ execve failed: " << strerror(errno) << std::endl;
		exit(1);
	}
	else {
		//parent process
		close(inputPipe[0]);
		close(outputPipe[1]);

		//send body to CGI if POST method
		if (req.getMethod() == "POST")
			write(inputPipe[1], req.getBody().c_str(), req.getBody().length());
		close(inputPipe[1]);

		//read from CGI output and send to client
		char buffer[4096];
		ssize_t bytes;
		std::ostringstream response;

		while((bytes = read(outputPipe[0], buffer, sizeof(buffer))) > 0)
			response.write(buffer, bytes);
		close(outputPipe[0]);
		waitpid(pid, NULL, 0); //wait for child
		std::string body = response.str();
		size_t headerEnd = body.find("\r\n\r\n");
		if (headerEnd != std::string::npos)
			body = body.substr(headerEnd + 4);
		std::ostringstream fullResponse;
		sendHtmlResponse(fd, 200, body);

		std::string responseStr = fullResponse.str();
		if (responseStr.empty()) {
			std::string errorBody = getErrorPageBody(500, config);
			sendHtmlResponse(fd, 500, errorBody);
			std::cerr << "❌ Empty CGI output — sending 500\n";
			return;
		}
		ssize_t sent = send(fd, responseStr.c_str(), responseStr.length(), 0);
		if (sent != (ssize_t)responseStr.length())
			std::cerr << "❌ CGI response was interrupted\n";
		std::cout << "📤 CGI output:\n" << responseStr << std::endl;
	}

}
