/* **********************************************
 * Fichier png_handle.h
 *
 * Mathieu Simard
 * Ce code peut être redistribué et utiliser sous lisence GPL
 *
 * Interface permettant d'ouvrir un fichier png à partir de libpng
 *
 * *********************************************/

#ifndef PNG_HANDLE_H
#define PNG_HANDLE_H

#include <iostream>
#include <vector>
#include "libpng-1.5.14/png.h"


/** *********************************************
 * @struct image_info
 * @var int width
 * @var int height
 * @var int rowbytes
 * @var png_byte color_type
 * @var png_byte bit_depth
 * @var png_byte channels_count
 * @var png_byte interlacing;
 * structure contenat les information d'une image
 ***********************************************/
struct image_info {
    int width;
    int height;
    int rowbytes;
    int color_type;
    int bit_depth;
    int channels_count;
    int interlacing;
};


/** *********************************************
 * @fn png_read_decode_file
 * @param std::string contenant le chemin d'accès à l'image
 * @param img_info& référence à la structure img_info par laquelles les information sur l'image seront retournée
 * @return unsigned char* tableau contenant l'image si succès ou NULL si erreur
 * Fonction permettrant de lire en mémoire un PNG Image et de retourner une structure row_pointer depuis libpng
 ***********************************************/
unsigned char* png_read_decode_file(std::string image, image_info& img_info);


/** *********************************************
 * @fn png_write_encode_file
 * @param std::string contenant le chemin où l'on veux enregistre l'image
 * @param img_info& référence à la structure img_info content les information de l'image
 * @param png_bytep* pointant sur une structure row_pointer contenant l'information des pixels
 * @return int 0 si succès -1 si erreur
 * Fonction permettrant d'écrire un png dans un fichier
 ***********************************************/
int png_write_encode_file(std::string image, image_info& img_info, unsigned char* row_pointer);


/** *********************************************
 * @fn convert_to_png_row_pointer
 * @param image_ingo contenant les informations sur l'image
 * @param unsigned_char* la structure row_pointer à convertir
 * @return png_bytep* contenant les donné des pixels pour libpng
 * Fonction permettrant de détruire une structure row_pointer;
 ***********************************************/
png_bytep* convert_to_png_row_pointer(image_info img_info, unsigned char* row_pointer);



/** *********************************************
 * @fn convert_to_table_row_pointer
 * @param int la hauteur de l'image qui sera contenu dans la structure row_pointer (en pixel)
 * @param int le nombre d'octets nécessaire par ligne (profondeur des channeua/8 * nb de channeaux * image width )
 * @param unsigned_char* tableau de pointeur sur les lignes de l'image
 * @return unsigned char* tableau contenat l'image
 * Fonction permettrant de construire une structure row_pointer;
 ***********************************************/
unsigned char* convert_to_table_row_pointer(int height, int rowbytes, png_bytep* row_pointers);

#endif //fin du ifndef PNG_HANDLE_H
