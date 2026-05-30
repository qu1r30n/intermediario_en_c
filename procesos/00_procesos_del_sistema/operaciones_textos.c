/* LIBRERIAS USADAS EN ESTE ARCHIVO:
 * - string.h: Manejo de cadenas y memoria (strlen, strcmp, memcpy)
 * - stdio.h: Entrada y salida estandar (printf, fopen, etc.)
 * - stdarg.h: Argumentos variables de funciones tipo printf
 * - time.h: Funciones de fecha y hora
 * - ../../cabeceras/cabeceras_procesos/00_cabeceras_del_sistema/operaciones_textos.h: Dependencia interna del proyecto
 * - stdlib.h: Memoria dinamica, conversiones y utilidades generales
 * - xc.h: Cabecera del compilador para microcontroladores PIC
 */
#include "../../cabeceras/cabeceras_procesos/00_cabeceras_del_sistema/operaciones_textos.h"
#include "../../cabeceras/cabeceras_procesos/00_cabeceras_del_sistema/operaciones_compu.h"
#include "../../cabeceras/cabeceras_procesos/00_cabeceras_del_sistema/tex_bas.h"
#include "../../cabeceras/cabeceras_procesos/00_cabeceras_del_sistema/var_fun_GG.h"
#include <stdarg.h> // va_list, va_start, va_end
#include <stdio.h>  // snprintf
#include <string.h> // strlen, memcpy, strstr
#include <time.h>   // time, localtime, strftime

#ifdef _WIN32
#include <stdlib.h> // malloc, realloc, free

#elif defined(__linux__)
#include <stdlib.h> // malloc, realloc, free

#elif defined(__XC)
#define _XTAL_FREQ 4000000
#include <xc.h>
/* PIC16F: Sin malloc disponible. Las funciones son stubs limitadas. */

#else
#include <stdlib.h>
#endif

/*
 * Uso: construye una respuesta estandar con codigo y separadores especiales.
 * Formato: codigo + separador + mensaje + separador + datos_extra.
 */
char *construir_retorno_estandar(int codigo, const char *separador, const char *mensaje, const char *datos_extra)
{
    char *retorno = NULL;                                                              // string final que se devolvera al llamador; ejemplo: "0�todo salio bien en el conmutador�detalle"
    const char *mensaje_final = (mensaje != NULL) ? mensaje : "sin_mensaje";           // mensaje seguro por si llega NULL; ejemplo: "sin_mensaje"
    const char *datos_extra_final = (datos_extra != NULL) ? datos_extra : "sin_datos"; // datos extra seguros por si llega NULL; ejemplo: "sin_datos"

    if (separador == NULL)
    {
        separador = GG_caracter_para_confirmacion_o_error[0]; // usa el separador principal por defecto si no se especifica; ejemplo: "�"
    }

    if (concatenar_formato_separado_por_variable(&retorno, NULL, "%d%s%s%s%s", codigo, separador, mensaje_final, separador, datos_extra_final) < 0)
    {
        free(retorno);
        return NULL;
    }

    return retorno;
}

/*
 * Uso: Ejecuta variable_string de forma segura.
 * Entrada ejemplo: variable_string(format, arg2)
 */
char *variable_string(const char *format, ...) // define una funcion que crea y retorna una cadena dinamica formateada // ejemplo: "ID=15"
{
    va_list args;        // declara la lista principal de argumentos variables para el formato recibido // ejemplo: "ID=%d"
    va_list args_copia;  // declara una copia de la lista para medir longitud sin consumir la original // ejemplo: copia de args
    char *buffer = NULL; // prepara el puntero que almacenar� el texto final reservado din�micamente // ejemplo: "hola mundo"
    int longitud = 0;    // guardar� la cantidad exacta de caracteres que requiere el texto formateado // ejemplo: 12

    if (format == NULL)
    {
        return NULL; // retorna NULL para indicar que no se pudo construir ninguna cadena // ejemplo: formato ausente
    }

    va_start(args, format);                            // inicia la lectura de los argumentos variables reales enviados a la funci�n // ejemplo: "%s-%d"
    va_copy(args_copia, args);                         // duplica la lista para usarla en el c�lculo de longitud sin perder la original // ejemplo: copia de args
    longitud = vsnprintf(NULL, 0, format, args_copia); // calcula cu�ntos caracteres necesita la salida sin escribirla todav�a // ejemplo: 8
    va_end(args_copia);                                // finaliza la copia temporal de argumentos porque ya no se seguir� usando // ejemplo: fin de args_copia

    if (longitud < 0)
    {
        va_end(args); // cierra la lista principal antes de salir por error de formato // ejemplo: formato inv�lido
        return NULL;  // retorna NULL para indicar fallo al medir el texto de salida // ejemplo: error de vsnprintf
    }

    buffer = (char *)malloc((size_t)longitud + 1); // reserva memoria suficiente para la cadena final incluyendo el terminador nulo // ejemplo: 9 bytes
    if (buffer == NULL)
    {
        va_end(args); // cierra la lista principal porque no se podr� continuar sin memoria // ejemplo: sin heap disponible
        return NULL;  // retorna NULL para avisar que la reserva din�mica no se logr� // ejemplo: malloc fall�
    }

    vsnprintf(buffer, (size_t)longitud + 1, format, args); // construye la cadena final dentro de buffer usando los argumentos originales // ejemplo: "ID=15"
    va_end(args);                                          // cierra la lista principal de argumentos una vez completado el formateo // ejemplo: fin de args
    return buffer;                                         // entrega al llamador la cadena reci�n creada en memoria din�mica // ejemplo: puntero a "ID=15"
}

/*
===============================================================================
 CORE_STRING_SPLIT
===============================================================================

 FUNCION: core_split

 Divide un texto usando un separador (string completo).
 ------------------------------------------------------------------------------

 EJEMPLO DE ENTRADA:

  txt = "7501020304050|5|SucursalNorte"
  sep = "|"

 RESULTADO EN MEMORIA:

  resultado[0] = "7501020304050"
  resultado[1] = "5"
  resultado[2] = "SucursalNorte"
  resultado[3] = NULL

 RETORNO:
  3

===============================================================================
*/

