/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/19 07:09:51 by kellen            #+#    #+#             */
/*   Updated: 2025/06/12 03:30:19 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>


/*
 * The Request class parses raw HTTP request strings.
 * It extracts the method and requested path (e.g., GET /index.html).
 */
class Request {
	public:
		Request(const std::string& raw);
		std::string getRawRequest() const;
		std::string getMethod() const;
		std::string getPath() const;
		std::string getBody() const;
		std::string getQuery() const;
//    std::string getTarget() const;
		const std::map<std::string, std::string>& getHeaders() const;
	private:
		std::string _method;
		std::string _target; //the original request URI (e.g. "/cgi-bin/hello.py?name=Bob")
		std::string _path; //path only (e.g. "/cgi-bin/hello.py")
		std::string _query; //query only (e.g. "name=Bob")
		std::string _version;
		std::string _raw;
		std::map<std::string, std::string> _headers;

		void parse(const std::string& raw);
		void parseHeaders(const std::string& HeaderBlock);
};

#endif // REQUEST_HPP