//
// Created by bourg on 03-01-22.
//
/**

Question 2 — – Question Programmation system (12 points)
 Nous vous demandons d’´ecrire la partie serveur d’une application client-serveur
 qui va stocker et traiter des images de taille m × n fournies par des clients.
Les images seront transmises comme suit par le client :
 d’abord le client enverra, sur le réseau, les dimensions m et n de la matrice qui stockera l’image,
 ensuite l’image sera transmise pixel par pixel ainsi que ligne par ligne.
Cette matrice stockera les informations d’un seul pixel par case
 et chaque pixel sera enregistré sous la forme d’un long int.
Cette image sous forme matricielle devra être transmise à la fonction
 ` traitement(long int** mat, int m ,int n)
 pour être traitée,
 mais devra également être stockée dans le dossier /home/history/
 avec comme nom de fichier
 index-adresseIP.img,
 où index reprend le nombre reçu d’images depuis le lancement du serveur et
 où adresseIP correspond a l’adresse IP du client qui a envoyé l’image,
 mais dont les points auront été remplacés au préalable par le caractère ’_’.
Vous pouvez considérer la fonction
 traitement(long int** mat, int m ,int n) comme déjà implantée,
 et donc utilisable telle quelle.
 Toutefois, vous devez implanter l’enregistrement de l’image dans le système de fichiers.
 Vous pouvez utiliser indifféremment le fork de processus ou le multithreading,
 mais vous devez justifier votre choix.
 Nous vous demandons d’implanter votre solution en C
 et de prévoir les différents traitements des erreurs pouvant survenir
 au cours de l’exécution de votre programme.

*/