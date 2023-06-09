#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE];
    pid_t pid;
    int status, pipefd[2];

    // Vérifier que la commande a été passée en argument
    if (argc < 2) {
        printf("Usage: %s <command> [arg1] [arg2] ...\n", argv[0]);
        exit(1);
    }

    // Créer un canal de communication entre le processus fils et le processus père
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Créer un processus fils
    pid = fork();

    if (pid == -1) {
        printf("Erreur lors de la création du processus fils.\n");
        exit(1);
    } else if (pid == 0) {
        // Nous sommes dans le processus fils

        // Fermer l'extrémité de lecture du canal
        close(pipefd[0]);

        // Rediriger la sortie standard vers l'extrémité d'écriture du canal
        dup2(pipefd[1], STDOUT_FILENO);

        // Exécuter la commande passée en argument
        execvp(argv[1], &argv[1]);

        // Si execvp renvoie, cela signifie qu'il y a eu une erreur
        printf("Erreur lors de l'exécution de la commande.\n");
        exit(1);
    } else {
        // Nous sommes dans le processus père

        // Fermer l'extrémité d'écriture du canal
        close(pipefd[1]);

        // Lire le résultat de l'exécution de la commande depuis l'extrémité de lecture du canal
        int nbytes = read(pipefd[0], buffer, BUFFER_SIZE);
// Attendre la fin du processus fils
        wait(&status);
        // Afficher le résultat
        if (nbytes > 0) {
            buffer[nbytes] = '\0'; // Ajouter le caractère de fin de chaîne
            printf("Le processus fils a retourné : %s\n", buffer);
        } else {
            printf("Erreur lors de la lecture du résultat depuis le canal.\n");
        }
        printf("Le processus fils a terminé avec le code de retour %d.\n", WEXITSTATUS(status));
    }

    return 0;
}