int split(const char *txt, const char *sep, char ***salida)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */

    /* -----------------------------------------------------------------------
    VALIDACI�N DE PAR�METROS
    -----------------------------------------------------------------------

    txt  -> texto original (ej: "A|B|C")
    sep  -> separador (ej: "|")
    salida  -> direcci�n donde se guardar� el arreglo resultante

    Si alguno es NULL ? error inmediato
    ----------------------------------------------------------------------- */
    if (txt == NULL || sep == NULL || salida == NULL) // al menos uno de los punteros obligatorios es nulo // ejemplo: txt=NULL al llamar split mal
    {
        RETORNAR_PROCESO_ESTANDAR(-1);
    }

    /* Obtener longitud del separador */
    size_t len_sep = strlen(sep); // longitud del separador para avanzar el puntero correctamente // ejemplo: 1 para "|"

    /* Si el separador est� vac�o, no tiene sentido dividir */
    if (len_sep == 0) // separador vacio imposibilita la division
    {
        RETORNAR_PROCESO_ESTANDAR(-1);
    }

    /* Capacidad inicial del arreglo din�mico
    Significa que inicialmente podemos guardar 4 fragmentos */
    int capacidad = 4; // slots iniciales del arreglo dinamico de punteros // ejemplo: puede guardar 4 fragmentos antes de realloc

    /* Cantidad real de fragmentos encontrados */
    int cantidad = 0; // contador de fragmentos extraidos hasta el momento // ejemplo: 0 al inicio, 3 al terminar "A|B|C"

    /* Reservar memoria para arreglo de punteros (char*) */
    char **resultado = malloc(sizeof(char *) * capacidad); // reserva arreglo inicial de 4 punteros char* // ejemplo: 4 * 8 = 32 bytes en x64

    /* Si falla malloc ? error */
    if (resultado == NULL) // verificacion de fallo en la asignacion de memoria
    {
        RETORNAR_PROCESO_ESTANDAR(-1);
    }

    /* inicio apunta al comienzo del fragmento actual */
    const char *inicio = txt; // cursor que marca el inicio del fragmento en proceso // ejemplo: apunta a "7501020304050|5"

    /* Buscar primera aparici�n del separador */
    const char *pos = strstr(inicio, sep); // primera posicion del separador dentro del texto // ejemplo: apunta a "|5|SucursalNorte"

    /* -----------------------------------------------------------------------
    BUCLE PRINCIPAL
    Mientras encontremos el separador en el texto
    ----------------------------------------------------------------------- */
    while (pos != NULL)
    {
        /* Calcular longitud del fragmento actual */
        size_t len_fragmento = (size_t)(pos - inicio); // diferencia de punteros da el largo del fragmento sin el sep // ejemplo: 13 para "7501020304050"

        /* Reservar memoria para el fragmento (+1 para '\0') */
        char *fragmento = malloc(len_fragmento + 1); // +1 para el terminador nulo al final del fragmento // ejemplo: 14 bytes

        if (fragmento == NULL) // fallo de malloc en fragmento intermedio
        {
            /* Liberar todo lo previamente reservado */
            for (int i = 0; i < cantidad; i++)
                free(resultado[i]); // libera cada fragmento ya guardado antes del fallo // ejemplo: libera resultado[0], resultado[1]

            free(resultado); // libera el arreglo de punteros principal
            RETORNAR_PROCESO_ESTANDAR(-1);
        }

        /* Copiar contenido del fragmento */
        memcpy(fragmento, inicio, len_fragmento); // copia exactamente len_fragmento bytes desde inicio // ejemplo: copia "7501020304050"

        /* Agregar terminador de cadena */
        fragmento[len_fragmento] = '\0'; // cierra la cadena copiada con terminador nulo // ejemplo: fragmento[13] = '\0'

        /* Si el arreglo est� lleno, duplicar capacidad */
        if (cantidad >= capacidad) // el arreglo no tiene mas espacio para otro puntero
        {
            capacidad *= 2; // duplica la capacidad para minimizar reallocaciones // ejemplo: 4 -> 8

            char **temp = realloc(resultado, sizeof(char *) * capacidad); // redimensiona el arreglo al doble de punteros // ejemplo: 8 * 8 = 64 bytes

            if (temp == NULL) // fallo de realloc durante expansion
            {
                free(fragmento); // libera el fragmento recien creado que no pudo guardarse

                for (int i = 0; i < cantidad; i++)
                    free(resultado[i]); // libera fragmentos ya almacenados antes del fallo

                free(resultado); // libera el arreglo de punteros original
                RETORNAR_PROCESO_ESTANDAR(-1);
            }

            resultado = temp; // apunta al bloque redimensionado valido // ejemplo: nuevo puntero con 8 slots
        }

        /* Guardar fragmento */
        resultado[cantidad] = fragmento; // almacena el puntero del fragmento en el slot actual // ejemplo: resultado[0] = "7501020304050"
        cantidad++;                      // incrementa el contador de fragmentos guardados // ejemplo: cantidad = 1

        /* Avanzar inicio despu�s del separador */
        inicio = pos + len_sep; // mueve el cursor al caracter despues del separador encontrado // ejemplo: apunta a "5|SucursalNorte"

        /* Buscar siguiente separador */
        pos = strstr(inicio, sep); // busca la siguiente aparicion del separador desde la nueva posicion // ejemplo: apunta a "|SucursalNorte"
    }

    /* -----------------------------------------------------------------------
    �LTIMO FRAGMENTO
    (lo que queda despu�s del �ltimo separador)
    ----------------------------------------------------------------------- */

    size_t len_final = strlen(inicio); // longitud del ultimo fragmento que queda despues del ultimo separador // ejemplo: 13 para "SucursalNorte"

    char *fragmento_final = malloc(len_final + 1); // reserva memoria para el ultimo fragmento mas terminador // ejemplo: 14 bytes

    if (fragmento_final == NULL) // fallo de malloc en el ultimo fragmento
    {
        for (int i = 0; i < cantidad; i++)
            free(resultado[i]); // libera todos los fragmentos previos ya almacenados

        free(resultado); // libera el arreglo principal de punteros
        RETORNAR_PROCESO_ESTANDAR(-1);
    }

    /* Copiar �ltimo fragmento incluyendo '\0' */
    memcpy(fragmento_final, inicio, len_final + 1); // copia el ultimo fragmento incluyendo su terminador nulo // ejemplo: "SucursalNorte\0"

    /* Expandir si es necesario */
    if (cantidad >= capacidad) // verifica si el arreglo necesita espacio para el ultimo fragmento
    {
        char **temp = realloc(resultado, sizeof(char *) * (capacidad + 1)); // expande exactamente un slot extra para el ultimo fragmento

        if (temp == NULL) // fallo de realloc al intentar expandir para el ultimo slot
        {
            free(fragmento_final); // libera el ultimo fragmento que no pudo guardarse

            for (int i = 0; i < cantidad; i++)
                free(resultado[i]); // libera todos los fragmentos previos

            free(resultado); // libera el arreglo principal
            RETORNAR_PROCESO_ESTANDAR(-1);
        }

        resultado = temp; // apunta al bloque expandido con espacio para el ultimo fragmento
    }

    /* Guardar �ltimo fragmento */
    resultado[cantidad] = fragmento_final; // almacena el ultimo fragmento en el arreglo // ejemplo: resultado[2] = "SucursalNorte"
    cantidad++;                            // incrementa el total para incluir el ultimo fragmento // ejemplo: cantidad = 3

    /* -----------------------------------------------------------------------
    AGREGAR NULL FINAL (estilo argv)
    Permite recorrer as�:
     while(resultado[i] != NULL)
    ----------------------------------------------------------------------- */

    char **temp = realloc(resultado, sizeof(char *) * (cantidad + 1)); // expande el arreglo un slot extra para el centinela NULL // ejemplo: 3 + 1 = 4 slots

    if (temp == NULL) // fallo al reservar espacio para el terminador NULL del arreglo
    {
        for (int i = 0; i < cantidad; i++)
            free(resultado[i]); // libera todos los fragmentos almacenados

        free(resultado); // libera el arreglo de punteros
        RETORNAR_PROCESO_ESTANDAR(-1);
    }

    resultado = temp;           // apunta al bloque final con espacio para el NULL centinela
    resultado[cantidad] = NULL; // marca el fin del arreglo estilo argv // ejemplo: resultado[3] = NULL

    /* Guardar arreglo final en par�metro de salida */
    *salida = resultado; // escribe el arreglo resultante en el puntero de salida // ejemplo: *salida apunta a {"7501020304050","5","SucursalNorte",NULL}

    /* Retornar cantidad de fragmentos encontrados */
    RETORNAR_PROCESO_ESTANDAR(cantidad); // cantidad total de fragmentos (sin contar el NULL final) // ejemplo: 3
}

/*
===============================================================================
 FUNCION: free_split
-------------------------------------------------------------------------------

 Libera completamente la memoria creada por core_split.

 Parametro:
  arreglo -> arreglo dinamico terminado en NULL

 Ejemplo interno:

  arreglo[0] = "7501020304050"
  arreglo[1] = "5"
  arreglo[2] = "SucursalNorte"
  arreglo[3] = NULL

 Esta funcion:
  ? Libera cada fragmento
  ? Libera el arreglo principal
  ? No deja memoria colgada
===============================================================================
*/

/*
 * Uso: Ejecuta free_split de forma segura.
 * Entrada ejemplo: free_split(arreglo)
 */
void free_split(char **arreglo)
{
    /* Si es NULL, no hacer nada */
    if (arreglo == NULL) // si el puntero es NULL no hay nada que liberar
    {
        return;
    }

    int i = 0; // indice para recorrer el arreglo de fragmentos

    /* Recorrer hasta encontrar NULL */
    while (arreglo[i] != NULL) // itera hasta el centinela NULL al final del arreglo
    {
        free(arreglo[i]); // libera cada cadena de texto almacenada // ejemplo: libera "7501020304050"
        i++;              // avanza al siguiente elemento del arreglo
    }

    /* Liberar arreglo principal */
    free(arreglo); // libera el arreglo de punteros luego de haber liberado cada fragmento
}

/*
===============================================================================
 EJEMPLO DE USO EN main
===============================================================================

#include <stdio.h>

int main()
{
 const char* linea = "7501020304050|5|SucursalNorte";

 char** partes;

 int n = split(linea, "|", &partes);

 if (n == -1)
 {
  printf("Error al dividir\n");
  RETORNAR_PROCESO_ESTANDAR(1);
 }

 printf("Fragmentos encontrados: %d\n", n);

 int i = 0;
 while (partes[i] != NULL)
 {
  printf("Parte %d: %s\n", i, partes[i]);
  i++;
 }

 free_split(partes);

 RETORNAR_PROCESO_ESTANDAR(0);
}

===============================================================================
*/

/*
 * Uso: Ejecuta texto_a_int_seguro de forma segura.
 * Entrada ejemplo: texto_a_int_seguro(texto, var_a_retornar)
 */
int texto_a_int_seguro(const char *texto, int *var_a_retornar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    int signo = 1; // signo del numero, positivo por defecto // ejemplo: 1 para "42", -1 para "-42"
    long temp = 0; // usamos long para detectar overflow

    if (texto == 0 || var_a_retornar == 0) // alguno de los punteros requeridos es nulo
        RETORNAR_PROCESO_ESTANDAR(-1);

    if (*texto == '-') // detecta signo negativo al inicio // ejemplo: "-15"
    {
        signo = -1; // marca el resultado como negativo
        texto++;    // avanza el cursor para ignorar el signo y procesar digitos
    }
    else if (*texto == '+') // detecta signo positivo explicito al inicio // ejemplo: "+7"
    {
        texto++; // avanza el cursor para ignorar el signo positivo
    }

    if (*texto < '0' || *texto > '9')
    {
        RETORNAR_PROCESO_ESTANDAR(-1); // no empieza con n�mero
    }

    while (*texto >= '0' && *texto <= '9') // procesa cada digito decimal del texto
    {

        temp = temp * 10 + (*texto - '0'); // construye el valor acumulando cada digito // ejemplo: 1->10->104->1042

        /* Detectar overflow para int 16-bit (PIC16F) */
        if (temp > 32767) // supera el maximo de int de 16 bits, invalido para PIC
        {
            RETORNAR_PROCESO_ESTANDAR(-1);
        }

        texto++; // avanza al siguiente caracter del texto
    }

    if (*texto != '\0')
    {
        RETORNAR_PROCESO_ESTANDAR(-1); // caracteres inv�lidos al final
    }

    *var_a_retornar = (int)(temp * signo); // escribe el resultado con signo en la variable de salida // ejemplo: -42
    RETORNAR_PROCESO_ESTANDAR(0);          // conversion exitosa sin errores
}

/*
 * Uso: Ejecuta texto_a_float_seguro de forma segura.
 * Entrada ejemplo: texto_a_float_seguro(texto, var_a_retornar)
 */
int texto_a_float_seguro(const char *texto, float *var_a_retornar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    float valor = 0.0f;    // acumulador del valor numerico absoluto // ejemplo: 12.5 para "12.5"
    float decimal = 0.1f;  // multiplicador para la parte decimal, se reduce por cada digito // ejemplo: 0.1, 0.01, 0.001
    int signo = 1;         // signo del numero, positivo por defecto
    int tiene_decimal = 0; // bandera que indica si ya se proceso el punto decimal // ejemplo: 0 antes del '.', 1 despues

    if (texto == 0 || var_a_retornar == 0) // alguno de los punteros requeridos es nulo
        RETORNAR_PROCESO_ESTANDAR(-1);

    if (*texto == '-') // detecta signo negativo al inicio // ejemplo: "-9.5"
    {
        signo = -1; // resultado sera negativo
        texto++;    // salta el caracter de signo
    }
    else if (*texto == '+') // detecta signo positivo explicito al inicio // ejemplo: "+3.14"
    {
        texto++; // salta el caracter de signo positivo
    }

    if ((*texto < '0' || *texto > '9') && *texto != '.') // primer caracter invalido: ni digito ni punto decimal
    {
        RETORNAR_PROCESO_ESTANDAR(-1);
    }

    while (*texto != '\0') // procesa caracter a caracter hasta el fin de la cadena
    {

        if (*texto >= '0' && *texto <= '9') // es un digito numerico valido
        {

            if (!tiene_decimal) // parte entera del numero
            {
                valor = valor * 10.0f + (*texto - '0'); // acumula digito en la parte entera // ejemplo: 1->12->12.0
            }
            else // parte fraccionaria del numero
            {
                valor += (*texto - '0') * decimal; // agrega el digito a la parte decimal // ejemplo: 0.5, 0.05
                decimal *= 0.1f;                   // reduce el peso del siguiente digito decimal // ejemplo: 0.1->0.01
            }
        }
        else if (*texto == '.') // caracter punto: inicio de la parte decimal
        {

            if (tiene_decimal) // segundo punto decimal encontrado: numero invalido
            {
                RETORNAR_PROCESO_ESTANDAR(-1); // dos puntos decimales
            }

            tiene_decimal = 1; // activa la bandera de procesamiento decimal
        }
        else
        {
            RETORNAR_PROCESO_ESTANDAR(-1); // car�cter inv�lido
        }

        texto++; // avanza al siguiente caracter del texto
    }

    *var_a_retornar = valor * signo; // escribe el resultado con signo en la variable de salida // ejemplo: -9.5
    RETORNAR_PROCESO_ESTANDAR(0);    // conversion exitosa sin errores
}

