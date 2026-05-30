#ifndef VAR_FUN_GG_H
#define VAR_FUN_GG_H

#define COLUMNAS 33
#define MAX_PRODUCTOS 1000

extern char *GG_archivos[][3];
extern char *GG_archivos_registros[][2];
extern char *GG_direccion_carpetas_base[];

typedef struct
{
    char *ruta;
    char *cabecera;
    char *extra;
} GG_ArchivoBaseNegocio;

typedef struct
{
    char *ruta;
    char *cabecera;
} GG_ArchivoInventarioPendiente;

extern GG_ArchivoBaseNegocio *GG_dir_nom_archivos;
extern GG_ArchivoInventarioPendiente *GG_direccion_hacer_inventarios;

/* nuevas variables globales aï¿½adidas */
extern int GG_indice_donde_comensar;
extern char *GG_cantidado_por_archivo;

extern char *GG_caracter_separacion[];
extern char *GG_caracter_separacion_2[];

/* separadores para funciones especï¿½ficas (13 elementos) */
extern char *GG_caracter_separacion_funciones_espesificas[];
extern char *GG_caracter_separacion_funciones_espesificas_2[];

extern char *GG_caracter_para_confirmacion_o_error[];
extern char *GG_caracter_para_confirmacion_o_error_2[];

extern char *GG_caracter_para_transferencia_entre_archivos[];
extern char *GG_caracter_para_transferencia_entre_archivos_2[];

extern char *GG_caracter_para_usar_como_enter_y_nuevo_mensaje[];
extern char *GG_caracter_para_usar_como_enter_y_nuevo_mensaje_2[];

extern char *GG_caracter_separacion_nom_parametro_de_valor[];
extern char *GG_caracter_separacion_nom_parametro_de_valor_2[];

extern char *GG_caracter_guardado_para_confirmacion[];

extern char *GG_id_programa;
extern char *GG_ultimo_retorno_estandar;

/*
 * Guarda el ultimo retorno textual estandar con formato:
 * codigo + separador + mensaje + separador + datos_extra.
 */
void establecer_ultimo_retorno_formateado(int codigo, int indice_capa, const char *nombre_funcion, const char *datos_extra);

/*
 * Devuelve el ultimo retorno textual estandar generado.
 */
const char *obtener_ultimo_retorno_formateado(void);

/*
 * En estos macros, la barra invertida \ al final de cada linea le dice al preprocesador
 * que la instruccion continua en la siguiente linea. Si se quita una de esas barras,
 * el macro se corta ahi mismo y lo de abajo queda como codigo suelto, generando errores.
 */
#define RETORNAR_MODELO_ESTANDAR(codigo)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    do                                                                                        /* inicia un bloque seguro para poder usar el macro como si fuera una sola instruccion */                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
    {                                                                                         /* abre el bloque local del macro para no contaminar variables externas; la barra final mantiene esta linea dentro del macro */                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          \
        int _retorno_tmp = (int)(codigo);                                                     /* convierte y guarda el codigo antes de retornarlo; la barra final une esta linea con la siguiente */                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   \
        establecer_ultimo_retorno_formateado(_retorno_tmp, 1, __func__, "otros datos extra"); /* guarda el retorno textual del modelo usando el nombre real de la funcion; la barra final evita que el macro termine antes de tiempo */                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
        return _retorno_tmp;                                                                  /* devuelve el mismo codigo int original sin cambiar la firma; la barra final hace que el cierre while siga perteneciendo al macro */                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    \
    } while (0) /* cierra el patron clasico de macro seguro */

#define RETORNAR_PROCESO_ESTANDAR(codigo)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              \
    do                                                                                        /* inicia un bloque seguro para encapsular el retorno estandar del proceso */                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            \
    {                                                                                         /* abre el alcance local del macro del proceso; la barra final mantiene el bloque unido */                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
        int _retorno_tmp = (int)(codigo);                                                     /* guarda el codigo que regresara la funcion; la barra final indica que el macro aun no termina */                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       \
        establecer_ultimo_retorno_formateado(_retorno_tmp, 2, __func__, "otros datos extra"); /* registra el retorno textual del proceso con el nombre de la funcion actual; la barra final conserva esta linea dentro del macro */                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    \
        return _retorno_tmp;                                                                  /* retorna el codigo int del proceso al llamador; la barra final une esta instruccion con el cierre while */                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             \
    } while (0) /* cierra el macro seguro */

