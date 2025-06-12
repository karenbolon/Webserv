/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/19 07:09:45 by kellen            #+#    #+#             */
/*   Updated: 2025/06/12 02:09:10 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>

/*
The Response class builds the HTTP response to be sent to the client.
 */
class Response {
	public:
		Response();
		static std::string getContentType(const std::string& path);
		static std::string buildHeader(int statusCode, size_t contentLength, const std::string& contentType);
		static std::string build(int statusCode, const std::string& body, const std::string& contentType);
};

#endif // RESPONSE_HPP
