/* **********************************************
 * Fichier config_handle.cpp
 *
 * Mathieu Simard
 * Ce code peut être redistribué et utiliser sous lisence GPL
 *
 * Comporte la permettant la lecture du fichier de configuration
 *
 * *********************************************/

#include "config_handle.h"

void read_config (char* fname,int &k, float* &k_matrix, std::list<std::string> &image_name, std::string &chemin_lecture, std::string &chemin_ecriture)
{
        std::string str;
        std::ifstream entryfile(fname);
        if (entryfile.is_open()) {
            if (entryfile.good()) {
                //Lecture et initialisation du paramètre k à partir du fichier de configuration
                getline(entryfile, str);
                std::istringstream(str) >> k;
                std::cout << k << std::endl;
                /* allocation dynamique de la mémoire pour la matrice
                 * Notez ici qu'affin de réduire l'empreinte mémoire on préfère utiliser
                 * un tableau contigue à une dimenssion plutôt qu'un tableau à deux dimenssion
                 * utilisant des pointeurs. les deux dimenssion sont simulés pas x*k+y
                 */
                k_matrix = new float[k * k];
                // Initialisation de la matrice avec les données provenant du fichier de configuration
                for (int i = 0; i < k; i++) {
                    getline(entryfile, str);
                    std::size_t curidx = 0, endidx;
                    for (int j = 0; j < k; j++) {
                        endidx = str.find_first_of(" \t", curidx);
                        if (endidx > curidx) {
                            std::istringstream(str.substr(curidx, endidx - curidx)) >> k_matrix[k * i + j];
                        } else if (curidx < str.length()) {
                            std::istringstream(str.substr(curidx, std::string::npos)) >> k_matrix[k * i + j];
                        }
                        curidx = endidx + 1;
                    }
                }
                //assignation des chemin_lecture et chemin_ecriture
                getline(entryfile, chemin_lecture);
                getline(entryfile, chemin_ecriture);
                std::cout << "Le chemin de lectre est: " << chemin_lecture << std::endl;
                std::cout << "Le chemin de lectre est: " << chemin_ecriture << std::endl;
                //Ajout de tout les nom d'images dans la liste image_name
                while (!entryfile.eof()) {
                    getline(entryfile, str);
                    if (str.length() > 0) {
                        image_name.push_back(str);
                        std::cout << image_name.back() << std::endl;
                    }
                }
                entryfile.close();
            } else {
                std::cout << fname << ": Le fichier spécifier est vide." << std::endl;
            }
        } else {
            std::cout << fname << ": Impossible d'ouvrir le fichier spécifier." << std::endl;
        }
}