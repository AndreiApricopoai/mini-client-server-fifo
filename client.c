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


#define SEND "Link-Client-to-Server"
#define RECEIVE "Link-SERVER-to-CLIENT"

char comanda[100]="";
char raspuns[100]="";





int main(void) {


    //CREARE CANALE DE COMUNICARE --------------------------------------------------
    //creare canal fifo sau verificare daca exista deja (CLIENT -> SERVER)
    if((mknod(SEND, S_IFIFO | 0666, 0) == -1) && (errno != EEXIST)){
        printf("Could not create fifo SEND file!\n");
        printf("Client closed.\n");
        return 1;
    }

    //creare canal fifo sau verificare daca exista deja (SERVER -> CLIENT)
    if((mknod(RECEIVE, S_IFIFO | 0666, 0) == -1) && (errno != EEXIST)){
        printf("Could not create fifo RECEIVE file!\n");
        printf("Client closed.\n");
        return 2;
    }
    //CREARE CANALE DE COMUNICARE --------------------------------------------------






    //(CLIENT -> SERVER)
    //aici se creeaza descriptorul fifo SEND si se deschide canalul in modul de scriere neblocant
    int fifo_descriptor_client_to_server = open(SEND, O_WRONLY | O_NONBLOCK);

    //aici verificam daca canalul a fost deschis cu succes sau nu
    if(fifo_descriptor_client_to_server == -1){
        printf("Error while opening the fifo SEND or the server is not opened!\n");
        printf("Client closed.\n");
        return 3;
    }
    //(CLIENT -> SERVER)







    //(SERVER -> CLIENT)
    //aici se creeaza descriptorul fifo RECEIVE si se deschide canalul in modul de citire 
    int fifo_descriptor_server_to_client = open(RECEIVE, O_RDONLY);

    //aici verificam daca canalul a fost deschis cu succes sau nu
    if(fifo_descriptor_server_to_client == -1){
        printf("Error while opening the fifo RECEIVE!\n");
        printf("Client closed.\n");
        return 4;
    }
    //(SERVER -> CLIENT)




    //trimitem comenzi catre server si primim raspuns

    while(1)
    {
        //citim comenzi de la tastatura
        int lungime_comanda = 0;
        if((lungime_comanda = read(0,comanda,sizeof(comanda))) == -1){
            printf("Error while reading from stdin!\n");
            printf("Client closed.\n");
            return 5;
        }
        comanda[lungime_comanda-1] = '\0';


        //trimitem comanda pentru analiza catre server 
        if(write(fifo_descriptor_client_to_server, comanda, lungime_comanda) == -1){
            printf("Error while writing to the fifo SEND!\n");
            printf("Client closed.\n");
            return 6;
        }



        //primim raspunsul de la server
        int lungime_raspuns = 0;
        if((lungime_raspuns = read(fifo_descriptor_server_to_client,raspuns,sizeof(raspuns)) == -1)){
            printf("Error while reading response from server!\n");
            printf("Client closed.\n");
            return 7;
        }


        
        //ANALIZAM RASPUNSUL DE LA SERVER
        if(strcmp(raspuns,"code100") == 0){
            printf("%ld-",strlen("Clientul se va inchide!"));
            printf("Clientul se va inchide!\n");
            printf("-----------------------------------------------------\n\n");
            break;
        }
        else if(strcmp(raspuns,"code101") == 0){
            printf("%ld-",strlen("Deconectat cu succes!"));
            printf("Deconectat cu succes!\n");
            printf("-----------------------------------------------------\n\n");
        }
        else if(strcmp(raspuns,"code102") == 0){
            printf("%ld-",strlen("Trebuie sa fii conectat pentru a utiliza comanda!"));
            printf("Trebuie sa fii conectat pentru a utiliza comanda!\n");
            printf("-----------------------------------------------------\n\n");
        }
        else if(strcmp(raspuns,"code103") == 0){
            printf("%ld-",strlen("Esti deja logat pe un cont!"));
            printf("Esti deja logat pe un cont!\n");
            printf("-----------------------------------------------------\n\n");
        }
        else if(strcmp(raspuns,"code104") == 0){
            printf("%ld-",strlen("Conectare esuata (username invalid)!"));
            printf("Conectare esuata (username invalid)!\n");
            printf("-----------------------------------------------------\n\n");
        }
        else if(strcmp(raspuns,"code105") == 0){
            printf("%ld-",strlen("Ai fost conectat cu succes!"));
            printf("Ai fost conectat cu succes!\n");
            printf("-----------------------------------------------------\n\n");
        }
        else if(strcmp(raspuns,"code404") == 0){
            printf("%ld-",strlen("Eroare: Comanda nu exista!"));
            printf("Eroare: Comanda nu exista!\n");
            printf("-----------------------------------------------------\n\n");
        }
        else{
            printf("%s\n",raspuns);
            printf("-----------------------------------------------------\n\n");
        }
        

    }

    //aici inchidem canalele fifo de comunicare
    if((close(fifo_descriptor_client_to_server) == -1) || (close(fifo_descriptor_server_to_client) == -1)) {

        printf("Error while closing fifo files!\n");
        return 8;

    }
 
   

    return 0;
}
