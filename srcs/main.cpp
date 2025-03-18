/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/25 10:38:08 by juramos           #+#    #+#             */
/*   Updated: 2025/03/13 13:58:03 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IRC.hpp"

// formatear Server.hpp y los cpp
// quitar includes que no use, quitar fts que no use, quitar macros/cosas del namespace que no use
// revisar nombres de variables(camelCase), revisar que no hay comentarios de debug, revisar spelling, revisar std::string response 
// ver que metodos puedo meter en private por clase
// Revisar nc ctrld vacio
// ultimo check con el subject
// quitar codigo comentado

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
