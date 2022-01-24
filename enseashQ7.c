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
const char rb[] = ">";
const char lb[] = "<";

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
    char **buff_arg; // Buffer pour stokcer les arguments

    int ret,status,signal,s,ms,i,j; // Retour write / status et signal du pid / temps en s et ls / Compteur / nombre d'espaces
    int where_rb; // Position du >
    int where_lb; // postiion du <
    int i_free; // Compteur de free()

    struct timespec start, stop; // Points temporels pour avoir le delta
    char *ptr; // Pointeur pour trier buff

    write(STDOUT_FILENO, msg_welcome, strlen(msg_welcome)); //Lecture de msg_welcome

while(1){
        write(STDOUT_FILENO, msg_comm, strlen(msg_comm)); // Indicateur de la ligne active de commande
        ret = read(STDIN_FILENO, buff, BUFF_SIZE); // Demande d'input utilisateur dans buff
        buff[ret-1] = 0; // Suppression d'un caractère en bout de liste (visiblement neccessaire)

        // Initialisations
        where_rb=-1;
        where_lb=-1;
        i=1;

        // Mise en forme de la commande
        ptr = strtok(buff, space); // Troncature du buffer au prochian espace, ici ptr = commande
        strcat(buff_com,ptr); // On stocke la commande
        buff_arg = malloc(BUFF_SIZE*sizeof(char*)); // Malloc du nombre d'élément dans buff_arg (max 64 args)
        buff_arg[0] = malloc(strlen(ptr)*sizeof(char)); // Malloc de la taille de la commande
        buff_arg[0] = ptr; // Ajout de la commande
        ptr = strtok(NULL, space); // Nouvelle troncature à un espace

        while (ptr != NULL) {
            if (ptr[0] == lb[0] && ptr[1]==0) {where_lb = i;} // Test pour trouver "<"
            if (ptr[0] == rb[0] && ptr[1]==0) {where_rb = i;} // Test pour trouver ">"
            buff_arg[i] = malloc(strlen(ptr)*sizeof(char)); // Malloc de la taille du ieme argument
            buff_arg[i] = ptr; // Ajout du ieme argument
            i++;
            ptr = strtok(NULL, space);
	    }
        buff_arg[i] = ptr;
        
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
            if(where_rb >= 0 && where_lb==-1){ // Commande si on détecte un ">"
                freopen(buff_arg[where_rb+1], "w", stdout); // redirection du fichier vers stdout
                for(j=where_rb;buff_arg[j]!=0;j++){buff_arg[j]=0;} // netoyage de la commande a partir du ">"
                execvp(buff_com,buff_arg); // Execution
                freopen("/dev/tty", "w", stdout); // Redirection de stdout vers son origine
            }
            else if(where_lb >= 0 && where_rb==-1){ // Commande si on détecte un "<"
                freopen(buff_arg[where_lb+1], "r", stdin); // redirection du fichier vers stdin
                for(j=where_lb;buff_arg[j]!=0;j++){buff_arg[j]=0;} // netoyage de la commande a partir du "<"
                execvp(buff_com,buff_arg); // Execution
                freopen("/dev/tty", "r", stdin); // Redirection de stdin vers son origine
            }
            else{ // Commande classique
                execvp(buff_com,buff_arg); // Exec de la commande tuant le fils si la commande existe
                write(STDOUT_FILENO, msg_undefined, strlen(msg_undefined)); //Message d'erreur
                exit(EXIT_FAILURE); //Kill du fils si l'exec n'a pas focntionné
            }
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
        memset(buff_arg,0,BUFF_SIZE);
        for (i_free = 0; i_free <= BUFF_SIZE; i_free++) {free(buff_arg[i_free]);} // Libération de l'espace de chaque char[] de buff_arg
        free(buff_arg);
    }
}