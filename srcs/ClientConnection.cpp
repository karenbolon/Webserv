/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 02:34:47 by kbolon            #+#    #+#             */
/*   Updated: 2025/07/08 20:54:04 by kbolon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

ClientConnection::ClientConnection(int fd) 
	: _fd(fd), _pendingSendBuffer(), _bytesSentSoFar(0), _wantsToWrite(false), 
	_cgiInputFd(-1), _cgiOutputFd(-1), _cgiPid(-1), _cgiOutputBuffer(), 
	_cgiInputBuffer(), _cgiStartTime(0), _cgiDone(false), _cgiRunning(false), 
	_chunkedState(IDLE) {
	int flags = fcntl(_fd, F_GETFL, 0);
	if (!(flags & O_NONBLOCK))
		fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
}

ClientConnection::~ClientConnection() {
}

int	ClientConnection::getFd() const {
	return _fd;
}

std::vector<char>& ClientConnection::getBuffer() { 
	return _buffer; 
}

// Non-blocking send logic
void ClientConnection::setPendingResponse(const std::string& response) {
    _pendingSendBuffer = response;
    _bytesSentSoFar = 0;
    _wantsToWrite = true;
}

bool ClientConnection::wantsToWrite() const { 
	return _wantsToWrite; 
}

std::string& ClientConnection::getPendingSendBuffer() { 
	return _pendingSendBuffer; 
}

size_t& ClientConnection::getBytesSentSoFar() {
	return _bytesSentSoFar; 
}

void ClientConnection::setCgiInputBuffer(const std::string& data) {
    _cgiInputBuffer = data;
}

void ClientConnection::clearSendState() {
    _pendingSendBuffer.clear();
    _bytesSentSoFar = 0;
    _wantsToWrite = false;
}

//request/response logic
/*
-We use non-blockeing recv() safely
-server reads one full file at a time per client connection
-gracefully handles slow clients and disconnects
-exits cleanly after each transfer
*/
int ClientConnection::recvFullRequest(int client_fd, const ServerConfig& config, ClientConnection* client, 
	std::vector<struct pollfd>& fds) {

	//switched to vector to handle images and pdfs
	char buffer[65536]; // 64KB buffer to speed up transfer
	int bytes = recv(client_fd, buffer, sizeof(buffer), 0);

	if (bytes <= 0) {
		if (bytes == 0)
			return bytes;
		else {
			//bc some transfers are huge, it doesn't always mean an error, the browser
			//could cut the connection early and the transfer will resume later
			if (this->getChunkState() == IN_PROGRESS) {
				std::cerr << "⚠️ Client disconnected during chunked transfer (not server error)\n";
				this->setChunkState(ERROR);
				return bytes;
			}
			std::cerr << "⚠️ Connection closed or recv failed during recvFullRequest\n";
			std::string body = getErrorPageBody(500, config);
			sendHtmlResponse(500, body, client, fds);
		}
		return bytes;
	}
	this->_buffer.insert(this->_buffer.end(), buffer, buffer + bytes);
	std::string reqStr(_buffer.begin(), _buffer.end());
	size_t headerEnd = reqStr.find("\r\n\r\n");
	if (headerEnd == std::string::npos) {
		std::cerr << "❌ Incomplete headers\n";
		std::string body = getErrorPageBody(400, config);
		sendHtmlResponse(400, body, client, fds);
		return bytes;
	}
	// Found end of headers, check for Content-Length
	size_t contentLengthPos = reqStr.find("Content-Length:");
	if (contentLengthPos != std::string::npos) {
		size_t valueStart = contentLengthPos + 15; // Length of "Content-Length:"
		size_t valueEnd = reqStr.find("\r\n", valueStart);
		if (valueEnd != std::string::npos) {
			std::string lengthStr = reqStr.substr(valueStart, valueEnd - valueStart);
			size_t totalContentLength = atoi(lengthStr.c_str());
			size_t bodyStart = headerEnd + 4;
			if (_buffer.size() - bodyStart < totalContentLength)
				return bytes;
		}
	}
	return bytes;
}