/*
Ejemplo:
 char salida[128] = "";
 concatenar_formato(salida, NULL, "%s", "producto");
 concatenar_formato(salida, "|", "%d", 5);
 concatenar_formato(salida, "|", "%.3f", 12.5f);

Resultado en salida:
 "producto|5|12.50"
*/
/*
 * Uso: Ejecuta concatenar_formato_separado_por_variable de forma segura.
 * Entrada ejemplo: concatenar_formato_separado_por_variable(destino, separador, formato, arg4)
 */
int concatenar_formato_separado_por_variable(char **destino, const char *separador, const char *formato, ...)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (destino == NULL || formato == NULL) // parametros obligatorios nulos
    {
        RETORNAR_PROCESO_ESTANDAR(-1);
    }

    if (*destino == NULL) // el destino aun no tiene buffer asignado, se inicializa
    {
        *destino = malloc(1); // reserva un byte para la cadena vacia inicial
        if (*destino == NULL) // fallo al reservar el buffer inicial
        {
            RETORNAR_PROCESO_ESTANDAR(-1);
        }
        (*destino)[0] = '\0'; // inicializa el buffer como cadena vacia
    }

    size_t largo_actual = strlen(*destino); // longitud actual del texto ya concatenado // ejemplo: 7 si destino = "nombre"

    const char *formato_a_usar = formato; // puntero al formato que se usara (puede cambiar si hay separador)
    char *formato_expandido = NULL;       // formato modificado con separadores intercalados, NULL si no aplica

    if (separador != NULL && separador[0] != '\0') // hay un separador valido que debe insertarse entre especificadores
    {
        size_t len_formato = strlen(formato); // longitud del string de formato // ejemplo: 5 para "%s|%d"
        size_t len_sep = strlen(separador);   // longitud del separador a intercalar // ejemplo: 1 para "|"
        size_t cantidad_specs = 0;            // conteo de especificadores de formato validos encontrados

        for (size_t i = 0; i < len_formato; i++) // recorre el string de formato para contar especificadores
        {
            if (formato[i] == '%') // inicio de un posible especificador de formato
            {
                i++; // avanza al caracter siguiente al '%'

                if (i >= len_formato) // '%' al final del formato sin especificador, se descarta
                {
                    break;
                }

                if (formato[i] == '%') // '%%' es literal '%', no un especificador real
                {
                    continue;
                }

                while (i < len_formato && strchr("diuoxXfFeEgGaAcspn", formato[i]) == NULL) // salta flags y precision hasta el tipo
                {
                    i++;
                }

                if (i < len_formato) // encontro un especificador valido
                {
                    cantidad_specs++; // cuenta el especificador encontrado // ejemplo: 2 para "%s|%d"
                }
            }
        }

        if (cantidad_specs > 0) // solo expande si hay especificadores que necesitan separador
        {
            size_t len_expandido = len_formato + (cantidad_specs * len_sep) + 1; // espacio para formato + separadores + nulo
            formato_expandido = malloc(len_expandido);                           // reserva buffer para el formato expandido

            if (formato_expandido == NULL) // fallo al reservar el formato expandido
            {
                RETORNAR_PROCESO_ESTANDAR(-1);
            }

            size_t j = 0; // indice de escritura en el formato expandido

            for (size_t i = 0; i < len_formato; i++) // recorre el formato original caracter a caracter
            {
                formato_expandido[j++] = formato[i]; // copia el caracter actual al formato expandido

                if (formato[i] == '%') // inicio de un especificador en el formato
                {
                    i++; // avanza al caracter de tipo

                    if (i >= len_formato) // '%' al final sin tipo, termina
                    {
                        break;
                    }

                    formato_expandido[j++] = formato[i]; // copia el caracter de tipo al formato expandido

                    if (formato[i] == '%') // '%%' literal, no agrega separador
                    {
                        continue;
                    }

                    while (i < len_formato && strchr("diuoxXfFeEgGaAcspn", formato[i]) == NULL) // salta modificadores hasta el tipo final
                    {
                        i++;
                        if (i < len_formato) // aun hay caracteres disponibles
                        {
                            formato_expandido[j++] = formato[i]; // copia el modificador al formato expandido
                        }
                    }

                    if (i < len_formato) // especificador completo encontrado, agrega separador despues
                    {
                        memcpy(formato_expandido + j, separador, len_sep); // inserta el separador despues del especificador // ejemplo: agrega "|"
                        j += len_sep;                                      // avanza el indice de escritura el largo del separador
                    }
                }
            }

            formato_expandido[j] = '\0';        // cierra el string del formato expandido
            formato_a_usar = formato_expandido; // usa el formato expandido en lugar del original
        }
    }

    va_list args;            // lista de argumentos variables para vsnprintf
    va_start(args, formato); // inicializa la lista de argumentos a partir del parametro 'formato'

    va_list args_len;                                              // copia de la lista de argumentos para calcular el tamano sin escribir
    va_copy(args_len, args);                                       // duplica la lista para no consumir los argumentos originales
    int necesarios = vsnprintf(NULL, 0, formato_a_usar, args_len); // calcula la cantidad de bytes necesarios sin escribir nada // ejemplo: 14
    va_end(args_len);                                              // libera la copia de argumentos usada para el calculo

    if (necesarios < 0) // vsnprintf fallo en el calculo de longitud
    {
        va_end(args);
        free(formato_expandido);
        RETORNAR_PROCESO_ESTANDAR(-1);
    }

    char *tmp = realloc(*destino, largo_actual + (size_t)necesarios + 1); // redimensiona el buffer destino para el texto nuevo // ejemplo: 7 + 14 + 1 = 22 bytes
    if (tmp == NULL)                                                      // fallo de realloc al ampliar el buffer
    {
        va_end(args);
        free(formato_expandido);
        RETORNAR_PROCESO_ESTANDAR(-1);
    }

    *destino = tmp;                                                                                  // apunta al buffer redimensionado con espacio para el nuevo texto
    int escritos = vsnprintf(*destino + largo_actual, (size_t)necesarios + 1, formato_a_usar, args); // escribe el nuevo texto al final del buffer existente // ejemplo: agrega "Tornillo|5|"
    va_end(args);                                                                                    // libera la lista de argumentos original

    free(formato_expandido); // libera el formato expandido si fue creado

    if (escritos < 0) // vsnprintf fallo durante la escritura real
    {
        RETORNAR_PROCESO_ESTANDAR(-1);
    }

    RETORNAR_PROCESO_ESTANDAR(0); // concatenacion exitosa
}

/*
 * Uso: Ejecuta concatenar_formato de forma segura.
 * Entrada ejemplo: concatenar_formato(destino, separador, formato, arg4)
 */
int concatenar_formato(char *destino, const char *separador, const char *formato, ...)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    imprimirMensaje_para_depurar("[concatenar_formato] destino(ptr): %p\n", (void *)destino);
    imprimirMensaje_para_depurar("[concatenar_formato] destino(valor): %s\n", destino ? destino : "(null)");
    imprimirMensaje_para_depurar("[concatenar_formato] separador: %s\n", separador ? separador : "(null)");
    imprimirMensaje_para_depurar("[concatenar_formato] formato: %s\n", formato ? formato : "(null)");

    if (destino == NULL || formato == NULL) // parametros obligatorios nulos, no se puede continuar
    {
        RETORNAR_PROCESO_ESTANDAR(-1);
    }

    size_t largo_actual = strlen(destino); // longitud actual del texto en el buffer destino // ejemplo: 8 si destino = "Tornillo"
    imprimirMensaje_para_depurar("[concatenar_formato] largo_actual: %zu\n", largo_actual);

    va_list args;            // lista de argumentos variables para vsprintf
    va_start(args, formato); // inicializa la lista de argumentos a partir del parametro 'formato'

    va_list args_preview;                                           // copia de argumentos para estimar el tamano antes de escribir
    va_copy(args_preview, args);                                    // duplica la lista original para no consumirla
    int args_estimados = vsnprintf(NULL, 0, formato, args_preview); // calcula bytes necesarios sin escribir // ejemplo: 5
    va_end(args_preview);                                           // libera la copia de estimacion
    imprimirMensaje_para_depurar("[concatenar_formato] args (estimado chars): %d\n", args_estimados);

    int escritos = vsprintf(destino + largo_actual, formato, args); // escribe directamente al final del buffer destino (requiere espacio suficiente) // ejemplo: agrega "5.00"
    imprimirMensaje_para_depurar("[concatenar_formato] escritos: %d\n", escritos);

    va_end(args); // libera la lista de argumentos original

    imprimirMensaje_para_depurar("[concatenar_formato] escritos: %d\n", escritos);

    if (escritos < 0) // vsprintf fallo durante la escritura
    {
        RETORNAR_PROCESO_ESTANDAR(-1);
    }

    /* Agregar separador al final del valor recien concatenado. */
    if (separador != NULL) // solo si hay separador definido
    {
        strcat(destino, separador); // agrega el separador al final del texto recien escrito // ejemplo: agrega "|"
    }

    imprimirMensaje_para_depurar("[concatenar_formato] destino(final): %s\n", destino);

    RETORNAR_PROCESO_ESTANDAR(0); // concatenacion exitosa
}

/* =======================
FUNCIONES NUEVAS DEL C#
======================== */

/*
 * Uso: Ejecuta join_paresido_simple de forma segura.
 * Entrada ejemplo: join_paresido_simple(caracter_union_filas, texto, n_texto, columnas_extraer, caracter_union_columnas)
 */
