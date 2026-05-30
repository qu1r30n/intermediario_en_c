#ifndef PROCESOS_OPERACIONES_TEXTOS_H
#define PROCESOS_OPERACIONES_TEXTOS_H

/* LIBRERIAS USADAS EN ESTE ARCHIVO:
 * - stddef.h: Tipos base como size_t y NULL
 */
#include <stddef.h>

#define CONCAT_TEXTO 0
#define CONCAT_INT 1
#define CONCAT_FLOAT 2

/*
 * Uso: Ejecuta split de forma segura.
 * Entrada ejemplo: split(txt, sep, salida)
 */
int split(const char *txt, const char *sep, char ***salida);

/* libera el arreglo terminado en NULL (no necesita longitud) */
/*
 * Uso: Ejecuta free_split de forma segura.
 * Entrada ejemplo: free_split(a)
 */
void free_split(char **a);

/*
 * Uso: Ejecuta texto_a_float_seguro de forma segura.
 * Entrada ejemplo: texto_a_float_seguro(texto, var_a_retornar)
 */
int texto_a_float_seguro(const char *texto, float *var_a_retornar);

/*
 * Uso: Ejecuta texto_a_int_seguro de forma segura.
 * Entrada ejemplo: texto_a_int_seguro(texto, var_a_retornar)
 */
int texto_a_int_seguro(const char *texto, int *var_a_retornar);

/* Version tipo printf: formatea y concatena al destino. */
/*
 * Uso: Ejecuta concatenar_formato_separado_por_variable de forma segura.
 * Entrada ejemplo: concatenar_formato_separado_por_variable(destino, separador, formato, arg4)
 */
int concatenar_formato_separado_por_variable(char **destino, const char *separador, const char *formato, ...);
/*
 * Uso: Ejecuta concatenar_formato de forma segura.
 * Entrada ejemplo: concatenar_formato(destino, separador, formato, arg4)
 */
int concatenar_formato(char *destino, const char *separador, const char *formato, ...);

/* Construye un retorno estandarizado: codigo + separador + mensaje + separador + datos extra. */
/*
 * Uso: Ejecuta construir_retorno_estandar de forma segura.
 * Entrada ejemplo: construir_retorno_estandar(0, GG_caracter_para_confirmacion_o_error[0], "ok", "detalle")
 */
char *construir_retorno_estandar(int codigo, const char *separador, const char *mensaje, const char *datos_extra);

/* Retorna un texto formateado en heap. El caller debe liberarlo con free(). */
/*
 * Uso: Ejecuta variable_string de forma segura.
 * Entrada ejemplo: variable_string(format, arg2)
 */
char *variable_string(const char *format, ...);

/* =======================
FUNCIONES NUEVAS DEL C#
======================== */

/* Unir filas con car�cter separador, extrayendo columnas espec�ficas opcionalmente. */
/*
 * Uso: Ejecuta join_paresido_simple de forma segura.
 * Entrada ejemplo: join_paresido_simple(caracter_union_filas, texto, n_texto, columnas_extraer, caracter_union_columnas)
 */
char *join_paresido_simple(char caracter_union_filas, char **texto, int n_texto, const char *columnas_extraer, const char *caracter_union_columnas);

/* Unir arreglo en string con separador, quitando celdas del inicio o final. */
/*
 * Uso: Ejecuta joineada_paraesida_y_quitador_de_extremos de forma segura.
 * Entrada ejemplo: joineada_paraesida_y_quitador_de_extremos(data, restar_cuantas, restar_primera_celda)
 */
char *joineada_paraesida_y_quitador_de_extremos(const char *data, int restar_cuantas, int restar_primera_celda);

/* Igual a anterior pero omitiendo celdas NULL. */
/*
 * Uso: Ejecuta joineada_paraesida_SIN_NULOS_y_quitador_de_extremos de forma segura.
 * Entrada ejemplo: joineada_paraesida_SIN_NULOS_y_quitador_de_extremos(data, restar_cuantas, restar_primera_celda)
 */
char *joineada_paraesida_SIN_NULOS_y_quitador_de_extremos(const char *data, int restar_cuantas, int restar_primera_celda);

/* Quitar separador final duplicado de una string. */
/*
 * Uso: Ejecuta Trimend_paresido de forma segura.
 * Entrada ejemplo: Trimend_paresido(texto)
 */
char *Trimend_paresido(const char *texto);

