/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:30 by kbolon            #+#    #+#             */
/*   Updated: 2025/07/07 14:56:38 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTCONNECTION_HPP
#define CLIENTCONNECTION_HPP

#include <string>

enum ClientState {
  READING_HEADERS,
  READING_BODY,
  REQUEST_COMPLETE
};

class ClientConnection {
  private:
    int               _fd;
    int               _cgiInputFd;
    int               _cgiOutputFd;
    pid_t             _cgiPid;
    std::string       _cgiOutputBuffer;
    std::string       _cgiInputBuffer;
    time_t            _cgiStartTime;
    bool              _cgiDone;
    bool              _requestReady;
    bool              _cgiRunning;
    std::vector<char> _buffer;

  public:
    ClientConnection(int fd);
    ~ClientConnection();

    std::string	      getRawRequest() const;
   
    int               getFd() const;
    int               getCgiOutputFd() const;
    int               getCgiInputFd() const;
    void              closeConnection();
    bool              isRequestComplete() const;
    int               recvFullRequest(int client_fd, const ServerConfig& config);
    void              setCgiFds(int input, int outputFd);
    void              setCgiPid(pid_t pid);
    void              markCgiRunning();
    void              markCgiDone();
    void              markRequestReady();
    bool              hasRequestReady() const;
    bool              isCgiRunning() const;
    bool              isCgiDone() const;
    pid_t             getCgiPid() const;
    std::string&      getCgiInputBuffer();
    std::string&      getCgiOutputBuffer();
    void              appendToCgiOutput(const char* data, size_t len);
    void              consumeCgiInput(size_t bytes);
    bool              cgiInputBufferEmpty() const;
    time_t            getCgiStartTime() const;
};

#endif // CLIENTCONNECTION_HPP
