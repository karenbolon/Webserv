/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 17:20:05 by kbolon            #+#    #+#             */
/*   Updated: 2025/06/12 00:39:15 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

ConfigParser::ConfigParser() {}

ConfigParser::~ConfigParser() {}

static bool	parseKeyValue(const std::string& line, std::string& key, std::string& value) {
	std::istringstream iss(line);
	if (!(iss >> key))
		return false;
	std::getline(iss, value);
	trim(value);
	value = cleanValue(value);
	return !value.empty(); //only return true if a value was found/parsed
}

std::vector<std::string> line_splitter(const std::string& line) {
	std::vector<std::string>	result;
	std::istringstream iss(line);
	std::string	token;
	while (iss >> token)
		result.push_back(token);
	return result;
}

void	ConfigParser::parseFile(const std::string& path) {
	std::ifstream file(path.c_str());
	if (!file.is_open()) {
		throw std::runtime_error("Could not open config file: " + path);
	}
	std::string line;
	while (std::getline(file, line)) {
		trim(line);
		if (line.empty() || line[0] == '#') {
			continue;
		}
		//check for 'server{' or 'server {'
		if (line.find("server") == 0 && line.find("{") != std::string::npos) {
			std::istringstream iss(line);
			ServerConfig	server;
			parseServerBlock(file, server);
			servers.push_back(server);
			continue;
		}
		else if (line.find("server") == 0)
			error("Couldn't read server block\n");
	}
}

void ConfigParser::parseServerBlock(std::ifstream& file, ServerConfig& server) {
	std::string	line;
	int depth = 1;
	while (std::getline(file, line)) {
		trim(line);
		// ❌ Skip comments and empty lines
		if (line.empty() || line[0] == '#')
			continue;
		if (line == "{") {
			depth++;
			continue;
		}
		if (line == "}") {
			depth--;
			if (depth == 0) {
				convertListenEntriesToPortsAndHost(server);
				applyDefaults(server);
				return;
			}
			continue;
		}
		//checks if location and '{' are on same line, but rejects 'location\{' or 'location\{'
		//like nginx
		if (line.find("location") == 0) {
			std::istringstream iss(line);
			std::string	type, path, brace;
			iss >> type >> path >> brace;
			if (brace != "{")
				throw std::runtime_error("Malformed location block: expected space between path and '{', e.g. 'location / {'\n");
			if (!path.empty()) {
				LocationConfig	location;
				location.path = path; //save URL path for location block (upload etc)
				parseLocationBlock(file, location);
				applyInheritance(location, server);
				for (size_t i = 0; i < server.locations.size(); ++i) {
					if (server.locations[i].path == location.path) {
						throw std::runtime_error("Duplicate location block for path: " + location.path);
					}
				}
				if (location.path[0] != '/') {
					throw std::runtime_error("Location path must start with '/'\n");
				}
				server.locations.push_back(location);
				continue;
			}
			else
				throw std::runtime_error( "Missing location path\n");
		}
		else if (line.find("location") == 0)
			throw std::runtime_error("couldn't read location block (possibly missing '{')\n");
		std::string key, value;
		if (parseKeyValue(line, key, value)) {
			server.raw[key] = value; //stores all items in the block to ensure we parse it
			if (value.empty()) {
				error("Value is empty for " + key + ", skipping line\n");
				continue;
			}
			parseServerDirective(server, key, value);
		}
	}
	if (server.ports.empty())
		throw std::runtime_error("❌ Missing or invalid 'listen' directive in server block\n");
	if (server.host.empty())
		error("Missing 'host' directive in server block, using default\n");
}

void ConfigParser::parseLocationBlock(std::ifstream& file, LocationConfig& location) {
	std::string	line;
	int depth = 1;

	while (std::getline(file, line)) {
		trim(line);
		// ❌ Skip comments and empty lines
		if (line.empty() || line[0] == '#') continue;
		if (line == "{") {
			depth++;
			continue;
		}
		if (line == "}") {
			depth--;
			if (depth == 0)
				return;
			continue;
		}
		std::string key, value;
		if (parseKeyValue(line, key, value)) {
			location.raw[key] = value;
			parseLocationDirective(location, key, value);
		}
	}
}

const	std::vector<ServerConfig>& ConfigParser::getServers() const {
	return servers;
}

void	ConfigParser::print() const {
	for (size_t i = 0; i < servers.size(); i++) {
		std::cout << "\n🌸 SERVER " << i + 1 << std::endl;
		servers[i].print();
	}
}

void	ConfigParser::parseServerDirective(ServerConfig& server, const std::string& key, const std::string& value) {
	//store the list of ports as raw data to be converted later
	if (key == "listen")
		server.listen_entries.push_back(value);
	else if (key == "port")
		error ("Unknown directive in server block: '" + key + "'\n");
	else if (key == "host")
		server.host = value;
	else if (key == "server_name")
		server.server_name = value;
	else if (key == "root")
		server.root = value;
	else if (key == "index")
		server.index = value;
	else if (key == "client_max_body_size")
		server.client_max_body_size = std::atol(value.c_str());
	else if (key == "error_page") {
		std::istringstream iss(value);
		int	code;
		std::string	path;
		if (iss >> code >> path) {
			if (server.error_pages.count(code))
				throw std::runtime_error("Duplicate error_page for: " + intToStr(code));
			server.error_pages[code] = path;
		}
		else
			error("Couldn't read error page\n");
	}
	else
		error("Unknown directive in server block: '" + key + "'\n");
}

void	ConfigParser::parseLocationDirective(LocationConfig& location, const std::string& key, const std::string& value) {
	if (key == "root") {
		location.root = value;
		location.root_set = true;
	}
	else if (key == "index") {
		location.index = value;
		location.index_set = true;
	}
	else if (key == "redirect")
		location.redirect = value;
	else if (key == "autoindex") {
		if (value == "on")
			location.autoindex = true;
	}
	else if (key == "upload_path")
		location.upload_path = value;
	else if (key == "return")
		location.returnStatusCode = std::atoi(value.c_str());
	else if (key == "methods" || key == "allowed_methods")
		location.methods = line_splitter(value);
	else if (key == "cgi_path")
		location.cgi_paths["default"] = value;
	else if (key == "cgi") {
		std::vector<std::string> parts = line_splitter(value);
		if (parts.size() == 2)
			location.cgi_paths[parts[0]] = parts[1];
		else
			error("Invalid CGI mapping: expected two arguments\n");
	}
	else
		error("Unknown directive in location block: '" + key + "'\n");
}

void	ConfigParser::applyInheritance(LocationConfig& location, const ServerConfig& server) {
	if (!location.root_set)
		location.root = server.root;
	if (!location.index_set)
		location.index = server.index;
	if (location.methods.empty()) {
		location.methods.push_back("GET");
		location.methods.push_back("POST");
		location.methods.push_back("DELETE");
	}
}

void	ConfigParser::error(const std::string& msg) const {
	std::cerr << "⚠️ ConfigParser: " << msg << std::endl;
}