/* Concatenar filas de archivo con separador. */
/*
 * Uso: Ejecuta concatenacion_filas_de_un_archivo de forma segura.
 * Entrada ejemplo: concatenacion_filas_de_un_archivo(ruta_archivo, poner_num_fila)
 */
char *concatenacion_filas_de_un_archivo(const char *ruta_archivo, int poner_num_fila);

/* Concatenar filas de arreglo. */
/*
 * Uso: Ejecuta concatenacion_filas_de_un_arreglo de forma segura.
 * Entrada ejemplo: concatenacion_filas_de_un_arreglo(arreglo, n_arreglo, poner_num_fila)
 */
char *concatenacion_filas_de_un_arreglo(char **arreglo, int n_arreglo, int poner_num_fila);

/* Concatenar arreglo 2D fila por fila. */
/*
 * Uso: Ejecuta concatenacion_filas_de_un_arreglo_bidimencional de forma segura.
 * Entrada ejemplo: concatenacion_filas_de_un_arreglo_bidimencional(arreglo_2d, filas, cols, poner_num_fila)
 */
char *concatenacion_filas_de_un_arreglo_bidimencional(const char **arreglo_2d, int filas, int cols, int poner_num_fila);

/* Concatenar dos strings con separador. */
/*
 * Uso: Ejecuta concatenacion_caracter_separacion de forma segura.
 * Entrada ejemplo: concatenacion_caracter_separacion(texto_actual, texto_agregar, separador)
 */
char *concatenacion_caracter_separacion(const char *texto_actual, const char *texto_agregar, const char *separador);

/* B�squeda en estructura profunda (nested) por columnas m�ltiples. */
/*
 * Uso: Ejecuta busqueda_profunda_string de forma segura.
 * Entrada ejemplo: busqueda_profunda_string(texto, columnas_recorrer, comparar)
 */
char *busqueda_profunda_string(const char *texto, const char *columnas_recorrer, const char *comparar);

/* B�squeda profunda retornando en formato especial final. */
/*
 * Uso: Ejecuta busqueda_profunda_comparacion_final_string de forma segura.
 * Entrada ejemplo: busqueda_profunda_comparacion_final_string(texto, columnas_recorrer, comparar)
 */
char *busqueda_profunda_comparacion_final_string(const char *texto, const char *columnas_recorrer, const char *comparar);

/* B�squeda profunda con YY (m�ltiples comparaciones). */
/*
 * Uso: Ejecuta busqueda_con_YY_profunda_texto_id_archivo de forma segura.
 * Entrada ejemplo: busqueda_con_YY_profunda_texto_id_archivo(texto, columnas_recorrer, comparaciones)
 */
char *busqueda_con_YY_profunda_texto_id_archivo(const char *texto, const char *columnas_recorrer, const char *comparaciones);

/* Edici�n/incremento recursiva en estructura profunda. */
/*
 * Uso: Ejecuta editar_incr_string_funcion_recursiva de forma segura.
 * Entrada ejemplo: editar_incr_string_funcion_recursiva(texto, columnas_recorrer, info_sustituir, edit_0_increm_1)
 */
char *editar_incr_string_funcion_recursiva(const char *texto, const char *columnas_recorrer, const char *info_sustituir, const char *edit_0_increm_1);

/* Edici�n profunda m�ltiple con comparaci�n final. */
/*
 * Uso: Ejecuta editar_inc_agregar_edicion_profunda_multiple_comparacion_final_string de forma segura.
 * Entrada ejemplo: editar_inc_agregar_edicion_profunda_multiple_comparacion_final_string(texto, indices_editar, info_editar, comparacion, edit_0_increm_1)
 */
char *editar_inc_agregar_edicion_profunda_multiple_comparacion_final_string(const char *texto, const char *indices_editar, const char *info_editar, const char *comparacion, const char *edit_0_increm_1);

/* Edici�n profunda m�ltiple versi�n ARR_FUN (wrapper). */
/*
 * Uso: Ejecuta ARR_FUN_SOLO_TEXTO_editar_inc_agregar_edicion_profunda_multiple de forma segura.
 * Entrada ejemplo: ARR_FUN_SOLO_TEXTO_editar_inc_agregar_edicion_profunda_multiple(datos)
 */
char *ARR_FUN_SOLO_TEXTO_editar_inc_agregar_edicion_profunda_multiple(const char *datos);

