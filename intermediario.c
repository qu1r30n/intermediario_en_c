#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cabeceras/cabeceras_procesos/00_cabeceras_del_sistema/operaciones_textos.h"
#include "cabeceras/cabeceras_procesos/00_cabeceras_del_sistema/var_fun_GG.h"

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

/*
 * leer — Lee todas las lineas del archivo f.
 * Retorna un arreglo de strings (char**) terminado en NULL,
 * o NULL si el archivo esta vacio o falla la memoria.
 * El caller debe liberar cada string y el arreglo con free().
 * El parametro total_out recibe la cantidad de lineas leidas.
 */
static char **leer(FILE *f, int *total_out)
{
    char **lineas = NULL; /* arreglo de lineas leidas        */
    int total = 0;        /* cantidad de lineas acumuladas   */
    int c;                /* caracter leido del archivo      */
    char *s = NULL;       /* buffer de la linea actual       */
    size_t n = 0;         /* longitud del buffer actual      */

    while ((c = fgetc(f)) != EOF)
    {
        /* Agrandar el buffer para el caracter mas el terminador */
        char *t = (char *)realloc(s, n + 2);
        if (t == NULL) { free(s); break; }
        s = t;
        s[n++] = (char)c;

        if (c == '\n') /* fin de linea: guardar y reiniciar buffer */
        {
            s[n] = '\0';
            char **tmp = (char **)realloc(lineas, (size_t)(total + 1) * sizeof(char *));
            if (tmp == NULL) { free(s); break; }
            lineas = tmp;
            lineas[total++] = s;
            s = NULL;
            n = 0;
        }
    }
    free(s); /* descartar fragmento sin salto de linea al final */

    *total_out = total;
    return lineas; /* NULL si no se leyo ninguna linea */
}

/*
 * agregar — Escribe linea al final del archivo f.
 */
static void agregar(FILE *f, const char *linea)
{
    fputs(linea, f);
}

/*
 * eliminar — Reescribe f conservando solo las lineas donde quitar[j] == 0.
 * Las lineas marcadas con quitar[j] == 1 quedan descartadas.
 * Trunca el archivo al nuevo tamano despues de reescribir.
 */
static void eliminar(FILE *f, char **lineas, int total, int *quitar)
{
    long pos;
    rewind(f); /* volver al inicio para sobreescribir */
    for (int j = 0; j < total; j++)
    {
        if (quitar[j] == 0) /* solo copiar las lineas no marcadas */
            fputs(lineas[j], f);
    }
    fflush(f);
    pos = ftell(f); /* posicion final = nuevo tamano del archivo */
    if (pos >= 0)
    {
#ifdef _WIN32
        _chsize_s(_fileno(f), (size_t)pos); /* truncar en Windows */
#else
        ftruncate(fileno(f), pos);          /* truncar en POSIX   */
#endif
    }
}

int main(void)
{
    /* Tabla de programas: [nombre, archivo_entrada, archivo_salida, NULL] */
    char *programas[][4] = {
        {"SISTEMA_QU1R30N", "conexion_arc/archivo_entrada.txt", "conexion_arc/archivo_salida.txt", NULL},
        {"NEXOPORTALARCANO", "conexion_arc/archivo_entrada.txt", "conexion_arc/archivo_salida.txt", NULL},
        {"INTERMEDIARIO",   "conexion_arc/archivo_entrada.txt", "conexion_arc/archivo_salida.txt", NULL},
        {NULL, NULL, NULL, NULL}
    };

    for (int i = 0; programas[i][0] != NULL; i++)
    {
        /* Abrir el archivo de salida en modo lectura/escritura */
        FILE *fs = fopen(programas[i][2], "r+");
        if (fs == NULL) continue; /* saltar si no se puede abrir */

        int total = 0;
        char **lineas = leer(fs, &total); /* leer todas las lineas */
        if (lineas == NULL) { fclose(fs); continue; } /* archivo vacio o error */

        int *quitar = (int *)calloc((size_t)total, sizeof(int));
        if (quitar == NULL) { fclose(fs); return 1; }

        /* Recorrer desde j=1 (j=0 es la cabecera del archivo) */
        for (int j = 1; j < total; j++)
        {
            char **partes = NULL;
            /* Separar la linea por el caracter de transferencia para obtener el destinatario */
            int n = split(lineas[j], GG_caracter_para_transferencia_entre_archivos[0], &partes);

            if (n > 0 && partes != NULL && partes[0] != NULL)
            {
                /* Buscar a que programa pertenece esta linea */
                for (int k = 0; programas[k][0] != NULL; k++)
                {
                    if (strcmp(partes[0], programas[k][0]) == 0)
                    {
                        /* Abrir el archivo de entrada del programa destino y agregar la linea */
                        FILE *fe = fopen(programas[k][1], "a");
                        if (fe != NULL)
                        {
                            agregar(fe, lineas[j]); /* escribir al destino */
                            fclose(fe);
                            quitar[j] = 1; /* marcar para eliminar del origen */
                        }
                        break;
                    }
                }
            }

            if (partes != NULL) free_split(partes); /* liberar el split */
        }

        /* Reescribir el archivo de salida sin las lineas transferidas */
        eliminar(fs, lineas, total, quitar);
        fclose(fs);

        /* Liberar memoria de lineas y arreglo de marcas */
        for (int j = 0; j < total; j++) free(lineas[j]);
        free(lineas);
        free(quitar);
    }

    return 0;
}
