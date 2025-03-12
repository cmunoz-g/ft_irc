/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/25 10:38:08 by juramos           #+#    #+#             */
/*   Updated: 2025/03/12 10:55:33 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IRC.hpp"

volatile sig_atomic_t g_running = 1;

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    if (argc != 3) {
        std::cerr << "Use: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    try {
        Server server(std::atoi(argv[1]), argv[2]);
        server.start();
        server.cleanup();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
