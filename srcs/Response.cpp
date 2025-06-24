/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/19 07:13:21 by kellen            #+#    #+#             */
/*   Updated: 2025/06/23 22:34:44 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

/*
* Constructor — currently does nothing, but good to have.
*/
Response::Response() {}

/*
* This function builds the HTTP header for a given status and content.
*/
std::string Response::buildHeader(int statusCode, size_t contentLength, const std::string& contentType) {
	std::ostringstream header;
	header << "HTTP/1.1 " << statusCode << " " << HttpStatus::getStatusMessages(statusCode) << "\r\n";
	header << "Content-Length: " << contentLength << "\r\n";
	header << "Content-Type: " << contentType << "; charset=utf-8\r\n";
	header << "Connection: close\r\n";  // ← Only add this if you want
	header << "\r\n";
	return header.str();
}

/*
 * This function returns the appropriate Content-Type for a given file path.
 */
std::string Response::getContentType(const std::string& path) {

	size_t dot = path.rfind('.');
	if (dot == std::string::npos)
		return "application/octet-stream";
	std::string ext = path.substr(dot);

	// Convert to lowercase for case-insensitive comparison
	for (size_t i = 0; i < ext.length(); ++i)
		ext[i] = std::tolower(ext[i]);

	// Enhanced image support
	if (ext == ".html") return "text/html";
	if (ext == ".css")  return "text/css";
	if (ext == ".js")   return "application/javascript";

	// Image formats - expanded and improved
	if (ext == ".png")  return "image/png";
	if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
	if (ext == ".gif")  return "image/gif";
	if (ext == ".bmp")  return "image/bmp";
	if (ext == ".webp") return "image/webp";
	if (ext == ".svg")  return "image/svg+xml";
	if (ext == ".ico")  return "image/x-icon";
	if (ext == ".tiff" || ext == ".tif") return "image/tiff";
	if (ext == ".avif") return "image/avif";

	// Other common types
	if (ext == ".txt")  return "text/plain";
	if (ext == ".json") return "application/json";
	if (ext == ".pdf")  return "application/pdf";
	if (ext == ".xml")  return "application/xml";

	// Video formats
	if (ext == ".mp4")  return "video/mp4";
	if (ext == ".webm") return "video/webm";
	if (ext == ".avi")  return "video/x-msvideo";

	// Audio formats
	if (ext == ".mp3")  return "audio/mpeg";
	if (ext == ".wav")  return "audio/wav";
	if (ext == ".ogg")  return "audio/ogg";

	return "application/octet-stream";
}

std::string Response::build(int statusCode, const std::string& body, const std::string& contentType) {
	std::ostringstream oss;
	oss << buildHeader(statusCode, body.size(), contentType);
	oss << body;
	return oss.str();
}