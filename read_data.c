#include <stdio.h>
#include <stdlib.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define buf_size 128

char buffer[buf_size];

int main(int argc, char const *argv[])
{
    int fd = 0;
    FILE *fpt;
    int x = 0;

    while(1){
        fd = open("/dev/alarma", O_RDWR);
        fpt = fopen("data.csv", "a+"); //Se crea el archivo y se setea para escribir los datos
        printf("fd = %d     fpt = %d    errno = %d ", fd, fpt, errno);

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
        fprintf(fpt,"%d,%s\n", x, buffer);
        printf("x=%d    led_value=%s\n", x, buffer);
        close(fd);
        fclose(fpt);
        usleep(500000); //500ms
        x++;
    }
    return 0;
}