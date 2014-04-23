/* **********************************************
 * Fichier png_handle.cpp
 *
 * Mathieu Simard
 * Ce code peut être redistribué et utiliser sous lisence GPL
 *
 * Code permettant d'ouvrir un fichier png à partir de libpng
 *
 * *********************************************/


#include "png_handle.h"

unsigned char* png_read_decode_file(std::string image, image_info& img_info)
{
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep* row_pointers;
    unsigned char header[8];
    /* ouverture du fichier et test sur sa validité en tant que png */
    FILE* imgfile = fopen(image.c_str(), "rb");
    if (!imgfile) {
        std::cout << "[png_read_decode_file] Impossible d'ouvrir l'image " << image << std::endl;
        return NULL;
    }
    fread(header, 1, 8, imgfile);
    if (png_sig_cmp(header, 0, 8)) {
        std::cout << "[png_read_decode_file] L'image " << image << " n'est pas au format PNG" << std::endl;
        fclose(imgfile);
        return NULL;
    }
    /* Initialisation de la structure de lecture */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        std::cout << "[png_read_decode_file] png_create_read_struct a échoué" << std::endl;
        fclose(imgfile);
        return NULL;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        std::cout << "[png_read_decode_file] png_create_info_struct a échoué" << std::endl;
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(imgfile);
        return NULL;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cout << "[png_read_decode_file] Erreur lors d'init_io" << std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(imgfile);
        return NULL;
    }
    png_init_io(png_ptr, imgfile);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    /*Initialisation des données sur l'image da img_info*/
    img_info.width = png_get_image_width(png_ptr, info_ptr);
    img_info.height = png_get_image_height(png_ptr, info_ptr);
    img_info.color_type = png_get_color_type(png_ptr, info_ptr);
    img_info.bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    img_info.interlacing = png_set_interlace_handling(png_ptr);
    img_info.channels_count = png_get_channels(png_ptr, info_ptr);
    img_info.rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    if (img_info.bit_depth <= 0 || img_info.bit_depth > 8) {
        if (img_info.bit_depth > 8)
            std::cout << "[png_read_decode_file] Erreur un bit_depth suppérieur à 8bits/canal n'est pas supporté pour le moment" << std::endl;
        else
            std::cout << "[png_read_decode_file] Erreur donnée bit_depth invalide" << std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(imgfile);
        return NULL;
    }
    if (img_info.channels_count != 3) {
        std::cout << "[png_read_decode_file] Erreur une nombre de canneaux différent de 3 n'est pas supporté pour le moment" << std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(imgfile);
        return NULL;
    }
    png_read_update_info(png_ptr, info_ptr);
    /* read file */
    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cout << "[png_read_decode_file] Erreur lors de read_image" << std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(imgfile);
        return NULL;
    }
    row_pointers = (png_bytep*) new png_bytep[img_info.height];
    for (int y = 0; y < img_info.height; y++)
        row_pointers[y] = (png_byte*) new png_byte[img_info.rowbytes];
    png_read_image(png_ptr, row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(imgfile);
    unsigned char* retour =  convert_to_table_row_pointer(img_info.height, img_info.rowbytes, row_pointers);
    for (int y = 0; y < img_info.height; y++)
        delete [] row_pointers[y];
    delete [] row_pointers;
    return retour;
}

int png_write_encode_file(std::string image, image_info& img_info, unsigned char* row_pointer)
{
    png_bytep* row_pointers = convert_to_png_row_pointer(img_info, row_pointer);
    png_structp png_ptr;
    png_infop info_ptr;
    FILE* imgfile = fopen(image.c_str(), "wb");
    if (!imgfile) {
        std::cout << "[png_write_encode_file] Impossible d'ouvrir le fichier cible" << image << std::endl;
        for (int y = 0; y < img_info.height; y++)
            delete [] row_pointers[y];
        delete [] row_pointers;
        return -1;
    }
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        std::cout << "[png_write_encode_file] png_create_read_struct a échoué" << std::endl;
        for (int y = 0; y < img_info.height; y++)
            delete [] row_pointers[y];
        delete [] row_pointers;
        fclose(imgfile);
        return -1;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        std::cout << "[png_write_encode_file] png_create_info_struct a échoué" << std::endl;
        png_destroy_write_struct(&png_ptr, NULL);
        for (int y = 0; y < img_info.height; y++)
            delete [] row_pointers[y];
        delete [] row_pointers;
        fclose(imgfile);
        return -1;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cout << "[png_write_encode_file] Erreur lors d'init_io" << std::endl;
        png_destroy_write_struct(&png_ptr, &info_ptr);
        for (int y = 0; y < img_info.height; y++)
            delete [] row_pointers[y];
        delete [] row_pointers;
        fclose(imgfile);
        return -1;
    }
    png_init_io(png_ptr, imgfile);
    png_set_IHDR(png_ptr, info_ptr, img_info.width, img_info.height, img_info.bit_depth, img_info.color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);
    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cout << "[png_write_encode_file] Erreur lors de l'écriture des informations" << std::endl;
        png_destroy_write_struct(&png_ptr, &info_ptr);
        for (int y = 0; y < img_info.height; y++)
            delete [] row_pointers[y];
        delete [] row_pointers;
        fclose(imgfile);
        return -1;
    }
    png_write_image(png_ptr, row_pointers);
    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cout << "[png_write_encode_file] Erreur lors de l'écriture de l'image" << std::endl;
        png_destroy_write_struct(&png_ptr, &info_ptr);
        for (int y = 0; y < img_info.height; y++)
            delete [] row_pointers[y];
        delete [] row_pointers;
        fclose(imgfile);
        return -1;
    }
    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    for (int y = 0; y < img_info.height; y++)
        delete [] row_pointers[y];
    delete [] row_pointers;
    fclose(imgfile);
    return 0;
}

png_bytep* convert_to_png_row_pointer(image_info img_info, unsigned char* row_pointer)
{
    png_bytep* row_pointers = (png_bytep*) new png_bytep[img_info.height];
    for (int y = 0; y < img_info.height; y++)
        row_pointers[y] = (png_byte*) new png_byte[img_info.rowbytes];
    for (int i = 0; i < img_info.height; i++) {
        for (int j = 0; j < img_info.rowbytes; j++) {
            row_pointers[i][j] = row_pointer[i * img_info.rowbytes + j];
        }
    }
    return row_pointers;
}

unsigned char* convert_to_table_row_pointer(int height, int rowbytes, png_bytep* row_pointers)
{
    unsigned char* row_pointer = new unsigned char[height * rowbytes];
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < rowbytes; j++) {
            row_pointer[i * rowbytes + j] = row_pointers[i][j];
        }
    }
    return row_pointer;
}