#define ARCH_INVENTARIO 0

/* Estructura para filas de configuraciï¿½n de ventanas emergentes */
#define MAX_CAMPOS_CONFIG 5
#define MAX_LONGITUD_CAMPO 256

typedef struct
{
    char tipo[16]; /* "1", "2", "4" etc */
    char nombre[MAX_LONGITUD_CAMPO];
    char validacion[MAX_LONGITUD_CAMPO];
    char valor_default[MAX_LONGITUD_CAMPO];
    char tipo_dato[32]; /* "TEXTO", "ENTERO_DECIMAL", etc */
} ConfigField;

/* Estructura para variables string temporales */
#define MAX_VAR_STRING 7
extern char *GG_variables_string[MAX_VAR_STRING];

/* Declaraciones de arreglos de configuraciï¿½n */
extern const char *GG_ventana_datos_conf[][5];
extern const char *GG_ventana_emergente_productos[][5];
extern const char *GG_ventana_COSAS_NO_ESTABAN_INVENTARIO[][5];
extern const char *GG_ventana_provedor[][5];
extern const char *GG_ventana_APRENDICES_E[][5];
extern const char *GG_ventana_afiliados_unificados[][5];
extern const char *GG_ventana_niv_afiliados_unificado[][5];
extern const char *GG_ventana_SUCUR[][5];
extern const char *GG_ventana_reg_dia[][5];
extern const char *GG_ventana_reg_mes[][5];
extern const char *GG_ventana_reg_año[][5];
extern const char *GG_ventana_reg_total[][5];
extern const char *GG_ventana_reg_prod_dia[][5];
extern const char *GG_ventana_reg_prod_mes[][5];
extern const char *GG_ventana_reg_prod_año[][5];
extern const char *GG_ventana_reg_prod_total[][5];
extern const char *GG_ventana_IMPUESTOS[][5];
extern const char *GG_ventana_DEDUSIBLES[][5];
extern const char *GG_ventana_HERRAMIENTAS[][5];
extern const char *GG_trabajos_dia[][5];

/* Alias globales: usar G_* en el codigo y mantener GG_* como fuente real. */
#define G_archivos GG_archivos
#define G_archivos_registros GG_archivos_registros
#define G_indice_donde_comensar GG_indice_donde_comensar
#define G_cantidado_por_archivo GG_cantidado_por_archivo
#define G_caracter_separacion GG_caracter_separacion
#define G_caracter_separacion_2 GG_caracter_separacion_2
#define G_caracter_separacion_funciones_espesificas GG_caracter_separacion_funciones_espesificas
#define G_caracter_separacion_funciones_espesificas_2 GG_caracter_separacion_funciones_espesificas_2
#define G_caracter_para_confirmacion_o_error GG_caracter_para_confirmacion_o_error
#define G_caracter_para_confirmacion_o_error_2 GG_caracter_para_confirmacion_o_error_2
#define G_caracter_para_transferencia_entre_archivos GG_caracter_para_transferencia_entre_archivos
#define G_caracter_para_transferencia_entre_archivos_2 GG_caracter_para_transferencia_entre_archivos_2
#define G_caracter_para_usar_como_enter_y_nuevo_mensaje GG_caracter_para_usar_como_enter_y_nuevo_mensaje
#define G_caracter_para_usar_como_enter_y_nuevo_mensaje_2 GG_caracter_para_usar_como_enter_y_nuevo_mensaje_2
#define G_caracter_separacion_nom_parametro_de_valor GG_caracter_separacion_nom_parametro_de_valor
#define G_caracter_separacion_nom_parametro_de_valor_2 GG_caracter_separacion_nom_parametro_de_valor_2
#define G_caracter_guardado_para_confirmacion GG_caracter_guardado_para_confirmacion
#define G_id_programa GG_id_programa
#define G_variables_string GG_variables_string
#define G_ventana_datos_conf GG_ventana_datos_conf
#define G_ventana_emergente_productos GG_ventana_emergente_productos
#define G_ventana_COSAS_NO_ESTABAN_INVENTARIO GG_ventana_COSAS_NO_ESTABAN_INVENTARIO
#define G_ventana_provedor GG_ventana_provedor
#define G_ventana_APRENDICES_E GG_ventana_APRENDICES_E
#define G_ventana_afiliados_unificados GG_ventana_afiliados_unificados
#define G_ventana_niv_afiliados_unificado GG_ventana_niv_afiliados_unificado
#define G_ventana_SUCUR GG_ventana_SUCUR
#define G_ventana_reg_dia GG_ventana_reg_dia
#define G_ventana_reg_mes GG_ventana_reg_mes
#define G_ventana_reg_año GG_ventana_reg_año
#define G_ventana_reg_total GG_ventana_reg_total
#define G_ventana_reg_prod_dia GG_ventana_reg_prod_dia
#define G_ventana_reg_prod_mes GG_ventana_reg_prod_mes
#define G_ventana_reg_prod_año GG_ventana_reg_prod_año
#define G_ventana_reg_prod_total GG_ventana_reg_prod_total
#define G_ventana_IMPUESTOS GG_ventana_IMPUESTOS
#define G_ventana_DEDUSIBLES GG_ventana_DEDUSIBLES
#define G_ventana_HERRAMIENTAS GG_ventana_HERRAMIENTAS
#define G_trabajos_dia GG_trabajos_dia