/* Edici�n profunda m�ltiple con m�ltiples chequeos. */
/*
 * Uso: Ejecuta editar_inc_agregar_edicion_profunda_multiple_comparacion_MULTIPLE_A_CHECAR de forma segura.
 * Entrada ejemplo: editar_inc_agregar_edicion_profunda_multiple_comparacion_MULTIPLE_A_CHECAR(texto, indices_editar, comparacion_con_edicion, edit_0_increm_1)
 */
char *editar_inc_agregar_edicion_profunda_multiple_comparacion_MULTIPLE_A_CHECAR(const char *texto, const char *indices_editar, const char *comparacion_con_edicion, const char *edit_0_increm_1);

/* Edici�n profunda m�ltiple simple. */
/*
 * Uso: Ejecuta editar_inc_edicion_profunda_multiple_string de forma segura.
 * Entrada ejemplo: editar_inc_edicion_profunda_multiple_string(texto, indices_editar, info_editar, edit_0_increm_1)
 */
char *editar_inc_edicion_profunda_multiple_string(const char *texto, const char *indices_editar, const char *info_editar, const char *edit_0_increm_1);

/* Edici�n profunda m�ltiple al final. */
/*
 * Uso: Ejecuta editar_inc_edicion_profunda_multiple_AL_FINAL_string de forma segura.
 * Entrada ejemplo: editar_inc_edicion_profunda_multiple_AL_FINAL_string(texto, indices_editar, info_editar, edit_0_increm_1)
 */
char *editar_inc_edicion_profunda_multiple_AL_FINAL_string(const char *texto, const char *indices_editar, const char *info_editar, const char *edit_0_increm_1);

/* Recorrer y reemplazar caracteres de separaci�n izq/dcha. */
/*
 * Uso: Ejecuta recorrer_caracter_separacion de forma segura.
 * Entrada ejemplo: recorrer_caracter_separacion(contenidoFila, izquierda_o_derecha, numero_veses)
 */
char *recorrer_caracter_separacion(const char *contenidoFila, const char *izquierda_o_derecha, int numero_veses);

/* Recorrer caracteres de separaci�n para funciones espec�ficas. */
/*
 * Uso: Ejecuta recorrer_caracter_separacion_funciones_espesificas de forma segura.
 * Entrada ejemplo: recorrer_caracter_separacion_funciones_espesificas(contenidoFila, izquierda_o_derecha, numero_veses)
 */
char *recorrer_caracter_separacion_funciones_espesificas(const char *contenidoFila, const char *izquierda_o_derecha, int numero_veses);

/* Extraer carpeta, nombre y extensi�n de ruta. */
/*
 * Uso: Ejecuta extraer_separado_carpetas_nombreArchivo_extencion de forma segura.
 * Entrada ejemplo: extraer_separado_carpetas_nombreArchivo_extencion(direccion_archivo)
 */
char **extraer_separado_carpetas_nombreArchivo_extencion(const char *direccion_archivo);

/* Desfragmentar ruta y devolver por salida: directorios, nombre de archivo y extencion. */
/*
 * Uso: Ejecuta desfragmentar_direccion de forma segura.
 * Entrada ejemplo: desfragmentar_direccion(direccion, retorna_directorios, retorna_nom_arch, retorna_extencion)
 */
int desfragmentar_direccion(const char *direccion, char **retorna_directorios, char **retorna_nom_arch, char **retorna_extencion);

/* Generar folio (ID �nico con timestamp). */
/*
 * Uso: Ejecuta generar_folio de forma segura.
 * Entrada ejemplo: generar_folio(formato_fecha_hora)
 */
char *generar_folio(const char *formato_fecha_hora);

/* Reemplazar m�ltiples caracteres seg�n arreglos paralelos. */
/*
 * Uso: Ejecuta ReemplazarCaracteres_de_texto_arreglo de forma segura.
 * Entrada ejemplo: ReemplazarCaracteres_de_texto_arreglo(info, caracteres_sep, n_sep, caracteres_sustitucion)
 */
char *ReemplazarCaracteres_de_texto_arreglo(const char *info, char **caracteres_sep, int n_sep, char **caracteres_sustitucion);

/* Reemplazar un separador por una sustitucion recibiendo solo strings. */
/*
 * Uso: Reemplaza todas las ocurrencias de sep en info por sust.
 * Entrada ejemplo: ReemplazarCaracteres_de_texto_string(info, "|", "~")
 */
char *ReemplazarCaracteres_de_texto_string(const char *info, const char *sep, const char *sust);

#endif // PROCESOS_OPERACIONES_TEXTOS_H
