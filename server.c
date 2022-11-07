#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <utmp.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <time.h>

#define SEND "Link-Client-to-Server"
#define RECEIVE "Link-SERVER-to-CLIENT"

char comanda[100] = "";
char raspuns[100] = "";
char username[100] = "";
char PID[50] = "";
int logged_in = 0;

int main(void)
{

    // CREARE CANALE DE COMUNICARE --------------------------------------------------
    // creare canal fifo sau verificare daca exista deja pentru (CLIENT -> SERVER)
    if ((mknod(SEND, S_IFIFO | 0666, 0) == -1) && (errno != EEXIST))
    {
        printf("Could not create fifo SEND file!\n");
        printf("Server closed.\n");
        return 1;
    }

    // creare canal fifo sau verificare daca exista deja pentru (SERVER -> CLIENT)
    if ((mknod(RECEIVE, S_IFIFO | 0666, 0) == -1) && (errno != EEXIST))
    {
        printf("Could not create fifo RECEIVE file!\n");
        printf("Server closed.\n");
        return 2;
    }
    // CREARE CANALE DE COMUNICARE --------------------------------------------------





    //(CLIENT -> SERVER)
    // aici se creeaza descriptorul fifo SEND si se deschide canalul in modul de citire
    int fifo_descriptor_client_to_server = open(SEND, O_RDONLY);

    // aici verificam daca canalul a fost deschis cu succes sau nu
    if (fifo_descriptor_client_to_server == -1)
    {
        printf("Error while opening the fifo SEND on the server!\n");
        printf("Server closed.\n");
        return 3;
    }
    //(CLIENT -> SERVER)



    //(SERVER -> CLIENT)
    // aici se creeaza descriptorul fifo RECEIVE si se deschide canalul in modul de scriere
    int fifo_descriptor_server_to_client = open(RECEIVE, O_WRONLY);

    // aici verificam daca canalul a fost deschis cu succes sau nu
    if (fifo_descriptor_server_to_client == -1)
    {
        printf("Error while opening the fifo RECEIVE!\n");
        printf("Client closed.\n");
        return 4;
    }
    //(SERVER -> CLIENT)



    // ascultam pentru comenzi

    while (1)
    {

        // aici citim comanda primita de la client
        int lungime_comanda = 0;
        if ((lungime_comanda = read(fifo_descriptor_client_to_server, comanda, sizeof(comanda))) == -1)
        {
            printf("Error while reading from client!");
            printf("Server closed.");
            return 5;
        }

        comanda[lungime_comanda - 1] = '\0';
        // aici eliminam enter-ul trimis si nu luam in considerare caracterul null
        lungime_comanda = lungime_comanda - 1;

        // afisam comanda primita pe terminalul serverului
        printf("Comanda primita este: <%s>\n", comanda);
        printf("Lungime comanda: %i\n\n", lungime_comanda);

        // verificam comanda primita daca este una valida
        if (strcmp(comanda, "quit") == 0)
        {
            logged_in = 0;
            strcpy(raspuns, "code100");
        }
        else if (strcmp(comanda, "logout") == 0)
        {

            if (logged_in == 1)
            {
                logged_in = 0;
                stpcpy(raspuns, "code101");
            }
            else
            {
                strcpy(raspuns, "code102");
            }
        }

        else if (strstr(comanda, "login : ") != NULL)
        {

            if (logged_in == 1)
            {
                strcpy(raspuns, "code103");
            }
            //--------------------------------login
            else
            {
                strcpy(username, comanda + 8);

                int fd[2];
                int file;
                // fd[0] = read
                // fd[1] = write

                if (pipe(fd) == -1)
                {
                    printf("Eroare la pipe!\n");
                    return 1;
                }

                int id;
                if ((id = fork()) == -1)
                {
                    printf("Eroare la fork!\n");
                    return 2;
                }

                // copil
                if (id == 0)
                {

                    // suntem in copil
                    close(fd[0]);
                    int exit_code = 1;

                    int ok = 0;
                    char text[100] = "";
                    int lungime_text;

                    file = open("/home/andrei/Desktop/retele/tema1/users.txt", O_RDONLY);
                    if (file == -1)
                    {
                        printf("Eroare la open file!");
                        return 2;
                    }

                    lungime_text = read(file, text, sizeof(text));

                    // printf("%s",test);

                    char *p = strtok(text, "\n");
                    while (p)
                    {
                        if (strcmp(username, p) == 0)
                        {
                            ok = 1;
                            break;
                        }
                        p = strtok(NULL, "\n");
                    }

                    if (ok == 1)
                    {
                        write(fd[1], "OK", 3);
                    }

                    else
                    {
                        write(fd[1], "NO", 3);
                    }

                    close(fd[1]);
                    exit(exit_code);
                }
                else
                {

                    // suntem in parinte
                    close(fd[1]);
                    int child_return;

                    char flag[10] = "";
                    wait(&child_return);
                    read(fd[0], flag, sizeof(flag));
                    if (strcmp(flag, "NO") == 0)
                        strcpy(raspuns, "code104");
                    if (strcmp(flag, "OK") == 0)
                    {
                        strcpy(raspuns, "code105");
                        logged_in = 1;
                    }

                    WEXITSTATUS(child_return);
                    close(fd[0]);
                }
            }
            //--------------------------------login
        }

        else if (strcmp(comanda, "get-logged-users") == 0)
        {
            //--------------------------------get-logged-users
            if (logged_in == 1)
            {

                int sockets[2];
                char buff[1000];

                if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1)
                {
                    printf("Eroare la socketpair!");
                    return 8;
                }

                int id = fork();
                if (id == -1)
                {
                    printf("Eroare la fork!");
                    return 9;
                }

                if (id != 0)
                {                      // PARENT
                    close(sockets[0]); // child socket
                    int child_return;
                    wait(&child_return);
                    read(sockets[1], buff, sizeof(buff));
                    strcpy(raspuns,buff);
                    WEXITSTATUS(child_return);
                    close(sockets[1]);
                }
                else
                {                      // CHILD
                    close(sockets[1]); // parent socket
                                   // get-logged users-code
                    int exit_code = 1;
                    char result[1500] = "";
                    char time[500] = "";
                    char info[100] = "";

                    struct utmp *u;

                    u = getutent();

                    while (u != NULL)
                    {
                        strcat(result,"\n");
                        strcat(result, "USER : ");
                        strncpy(info, u->ut_user, 32);
                        info[32] = '\0';
                        strcat(result, info);
                        strcat(result, "\n");

                        strcat(result, "HOST : ");
                        strncpy(info, u->ut_host, 32);
                        info[32] = '\0';
                        strcat(result, info);
                        strcat(result, "\n");

                        strcat(result, "TIME : ");
                        sprintf(time,"%d",u->ut_tv.tv_sec);
                        strcat(result, time);

                        strcat(result, "\n");

                        u = getutent();
                    }
                    

                    write(sockets[0],result, sizeof(result));

                    close(sockets[0]);
                    exit(exit_code);
                }
            }
            else
            {
                strcpy(raspuns, "code102");
            }
            //--------------------------------get-logged-users
        }

        else if (strstr(comanda, "get-proc-info : ") != NULL)
        {
            //--------------------------------get-proc-info
            if (logged_in == 1)
            {
                strcpy(PID, comanda + 16);

                int fd[2];
                int file;
                // fd[0] = read
                // fd[1] = write

                if (pipe(fd) == -1)
                {
                    printf("Eroare la pipe!\n");
                    return 1;
                }

                int id;
                if ((id = fork()) == -1)
                {
                    printf("Eroare la fork!\n");
                    return 2;
                }

                // copil
                if (id == 0)
                {

                    // suntem in copil
                    close(fd[0]);
                    int exit_code = 2;

                    char text_from_file[1500] = "";
                    char path[100] = "/proc/\0";
                    char result[300] = "";

                    strcat(path, PID);
                    strcat(path, "/status");

                    file = open(path, O_RDONLY);
                    if (file == -1)
                    {
                        strcpy(result, "Nu s-a putut deschide fisierul sau este inexistent!\n");
                        write(fd[1], result, sizeof(result));
                        exit(exit_code);
                    }

                    if (read(file, text_from_file, sizeof(text_from_file)) == -1)
                    {
                        printf("Eroare la citire din fisier pid!");
                        return 3;
                    }

                    char *p = strtok(text_from_file, "\n");
                    strcat(result,"\n");

                    while (p)
                    {

                        if (strstr(p, "Name"))
                        {
                            strcat(result, p);
                            strcat(result, "\n");
                        }

                        if (strstr(p, "State"))
                        {
                            strcat(result, p);
                            strcat(result, "\n");
                        }

                        if (strstr(p, "PPid"))
                        {
                            strcat(result, p);
                            strcat(result, "\n");
                        }

                        if (strstr(p, "Uid"))
                        {
                            strcat(result, p);
                            strcat(result, "\n");
                        }
                        if (strstr(p, "VmSize"))
                        {
                            strcat(result, p);
                            strcat(result, "\n");
                        }

                        p = strtok(NULL, "\n");
                    }

                    write(fd[1], result, sizeof(result));

                    close(fd[1]);
                    exit(exit_code);
                }
                else
                {

                    // suntem in parinte
                    close(fd[1]);
                    int child_return;
                    char result[300] = "";

                    wait(&child_return);

                    read(fd[0], result, sizeof(result));
                    strcpy(raspuns, result);

                    WEXITSTATUS(child_return);
                    close(fd[0]);
                }
            }
            else
            {
                strcpy(raspuns, "code102");
            }
            //--------------------------------get-proc-info
        }

        else
        {
            strcpy(raspuns, "code404");
            printf("Eroare: Comanda nu exista!\n\n");
        }

        // trimitem raspuns la client
        if (write(fifo_descriptor_server_to_client, raspuns, sizeof(raspuns)) == -1)
        {
            printf("Error while writing response to the fifo!");
            printf("Client closed.");
            return 6;
        }
    }

    // aici inchidem canalele fifo de comunicare
    if ((close(fifo_descriptor_client_to_server) == -1) || (close(fifo_descriptor_server_to_client) == -1))
    {

        printf("Error while closing fifo files!\n");
        return 7;
    }

    return 0;
}
