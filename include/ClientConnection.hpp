/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: keramos- <keramos-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:30 by kbolon            #+#    #+#             */
/*   Updated: 2025/06/11 17:35:38 by keramos-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include "../include/ServerConfig.hpp"

enum ClientState {
  READING_HEADERS,
  READING_BODY,
  REQUEST_COMPLETE
};

class ClientConnection {
  private:
    int               _fd;
    std::vector<char> _buffer;

  public:
    ClientConnection(int fd);
    ~ClientConnection();

    std::string	getRawRequest() const;
    int         getFd() const;
    void        closeConnection();
    bool        isRequestComplete() const;
    int        recvFullRequest(int client_fd, const ServerConfig& config);
};
