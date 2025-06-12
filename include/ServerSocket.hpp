/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:08 by kbolon            #+#    #+#             */
/*   Updated: 2025/06/12 00:33:16 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERSOCKET_HPP
# define SERVERSOCKET_HPP

#include "ServerConfig.hpp"
# include <netinet/in.h>
#include <vector>
#include <string>

class ServerConfig;

//external helpers
int		safe_socket(int domain, int type, int protocol);
bool	safe_bind(int fd, sockaddr_in & addr);
bool	safe_listen(int socket, int backlog);

class ServerSocket {
  private:
    int           _fd;
    ServerConfig  _config;
  public:
    ServerSocket();
    ~ServerSocket();

    bool	init(int port, const std::string& host);
    void	setConfig(const ServerConfig& config);
    const	ServerConfig& getConfig() const;

    int		acceptClient();
    void	closeSocket();
    int		getFD();
};

#endif // SERVERSOCKET_HPP