bool ClientConnection::isRequestComplete() const {
	std::string reqStr(_buffer.begin(), _buffer.end());
	size_t headerEnd = reqStr.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return false;
	size_t contentLengthPos = reqStr.find("Content-Length:");
	if (contentLengthPos == std::string::npos)
		return true;
	size_t valueStart = contentLengthPos + 15; // Length of "Content-Length:"
	size_t valueEnd = reqStr.find("\r\n", valueStart);
	std::string lengthStr = reqStr.substr(valueStart, valueEnd - valueStart);
	size_t totalLength = atoi(lengthStr.c_str());
	size_t bodyStart = headerEnd + 4;
	return (_buffer.size() - bodyStart >= totalLength);
}

std::string ClientConnection::getRawRequest() const {
	return std::string(_buffer.begin(), _buffer.end());
}

//CGI
int	ClientConnection::getCgiInputFd() const {
	return _cgiInputFd;
}

int	ClientConnection::getCgiOutputFd() const {
	return _cgiOutputFd;
}

void	ClientConnection::setCgiFds(int inputFd, int outputFd) {
	_cgiInputFd = inputFd;
	_cgiOutputFd = outputFd;
}

void	ClientConnection::setCgiPid(pid_t pid) {
	_cgiPid = pid;
}

void	ClientConnection::markCgiRunning() {
	_cgiRunning = true;
	_cgiDone = false;
	_cgiStartTime = time(NULL);
}

void	ClientConnection::markCgiDone() {
	_cgiRunning = false;
	_cgiDone = true;
}

bool ClientConnection::isCgiRunning() const {
	return _cgiRunning;
}

bool ClientConnection::isCgiDone() const {
	return _cgiDone;
}

pid_t ClientConnection::getCgiPid() const { 
	return _cgiPid; 
}

std::string&	ClientConnection::getCgiInputBuffer() {
	return _cgiInputBuffer;
}

std::string&	ClientConnection::getCgiOutputBuffer() {
	return _cgiOutputBuffer;
}

void ClientConnection::appendToCgiOutput(const char* data, size_t len) {
	_cgiOutputBuffer.append(data, len);
}

void ClientConnection::consumeCgiInput(size_t bytes) {
	    if (bytes >= _cgiInputBuffer.size())
        _cgiInputBuffer.clear();
    else
        _cgiInputBuffer.erase(0, bytes);
}

bool	ClientConnection::cgiInputBufferEmpty() const {
	return _cgiInputBuffer.empty();
}

time_t ClientConnection::getCgiStartTime() const {
	return _cgiStartTime;
}

void ClientConnection::setCgiStartTime(time_t t) {
	_cgiStartTime = t;
}

void ClientConnection::setChunkFilePath(const std::string& path) {
	_chunkFilePath = path;
}
std::string ClientConnection::getChunkFilePath() const {
	return _chunkFilePath;
}
std::ifstream& ClientConnection::getChunkFileStream() {
	return _chunkFileStream;
}
bool ClientConnection::openChunkFile() {
	_chunkFileStream.open(_chunkFilePath.c_str(), std::ios::binary);
	return _chunkFileStream.is_open();
}
void ClientConnection::closeChunkFile() {
	if (_chunkFileStream.is_open())
		_chunkFileStream.close();
}

void  ClientConnection::setChunkState(ChunkState state) {
	_chunkedState = state;
}

ChunkState	ClientConnection::getChunkState() const {
	return _chunkedState;
}

bool ClientConnection::isChunkedDone() const {
    return _chunkedState == DONE;
}

bool ClientConnection::isChunkedError() const {
    return _chunkedState == ERROR;
}

void ClientConnection::resetChunkedFlags() {
    _chunkedState = IDLE;
}