char *join_paresido_simple(char caracter_union_filas, char **texto, int n_texto, const char *columnas_extraer, const char *caracter_union_columnas)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!texto || n_texto <= 0) // arreglo nulo o sin filas, retorna cadena vacia
        return calloc(1, 1);

    char *resultado = malloc(65536); // buffer de trabajo de 64 KB para el texto ensamblado
    if (!resultado)                  // fallo de malloc al reservar el buffer de resultado
        return NULL;
    resultado[0] = '\0'; // inicializa el buffer como cadena vacia

    if (columnas_extraer != NULL && strlen(columnas_extraer) > 0) // modo de extraccion selectiva de columnas
    {
        for (int i = 0; i < n_texto; i++) // itera cada fila del arreglo
        {
            char **partes = NULL;                                      // fragmentos de la fila actual separados por el separador de columnas
            int n = split(texto[i], caracter_union_columnas, &partes); // divide la fila actual en columnas // ejemplo: "Tornillo|5|12.50" -> 3 partes
            if (n > 0)                                                 // la fila tiene al menos una columna
            {
                char **cols_extraer = NULL;                                                   // indices de columnas a extraer, parseados
                int n_cols = split(columnas_extraer, caracter_union_columnas, &cols_extraer); // parsea la lista de indices de columnas // ejemplo: "0|2" -> ["0","2"]

                for (int j = 0; j < n_cols; j++) // itera cada indice de columna a extraer
                {
                    int col_idx = atoi(cols_extraer[j]);                // convierte el indice de texto a entero // ejemplo: "2" -> 2
                    if (col_idx < n && col_idx >= 0 && partes[col_idx]) // indice valido y columna existente
                    {
                        if (j > 0)                                                                      // no es la primera columna extraida de esta fila
                            strncat(resultado, caracter_union_columnas, 65535 - strlen(resultado) - 1); // agrega separador entre columnas extraidas
                        strncat(resultado, partes[col_idx], 65535 - strlen(resultado) - 1);             // agrega el valor de la columna al resultado // ejemplo: "Tornillo"
                    }
                }
                free_split(cols_extraer);                                                 // libera los indices de columnas parseados
                strncat(resultado, &caracter_union_filas, 65535 - strlen(resultado) - 1); // agrega el separador de filas al final de la fila procesada
            }
            free_split(partes); // libera la fila dividida en columnas
        }
    }
    else // modo sin extraccion selectiva: une todas las filas tal cual
    {
        for (int i = 0; i < n_texto; i++) // itera cada fila del arreglo
        {
            if (i > 0)                                                                    // no es la primera fila
                strncat(resultado, &caracter_union_filas, 65535 - strlen(resultado) - 1); // agrega separador de filas entre elementos
            if (texto[i])                                                                 // la fila actual no es nula
                strncat(resultado, texto[i], 65535 - strlen(resultado) - 1);              // agrega la fila completa al resultado
        }
    }

    size_t len = strlen(resultado);                            // longitud del resultado antes de limpiar el ultimo separador
    if (len > 0 && resultado[len - 1] == caracter_union_filas) // el resultado termina con un separador de mas
    {
        resultado[len - 1] = '\0'; // elimina el separador sobrante al final del resultado
    }

    return resultado; // retorna el texto ensamblado
}

/*
 * Uso: Ejecuta joineada_paraesida_y_quitador_de_extremos de forma segura.
 * Entrada ejemplo: joineada_paraesida_y_quitador_de_extremos(data, restar_cuantas, restar_primera_celda)
 */
char *joineada_paraesida_y_quitador_de_extremos(const char *data, int restar_cuantas, int restar_primera_celda)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!data) // datos de entrada nulos
        return calloc(1, 1);

    char *resultado = malloc(65536); // buffer de trabajo de 64 KB
    if (!resultado)                  // fallo de malloc
        return NULL;
    resultado[0] = '\0'; // inicializa el buffer como cadena vacia

    char **partes = NULL;                                    // fragmentos de la cadena separada por el separador primario
    int n = split(data, GG_caracter_separacion[0], &partes); // divide el texto en columnas usando el separador primario // ejemplo: "ID|nombre|precio" -> 3 partes
    if (n <= 0)                                              // no se pudo dividir o no hay partes
    {
        free_split(partes); // libera aunque este vacio para evitar fugas
        return resultado;
    }

    if (restar_primera_celda) // modo: saltar las primeras 'restar_cuantas' columnas (quitar desde la izquierda)
    {
        for (int i = restar_cuantas; i < n; i++) // itera desde la columna indicada hasta el final
        {
            if (i > restar_cuantas)                                                           // no es la primera columna del resultado
                strncat(resultado, GG_caracter_separacion[0], 65535 - strlen(resultado) - 1); // agrega separador entre columnas
            if (partes[i])                                                                    // columna no nula
                strncat(resultado, partes[i], 65535 - strlen(resultado) - 1);                 // agrega la columna al resultado // ejemplo: "nombre|precio"
        }
    }
    else // modo: tomar solo las primeras 'n - restar_cuantas' columnas (quitar desde la derecha)
    {
        int cantidad_retornar = n - restar_cuantas; // cantidad de columnas a incluir en el resultado // ejemplo: 3 - 1 = 2
        for (int i = 0; i < cantidad_retornar; i++) // itera solo las columnas que se deben incluir
        {
            if (i > 0)                                                                        // no es la primera columna
                strncat(resultado, GG_caracter_separacion[0], 65535 - strlen(resultado) - 1); // agrega separador entre columnas
            if (partes[i])                                                                    // columna no nula
                strncat(resultado, partes[i], 65535 - strlen(resultado) - 1);                 // agrega la columna al resultado // ejemplo: "ID|nombre"
        }
    }

    free_split(partes); // libera las partes divididas
    return resultado;   // retorna el texto con extremos recortados
}

/*
 * Uso: Ejecuta Trimend_paresido de forma segura.
 * Entrada ejemplo: Trimend_paresido(texto)
 */
char *Trimend_paresido(const char *texto)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!texto) // texto de entrada nulo
        return calloc(1, 1);

    char *resultado = malloc(strlen(texto) + 1); // reserva exactamente el mismo espacio que el texto original
    if (!resultado)                              // fallo de malloc
        return NULL;

    char **partes = NULL;                                     // fragmentos del texto dividido por separador primario
    int n = split(texto, GG_caracter_separacion[0], &partes); // divide el texto en columnas // ejemplo: "A|B|" -> ["A","B",""]
    if (n <= 0)                                               // no se pudo dividir
    {
        free_split(partes);       // libera aunque este vacio
        strcpy(resultado, texto); // retorna el texto original sin modificar
        return resultado;
    }

    resultado[0] = '\0';
    int hasta = n;                                            // cantidad de columnas a incluir, se ajusta si la ultima esta vacia
    if (n > 0 && partes[n - 1] && strlen(partes[n - 1]) == 0) // la ultima columna esta vacia (separador sobrante al final)
        hasta = n - 1;                                        // excluye la ultima columna vacia del resultado // ejemplo: ["A","B",""] -> hasta=2

    for (int i = 0; i < hasta; i++) // construye el resultado sin la columna vacia final
    {
        if (i > 0)                                                            // no es la primera columna
            strncat(resultado, GG_caracter_separacion[0], strlen(texto) - 1); // agrega separador entre columnas
        if (partes[i])                                                        // columna no nula
            strncat(resultado, partes[i], strlen(texto) - 1);                 // agrega el valor de la columna // ejemplo: "A"
    }

    free_split(partes); // libera las partes divididas
    return resultado;   // retorna el texto sin separador final sobrante
}

/*
 * Uso: Ejecuta concatenacion_filas_de_un_archivo de forma segura.
 * Entrada ejemplo: concatenacion_filas_de_un_archivo(ruta_archivo, poner_num_fila)
 */
char *concatenacion_filas_de_un_archivo(const char *ruta_archivo, int poner_num_fila)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    int n_lineas = 0;                                      // contador de lineas leidas del archivo
    char **lineas = leer_archivo(ruta_archivo, &n_lineas); // carga todas las lineas del archivo en memoria // ejemplo: lineas del inventario
    if (!lineas)                                           // fallo al leer el archivo o archivo vacio
        return calloc(1, 1);

    char *resultado = malloc(65536); // buffer de 64 KB para el texto concatenado
    if (!resultado)                  // fallo de malloc
    {
        free_lineas(lineas, n_lineas); // libera lineas antes de salir
        return NULL;
    }
    resultado[0] = '\0'; // inicializa el buffer como cadena vacia

    for (int i = 0; i < n_lineas; i++) // itera cada linea del archivo
    {
        char prefijo[32] = "";                             // buffer para el prefijo de numero de linea
        if (poner_num_fila)                                // el llamador solicito numerar cada linea
            snprintf(prefijo, sizeof(prefijo), "%d) ", i); // genera prefijo numerico // ejemplo: "2) "

        if (strlen(resultado) > 0)                                                        // no es la primera linea, agrega separador antes
            strncat(resultado, GG_caracter_separacion[0], 65535 - strlen(resultado) - 1); // agrega separador entre lineas
        strncat(resultado, prefijo, 65535 - strlen(resultado) - 1);                       // agrega el prefijo numerico (puede ser vacio)
        strncat(resultado, lineas[i], 65535 - strlen(resultado) - 1);                     // agrega el contenido de la linea al resultado
    }

    free_lineas(lineas, n_lineas); // libera todas las lineas del archivo
    return resultado;              // retorna todas las lineas concatenadas con separador
}

/*
 * Uso: Ejecuta concatenacion_filas_de_un_arreglo de forma segura.
 * Entrada ejemplo: concatenacion_filas_de_un_arreglo(arreglo, n_arreglo, poner_num_fila)
 */
char *concatenacion_filas_de_un_arreglo(char **arreglo, int n_arreglo, int poner_num_fila)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    char *resultado = malloc(65536); // buffer de 64 KB para el resultado concatenado
    if (!resultado)                  // fallo de malloc
        return NULL;
    resultado[0] = '\0'; // inicializa como cadena vacia

    for (int i = 0; i < n_arreglo; i++) // itera cada elemento del arreglo
    {
        if (!arreglo[i]) // elemento nulo, se salta
            continue;

        char prefijo[32] = "";                             // buffer para el prefijo numerico opcional
        if (poner_num_fila)                                // el llamador pidio numerar filas
            snprintf(prefijo, sizeof(prefijo), "%d) ", i); // genera prefijo con numero de fila // ejemplo: "3) "

        if (strlen(resultado) > 0)                                                        // no es el primer elemento, agrega separador
            strncat(resultado, GG_caracter_separacion[0], 65535 - strlen(resultado) - 1); // agrega separador entre filas del arreglo
        strncat(resultado, prefijo, 65535 - strlen(resultado) - 1);                       // agrega el prefijo numerico (puede ser vacio)
        strncat(resultado, arreglo[i], 65535 - strlen(resultado) - 1);                    // agrega el contenido del elemento actual // ejemplo: "Tornillo|5"
    }

    return resultado; // retorna los elementos del arreglo concatenados con separador
}

