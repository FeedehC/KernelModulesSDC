#include <stdio.h>
#include <stdlib.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define buf_size 256

char buffer[buf_size];

int main(int argc, char const *argv[])
{
    int fd = 0;
    FILE *fpt;
    int counta = 0;

    while(1){
        fd = open("/dev/alarma", O_RDWR);
        fpt = fopen("data.csv", "a+"); //Se crea el archivo y se setea para escribir los datos
        if(fd < 0 || fpt < 0){
            printf("Error al abrir el archivo\n");
            exit(EXIT_FAILURE);
        }
        memset(buffer, '\0', buf_size); //Se inicializa el buffer con todos ceros
        if(read(fd, buffer, buf_size)){
            printf("Error al leer los datos\n");
            exit(EXIT_FAILURE);
        }
        //Formateo el string y lo envio al archivo fpt
        fprintf(fpt,"%d,%s\n", counta, buffer);
        close(fd);
        fclose(fpt);
        usleep(500000); //500ms
        counta++;
    }
    return 0;
}