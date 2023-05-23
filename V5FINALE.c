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
	printf("j'ouvre le fichier\n");
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
		printf("Bonjour, je suis le processus fils\n");
                // Fermer l'extrémité de lecture du canal
                close(pipefd[0]);

                // Rediriger la sortie standard vers l'extrémité d'écriture du canal
                dup2(pipefd[1], STDOUT_FILENO);

                // Exécuter la commande
                //execl("/!bin/sh", "sh", "-c", buffer, (char *) NULL);
                char *args[] = {"sh", "-c", buffer, NULL};
                execvp(args[0], args);
                // Si exevp renvoie, cela signifie qu'il y a eu une erreur
                printf("Erreur lors de l'exécution de la commande.\n");
                exit(1);
            } else {
                // Nous sommes dans le processus père
		printf("Bonjour, je suis le processus père\n");
                // Fermer l'extrémité d'écriture du canal
                close(pipefd[1]);

                // Lire le résultat de l'exécution de la commande depuis l'extrémité de lecture du canal
                char result[BUFFER_SIZE];
                int nbytes;
    		int total_bytes = 0;
                while ((nbytes = read(pipefd[0], result + total_bytes, BUFFER_SIZE - total_bytes)) > 0) {
                    total_bytes += nbytes;
                    printf("Lecture en cours\n");
                }

                // Fermer l'extrémité de lecture du canal
                close(pipefd[0]);

                // Attendre la fin du processus fils spécifique créé par le fork() actuel
                waitpid(pid, &status, 0);

                // Afficher le résultat
                if (total_bytes >= 0) {
                    result[total_bytes] = '\0'; // Ajouter le caractère de fin de chaîne
                    printf("Le processus fils a retourné pour la commande \"%s\" : %s\n", buffer, result);
                } else {
                    printf("Erreur lors de la lecture du résultat depuis le canal.\n");
                }
                printf("Le processus fils a terminé avec le code de retour %d.\n\n\n", WEXITSTATUS(status));
                // condition qui arrète le programme en cas d'erreurs (choix personnel)
                if(WEXITSTATUS(status)!=0){exit(1);}
            }

    	}	
    
	    // Fermer le fichier shell
	    fclose(shell_file);
}
    return 0;
}