/*
 * Uso: Ejecuta concatenacion_caracter_separacion de forma segura.
 * Entrada ejemplo: concatenacion_caracter_separacion(texto_actual, texto_agregar, separador)
 */
char *concatenacion_caracter_separacion(const char *texto_actual, const char *texto_agregar, const char *separador)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!texto_agregar) // el texto a agregar es nulo, no hay nada que construir
        return calloc(1, 1);

    size_t len_actual = texto_actual ? strlen(texto_actual) : 0; // longitud del texto base, 0 si es nulo // ejemplo: 8 para "Tornillo"
    size_t len_agregar = strlen(texto_agregar);                  // longitud del texto que se agrega // ejemplo: 4 para "5.00"
    size_t len_sep = separador ? strlen(separador) : 0;          // longitud del separador, 0 si no hay // ejemplo: 1 para "|"

    char *resultado = malloc(len_actual + len_agregar + len_sep + 10); // reserva el espacio exacto mas un margen de seguridad
    if (!resultado)                                                    // fallo de malloc
        return NULL;

    resultado[0] = '\0'; // inicializa como cadena vacia antes de construir

    if (texto_actual != NULL && strlen(texto_actual) > 0) // hay texto base valido que copiar
    {
        strcpy(resultado, texto_actual);            // copia el texto base al inicio del resultado // ejemplo: "Tornillo"
        if (separador)                              // hay separador que insertar entre el texto base y el nuevo
            strncat(resultado, separador, len_sep); // agrega el separador despues del texto base // ejemplo: "Tornillo|"
    }

    strncat(resultado, texto_agregar, len_agregar); // agrega el nuevo texto al final // ejemplo: "Tornillo|5.00"
    return resultado;                               // retorna la cadena construida con base + separador + agregado
}

/*
 * Uso: Ejecuta generar_folio de forma segura.
 * Entrada ejemplo: generar_folio(formato_fecha_hora)
 */
char *generar_folio(const char *formato_fecha_hora)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    char *folio = malloc(256); // buffer de 256 bytes para el folio generado // ejemplo: "260405142530"
    if (!folio)                // fallo de malloc
        return NULL;

    time_t t = time(NULL);              // obtiene el tiempo actual del sistema en segundos desde epoch
    struct tm *tm_info = localtime(&t); // convierte el tiempo a estructura local con dia, hora, etc.

    if (formato_fecha_hora == NULL) // sin formato especificado, usa el formato por defecto del sistema
    {
        strftime(folio, 256, "%y%m%d%H%M%S", tm_info); // genera folio con formato AAMMDDHHMMSS // ejemplo: "260405142530"
    }
    else
    {
        strftime(folio, 256, formato_fecha_hora, tm_info); // genera folio con el formato personalizado del llamador // ejemplo: "%d/%m/%Y"
    }

    return folio; // retorna el folio como cadena dinamica, el llamador debe liberarla
}

/*extraer_separado_carpetas_nombreArchivo_extencion:
 Entrada: "C:\carpeta1\carpeta2\archivo.txt"
 Salida:
  resultado[0] = "C:\carpeta1\carpeta2"
  resultado[1] = "archivo"
  resultado[2] = "txt"
  resultado[3] = NULL

char **extraer_separado_carpetas_nombreArchivo_extencion(const char *direccion_archivo)
{
 char **resultado = malloc(3 * sizeof(char *));
 if (!resultado)
  return NULL;

 for (int i = 0; i < 3; i++)
 {
  resultado[i] = malloc(512);
  if (resultado[i])
resultado[i][0] = '\0';
 }

 if (!direccion_archivo)
  return resultado;

 char **partes = NULL;
 int n = split(direccion_archivo, "\\", &partes);
 if (n > 0)
 {
  for (int i = 0; i < n - 1; i++)
  {
strncat(resultado[0], partes[i], 510 - strlen(resultado[0]));
if (i < n - 2)
 strncat(resultado[0], "\\", 510 - strlen(resultado[0]));
  }

  char **nom_ext = NULL;
  int n_ne = split(partes[n - 1], ".", &nom_ext);
  if (n_ne > 0)
  {
strncpy(resultado[1], nom_ext[0], 511);
if (n_ne > 1)
 strncpy(resultado[2], nom_ext[1], 511);
free_split(nom_ext);
  }
  free_split(partes);
 }

 return resultado;
}

*/

/*
 * Uso: Ejecuta desfragmentar_direccion de forma segura.
 * Entrada ejemplo: desfragmentar_direccion(direccion, retorna_directorios, retorna_nom_arch, retorna_extencion)
 */
int desfragmentar_direccion(const char *direccion, char **retorna_directorios, char **retorna_nom_arch, char **retorna_extencion)
{
    // Arreglos temporales para almacenar partes de la ruta y nombre.ext.
    char **partes = NULL;  // fragmentos de la ruta separados por '\'
    char **nom_ext = NULL; // nombre y extension separados por '.'

    // Validacion basica de parametros de entrada/salida.
    if (!direccion || !retorna_directorios || !retorna_nom_arch || !retorna_extencion)
    {
        RETORNAR_PROCESO_ESTANDAR(-1);
    }

    // Inicializar salidas en NULL para evitar punteros basura si algo falla.
    *retorna_directorios = NULL; // directorio padre de la ruta // ejemplo: "C:\espacios\20260406224536_ferreteria_dan"
    *retorna_nom_arch = NULL;    // nombre del archivo sin extension // ejemplo: "inventario"
    *retorna_extencion = NULL;   // extension del archivo // ejemplo: "txt"

    // 1) Dividir la direccion completa por separador de carpetas.
    int n_partes = split(direccion, "\\", &partes); // divide la ruta completa por backslash // ejemplo: 3 partes para "C:\espacios\inventario.txt"
    if (n_partes <= 0 || !partes)                   // la ruta no pudo dividirse
    {
        RETORNAR_PROCESO_ESTANDAR(-1);
    }

    // 2) Unir todo menos la ultima parte como directorios.
    for (int i = 0; i < n_partes - 1; i++) // itera todos los segmentos excepto el ultimo (archivo.ext)
    {
        if (concatenar_formato_separado_por_variable(retorna_directorios, NULL, (*retorna_directorios && (*retorna_directorios)[0] != '\0') ? "\\%s" : "%s", partes[i]) < 0) // fallo al concatenar el segmento de directorio
        {
            free_split(partes); // libera los segmentos antes de retornar error
            RETORNAR_PROCESO_ESTANDAR(-1);
        }
    }

    // 3) Tomar la ultima parte (archivo.ext) y dividirla por '.'.
    int n_nom_ext = split(partes[n_partes - 1], ".", &nom_ext); // divide "inventario.txt" en ["inventario","txt"] // ejemplo: n_nom_ext = 2
    if (n_nom_ext <= 0 || !nom_ext || !nom_ext[0])              // no se pudo extraer el nombre del archivo
    {
        free_split(partes); // libera los segmentos de ruta
        RETORNAR_PROCESO_ESTANDAR(-1);
    }

    // 4) Retornar nombre de archivo y extension (vacia si no existe).
    if (concatenar_formato_separado_por_variable(retorna_nom_arch, NULL, "%s", nom_ext[0]) < 0 ||                                     // copia el nombre del archivo // ejemplo: "inventario"
        concatenar_formato_separado_por_variable(retorna_extencion, NULL, "%s", (n_nom_ext > 1 && nom_ext[1]) ? nom_ext[1] : "") < 0) // copia la extension o cadena vacia // ejemplo: "txt"
    {
        free_split(nom_ext); // libera nombre.ext antes de retornar error
        free_split(partes);  // libera la ruta antes de retornar error
        RETORNAR_PROCESO_ESTANDAR(-1);
    }

    // 5) Liberar temporales y terminar con exito.
    free_split(nom_ext);          // libera los fragmentos de nombre.extension
    free_split(partes);           // libera los segmentos de la ruta completa
    RETORNAR_PROCESO_ESTANDAR(0); // desfragmentacion exitosa
}

/*
 * Uso: Ejecuta ReemplazarCaracteres_de_texto_arreglo de forma segura.
 * Entrada ejemplo: ReemplazarCaracteres_de_texto_arreglo(info, caracteres_sep, n_sep, caracteres_sustitucion)
 */
char *ReemplazarCaracteres_de_texto_arreglo(const char *info, char **caracteres_sep, int n_sep, char **caracteres_sustitucion)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!info) // texto de entrada nulo
        return calloc(1, 1);

    char *resultado = malloc(strlen(info) * 2 + 1); // buffer del doble del tamano original por si los reemplazos son mas largos
    if (!resultado)                                 // fallo de malloc
        return NULL;
    strcpy(resultado, info); // copia el texto original como punto de partida para los reemplazos

    for (int i = 0; i < n_sep; i++) // itera cada par separador/sustitucion
    {
        if (!caracteres_sep[i] || !caracteres_sustitucion[i]) // par incompleto, se salta
            continue;

        char *temp = malloc(strlen(resultado) * 2 + 1024); // buffer temporal con margen para reemplazos mas largos
        if (!temp)                                         // fallo de malloc
        {
            free(resultado); // libera el resultado parcial antes de retornar
            return NULL;
        }
        temp[0] = '\0'; // inicializa el buffer temporal como cadena vacia

        const char *pos = resultado;             // cursor de lectura sobre el resultado actual
        const char *found;                       // puntero a la primera ocurrencia del separador buscado
        int sep_len = strlen(caracteres_sep[i]); // longitud del separador a reemplazar // ejemplo: 1 para "|"

        while ((found = strstr(pos, caracteres_sep[i])) != NULL) // busca siguiente ocurrencia del separador
        {
            strncat(temp, pos, found - pos);         // copia el fragmento anterior a la ocurrencia encontrada
            strcat(temp, caracteres_sustitucion[i]); // inserta la sustitucion en lugar del separador encontrado // ejemplo: cambia "|" por "~"
            pos = found + sep_len;                   // avanza el cursor despues del separador reemplazado
        }
        strcat(temp, pos); // agrega el fragmento final que queda despues de la ultima ocurrencia

        free(resultado);  // libera el buffer anterior que fue reemplazado
        resultado = temp; // apunta al nuevo buffer con los reemplazos aplicados
    }

    return resultado; // retorna el texto con todos los reemplazos aplicados
}

