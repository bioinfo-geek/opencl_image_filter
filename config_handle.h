/* **********************************************
 * Fichier config_handle.h
 *
 * Mathieu Simard
 * Ce code peut être redistribué et utiliser sous lisence GPL
 *
 * Comporte l'interface permettant la lecture du fichier de configuration
 *
 * *********************************************/

#include <iostream>
#include <list>
#include <fstream>
#include <string>
#include <sstream>


/** *********************************************
 * @fn read_config
 * @param char* fname nom du fichier de configuration
 * @param int& k référence à la taille de la matrice
 * @param float* k_matrix matrice de convolution
 * @param std::list<string> image_name liste des images
 * @param string chemin en lecture
 * @param string chemin d'écriture
 * Fonction permettrant de lire le fichier de configuration
 ***********************************************/
void read_config (char* fname,int &k, float* &k_matrix, std::list<std::string> &image_name, std::string &chemin_lecture, std::string &chemin_ecriture);