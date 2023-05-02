#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE];
    pid_t pid;
    int status;

    // Vérifier que la commande a été passée en argument
    if (argc < 2) {
        printf("Usage: %s <command> [arg1] [arg2] ...\n", argv[0]);
        exit(1);
    }

    // Créer un processus fils
    pid = fork();

    if (pid == -1) {
        printf("Erreur lors de la création du processus fils.\n");
        exit(1);
    } else if (pid == 0) {
        // Nous sommes dans le processus fils

        // Exécuter la commande passée en argument
        execvp(argv[1], &argv[1]);

        // Si execvp renvoie, cela signifie qu'il y a eu une erreur
        printf("Erreur lors de l'exécution de la commande.\n");
        exit(1);
    } else {
        // Nous sommes dans le processus père

        // Attendre la fin du processus fils
        wait(&status);

        // Afficher le résultat
        printf("Le processus fils a terminé avec le code de retour %d.\n", WEXITSTATUS(status));
    }

    return 0;
}
