#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>

// Constantes
#define BUFF_SIZE 64
#define MILLION 1000000L
#define UNUSED(x) (void)(x)

// UI
const char msg_welcome[] = "Bienvenue sur le Shell de l'ENSEA\nPour quitter, tapez 'exit'\n";
const char msg_undefined[] = "Commande non reconnue\n";
const char msg_comm[] = ">>";

int main(int argc, char *argv[]){
    UNUSED(argc);
    UNUSED(argv);
    char buff[BUFF_SIZE];

    int ret=0; // Retour write

    write(STDOUT_FILENO, msg_welcome, strlen(msg_welcome)); //Lecture de msg_welcome

while(1){
        write(STDOUT_FILENO, msg_comm, strlen(msg_comm)); // Indicateur de la ligne active de commande
        ret = read(STDIN_FILENO, buff, BUFF_SIZE); // Demande d'input utilisateur dans buff
        UNUSED(ret);
    }
}