/*
 * Uso: Reemplaza todas las ocurrencias de un string separador por un string de sustitucion.
 * Entrada ejemplo: ReemplazarCaracteres_de_texto_string(info, "|", "~")
 */
char *ReemplazarCaracteres_de_texto_string(const char *info, const char *sep, const char *sust)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!info) // texto de entrada nulo
    {
        return calloc(1, 1);
    }
    if (!sep || !sust || sep[0] == '\0') // separador o sustitucion nulos, retorna copia del original
    {
        char *copia = malloc(strlen(info) + 1);
        if (!copia)
        {
            return NULL;
        }
        strcpy(copia, info);
        return copia;
    }

    int sep_len = strlen(sep); // longitud del separador a reemplazar

    char *resultado = malloc(strlen(info) * 2 + 1024); // buffer con margen para reemplazos mas largos
    if (!resultado)                                     // fallo de malloc
    {
        return NULL;
    }
    resultado[0] = '\0'; // inicializa el buffer como cadena vacia

    const char *pos = info;   // cursor de lectura sobre el texto original
    const char *found;        // puntero a la siguiente ocurrencia del separador

    while ((found = strstr(pos, sep)) != NULL) // busca siguiente ocurrencia del separador
    {
        strncat(resultado, pos, found - pos); // copia el fragmento anterior a la ocurrencia encontrada
        strcat(resultado, sust);              // inserta la sustitucion en lugar del separador // ejemplo: cambia "|" por "~"
        pos = found + sep_len; // avanza el cursor despues del separador reemplazado
    }
    strcat(resultado, pos); // agrega el fragmento final despues de la ultima ocurrencia

    return resultado; // retorna el texto con todos los reemplazos aplicados
}

/* =========================================================================
BUSQUEDA PROFUNDA - Buscar en estructuras anidadas por columnas
========================================================================= */

/*
 * Uso: Ejecuta busqueda_profunda_string de forma segura.
 * Entrada ejemplo: busqueda_profunda_string(texto, columnas_recorrer, comparar)
 */
char *busqueda_profunda_string(const char *texto, const char *columnas_recorrer, const char *comparar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!texto || !columnas_recorrer || !comparar) // alguno de los parametros requeridos es nulo
        return calloc(1, 1);

    /* Usar split para acceder a filas usando separador primario */
    char **filas = NULL;                                           // arreglo de filas del texto dividido
    int n_filas = split(texto, GG_caracter_separacion[0], &filas); // divide el texto en filas usando el separador primario // ejemplo: GG_caracter_separacion[0]="|"

    char *resultado = malloc(65536); // buffer de 64 KB para acumular las filas que coincidan
    if (!resultado)                  // fallo de malloc
        return NULL;
    resultado[0] = '\0'; // inicializa el buffer como cadena vacia

    int encontrado = 0;               // bandera que indica si ya se encontro al menos una coincidencia
    for (int i = 0; i < n_filas; i++) // itera cada fila del texto
    {
        if (!filas[i]) // fila nula, se salta
            continue;

        /* Split la fila por segundo separador para acceder a columnas */
        char **columnas = NULL;                                             // columnas de la fila actual
        int n_cols = split(filas[i], GG_caracter_separacion[1], &columnas); // divide la fila en columnas usando el segundo separador // ejemplo: GG_caracter_separacion[1]="�"

        /* Recorrer columnas seg�n �ndices especificados */
        const char *pos = columnas_recorrer; // cursor sobre la lista de indices de columnas a comparar
        while (pos && *pos)                  // mientras haya indices por procesar
        {
            int col_idx = atoi(pos);                                   // convierte el indice de texto a entero // ejemplo: "2" -> 2
            if (col_idx >= 0 && col_idx < n_cols && columnas[col_idx]) // indice valido y columna existente
            {
                if (strcmp(columnas[col_idx], comparar) == 0) // la columna coincide con el valor buscado
                {
                    if (encontrado)                                   // ya hubo una coincidencia previa, agrega separador
                        strcat(resultado, GG_caracter_separacion[0]); // separa las filas coincidentes en el resultado
                    strcat(resultado, filas[i]);                      // agrega la fila completa al resultado // ejemplo: "20260406224536_ferreteria_dan|Tornillo|5"
                    encontrado = 1;                                   // marca que se encontro al menos una coincidencia
                    break;                                            // no sigue revisando columnas de esta fila
                }
            }
            /* Buscar siguiente �ndice separado por '|' */
            while (pos && *pos && *pos != '|') // avanza hasta el siguiente separador de indices
                pos++;
            if (*pos == '|') // salta el separador de indices
                pos++;
        }

        free_split(columnas); // libera las columnas de la fila actual
    }

    free_split(filas); // libera todas las filas del texto dividido
    return resultado;  // retorna las filas que coincidieron con el valor buscado
}

/* =========================================================================
B�SQUEDA PROFUNDA CON FORMATO FINAL
========================================================================= */

/*
 * Uso: Ejecuta busqueda_profunda_comparacion_final_string de forma segura.
 * Entrada ejemplo: busqueda_profunda_comparacion_final_string(texto, columnas_recorrer, comparar)
 */
char *busqueda_profunda_comparacion_final_string(const char *texto, const char *columnas_recorrer, const char *comparar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!texto || !columnas_recorrer || !comparar) // parametros obligatorios nulos
        return calloc(1, 1);

    /* Similar a busqueda_profunda_string pero retorna con formato especial */
    char **filas = NULL;                                           // arreglo de filas del texto dividido
    int n_filas = split(texto, GG_caracter_separacion[0], &filas); // divide en filas usando el separador primario

    char *resultado = malloc(65536); // buffer de 64 KB para la ultima fila coincidente
    if (!resultado)                  // fallo de malloc
        return NULL;
    resultado[0] = '\0'; // inicializa como cadena vacia

    int encontrado = 0;             // bandera de si alguna fila coincidio
    const char *ultima_fila = NULL; // puntero a la ultima fila que cumplio la comparacion

    for (int i = 0; i < n_filas; i++) // itera cada fila
    {
        if (!filas[i]) // fila nula, se salta
            continue;

        char **columnas = NULL;                                             // columnas de la fila actual
        int n_cols = split(filas[i], GG_caracter_separacion[1], &columnas); // divide la fila en columnas con el segundo separador

        const char *pos = columnas_recorrer; // cursor sobre los indices de columnas a comparar
        while (pos && *pos)                  // mientras haya indices
        {
            int col_idx = atoi(pos);                                   // convierte el indice de texto a entero
            if (col_idx >= 0 && col_idx < n_cols && columnas[col_idx]) // indice y columna validos
            {
                if (strcmp(columnas[col_idx], comparar) == 0) // coincidencia encontrada
                {
                    ultima_fila = filas[i]; // guarda referencia a la ultima fila coincidente
                    encontrado = 1;         // marca que hubo coincidencia
                    break;                  // no sigue buscando en esta fila
                }
            }
            while (pos && *pos && *pos != '|') // avanza al siguiente indice
                pos++;
            if (*pos == '|') // salta el separador entre indices
                pos++;
        }

        free_split(columnas); // libera columnas de esta fila
    }

    if (encontrado && ultima_fila)      // hubo al menos una fila coincidente
        strcpy(resultado, ultima_fila); // copia la ultima fila coincidente al resultado // ejemplo: ultima fila con el valor buscado

    free_split(filas); // libera todas las filas
    return resultado;  // retorna solo la ultima fila que coincidio
}

/* =========================================================================
B�SQUEDA CON M�LTIPLES CONDICIONES (YY)
========================================================================= */

char *busqueda_con_YY_profunda_texto_id_archivo(const char *texto, const char *columnas_recorrer, const char *comparaciones)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!texto || !columnas_recorrer || !comparaciones) // parametros requeridos nulos
        return calloc(1, 1);

    char **filas = NULL;                                           // arreglo de filas del texto dividido
    int n_filas = split(texto, GG_caracter_separacion[0], &filas); // divide en filas con el separador primario

    char *resultado = malloc(65536); // buffer de 64 KB para las filas que cumplan todas las condiciones
    if (!resultado)                  // fallo de malloc
        return NULL;
    resultado[0] = '\0'; // inicializa como cadena vacia

    /* Parsear comparaciones m�ltiples (separadas por alg�n delimitador) */
    char **comparaciones_arr = NULL;                                     // arreglo de condiciones individuales
    int n_comparaciones = split(comparaciones, "|", &comparaciones_arr); // separa las condiciones por '|' // ejemplo: "0:ID123|2:Tornillo"

    int encontrado = 0;               // bandera de si alguna fila paso todas las condiciones
    for (int i = 0; i < n_filas; i++) // itera cada fila
    {
        if (!filas[i]) // fila nula, se salta
            continue;

        char **columnas = NULL;                                             // columnas de la fila actual
        int n_cols = split(filas[i], GG_caracter_separacion[1], &columnas); // divide la fila en columnas

        int todas_coinciden = 1; // asume que la fila cumple todas las condiciones hasta que se demuestre lo contrario

        /* Verificar todas las condiciones */
        for (int j = 0; j < n_comparaciones && todas_coinciden; j++) // itera cada condicion de busqueda
        {
            if (!comparaciones_arr[j]) // condicion nula, se salta
                continue;

            /* Parsear "col_idx:valor" */
            char temp[1024];                    // buffer temporal para procesar la condicion
            strcpy(temp, comparaciones_arr[j]); // copia la condicion para poder modificarla
            char *colon = strchr(temp, ':');    // busca el separador ':' entre indice y valor // ejemplo: "2:Tornillo" -> colon apunta a ":Tornillo"

            if (colon) // encontro el separador, la condicion tiene formato valido
            {
                *colon = '\0';                          // divide el string en indice y valor separados
                int col_idx = atoi(temp);               // convierte el indice de texto a entero // ejemplo: "2" -> 2
                const char *valor_esperado = colon + 1; // el valor esperado empieza despues del ':' // ejemplo: "Tornillo"

                if (col_idx < 0 || col_idx >= n_cols || !columnas[col_idx] || strcmp(columnas[col_idx], valor_esperado) != 0) // alguna condicion no se cumple
                {
                    todas_coinciden = 0; // la fila no pasa esta condicion
                }
            }
        }

        if (todas_coinciden) // la fila cumplio todas las condiciones YY
        {
            if (encontrado)                                   // ya hubo una coincidencia previa
                strcat(resultado, GG_caracter_separacion[0]); // agrega separador entre filas coincidentes
            strcat(resultado, filas[i]);                      // agrega la fila que paso todas las condiciones // ejemplo: fila del producto buscado
            encontrado = 1;                                   // marca que hay al menos una coincidencia
        }

        free_split(columnas); // libera las columnas de esta fila
    }

    free_split(comparaciones_arr); // libera las condiciones parseadas
    free_split(filas);             // libera las filas del texto
    return resultado;              // retorna las filas que cumplieron todas las condiciones
}