/* Funciones para reinicializar estructuras */
/*
 * Uso: Ejecuta RecargarVentanaEmergenteDatosConfiguracion de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteDatosConfiguracion(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteDatosConfiguracion(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergenteProductos de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteProductos(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteProductos(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergente_Cosas_que_no_estaban de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergente_Cosas_que_no_estaban(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergente_Cosas_que_no_estaban(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergenteProvedor de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteProvedor(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteProvedor(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergenteAPRENDICES_E de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteAPRENDICES_E(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteAPRENDICES_E(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergenteAfiliados_unificados de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteAfiliados_unificados(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteAfiliados_unificados(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergente_niv_afiliados_unificado de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergente_niv_afiliados_unificado(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergente_niv_afiliados_unificado(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergenteSUCUR de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteSUCUR(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteSUCUR(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergenteRegDia de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteRegDia(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteRegDia(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergenteRegMes de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteRegMes(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteRegMes(const char *al_finalizar_que_borrar);
void RecargarVentanaEmergenteRegAño(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergenteRegTotal de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteRegTotal(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteRegTotal(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergenteReg_prod_Dia de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteReg_prod_Dia(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteReg_prod_Dia(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergenteReg_prod_Mes de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteReg_prod_Mes(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteReg_prod_Mes(const char *al_finalizar_que_borrar);
void RecargarVentanaEmergenteReg_prod_Año(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergenteReg_prod_total de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteReg_prod_total(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteReg_prod_total(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergenteImpuestos de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteImpuestos(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteImpuestos(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergenteDedusibles de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteDedusibles(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteDedusibles(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergente_HERRAMIENTAS de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergente_HERRAMIENTAS(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergente_HERRAMIENTAS(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarVentanaEmergente_TRABAJOS_DIA de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergente_TRABAJOS_DIA(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergente_TRABAJOS_DIA(const char *al_finalizar_que_borrar);
/*
 * Uso: Ejecuta RecargarArregloArchivos_dir_nom_archivos de forma segura.
 * Entrada ejemplo: RecargarArregloArchivos_dir_nom_archivos()
 */
void RecargarArregloArchivos_dir_nom_archivos(void);
/*
 * Uso: Ejecuta RecargarArregloDireccionInventarios de forma segura.
 * Entrada ejemplo: RecargarArregloDireccionInventarios()
 */
void RecargarArregloDireccionInventarios(void);

/* Funciï¿½n auxiliar para concatenar columnas */
/*
 * Uso: Ejecuta columnas_concatenadas de forma segura.
 * Entrada ejemplo: columnas_concatenadas(arreglo, filas, id_columna, caracter_separacion)
 */
char *columnas_concatenadas(const char *arreglo[][5], int filas, int id_columna, const char *caracter_separacion);

#endif