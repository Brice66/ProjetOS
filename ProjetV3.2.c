#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    pid_t pid;
    int status, pipefd[2];

    // Vérifier que les fichiers shell ont été passés en argument
    if (argc < 2) {
        printf("Usage: %s <shell_script1> <shell_script2> ... <shell_scriptN>\n", argv[0]);
        exit(1);
    }

    // Boucle pour exécuter chaque fichier shell
    for (int i = 1; i < argc; i++) {

        // Ouvrir le fichier shell
        FILE* shell_file = fopen(argv[i], "r");
        if (!shell_file) {
            perror("Erreur lors de l'ouverture du fichier");
            exit(EXIT_FAILURE);
        }

        // Créer un canal de communication entre le processus fils et le processus père 
        // crée un canal de communication et vérifie si la création a réussi
        if (pipe(pipefd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        // Boucle pour exécuter chaque ligne du fichier shell
        char buffer[BUFFER_SIZE];
        while (fgets(buffer, BUFFER_SIZE, shell_file) != NULL) {
            // Supprimer le caractère de fin de ligne
            buffer[strcspn(buffer, "\n")] = 0;

            // Créer un processus fils pour exécuter la commande
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

                // Exécuter la commande
                execl("/bin/sh", "sh", "-c", buffer, (char *) NULL);

                // Si execl renvoie, cela signifie qu'il y a eu une erreur
                printf("Erreur lors de l'exécution de la commande.\n");
                exit(1);
            } else {
                // Nous sommes dans le processus père

                // Fermer l'extrémité d'écriture du canal
                close(pipefd[1]);

                // Lire le résultat de l'exécution de la commande depuis l'extrémité de lecture du canal
                char result[BUFFER_SIZE];
                int total_bytes = 0;
                int nbytes;
                while ((nbytes = read(pipefd[0], result + total_bytes, BUFFER_SIZE - total_bytes)) > 0) {
                    total_bytes += nbytes;
                }

                // Attendre la fin du processus fils
                wait(&status);

                // Afficher le résultat
                if (total_bytes > 0) {
                    result[total_bytes] = '\0'; // Ajouter le caractère de fin de chaîne
                    printf("Le processus fils a retourné pour la commande \"%s\" : %s\n", buffer, result);
                } else {
                    printf("Erreur lors de la lecture du résultat depuis le canal.\n");
                }
                printf("Le processus fils a terminé avec le code de retour %d.\n", WEXITSTATUS(status));
        }

    }
    
    // Fermer le fichier shell
    fclose(shell_file);
}
    return 0;
}

/*
La boucle while lit les données du canal de communication (pipefd[0]) dans le tableau result à partir de la position result + total_bytes. 
La fonction read est utilisée pour lire les données. La lecture se fait par morceaux de taille BUFFER_SIZE - total_bytes, 
et le nombre de bytes lus est stocké dans la variable nbytes.

À chaque itération de la boucle, le nombre de bytes lus (nbytes) est ajouté à la variable total_bytes pour suivre le nombre total de bytes lus jusqu'à présent.

La boucle continue de lire les données jusqu'à ce qu'il n'y ait plus de données à lire (nbytes <= 0), 
ce qui signifie que le processus fils a terminé d'écrire dans le canal de communication.

Ensuite, la ligne wait(&status) attend la fin du processus fils. La fonction wait suspend l'exécution du processus père jusqu'à ce que le processus fils se termine, 
et le code de retour du processus fils est stocké dans la variable status.

Après avoir attendu la fin du processus fils, le code vérifie si des bytes ont été lus à partir du canal (if (total_bytes > 0)). 
Si c'est le cas, il ajoute un caractère de fin de chaîne ('\0') à la fin du tableau result et affiche le résultat avec la commande exécutée.

Si aucune donnée n'a été lue à partir du canal, il affiche un message d'erreur indiquant qu'il y a eu une erreur lors de la lecture du résultat.

Enfin, la ligne printf("Le processus fils a terminé avec le code de retour %d.\n", WEXITSTATUS(status)); 
affiche le code de retour du processus fils à l'aide de la macro WEXITSTATUS. Elle extrait le code de retour à partir de la variable status retournée par la fonction wait.
*/