/* =========================================================================
EDICI�N CON INCREMENTO RECURSIVO
========================================================================= */

/*
 * Uso: Ejecuta editar_incr_string_funcion_recursiva de forma segura.
 * Entrada ejemplo: editar_incr_string_funcion_recursiva(texto, columnas_recorrer, info_sustituir, edit_0_increm_1)
 */
char *editar_incr_string_funcion_recursiva(const char *texto, const char *columnas_recorrer, const char *info_sustituir, const char *edit_0_increm_1)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!texto || !columnas_recorrer) // parametros requeridos nulos
        return calloc(1, 1);

    int es_incremento = edit_0_increm_1 && strcmp(edit_0_increm_1, "1") == 0; // modo incremento si el flag es "1" // ejemplo: "1" -> suma 1 al valor numerico

    char **filas = NULL;                                           // arreglo de filas del texto dividido
    int n_filas = split(texto, GG_caracter_separacion[0], &filas); // divide en filas con el separador primario

    char *resultado = malloc(65536); // buffer de 64 KB para el texto editado
    if (!resultado)                  // fallo de malloc
        return NULL;
    resultado[0] = '\0'; // inicializa como cadena vacia

    for (int i = 0; i < n_filas; i++) // itera cada fila del texto
    {
        if (i > 0)                                        // no es la primera fila
            strcat(resultado, GG_caracter_separacion[0]); // agrega separador entre filas reconstruidas

        if (!filas[i]) // fila nula, se salta
            continue;

        char **columnas = NULL;                                             // columnas de la fila actual
        int n_cols = split(filas[i], GG_caracter_separacion[1], &columnas); // divide la fila en columnas con el segundo separador

        /* Parsear �ndices de columnas a editar */
        const char *pos = columnas_recorrer; // cursor sobre la lista de indices de columnas a editar
        while (pos && *pos)                  // mientras haya indices por procesar
        {
            int col_idx = atoi(pos);              // convierte el indice de texto a entero // ejemplo: "2" -> 2
            if (col_idx >= 0 && col_idx < n_cols) // indice valido dentro del rango de columnas
            {
                if (es_incremento && columnas[col_idx]) // modo incremento: suma 1 al valor actual
                {
                    /* Incrementar valor num�rico */
                    int val = atoi(columnas[col_idx]);                 // convierte el valor actual de la columna a entero // ejemplo: "5" -> 5
                    char new_val[64];                                  // buffer para el nuevo valor como texto
                    snprintf(new_val, sizeof(new_val), "%d", val + 1); // calcula el valor incrementado // ejemplo: 5 + 1 = 6
                    strcpy(columnas[col_idx], new_val);                // sobreescribe la columna con el nuevo valor // ejemplo: columna[2] = "6"
                }
                else if (info_sustituir && columnas[col_idx]) // modo reemplazo: pone el valor nuevo
                {
                    /* Reemplazar con nuevo valor */
                    strcpy(columnas[col_idx], info_sustituir); // reemplaza el valor de la columna // ejemplo: columna[2] = "Tornillo Grande"
                }
            }
            while (pos && *pos && *pos != '|') // avanza al siguiente separador de indices
                pos++;
            if (*pos == '|') // salta el separador
                pos++;
        }

        /* Reconstruir fila */
        for (int j = 0; j < n_cols; j++) // reassembla las columnas editadas en una fila
        {
            if (j > 0)                                        // no es la primera columna
                strcat(resultado, GG_caracter_separacion[1]); // agrega separador entre columnas reconstruidas
            if (columnas[j])                                  // columna no nula
                strcat(resultado, columnas[j]);               // agrega el valor de la columna al resultado
        }

        free_split(columnas); // libera las columnas de esta fila
    }

    free_split(filas); // libera todas las filas
    return resultado;  // retorna el texto con las columnas indicadas editadas/incrementadas
}

/* =========================================================================
EDICI�N PROFUNDA M�LTIPLE CON COMPARACI�N FINAL
========================================================================= */

/*
 * Uso: Ejecuta editar_inc_agregar_edicion_profunda_multiple_comparacion_final_string de forma segura.
 * Entrada ejemplo: editar_inc_agregar_edicion_profunda_multiple_comparacion_final_string(texto, indices_editar, info_editar, comparacion, edit_0_increm_1)
 */
char *editar_inc_agregar_edicion_profunda_multiple_comparacion_final_string(const char *texto, const char *indices_editar, const char *info_editar, const char *comparacion, const char *edit_0_increm_1)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!texto || !indices_editar)
        return calloc(1, 1);

    /* Combina b�squeda con comparaci�n y edici�n */
    char *encontrado = busqueda_profunda_comparacion_final_string(texto, indices_editar, comparacion);

    if (encontrado && strlen(encontrado) > 0)
    {
        char *editado = editar_incr_string_funcion_recursiva(encontrado, indices_editar, info_editar, edit_0_increm_1);
        free(encontrado);
        return editado;
    }

    free(encontrado);
    return calloc(1, 1);
}

/* =========================================================================
WRAPPER ARR_FUN - Edici�n con estructura simplificada
========================================================================= */

/*
 * Uso: Ejecuta ARR_FUN_SOLO_TEXTO_editar_inc_agregar_edicion_profunda_multiple de forma segura.
 * Entrada ejemplo: ARR_FUN_SOLO_TEXTO_editar_inc_agregar_edicion_profunda_multiple(datos)
 */
char *ARR_FUN_SOLO_TEXTO_editar_inc_agregar_edicion_profunda_multiple(const char *datos)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!datos)
        return calloc(1, 1);

    /* Parsear estructura: "texto|indices|info|edit_mode" */
    char **partes = NULL;
    int n_partes = split(datos, "|", &partes);

    char *resultado = calloc(1, 1);

    if (n_partes >= 3)
    {
        resultado = editar_incr_string_funcion_recursiva(partes[0], partes[1], partes[2], n_partes > 3 ? partes[3] : "0");
    }

    free_split(partes);
    return resultado;
}

/* =========================================================================
EDICI�N CON M�LTIPLES CHEQUEOS
========================================================================= */

/*
 * Uso: Ejecuta editar_inc_agregar_edicion_profunda_multiple_comparacion_MULTIPLE_A_CHECAR de forma segura.
 * Entrada ejemplo: editar_inc_agregar_edicion_profunda_multiple_comparacion_MULTIPLE_A_CHECAR(texto, indices_editar, comparacion_con_edicion, edit_0_increm_1)
 */
char *editar_inc_agregar_edicion_profunda_multiple_comparacion_MULTIPLE_A_CHECAR(const char *texto, const char *indices_editar, const char *comparacion_con_edicion, const char *edit_0_increm_1)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!texto || !indices_editar || !comparacion_con_edicion)
        return calloc(1, 1);

    /* Parsear comparaci�n_con_edicion para extraer comparaci�n e info_editar */
    char temp[1024];
    strcpy(temp, comparacion_con_edicion);
    char *coma = strchr(temp, ',');

    char *comparar_val = temp;
    char *editar_val = "";

    if (coma)
    {
        *coma = '\0';
        editar_val = coma + 1;
    }

    /* Buscar filas que coincidan y editarlas */
    char **filas = NULL;
    int n_filas = split(texto, GG_caracter_separacion[0], &filas);

    char *resultado = malloc(65536);
    if (!resultado)
        return NULL;
    resultado[0] = '\0';

    for (int i = 0; i < n_filas; i++)
    {
        if (i > 0)
            strcat(resultado, GG_caracter_separacion[0]);

        if (!filas[i])
            continue;

        /* Verificar si esta fila coincide con la b�squeda */
        char **columnas = NULL;
        int n_cols = split(filas[i], GG_caracter_separacion[1], &columnas);

        int debe_editar = 0;
        const char *pos = indices_editar;
        while (pos && *pos)
        {
            int col_idx = atoi(pos);
            if (col_idx >= 0 && col_idx < n_cols && columnas[col_idx] && strcmp(columnas[col_idx], comparar_val) == 0)
            {
                debe_editar = 1;
                break;
            }
            while (pos && *pos && *pos != '|')
                pos++;
            if (*pos == '|')
                pos++;
        }

        if (debe_editar)
        {
            pos = indices_editar;
            while (pos && *pos)
            {
                int col_idx = atoi(pos);
                if (col_idx >= 0 && col_idx < n_cols)
                {
                    int es_incremento = edit_0_increm_1 && strcmp(edit_0_increm_1, "1") == 0;
                    if (columnas[col_idx] && strcmp(columnas[col_idx], comparar_val) == 0)
                    {
                        if (es_incremento)
                        {
                            int val = atoi(columnas[col_idx]);
                            snprintf(columnas[col_idx], 64, "%d", val + 1);
                        }
                        else if (*editar_val)
                        {
                            strcpy(columnas[col_idx], editar_val);
                        }
                    }
                }
                while (pos && *pos && *pos != '|')
                    pos++;
                if (*pos == '|')
                    pos++;
            }
        }

        for (int j = 0; j < n_cols; j++)
        {
            if (j > 0)
                strcat(resultado, GG_caracter_separacion[1]);
            if (columnas[j])
                strcat(resultado, columnas[j]);
        }

        free_split(columnas);
    }

    free_split(filas);
    return resultado;
}

/* =========================================================================
EDICI�N PROFUNDA M�LTIPLE SIMPLE
========================================================================= */

/*
 * Uso: Ejecuta editar_inc_edicion_profunda_multiple_string de forma segura.
 * Entrada ejemplo: editar_inc_edicion_profunda_multiple_string(texto, indices_editar, info_editar, edit_0_increm_1)
 */
