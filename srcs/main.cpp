/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmunoz-g <cmunoz-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/25 10:38:08 by juramos           #+#    #+#             */
/*   Updated: 2025/02/25 12:57:52 by cmunoz-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IRC.hpp"

// to do
/*

    - limpiar el codigo, anadir casos limites y tests
        - terminar de testear MODE
            - cuando mando /MODE cars asdfaius, parece como que el cliente se piensa que he aceptado esos modos (aunque no se guardan en los modos del servidor)
            - habia un problema al setear el modo +i, daba error
        - arreglar JOIN:
            - arreglar el join a varios canales
            - arreglar lo del MODE_I en JOIN
            - arreglar lo del MODE_K en JOIN
        - checkear el CAP
        - revisar respuesta 461 en handleInviteCommand
        - comprobar el comentario en Server::start para asegurarme de si es cierto
        - overload de == para Clienty Channel?
        - cambiar las bool ft de juan a void (eg la de removeClient), tb revisar lo del enabled en setMode
        - revisar el comentario de setTopic
        - que hacer con kickClient ? revisar subject
        - juntar la de notifyModeChange para que quede algo mas escueta
        - revisar Client::sendMessage, setOperatorStatus
        - hacer funcion de error ? con throws hasta main puedo creo
        - arreglar irssi conectandose sin contrasena
        - anadir ft helpers (eg si un channel existe)
        - organizar ft en diferentes archivos
        - bug: cuando conectas un cliente, desconectas, y vuelves a conectar explota
        - comprobar la memoria
    
    
    - testear con varios clientes a la vez
        - KICK, INVITE, ETC...
        - testear checkUniqueNick
    - revisar tutoriales, otros repos
    - testeos intensivos, pulir, revisar errores
    - comentarios en el codigo

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
