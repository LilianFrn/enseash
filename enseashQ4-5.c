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
const char msg_exit[] = "Shutting down...\n";
const char msg_comm[] = ">>";

// Retour
const char stat_exit[] = "[exit:";
const char stat_sign[] = "[sign:";
const char stat_sep[] = "|";
const char stat_fins[] = "s]";
const char stat_finms[] = "ms]";

// Fonctionalités
const char inp_exit[] = "exit";
const char space[] = " ";

// Fonction d'affichage du retour
void commStatus(int s, int ms, int signal){

    char stat[20]; // Stockage du retour
    char stat_buff[10]; // Stockage des nombres

    // choix entre [exit et [sign
    if (signal == 0) {strcat(stat,stat_exit);}
    else {strcat(stat,stat_sign);}
    // Ajout du numéro du signal
    sprintf(stat_buff,"%d",signal);
    strcat(stat,stat_buff);

    strcat(stat,stat_sep);

    if (s > 0) { // Si le temps est > 1s
        sprintf(stat_buff,"%d",s);
        strcat(stat,stat_buff);
        strcat(stat,stat_fins);
    }
    else { // Temps en ms
        sprintf(stat_buff,"%d",ms);
        strcat(stat,stat_buff);
        strcat(stat,stat_finms);
    }
    write(STDOUT_FILENO,stat,strlen(stat));
}

int main(int argc, char *argv[]){

    UNUSED(argc);
    UNUSED(argv);

    char buff[BUFF_SIZE]; // Buffer d'input
    char buff_com[BUFF_SIZE] = ""; // Buffer pour stocker la commande 

    int ret,status,signal,s,ms; // Retour write / status et signal du pid / temps en s et ls / Compteur

    struct timespec start, stop; // Points temporels pour avoir le delta
    char *ptr; // Pointeur pour trier buff

    write(STDOUT_FILENO, msg_welcome, strlen(msg_welcome)); //Lecture de msg_welcome

while(1){
        write(STDOUT_FILENO, msg_comm, strlen(msg_comm)); // Indicateur de la ligne active de commande
        ret = read(STDIN_FILENO, buff, BUFF_SIZE); // Demande d'input utilisateur dans buff
        buff[ret-1] = 0; // Suppression d'un caractère en bout de liste (visiblement neccessaire)

        ptr = strtok(buff, space); // Troncature du buffer au prochian espace, ici ptr = commande
        strcat(buff_com,ptr); // On stocke la commande

        clock_gettime(CLOCK_REALTIME, &start); // Démarrage de la clock

        // exit
        if((strcmp(inp_exit, buff)==0) || (buff[-1] == 0 && ret==0)){ //Comparaison de buff avec inp_exit ou détection d'une input nulle (ctrl-d)
            write(STDOUT_FILENO, msg_exit, strlen(msg_exit));
            exit(EXIT_SUCCESS);
        }

        // fork()
        pid_t pid = fork();

        if(pid == -1){ // Test d'erreur
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0){
            // Commande classique
            execlp(buff_com, buff_com, (char *) NULL);// Exec de la commande tuant le fils si la commande existe
            write(STDOUT_FILENO, msg_undefined, strlen(msg_undefined)); //Message d'erreur
            exit(EXIT_FAILURE); //Kill du fils si l'exec n'a pas focntionné
        }
        else{
            wait(&status);
            if(WIFEXITED(status)){ // retour si bonne execution
                signal = 0;
            }
            if(WIFSIGNALED(status)){ // retour si interrompu par un signal
                signal = WTERMSIG(status);
            }
        }

        // Retour
        clock_gettime(CLOCK_REALTIME, &stop); // Arrêt de la clock
        s = floor((stop.tv_sec - start.tv_sec)); // Delta des secondes
        ms = floor((stop.tv_nsec - start.tv_nsec)/MILLION); // Delta des millisecondes
        commStatus(s,ms,signal); // Fonction d'affichage du retour

        // Réinitialisation
        memset(buff_com,0,BUFF_SIZE);
    }
}