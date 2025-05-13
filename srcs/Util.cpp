/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Util.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/22 01:56:46 by kellen            #+#    #+#             */
/*   Updated: 2025/05/13 11:58:18 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/WebServ.hpp"

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

bool fileExists(const std::string& path) {
	std::ifstream file(path.c_str());
	return file.good();
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
		configPath = "config/default.conf";
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