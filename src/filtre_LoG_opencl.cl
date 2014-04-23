/* **********************************************
 * Fichier filtre_LoG.cpp
 *
 * Mathieu Simard
 * Ce code peut être redistribué et utiliser sous lisence GPL
 *
 * fichier source contenant les fonction permettant
 * appliquer le filtre LoG
 *
 * *********************************************/

// Kernel kernel_log_filter_application
// servant à l'application du filtre à l'image
__kernel void kernel_log_filter_3channel(
int outrowbites,
int outheight,
int k,
__global const float* k_matrix,
__global const uchar* inImage,
__global uchar* outImage
)
{
    int position = get_global_id(0);
    int wssize = get_global_size(0);
    float somme;
    float decallage;
  for (int nl=0; (nl*wssize+position) < (outheight*outrowbites) ;nl++)
  {
	decallage = convert_float(nl*wssize+position)/ convert_float(outrowbites);
	somme = 0;
	for(int i=0; i<k;i++)
	{
	  for(int j=0; j<k;j++)
	  {
	    somme += convert_float(k_matrix[i*k+j]*
	    (convert_float(
	      inImage[(nl*wssize+position) + convert_int(decallage)*((k/2)*2*3) 
	      + j*3 + (i*(outrowbites+((k/2)*2*3)))]
	    )));
	  }
	}
	outImage[nl*wssize+position] = convert_uchar_sat(somme);
      }
}
