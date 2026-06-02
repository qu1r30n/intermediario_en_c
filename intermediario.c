#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

/* =========================================================
   CONFIGURACION DE PROGRAMAS
   [0] = ID
   [1] = ARCHIVO_ENTRADA
   [2] = ARCHIVO_SALIDA
   ========================================================= */

char *programas[][3] =
{
    {"SISTEMA_QU1R30N",       "conexion_arc/archivo_entrada.txt",     "conexion_arc/archivo_salida.txt"},
    {"carcasa_punto_de_venta","C:/conexion_arc/archivo_entrada.txt",  "C:/conexion_arc/archivo_salida.txt"},
    {NULL, NULL, NULL}
};

/* =========================================================
   SEPARADORES DEL PROTOCOLO

   FORMATO:

   ID_DESTINO┴ID_ORIGEN■COMANDO■DATOS
   ========================================================= */

#define SEPARADOR_DESTINO "┴"
#define SEPARADOR_CAMPO   "■"

/* =========================================================
   REGISTRAR ERROR
   ========================================================= */

static void registrar_error(
    const char *archivo,
    const char *id_esperado,
    const char *id_recibido,
    const char *linea)
{
    FILE *f = fopen("errores.txt", "a");

    if (f == NULL)
    {
        return;
    }

    time_t ahora = time(NULL);
    struct tm *fecha = localtime(&ahora);

    char fecha_texto[64];

    strftime(
        fecha_texto,
        sizeof(fecha_texto),
        "%Y-%m-%d %H:%M:%S",
        fecha);

    fprintf(
        f,
        "\n====================================================\n"
        "ERROR DE IDENTIDAD DETECTADO\n"
        "FECHA: %s\n"
        "ARCHIVO: %s\n"
        "ID ESPERADO: %s\n"
        "ID RECIBIDO: %s\n"
        "MENSAJE: %s\n"
        "====================================================\n",
        fecha_texto,
        archivo,
        id_esperado,
        id_recibido,
        linea
    );

    fclose(f);
}
/* =========================================================
   LEER TODAS LAS LINEAS
   ========================================================= */

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

/* =========================================================
   AGREGAR LINEA
   ========================================================= */

static void agregar(FILE *f, const char *linea)
{
    fputs(linea, f);
}

/* =========================================================
   ELIMINAR LINEAS MARCADAS
   ========================================================= */

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

/* =========================================================
   OBTENER DESTINO

   DESTINO┴ORIGEN■COMANDO■DATOS
   ^^^^^^^
   ========================================================= */

static int obtener_destino(
    const char *linea,
    char *destino,
    size_t tam)
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

    if (len >= tam)
    {
        len = tam - 1;
    }

    memcpy(destino, linea, len);

    destino[len] = '\0';

    return 1;
}

/* =========================================================
   OBTENER ORIGEN

   DESTINO┴ORIGEN■COMANDO■DATOS
            ^^^^^^
   ========================================================= */

static int obtener_origen(
    const char *linea,
    char *origen,
    size_t tam)
{
    char *p1;
    char *p2;
    size_t len;

    if (linea == NULL || origen == NULL)
    {
        return 0;
    }

    p1 = strstr(linea, SEPARADOR_DESTINO);

    if (p1 == NULL)
    {
        return 0;
    }

    p1 += strlen(SEPARADOR_DESTINO);

    p2 = strstr(p1, SEPARADOR_CAMPO);

    if (p2 == NULL)
    {
        return 0;
    }

    len = (size_t)(p2 - p1);

    if (len >= tam)
    {
        len = tam - 1;
    }

    memcpy(origen, p1, len);

    origen[len] = '\0';

    return 1;
}

/* =========================================================
   MAIN
   ========================================================= */

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
            char destino[256];
            char origen[256];

            if (!obtener_destino(
                    lineas[j],
                    destino,
                    sizeof(destino)))
            {
                continue;
            }

            if (!obtener_origen(
                    lineas[j],
                    origen,
                    sizeof(origen)))
            {
                continue;
            }

            /*
             * VALIDACION DE IDENTIDAD
             *
             * El ID_ORIGEN declarado en el mensaje
             * debe coincidir con el dueño real
             * del archivo de salida.
             */

            if (strcmp(origen, programas[i][0]) != 0)
            {
                registrar_error(
                    programas[i][2],
                    programas[i][0],
                    origen,
                    lineas[j]);

                quitar[j] = 1;

                continue;
            }

            for (int k = 0; programas[k][0] != NULL; k++)
            {
                if (strcmp(destino, programas[k][0]) == 0)
                {
                    FILE *fe =
                        fopen(programas[k][1], "a");

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

        eliminar(
            fs,
            lineas,
            total,
            quitar);

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