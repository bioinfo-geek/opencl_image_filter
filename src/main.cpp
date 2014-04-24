/* **********************************************
 * Fichier main.cpp
 *
 * Mathieu Simard
 * Ce code peut être redistribué et utiliser sous lisence GPL
 *
 * *********************************************/

#include <iostream>
#include <list>
#include <fstream>
#include <string>
#include <sstream>
#include <time.h>
#include <sys/stat.h>
#include <cstdlib>
#include <CL/cl.hpp>
#include "opencl_error_info.hpp"
#include "png_handle.h"
#include "config_handle.h"

int main(int argc, char** argv)
{
    int k;
    float* k_matrix = NULL;
    png_bytep in_row_pointers;
    png_bytep out_row_pointers;
    image_info img_infos;
    std::list<std::string> image_name;
    std::string chemin_lecture;
    std::string chemin_ecriture;
    // Si on a insuffisament d'argument, on ne peu effectuer les opération sur les images
    if (argc != 2) {
        std::cout << argv[0] << ": Seul un nom de fichier de configuration est attendu en paramètre." << std::endl;
        return -1;
    } else {
        timespec chrono_start, chrono_lap, chrono_current;
        clock_gettime(CLOCK_MONOTONIC, &chrono_start);
        time_t now = time(0);
        tm* localtm = localtime(&now);
        std::cout << "L'heure et la date locale est: " << asctime(localtm) << std::endl;
        //Lecture du fichier de configuration
        read_config (argv[1],k,k_matrix,image_name,chemin_lecture,chemin_ecriture);
        //Initialisation de l'environnement openCL et création du kernel
        cl_int ocl_error = 0;
        cl_platform_id ocl_platform;
        cl_uint ocl_num_platforms = 0;
        cl_context ocl_context;
        cl_command_queue ocl_queue;
        cl_device_id ocl_device;
        // Platform initialisation
        ocl_error = clGetPlatformIDs(1,&ocl_platform,&ocl_num_platforms);
        if (ocl_error != CL_SUCCESS) {
            std::cout << "Error getting platform id: " << descriptionOfError(ocl_error) << std::endl;
            delete [] k_matrix;
            exit(ocl_error);
        }
        // Device initialisation (choix du device type GPU)
        ocl_error = clGetDeviceIDs(ocl_platform, CL_DEVICE_TYPE_GPU, 1, &ocl_device, NULL);
        if (ocl_error != CL_SUCCESS) {
            std::cout << "Error getting device ids: " <<  descriptionOfError(ocl_error) << std::endl;
            delete [] k_matrix;
            exit(ocl_error);
        }
        // Context initialisation
        ocl_context = clCreateContext(0, 1, &ocl_device, NULL, NULL, &ocl_error);
        if (ocl_error != CL_SUCCESS) {
            std::cout << "Error creating context: " <<  descriptionOfError(ocl_error) << std::endl;
            delete [] k_matrix;
            exit(ocl_error);
        }
        // Command-queue initialisation
        ocl_queue = clCreateCommandQueue(ocl_context, ocl_device, CL_QUEUE_PROFILING_ENABLE, &ocl_error);
        if (ocl_error != CL_SUCCESS) {
            std::cout << "Error creating command queue: " << descriptionOfError(ocl_error) << std::endl;
            delete [] k_matrix;
            clReleaseContext(ocl_context);
            exit(ocl_error);
        }
        //Fin de l'initialisation de l'environnement opencl
        // allocation des objets en mémoire globale utilisé pour tout le programme
        //ocl_k_matrix la matrice de dimention k*k
        cl_mem ocl_k_matrix = clCreateBuffer(ocl_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*k*k, k_matrix, &ocl_error);
        if (ocl_error != CL_SUCCESS) {
            std::cout << "Error creating memory buffer: " << descriptionOfError(ocl_error) << std::endl;
            delete [] k_matrix;
            clReleaseCommandQueue(ocl_queue);
            clReleaseContext(ocl_context);
            exit(ocl_error);
        }
        //Libération de la mémoire occupé par la matrice K en espace HOST
        delete [] k_matrix;
        // fin de l'allocation des objets openCL globaux
        //construction du kernel de traitement opencl
        //Lecture des information nécessaire pour la construction du kernel openCL
        struct stat filestatus;
        stat( "filtre_LoG_opencl.cl", &filestatus );
        size_t src_size = filestatus.st_size;
        std::ifstream tmpifs;
        int flength;
        tmpifs.open("filtre_LoG_opencl.cl");
        tmpifs.seekg(0, std::ios::end);
        flength = tmpifs.tellg();
        tmpifs.seekg(0, std::ios::beg);
        char* sourceread = new char[flength+1];
        tmpifs.read(sourceread, flength);
        tmpifs.close();
        const char* source = sourceread;
        //Création du programme openCL
        cl_program ocl_program = clCreateProgramWithSource(ocl_context, 1, &source, &src_size, &ocl_error);
        if (ocl_error != CL_SUCCESS) {
            std::cout << "[clCreateProgramWithSource] Error creating memory buffer: " << descriptionOfError(ocl_error) << std::endl;
            clReleaseCommandQueue(ocl_queue);
            clReleaseContext(ocl_context);
            exit(ocl_error);
        }
        // Compilation du programme openCL
        ocl_error = clBuildProgram(ocl_program, 1, &ocl_device, NULL, NULL, NULL);
        if (ocl_error != CL_SUCCESS) {
            std::cout << "[clBuildProgram] Error creating memory buffer: " << descriptionOfError(ocl_error) << std::endl;
            clReleaseProgram(ocl_program);
            clReleaseCommandQueue(ocl_queue);
            clReleaseContext(ocl_context);
            exit(ocl_error);
        }
        // Affichage des logs
        char* build_log;
        size_t log_size;
        clGetProgramBuildInfo(ocl_program, ocl_device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        build_log = new char[log_size+1];
        clGetProgramBuildInfo(ocl_program, ocl_device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
        build_log[log_size] = '\0';
        std::cout << build_log << std::endl;
        delete[] build_log;
        // Extraction du kernel
        cl_kernel ocl_LoG_filter_kernel = clCreateKernel(ocl_program, "kernel_log_filter_3channel", &ocl_error);
        if (ocl_error != CL_SUCCESS) {
            std::cout << "[clCreateKernel] Error creating memory buffer: " << descriptionOfError(ocl_error) << std::endl;
            clReleaseProgram(ocl_program);
            clReleaseCommandQueue(ocl_queue);
            clReleaseContext(ocl_context);
            exit(ocl_error);
        }
        //supression des source après la création du kernel
        delete[] source;
        //fin de la construction du kernel de traitement opencl
	//Fin de l'initialisation de l'environnement openCL et création du kernel
        //On commence le traitement de la liste d'image
        while (!image_name.empty()) {
            std::string tmpstr;
            clock_gettime(CLOCK_MONOTONIC, &chrono_lap);
            in_row_pointers = png_read_decode_file(tmpstr.append(chemin_lecture).append(image_name.front()), img_infos);
            clock_gettime(CLOCK_MONOTONIC, &chrono_current);
            std::cout << "La lecture de l'image " << tmpstr << " à pris: " << (double)((chrono_current.tv_sec * 1000000000 + chrono_current.tv_nsec) - chrono_lap.tv_sec * 1000000000 + chrono_lap.tv_nsec) / (double)(1000000000) << "s" << std::endl;
            if (!in_row_pointers) {
                std::cout << "Impossible de traiter l'image:" << tmpstr << std::endl;
            } else {
                clock_gettime(CLOCK_MONOTONIC, &chrono_lap);
                image_info img_info_out;
                img_info_out.bit_depth = img_infos.bit_depth;
                img_info_out.channels_count = img_infos.channels_count;
                img_info_out.color_type = img_infos.color_type;
                img_info_out.height = img_infos.height - (k / 2) * 2;
                img_info_out.width = img_infos.width - (k / 2) * 2;
                img_info_out.interlacing = img_infos.interlacing;
                img_info_out.rowbytes = img_info_out.width * img_info_out.channels_count * img_info_out.bit_depth / 8;
                std::cout << "Traitement de l'image: " << tmpstr << std::endl;
                //création des taches et mémoire associée et lancement de la tache sur le GPU
                cl_mem ocl_inImage = clCreateBuffer(ocl_context, CL_MEM_COPY_HOST_PTR, sizeof(char)*(img_infos.height*img_infos.rowbytes), in_row_pointers, &ocl_error);
                if (ocl_error != CL_SUCCESS) {
                    std::cout << "Error creating memory buffer: " << descriptionOfError(ocl_error) << std::endl;
                    std::cout << "Impossible de traiter l'image:" << tmpstr << std::endl;
                }
                else {
                    cl_mem ocl_outImage = clCreateBuffer(ocl_context, CL_MEM_READ_WRITE, sizeof(char)* (img_info_out.height*img_info_out.rowbytes) ,NULL, &ocl_error);
                    if (ocl_error != CL_SUCCESS) {
                        clReleaseMemObject(ocl_inImage);
                        std::cout << "Error creating memory buffer: " << descriptionOfError(ocl_error) << std::endl;
                        std::cout << "Impossible de traiter l'image:" << tmpstr << std::endl;
                    }
                    else {
                        // Total number of work-items
			cl_event krun_event;
                        size_t local_ws;
			size_t global_ws = img_info_out.height*img_info_out.rowbytes;
                        clGetKernelWorkGroupInfo(ocl_LoG_filter_kernel, ocl_device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &local_ws, NULL);
                        //Préparation du lancement du kernel
                        ocl_error = clSetKernelArg(ocl_LoG_filter_kernel, 0, sizeof(int), &(img_info_out.rowbytes));
                        ocl_error |= clSetKernelArg(ocl_LoG_filter_kernel, 1, sizeof(int), &(img_info_out.height));
                        ocl_error |= clSetKernelArg(ocl_LoG_filter_kernel, 2, sizeof(int), &k);
                        ocl_error |= clSetKernelArg(ocl_LoG_filter_kernel, 3, sizeof(cl_mem), &ocl_k_matrix);
                        ocl_error |= clSetKernelArg(ocl_LoG_filter_kernel, 4, sizeof(cl_mem), &ocl_inImage);
                        ocl_error |= clSetKernelArg(ocl_LoG_filter_kernel, 5, sizeof(cl_mem), &ocl_outImage);
                        if (ocl_error != CL_SUCCESS) {
                            delete[] in_row_pointers;
                            delete[] out_row_pointers;
                            clReleaseMemObject(ocl_inImage);
                            clReleaseMemObject(ocl_outImage);
                            clReleaseKernel(ocl_LoG_filter_kernel);
                            clReleaseProgram(ocl_program);
                            clReleaseMemObject(ocl_k_matrix);
                            clReleaseCommandQueue(ocl_queue);
                            clReleaseContext(ocl_context);
                            std::cout << "Error setting kernel arguments: " << descriptionOfError(ocl_error) << std::endl;
                            exit(ocl_error);
                        }
                        // Lancement du kernel
                        ocl_error = clEnqueueNDRangeKernel(ocl_queue, ocl_LoG_filter_kernel, 1, NULL,&global_ws,NULL, 0, NULL, &krun_event);
                        if (ocl_error != CL_SUCCESS) {
                            std::cout << "Error starting kernel: " << descriptionOfError(ocl_error) << std::endl;
                            delete[] in_row_pointers;
                            clReleaseMemObject(ocl_inImage);
                            clReleaseMemObject(ocl_outImage);
                            clReleaseKernel(ocl_LoG_filter_kernel);
                            clReleaseProgram(ocl_program);
                            clReleaseMemObject(ocl_k_matrix);
                            clReleaseCommandQueue(ocl_queue);
                            clReleaseContext(ocl_context);
                            exit(ocl_error);
                        }
                        ocl_error = clFinish(ocl_queue);
                        if (ocl_error != CL_SUCCESS) {
                            std::cout << "Error running kernel: " << descriptionOfError(ocl_error) << std::endl;
                            delete[] in_row_pointers;
                            clReleaseMemObject(ocl_inImage);
                            clReleaseMemObject(ocl_outImage);
                            clReleaseKernel(ocl_LoG_filter_kernel);
                            clReleaseProgram(ocl_program);
                            clReleaseMemObject(ocl_k_matrix);
                            clReleaseCommandQueue(ocl_queue);
                            clReleaseContext(ocl_context);
                            exit(ocl_error);
                        }
                        //Affichage du temps d'exécution du kernel
                        cl_ulong time_start, time_end;
			double total_time;
			clGetEventProfilingInfo(krun_event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
			clGetEventProfilingInfo(krun_event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
			total_time = time_end - time_start;
			std::cout << "Temps d'exécution du kernel:"<< (total_time / 1000000000) << std::endl;
                        //retour du traitement
                        out_row_pointers = new png_byte[img_info_out.rowbytes*img_info_out.height];
                        // Lecture des données de sortie
                        ocl_error =  clEnqueueReadBuffer(ocl_queue, ocl_outImage, CL_TRUE, 0, sizeof(char)* img_info_out.height*img_info_out.rowbytes  , out_row_pointers, 0, NULL, NULL);
                        if (ocl_error != CL_SUCCESS) {
                            std::cout << "Error getting data from GPU: " << descriptionOfError(ocl_error) << std::endl;
                            delete[] out_row_pointers;
                            delete[] in_row_pointers;
                            clReleaseMemObject(ocl_inImage);
                            clReleaseMemObject(ocl_outImage);
                            clReleaseKernel(ocl_LoG_filter_kernel);
                            clReleaseProgram(ocl_program);
                            clReleaseMemObject(ocl_k_matrix);
                            clReleaseCommandQueue(ocl_queue);
                            clReleaseContext(ocl_context);
                            exit(ocl_error);
                        }
                        ocl_error = clFinish(ocl_queue);
                        if (ocl_error != CL_SUCCESS) {
                            std::cout << "Error running kernel: " << descriptionOfError(ocl_error) << std::endl;
			    delete[] out_row_pointers;
                            delete[] in_row_pointers;
                            clReleaseMemObject(ocl_inImage);
                            clReleaseMemObject(ocl_outImage);
                            clReleaseKernel(ocl_LoG_filter_kernel);
                            clReleaseProgram(ocl_program);
                            clReleaseMemObject(ocl_k_matrix);
                            clReleaseCommandQueue(ocl_queue);
                            clReleaseContext(ocl_context);
                            exit(ocl_error);
                        }
                        //suppression des objets alloués pour le traitement de l'image
                        clReleaseMemObject(ocl_inImage);
                        clReleaseMemObject(ocl_outImage);
                    }
                } // Fin du traitement de l'Image avec OpenCL
                //Si le traitement de l'image à réussit
                if(ocl_error == CL_SUCCESS) {
                    clock_gettime(CLOCK_MONOTONIC, &chrono_current);
                    std::cout << "La répartition du travail et le traitement de l'image à pris: " << (double)((chrono_current.tv_sec * 1000000000 + chrono_current.tv_nsec) - chrono_lap.tv_sec * 1000000000 + chrono_lap.tv_nsec) / (double)(1000000000) << "s" << std::endl;
                    clock_gettime(CLOCK_MONOTONIC, &chrono_lap);
                    tmpstr.clear();

                    if (png_write_encode_file(tmpstr.append(chemin_ecriture).append(image_name.front()), img_info_out, out_row_pointers) == 0) {
                        clock_gettime(CLOCK_MONOTONIC, &chrono_current);
                        std::cout << "L'exportation de l'image " << tmpstr << " à pris: " << (double)((chrono_current.tv_sec * 1000000000 + chrono_current.tv_nsec) - chrono_lap.tv_sec * 1000000000 + chrono_lap.tv_nsec) / (double)(1000000000) << "s" << std::endl;
                    } else
                        std::cout << "Une erreur est survenu lors de l'exportation" << std::endl;
                }
                //on détruit les row_pointer
                delete[]  out_row_pointers;
                out_row_pointers = NULL;
                delete[] in_row_pointers;
                in_row_pointers = NULL;
            }
            image_name.pop_front();
        }
//Complete OpenCl cleanup
        clReleaseKernel(ocl_LoG_filter_kernel);
        clReleaseProgram(ocl_program);
        clReleaseMemObject(ocl_k_matrix);
        clReleaseCommandQueue(ocl_queue);
        clReleaseContext(ocl_context);

        clock_gettime(CLOCK_MONOTONIC, &chrono_current);
        std::cout << "Temps total: " << (double)((chrono_current.tv_sec * 1000000000 + chrono_current.tv_nsec) - chrono_start.tv_sec * 1000000000 + chrono_start.tv_nsec) / (double)(1000000000) << std::endl;
        return 0;
    }
}

