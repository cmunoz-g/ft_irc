/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/25 10:38:08 by juramos           #+#    #+#             */
/*   Updated: 2025/03/04 12:49:35 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IRC.hpp"

// to do
/*
    - testear con varios clientes a la vez
        - KICK, INVITE, ETC...
    - revisar tutoriales, otros repos
    - REVISAR todos los codigos, 401 404 405, etc etc
    - testeos intensivos, pulir, revisar errores
        - volver a revisar el comportamiento esperado de MODE, especialmente cuando hacemso MODE #channel o , k y l
        - pasar valgrind
    - poner todo bonito
        - revisar consturctores y copy constructors
        - comprobar que cumplo todos los requisitos de clases canonicas etc
        - eliminar ft que no se utilicen (candidatos: notifyModeChange)
        - comentarios
    - comentarios en el codigo

*/

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Use: " << argv[0] << " <port> <password>" << std::endl;
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
