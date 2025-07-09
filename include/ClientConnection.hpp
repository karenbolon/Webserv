/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 13:53:30 by kbolon            #+#    #+#             */
/*   Updated: 2025/07/09 19:02:53 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTCONNECTION_HPP
#define CLIENTCONNECTION_HPP

#include <string>
#include <vector>
#include <fstream>

enum ChunkState {
  IDLE,
  IN_PROGRESS,
  DONE,
  ERROR
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
    
    //chunk transfer
    ChunkState        _chunkedState;
    std::ifstream     _chunkFileStream;
    std::string       _chunkFilePath;

  public:
    ClientConnection(int fd);
    ~ClientConnection();

    int                 getFd() const;
    std::vector<char>&  getBuffer();

    //request/response
    bool              isRequestComplete() const;
    int               recvFullRequest(int client_fd, const ServerConfig& config, ClientConnection* client, std::vector<struct pollfd>& fds);

    //non-blocking send
    void              setPendingResponse(const std::string& response);
    bool              wantsToWrite() const;
    std::string&      getPendingSendBuffer();
    size_t&           getBytesSentSoFar();
    void              clearSendState();
    
    //CGI
    void              setCgiInputBuffer(const std::string& data);
    int               getCgiOutputFd() const;
    int               getCgiInputFd() const;
    void              setCgiFds(int input, int outputFd);
    void              setCgiPid(pid_t pid);
    void              markCgiRunning();
    void              markCgiDone();
    void              setCgiRunning(bool val);
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

    //chunked
    void              setChunkFilePath(const std::string& path);
    std::string       getChunkFilePath() const;
    std::ifstream&    getChunkFileStream();
    bool              openChunkFile();
    void              closeChunkFile();
    void              setChunkState(ChunkState state);
    ChunkState        getChunkState() const;
    bool              isChunkedDone() const;
    bool              isChunkedError() const;
    void              resetChunkedFlags();
    void              closeCgiFds();
     
    std::string	      getRawRequest() const;
    
};

#endif // CLIENTCONNECTION_HPP
