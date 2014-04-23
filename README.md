opencl_image_filter
===================

opencl implementation of an png image filter

Le programme prend en entré un fichier de configuration comme test.conf

sur la première ligne se trouvant la taille nXn de la matrice du filtre
sur les n lignes suivante la matrice avec les n valeurs de chaques ligne séparé d'un espace
sur la lisgne suivante le dossier ou se trouve les image d'entrée
sur la lisgne suivante le dossier de sortie
les lignes suivantes sont les nom des fichier à traiter

Notez que seul les images png 3 chanaux de 8 bits sont suportées