/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStatus.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kellen <kellen@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:22:30 by kbolon            #+#    #+#             */
/*   Updated: 2025/06/12 00:20:05 by kellen           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSTATUS_HPP
#define HTTPSTATUS_HPP

#include <string>
#include <map>

class HttpStatus {
  public:
    static const std::map<int, std::string>& statusMessages();
	static std::string getStatusMessages(int code);
};

#endif // HTTPSTATUS_HPP