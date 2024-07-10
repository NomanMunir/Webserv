/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 11:29:39 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/10 12:21:33 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef PROCESS_HPP
# define PROCESS_HPP

#include "Request.hpp"

class Process
{
	public:
		Process(Request &request);
		~Process();
	private:
		Request request;
		void handleGetRequest(Request &request);

};

#endif // PROCESS_HPP