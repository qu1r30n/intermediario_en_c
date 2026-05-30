#ifndef OPERACIONES_COMPU_H
#define OPERACIONES_COMPU_H

/*
 * Uso: Ejecuta delay_ms de forma segura.
 * Entrada ejemplo: delay_ms(ms)
 */
void delay_ms(unsigned int ms);

/*
 * Uso: Ejecuta fechaActual de forma segura.
 * Entrada ejemplo: fechaActual(b, f)
 */
void fechaActual(char *b, const char *f);

/*
 * Uso: Ejecuta imprimirMensaje_para_depurar de forma segura.
 * Entrada ejemplo: imprimirMensaje_para_depurar(format, arg2)
 */
void imprimirMensaje_para_depurar(const char *format, ...);

#endif