char *editar_inc_edicion_profunda_multiple_string(const char *texto, const char *indices_editar, const char *info_editar, const char *edit_0_increm_1)
{
    /* Simplemente aplica la edici�n a todas las filas en los �ndices especificados */
    char **filas = NULL;
    int n_filas = split(texto, GG_caracter_separacion[0], &filas);

    char *resultado = malloc(65536);
    if (!resultado)
        return NULL;
    resultado[0] = '\0';

    int es_incremento = edit_0_increm_1 && strcmp(edit_0_increm_1, "1") == 0;

    for (int i = 0; i < n_filas; i++)
    {
        if (i > 0)
            strcat(resultado, GG_caracter_separacion[0]);

        if (!filas[i])
            continue;

        char **columnas = NULL;
        int n_cols = split(filas[i], GG_caracter_separacion[1], &columnas);

        const char *pos = indices_editar;
        while (pos && *pos)
        {
            int col_idx = atoi(pos);
            if (col_idx >= 0 && col_idx < n_cols && columnas[col_idx])
            {
                if (es_incremento)
                {
                    int val = atoi(columnas[col_idx]);
                    snprintf(columnas[col_idx], 64, "%d", val + 1);
                }
                else if (info_editar)
                {
                    strcpy(columnas[col_idx], info_editar);
                }
            }
            while (pos && *pos && *pos != '|')
                pos++;
            if (*pos == '|')
                pos++;
        }

        for (int j = 0; j < n_cols; j++) // reconstruye la fila con columnas modificadas
        {
            if (j > 0)                                        // no es la primera columna
                strcat(resultado, GG_caracter_separacion[1]); // agrega separador entre columnas
            if (columnas[j])                                  // columna no nula
                strcat(resultado, columnas[j]);               // agrega el valor de la columna al resultado
        }

        free_split(columnas); // libera las columnas de esta fila
    }

    free_split(filas); // libera las filas divididas
    return resultado;  // retorna el texto con todas las filas editadas en los indices indicados
}

/* =========================================================================
EDICI�N PROFUNDA AL FINAL
========================================================================= */

/*
 * Uso: Ejecuta editar_inc_edicion_profunda_multiple_AL_FINAL_string de forma segura.
 * Entrada ejemplo: editar_inc_edicion_profunda_multiple_AL_FINAL_string(texto, indices_editar, info_editar, edit_0_increm_1)
 */
char *editar_inc_edicion_profunda_multiple_AL_FINAL_string(const char *texto, const char *indices_editar, const char *info_editar, const char *edit_0_increm_1)
{
    /* Edita solo la �ltima fila que coincide */
    char **filas = NULL;                                           // arreglo de filas del texto dividido
    int n_filas = split(texto, GG_caracter_separacion[0], &filas); // divide en filas con el separador primario

    char *resultado = malloc(65536); // buffer de 64 KB para el texto resultante
    if (!resultado)                  // fallo de malloc
        return NULL;
    resultado[0] = '\0'; // inicializa como cadena vacia

    int es_incremento = edit_0_increm_1 && strcmp(edit_0_increm_1, "1") == 0; // modo incremento si el flag es "1"

    /* Primera pasada: copiar todas las filas */
    for (int i = 0; i < n_filas; i++) // itera cada fila del texto
    {
        if (i > 0)                                        // no es la primera fila
            strcat(resultado, GG_caracter_separacion[0]); // agrega separador entre filas

        if (!filas[i]) // fila nula, se salta
            continue;

        /* En la �ltima iteraci�n, editar */
        if (i == n_filas - 1) // es la ultima fila: aplica la edicion
        {
            char **columnas = NULL;                                             // columnas de la ultima fila
            int n_cols = split(filas[i], GG_caracter_separacion[1], &columnas); // divide la ultima fila en columnas

            const char *pos = indices_editar; // cursor sobre los indices de columnas a editar
            while (pos && *pos)
            {
                int col_idx = atoi(pos);
                if (col_idx >= 0 && col_idx < n_cols && columnas[col_idx])
                {
                    if (es_incremento) // modo incremento: suma 1
                    {
                        int val = atoi(columnas[col_idx]);              // obtiene el valor actual como entero // ejemplo: "10" -> 10
                        snprintf(columnas[col_idx], 64, "%d", val + 1); // escribe el valor incrementado como texto // ejemplo: "11"
                    }
                    else if (info_editar) // modo reemplazo: pone el nuevo valor
                    {
                        strcpy(columnas[col_idx], info_editar); // reemplaza el valor de la columna // ejemplo: "Descontinuado"
                    }
                }
                while (pos && *pos && *pos != '|') // avanza al siguiente separador de indices
                    pos++;
                if (*pos == '|') // salta el separador de indices
                    pos++;
            }

            for (int j = 0; j < n_cols; j++) // reconstruye la ultima fila con columnas editadas
            {
                if (j > 0)                                        // no es la primera columna
                    strcat(resultado, GG_caracter_separacion[1]); // agrega separador entre columnas
                if (columnas[j])                                  // columna no nula
                    strcat(resultado, columnas[j]);               // agrega el valor de la columna al resultado
            }

            free_split(columnas); // libera las columnas de la ultima fila
        }
        else
        {
            strcat(resultado, filas[i]); // copia las filas anteriores sin modificar
        }
    }

    free_split(filas); // libera las filas divididas
    return resultado;  // retorna el texto con la ultima fila editada
}

/* =========================================================================
RECORRER CARACTERES DE SEPARACI�N
========================================================================= */

/*
 * Uso: Ejecuta recorrer_caracter_separacion de forma segura.
 * Entrada ejemplo: recorrer_caracter_separacion(contenidoFila, izquierda_o_derecha, numero_veses)
 */
char *recorrer_caracter_separacion(const char *contenidoFila, const char *izquierda_o_derecha, int numero_veses)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!contenidoFila) // texto de entrada nulo
        return calloc(1, 1);

    char **partes = NULL;                                                    // arreglo de partes divididas por el separador primario
    int n_partes = split(contenidoFila, GG_caracter_separacion[0], &partes); // divide el texto en partes con el separador primario // ejemplo: 4 partes para "A|B|C|D"

    char *resultado = malloc(65536); // buffer de 64 KB para el resultado
    if (!resultado)                  // fallo de malloc
        return NULL;
    resultado[0] = '\0'; // inicializa como cadena vacia

    int es_izquierda = izquierda_o_derecha && strcmp(izquierda_o_derecha, "izq") == 0; // modo izquierda: descarta los primeros N elementos // ejemplo: "izq"

    if (es_izquierda) // desplazamiento desde la izquierda: salta los primeros 'numero_veses' elementos
    {
        /* Saltar primeros 'numero_veses' elementos */
        for (int i = numero_veses; i < n_partes; i++) // itera desde el elemento indicado hasta el final
        {
            if (i > numero_veses)                             // no es el primer elemento del resultado
                strcat(resultado, GG_caracter_separacion[0]); // agrega separador entre elementos
            if (partes[i])                                    // elemento no nulo
                strcat(resultado, partes[i]);                 // agrega el elemento al resultado // ejemplo: "C|D" si numero_veses=2
        }
    }
    else // desplazamiento desde la derecha: toma solo los primeros 'n - numero_veses' elementos
    {
        /* Tomar hasta 'n - numero_veses' elementos */
        int hasta = n_partes - numero_veses;            // cantidad de elementos a incluir // ejemplo: 4 - 2 = 2
        for (int i = 0; i < hasta && i < n_partes; i++) // itera hasta la cantidad calculada
        {
            if (i > 0)                                        // no es el primer elemento
                strcat(resultado, GG_caracter_separacion[0]); // agrega separador entre elementos
            if (partes[i])                                    // elemento no nulo
                strcat(resultado, partes[i]);                 // agrega el elemento al resultado // ejemplo: "A|B"
        }
    }

    free_split(partes); // libera las partes divididas
    return resultado;   // retorna el texto con los extremos recortados por separador
}

/* =========================================================================
RECORRER CARACTERES DE SEPARACI�N - FUNCI�N ESPEC�FICA
========================================================================= */

/*
 * Uso: Ejecuta recorrer_caracter_separacion_funciones_espesificas de forma segura.
 * Entrada ejemplo: recorrer_caracter_separacion_funciones_espesificas(contenidoFila, izquierda_o_derecha, numero_veses)
 */
char *recorrer_caracter_separacion_funciones_espesificas(const char *contenidoFila, const char *izquierda_o_derecha, int numero_veses)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!contenidoFila) // texto de entrada nulo
        return calloc(1, 1);

    /* Usar separador espec�fico si est� disponible */
    char separador[16];                                                     // buffer para el separador a usar en esta funcion
    if (GG_caracter_separacion_funciones_espesificas[0])                    // hay un separador especifico configurado para funciones
        strcpy(separador, GG_caracter_separacion_funciones_espesificas[0]); // usa el separador especifico de funciones // ejemplo: "~"
    else
        strcpy(separador, GG_caracter_separacion[0]); // usa el separador primario como fallback // ejemplo: "|"

    char **partes = NULL;                                    // arreglo de partes divididas por el separador
    int n_partes = split(contenidoFila, separador, &partes); // divide el texto usando el separador configurado

    char *resultado = malloc(65536); // buffer de 64 KB para el resultado
    if (!resultado)                  // fallo de malloc
        return NULL;
    resultado[0] = '\0'; // inicializa como cadena vacia

    int es_izquierda = izquierda_o_derecha && strcmp(izquierda_o_derecha, "izq") == 0; // modo izquierda: descarta los primeros N elementos

    if (es_izquierda) // descarta elementos desde la izquierda
    {
        for (int i = numero_veses; i < n_partes; i++) // itera desde el elemento indicado hasta el final
        {
            if (i > numero_veses)             // no es el primer elemento del resultado
                strcat(resultado, separador); // agrega separador entre elementos
            if (partes[i])                    // elemento no nulo
                strcat(resultado, partes[i]); // agrega el elemento al resultado
        }
    }
    else // descarta elementos desde la derecha
    {
        int hasta = n_partes - numero_veses;            // cantidad de elementos a incluir // ejemplo: 4 - 1 = 3
        for (int i = 0; i < hasta && i < n_partes; i++) // itera hasta la cantidad calculada
        {
            if (i > 0)                        // no es el primer elemento
                strcat(resultado, separador); // agrega separador entre elementos
            if (partes[i])                    // elemento no nulo
                strcat(resultado, partes[i]); // agrega el elemento al resultado
        }
    }

    free_split(partes); // libera las partes divididas
    return resultado;   // retorna el texto con los extremos recortados usando el separador especifico
}
