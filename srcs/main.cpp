/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/25 10:38:08 by juramos           #+#    #+#             */
/*   Updated: 2025/02/24 12:06:17 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IRC.hpp"

// to do
/*

    - limpiar el codigo, anadir casos limites y tests (ie: arreglar MODE, join a varios canales, si irssi se conecta sin contraseña
        gestionar error, anadir ft helpers (por ejemplo para comprobar si un channel existe), organizar fts y ver si quiero
        separarlas en diferentes archivos)
    - testear con varios clientes a la vez
        - KICK, INVITE, ETC...
    - revisar tutoriales, otros repos
    - testeos intensivos, pulir, revisar errores

*/

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <puerto> <contraseña>" << std::endl;
        return 1;
    }
    try {
        Server server(std::atoi(argv[1]), argv[2]);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
