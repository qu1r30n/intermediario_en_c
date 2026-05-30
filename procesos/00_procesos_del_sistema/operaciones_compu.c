/* LIBRERIAS USADAS EN ESTE ARCHIVO:
 * - ../../cabeceras/cabeceras_procesos/00_cabeceras_del_sistema/operaciones_compu.h: Dependencia interna del proyecto
 * - stdio.h: Entrada y salida estandar (printf, fopen, etc.)
 * - stdarg.h: Argumentos variables de funciones tipo printf
 * - windows.h: Dependencia interna del proyecto
 * - time.h: Fecha y hora
 * - xc.h: Cabecera del compilador para microcontroladores PIC
 */
#include "../../cabeceras/cabeceras_procesos/00_cabeceras_del_sistema/operaciones_compu.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <time.h>
#include <windows.h>

#elif defined(__linux__)
#include <time.h>

#elif defined(__XC)
#define _XTAL_FREQ 4000000
#include <xc.h>
#endif

/*
 * Uso: Ejecuta imprimirMensaje_para_depurar de forma segura.
 * Entrada ejemplo: imprimirMensaje_para_depurar(format, arg2)
 */
void imprimirMensaje_para_depurar(const char *format, ...) // define una funcion de depuracion con argumentos variables para imprimir mensajes formateados // ejemplo: "Valor=%d"
{
    // ---------------------------
    // Los "..." significan que la función puede recibir
    // un número indefinido de argumentos adicionales,
    // igual que printf.
    // Por ejemplo:
    // imprimirMensaje_para_depurar("Valor=%d, puntero=%p\n", 42, ptr);
    // El 42 y ptr son los "argumentos variables"
    // ---------------------------

    va_list args;           // declara la lista que almacenará los argumentos variables recibidos // ejemplo: 42, ptr
    va_start(args, format); // inicializa la lectura de argumentos variables usando format como último parámetro fijo // ejemplo: "Valor=%d"

#ifdef PIC16F
    char buffer[80];                                 // crea un buffer temporal para formatear el mensaje antes de imprimirlo // ejemplo: "Valor=42\n"
    vsnprintf(buffer, sizeof(buffer), format, args); // escribe el texto formateado dentro de buffer respetando su tamaño // ejemplo: "Temperatura=25"
    printf("%s", buffer);                            // envía el contenido formateado al stdout del entorno embebido // ejemplo: "Estado listo"
#else
    vprintf(format, args); // imprime directamente el formato usando la lista de argumentos variables ya preparada // ejemplo: "Valor=42\n"
#endif

    va_end(args); // cierra el uso de la lista variádica para liberar su estado interno // ejemplo: fin de lectura de args
}

/* =========================
   DELAY PORTABLE
========================= */

/*
 * Uso: Ejecuta delay_ms de forma segura.
 * Entrada ejemplo: delay_ms(ms)
 */
void delay_ms(unsigned int ms) // define una funcion de espera portable basada en milisegundos // ejemplo: 500
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */

#ifdef _WIN32
    Sleep(ms); // pausa la ejecución la cantidad de milisegundos indicada usando la API de Windows // ejemplo: 500

#elif defined(__linux__)
    struct timespec ts;                 // declara la estructura que representa el tiempo de espera en segundos y nanosegundos // ejemplo: {0, 500000000}
    ts.tv_sec = ms / 1000;              // convierte la parte entera de milisegundos a segundos completos // ejemplo: 2500 -> 2
    ts.tv_nsec = (ms % 1000) * 1000000; // convierte los milisegundos restantes a nanosegundos // ejemplo: 500 -> 500000000
    nanosleep(&ts, NULL);               // suspende el hilo actual durante el tiempo calculado en ts // ejemplo: espera de 2.5 segundos

#elif defined(__XC)
    while (ms--) // repite una espera de 1 ms hasta consumir el total solicitado en XC
    {
        __delay_ms(1); // ejecuta un retardo de un milisegundo usando la primitiva del compilador XC // ejemplo: 1 ms
    }
#endif
}

/* =========================
   FECHA ACTUAL PORTABLE
========================= */

/*
 * Uso: Ejecuta fechaActual de forma segura.
 * Entrada ejemplo: fechaActual(buffer, formato)
 */
void fechaActual(char *buffer, const char *formato) // define una funcion que escribe la fecha actual en buffer usando el formato indicado // ejemplo: "%d/%m/%Y %H:%M"
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */

#if defined(_WIN32) || defined(__linux__)

    time_t t = time(NULL);                        // obtiene la marca de tiempo actual del sistema en segundos desde epoch // ejemplo: 1770000000
    strftime(buffer, 20, formato, localtime(&t)); // formatea la fecha actual dentro de buffer usando el patrón solicitado // ejemplo: "05/05/2026 14:30"

#elif defined(__XC)

    /* PIC16F no tiene reloj real.
       Aquí ponemos un placeholder.
       Luego puedes reemplazarlo con un RTC real. */

    const char *fakeDate = "01/01/2000 00:00"; // define una fecha simulada porque el PIC no dispone de reloj real // ejemplo: "01/01/2000 00:00"

    while (*fakeDate) // recorre cada carácter de la fecha simulada hasta encontrar el terminador nulo
    {
        *buffer++ = *fakeDate++; // copia un carácter de fakeDate a buffer y avanza ambos punteros // ejemplo: copia '0'
    }
    *buffer = '\0'; // cierra la cadena resultante con terminador nulo al final de la copia // ejemplo: fin de texto

#endif
}