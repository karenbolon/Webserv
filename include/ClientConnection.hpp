/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:30 by kbolon            #+#    #+#             */
/*   Updated: 2025/07/08 17:42:58 by kbolon           ###   ########.fr       */
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
    std::vector<char> _buffer;

    //for non-blocking send
    std::string       _pendingSendBuffer;
    size_t            _bytesSentSoFar;
    bool              _wantsToWrite;
   
    
    //CGI
    int               _cgiInputFd;
    int               _cgiOutputFd;
    pid_t             _cgiPid;
    std::string       _cgiOutputBuffer;
    std::string       _cgiInputBuffer;
    time_t            _cgiStartTime;
    bool              _cgiDone;
    bool              _cgiRunning;
    bool              _chunkedInProgress;

  public:
    ClientConnection(int fd);
    ~ClientConnection();

    int                 getFd() const;
    std::vector<char>&  getBuffer();

    //non-blocking send
    void              setCgiInputBuffer(const std::string& data);
    void              setPendingResponse(const std::string& response);
    bool              wantsToWrite() const;
    std::string&      getPendingSendBuffer();
    size_t&           getBytesSentSoFar();
    void              clearSendState();

    //request/response
    bool              isRequestComplete() const;
    int               recvFullRequest(int client_fd, const ServerConfig& config, ClientConnection* client, std::vector<struct pollfd>& fds);

    //CGI
    int               getCgiOutputFd() const;
    int               getCgiInputFd() const;
    void              setCgiFds(int input, int outputFd);
    void              setCgiPid(pid_t pid);
    void              markCgiRunning();
    void              markCgiDone();
    bool              isCgiRunning() const;
    bool              isCgiDone() const;
    pid_t             getCgiPid() const;
    std::string&      getCgiInputBuffer();
    std::string&      getCgiOutputBuffer();
    void              appendToCgiOutput(const char* data, size_t len);
    void              consumeCgiInput(size_t bytes);
    bool              cgiInputBufferEmpty() const;
    time_t            getCgiStartTime() const;
    void              setCgiStartTime(time_t t);
    void              setChunkedInProgress(bool val);
    bool              isChunkedInProgress() const;
     
    std::string	      getRawRequest() const;
    
};

#endif // CLIENTCONNECTION_HPP
