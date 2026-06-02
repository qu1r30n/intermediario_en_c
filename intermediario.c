#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

/* [0]=ID  [1]=archivo entrada  [2]=archivo salida */
char *programas[][3] =
{
    {"SISTEMA_QU1R30N",  "conexion_arc/archivo_entrada.txt", "conexion_arc/archivo_salida.txt"},
    {"NEXOPORTALARCANO", "conexion_arc/archivo_entrada.txt", "conexion_arc/archivo_salida.txt"},
    {"INTERMEDIARIO",    "conexion_arc/archivo_entrada.txt", "conexion_arc/archivo_salida.txt"},
    {NULL, NULL, NULL}
};

#define SEPARADOR_DESTINO "┴"

/*
 * Lee todas las lineas del archivo.
 */
static char **leer(FILE *f, int *total_out)
{
    char **lineas = NULL;
    int total = 0;
    int c;
    char *s = NULL;
    size_t n = 0;

    while ((c = fgetc(f)) != EOF)
    {
        char *t = (char *)realloc(s, n + 2);

        if (t == NULL)
        {
            free(s);
            break;
        }

        s = t;
        s[n++] = (char)c;

        if (c == '\n')
        {
            s[n] = '\0';

            char **tmp =
                (char **)realloc(
                    lineas,
                    (size_t)(total + 1) * sizeof(char *));

            if (tmp == NULL)
            {
                free(s);
                break;
            }

            lineas = tmp;
            lineas[total++] = s;

            s = NULL;
            n = 0;
        }
    }

    if (s != NULL)
    {
        s[n] = '\0';

        char **tmp =
            (char **)realloc(
                lineas,
                (size_t)(total + 1) * sizeof(char *));

        if (tmp != NULL)
        {
            lineas = tmp;
            lineas[total++] = s;
        }
        else
        {
            free(s);
        }
    }

    *total_out = total;
    return lineas;
}

/*
 * Agrega una linea al archivo.
 */
static void agregar(FILE *f, const char *linea)
{
    fputs(linea, f);
}

/*
 * Elimina las lineas marcadas.
 */
static void eliminar(FILE *f, char **lineas, int total, int *quitar)
{
    long pos;

    rewind(f);

    for (int j = 0; j < total; j++)
    {
        if (quitar[j] == 0)
        {
            fputs(lineas[j], f);
        }
    }

    fflush(f);

    pos = ftell(f);

    if (pos >= 0)
    {
#ifdef _WIN32
        _chsize_s(_fileno(f), (size_t)pos);
#else
        ftruncate(fileno(f), pos);
#endif
    }
}

/*
 * Obtiene el ID destino.
 *
 * Ejemplo:
 * SISTEMA_QU1R30N┴NEXOPORTALARCANO■COMANDO■DATOS
 *
 * Resultado:
 * SISTEMA_QU1R30N
 */
static int obtener_destinatario(
    const char *linea,
    char *destino,
    size_t tam_destino)
{
    char *p;

    if (linea == NULL || destino == NULL)
    {
        return 0;
    }

    p = strstr(linea, SEPARADOR_DESTINO);

    if (p == NULL)
    {
        return 0;
    }

    size_t len = (size_t)(p - linea);

    if (len >= tam_destino)
    {
        len = tam_destino - 1;
    }

    memcpy(destino, linea, len);
    destino[len] = '\0';

    return 1;
}

int main(void)
{
    for (int i = 0; programas[i][0] != NULL; i++)
    {
        FILE *fs = fopen(programas[i][2], "r+");

        if (fs == NULL)
        {
            continue;
        }

        int total = 0;

        char **lineas = leer(fs, &total);

        if (lineas == NULL)
        {
            fclose(fs);
            continue;
        }

        int *quitar =
            (int *)calloc((size_t)total, sizeof(int));

        if (quitar == NULL)
        {
            fclose(fs);

            for (int j = 0; j < total; j++)
            {
                free(lineas[j]);
            }

            free(lineas);

            return 1;
        }

        for (int j = 1; j < total; j++)
        {
            char destinatario[256];

            if (obtener_destinatario(
                    lineas[j],
                    destinatario,
                    sizeof(destinatario)))
            {
                for (int k = 0; programas[k][0] != NULL; k++)
                {
                    if (strcmp(destinatario, programas[k][0]) == 0)
                    {
                        FILE *fe = fopen(programas[k][1], "a");

                        if (fe != NULL)
                        {
                            agregar(fe, lineas[j]);

                            fclose(fe);

                            quitar[j] = 1;
                        }

                        break;
                    }
                }
            }
        }

        eliminar(fs, lineas, total, quitar);

        fclose(fs);

        for (int j = 0; j < total; j++)
        {
            free(lineas[j]);
        }

        free(lineas);
        free(quitar);
    }

    return 0;
}