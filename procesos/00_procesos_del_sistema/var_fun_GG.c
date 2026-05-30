/* LIBRERIAS USADAS EN ESTE ARCHIVO:
 * - stdio.h: Entrada y salida estandar (printf, fopen, etc.)
 * - stdlib.h: Memoria dinamica, conversiones y utilidades generales
 * - string.h: Manejo de cadenas y memoria (strlen, strcmp, memcpy)
 * - ../../cabeceras/cabeceras_procesos/00_cabeceras_del_sistema/var_fun_GG.h: Dependencia interna del proyecto
 */
#include "../../cabeceras/cabeceras_procesos/00_cabeceras_del_sistema/var_fun_GG.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char *GG_ultimo_retorno_estandar = NULL;            // ultimo retorno textual estandar generado por modelo/proceso/conmutador; ejemplo: "0╠todo salio bien en este modelo llamado x╠otros datos extra"
static char *GG_acumulador_retorno_procesos = NULL; // acumula respuestas de procesos para incrustarlas en el retorno del modelo

static char *duplicar_texto_retorno(const char *texto)
{
    if (texto == NULL) // corta de inmediato si no hay texto que copiar
    {
        return NULL; // retorna nulo para indicar que no pudo generarse una copia valida
    }

    size_t largo = strlen(texto) + 1;    // calcula el largo total incluyendo el terminador nulo
    char *copia = (char *)malloc(largo); // reserva memoria dinamica para la copia del texto
    if (copia == NULL)
    {
        return NULL; // retorna nulo si la reserva de memoria fallo
    }

    memcpy(copia, texto, largo); // copia el contenido completo del texto original a la nueva memoria
    return copia;                // entrega la copia al llamador
}

static void acumular_texto_con_separador(char **acumulador, const char *nuevo_texto, const char *separador)
{
    if (acumulador == NULL || nuevo_texto == NULL || nuevo_texto[0] == '\0') // valida punteros y evita agregar texto vacio
    {
        return; // abandona la acumulacion si la entrada no sirve
    }

    if (*acumulador == NULL) // si aun no existe acumulado, el nuevo texto sera la base
    {
        *acumulador = duplicar_texto_retorno(nuevo_texto); // crea una copia independiente del primer retorno recibido
        return;                                            // termina porque ya quedo inicializado el acumulador
    }

    size_t largo_actual = strlen(*acumulador);                                            // mide cuanto texto hay actualmente acumulado
    size_t largo_nuevo = strlen(nuevo_texto);                                             // mide el nuevo texto que sera agregado
    size_t largo_sep = (separador != NULL) ? strlen(separador) : 0;                       // mide el separador o usa cero si no hay
    char *tmp = (char *)realloc(*acumulador, largo_actual + largo_sep + largo_nuevo + 1); // expande el buffer para texto actual, separador, nuevo texto y terminador

    if (tmp == NULL)
    {
        return; // si falla realloc se conserva el acumulador anterior y se sale sin romperlo
    }

    *acumulador = tmp; // actualiza el puntero del acumulador al nuevo bloque valido
    if (largo_sep > 0) // agrega separador solo cuando exista uno configurado
    {
        memcpy(*acumulador + largo_actual, separador, largo_sep); // copia el separador al final del texto actual
        largo_actual += largo_sep;                                // avanza la posicion de escritura despues del separador
    }

    memcpy(*acumulador + largo_actual, nuevo_texto, largo_nuevo + 1); // pega el nuevo texto incluyendo el terminador final
}

void establecer_ultimo_retorno_formateado(int codigo, int indice_capa, const char *nombre_funcion, const char *datos_extra)
{
    const char *nombre_seguro = (nombre_funcion != NULL) ? nombre_funcion : "funcion_desconocida"; // garantiza un nombre valido aunque el macro no lo mande
    const char *extra_seguro = (datos_extra != NULL) ? datos_extra : "otros datos extra";          // garantiza un bloque extra por defecto
    const char *separador = GG_caracter_para_confirmacion_o_error[0];                              // inicia con el separador del conmutador
    const char *plantilla_ok = "todo salio bien en el conmutador";                                 // mensaje base para casos exitosos
    const char *plantilla_error = "error en el conmutador";                                        // mensaje base para casos de error
    char mensaje[256];                                                                             // buffer temporal donde se arma el mensaje legible de la capa
    int escritos_mensaje = 0;                                                                      // guarda cuantos caracteres escribio snprintf en mensaje
    int necesarios = 0;                                                                            // guarda el tamaño exacto necesario para reservar el retorno final
    char *nuevo = NULL;                                                                            // recibira el retorno completo de modelo o conmutador
    char *retorno_proceso = NULL;                                                                  // recibira el retorno atomico del proceso

    if (indice_capa == 1) // configura textos y separador de la capa modelo
    {
        separador = GG_caracter_para_confirmacion_o_error[1];                                                                 // cambia al separador especifico del modelo
        plantilla_ok = "todo salio bien en este modelo llamado %s";                                                           // define plantilla de exito del modelo
        plantilla_error = "error en este modelo llamado %s";                                                                  // define plantilla de error del modelo
        escritos_mensaje = snprintf(mensaje, sizeof(mensaje), (codigo >= 0) ? plantilla_ok : plantilla_error, nombre_seguro); // construye el mensaje usando el nombre de la funcion
    }
    else if (indice_capa == 2) // configura textos y separador de la capa proceso
    {
        separador = GG_caracter_para_confirmacion_o_error[4];                                                                 // usa el separador atomico codigo⛟mensaje del proceso
        plantilla_ok = "todo salio bien en este proseso llamado %s";                                                          // plantilla de exito del proceso
        plantilla_error = "error en este proseso llamado %s";                                                                 // plantilla de error del proceso
        escritos_mensaje = snprintf(mensaje, sizeof(mensaje), (codigo >= 0) ? plantilla_ok : plantilla_error, nombre_seguro); // construye el mensaje final del proceso
    }
    else // deja configurado el caso del conmutador u otra capa base
    {
        separador = GG_caracter_para_confirmacion_o_error[0];                                                        // reafirma el separador del conmutador
        escritos_mensaje = snprintf(mensaje, sizeof(mensaje), "%s", (codigo >= 0) ? plantilla_ok : plantilla_error); // copia el mensaje fijo del conmutador
    }

    if (escritos_mensaje < 0) // detecta error al construir el texto base del mensaje
    {
        free(GG_ultimo_retorno_estandar);                                                                       // libera cualquier retorno previo antes de reemplazarlo
        GG_ultimo_retorno_estandar = duplicar_texto_retorno("-1╣error al construir mensaje╣otros datos extra"); // guarda un retorno de falla controlada
        return;                                                                                                 // termina porque no puede continuar sin mensaje base
    }

    if (indice_capa == 2) // ruta especial: el proceso produce retorno atomico y se acumula para el modelo
    {
        necesarios = snprintf(NULL, 0, "%d%s%s", codigo, separador, mensaje); // calcula el tamaño exacto de codigo⛟mensaje
        if (necesarios < 0)
        {
            free(GG_ultimo_retorno_estandar);                                                                      // libera el retorno previo antes de dejar el error actual
            GG_ultimo_retorno_estandar = duplicar_texto_retorno("-1╣error al calcular retorno╣otros datos extra"); // registra un error controlado de tamaño
            return;                                                                                                // sale porque no puede reservar memoria correctamente
        }

        retorno_proceso = (char *)malloc((size_t)necesarios + 1); // reserva memoria exacta para el retorno del proceso
        if (retorno_proceso == NULL)
        {
            free(GG_ultimo_retorno_estandar);                                                             // libera el retorno anterior antes de reportar falta de memoria
            GG_ultimo_retorno_estandar = duplicar_texto_retorno("-1╣error de memoria╣otros datos extra"); // deja un retorno textual claro de fallo de memoria
            return;                                                                                       // termina porque no existe buffer donde escribir
        }

        snprintf(retorno_proceso, (size_t)necesarios + 1, "%d%s%s", codigo, separador, mensaje);                                  // escribe el retorno atomico del proceso en memoria reservada
        acumular_texto_con_separador(&GG_acumulador_retorno_procesos, retorno_proceso, GG_caracter_para_confirmacion_o_error[3]); // lo agrega al acumulador de procesos separado por ⛔

        free(GG_ultimo_retorno_estandar);                                     // libera el ultimo retorno global antes de sustituirlo
        GG_ultimo_retorno_estandar = duplicar_texto_retorno(retorno_proceso); // guarda el proceso actual como ultimo retorno observable
        free(retorno_proceso);                                                // libera el buffer temporal del proceso porque ya fue duplicado
        return;                                                               // termina aqui porque la capa proceso ya quedo registrada
    }

    if (indice_capa == 1 && GG_acumulador_retorno_procesos != NULL && GG_acumulador_retorno_procesos[0] != '\0') // si el modelo ejecuto procesos, reemplaza su extra por esa cadena acumulada
    {
        extra_seguro = GG_acumulador_retorno_procesos; // hace que el retorno del modelo incruste todos los procesos ejecutados
    }

    necesarios = snprintf(NULL, 0, "%d%s%s%s%s", codigo, separador, mensaje, separador, extra_seguro); // calcula el tamaño para el retorno completo de modelo o conmutador
    if (necesarios < 0)
    {
        free(GG_ultimo_retorno_estandar);                                                                      // libera el contenido previo antes de reportar el error nuevo
        GG_ultimo_retorno_estandar = duplicar_texto_retorno("-1╣error al calcular retorno╣otros datos extra"); // deja un retorno de error por calculo fallido
        return;                                                                                                // aborta porque no se puede reservar el bloque exacto
    }

    nuevo = (char *)malloc((size_t)necesarios + 1); // reserva memoria para el retorno final completo
    if (nuevo == NULL)
    {
        free(GG_ultimo_retorno_estandar);                                                             // limpia el retorno anterior antes de registrar la falla actual
        GG_ultimo_retorno_estandar = duplicar_texto_retorno("-1╣error de memoria╣otros datos extra"); // deja un retorno textual de error de memoria
        return;                                                                                       // termina porque no hay buffer disponible
    }

    snprintf(nuevo, (size_t)necesarios + 1, "%d%s%s%s%s", codigo, separador, mensaje, separador, extra_seguro); // escribe el retorno completo con su separador de capa y datos extra

    free(GG_ultimo_retorno_estandar);   // libera el ultimo retorno global antes de reemplazarlo por el nuevo
    GG_ultimo_retorno_estandar = nuevo; // actualiza el puntero global con el retorno ya armado

    if (indice_capa == 1) // al terminar el modelo se debe limpiar el acumulado de procesos para el siguiente comando
    {
        free(GG_acumulador_retorno_procesos);  // libera la cadena acumulada de procesos usada por este modelo
        GG_acumulador_retorno_procesos = NULL; // deja el acumulador listo y vacio para la siguiente ejecucion
    }
}

const char *obtener_ultimo_retorno_formateado(void) { return GG_ultimo_retorno_estandar; }

int GG_indice_donde_comensar = 1; // índice de inicio para operaciones de listado; ejemplo: 1

char *GG_cantidado_por_archivo = "100"; // cantidad máxima de registros por archivo; ejemplo: "100"

char *GG_caracter_separacion[] = {"|", "°", "¬", "╦", "╔"};   // separadores de columna para archivos de datos; ejemplo: "|"
char *GG_caracter_separacion_2[] = {"⚭", "⚮", "⚯", "⚰", "⚱"}; // separadores alternativos de columna (set 2); ejemplo: "⚭"

char *GG_caracter_separacion_funciones_espesificas[] = {"~", "§", "¶", "╬", "╝", "╩", "║", "╗", "┐", "└", "┬", "├", "┼"};   // separadores para parsing de comandos del conmutador; ejemplo: "~"
char *GG_caracter_separacion_funciones_espesificas_2[] = {"⚲", "⚳", "⚴", "⚵", "⚶", "⚷", "⚸", "⚺", "⚻", "⚼", "⚿", "⛊", "⛋"}; // separadores alternativos para parsing de comandos (set 2); ejemplo: "⚲"

char *GG_caracter_para_confirmacion_o_error[] = {"╣", "╠", "⛐", "⛔", "⛟"};   // caracteres para señalizar confirmación o error en comunicación; ejemplo: "╣"
char *GG_caracter_para_confirmacion_o_error_2[] = {"⛑", "⛒", "⛠", "⛡", "⛎"}; // caracteres alternativos para confirmación o error (set 2); ejemplo: "⛑"

char *GG_caracter_para_transferencia_entre_archivos[] = {"■", "┴", "¤"};   // separadores para transferencia de datos entre archivos; ejemplo: "■"
char *GG_caracter_para_transferencia_entre_archivos_2[] = {"⛕", "⛘", "⛍"}; // separadores alternativos para transferencia entre archivos (set 2); ejemplo: "⛕"

char *GG_caracter_para_usar_como_enter_y_nuevo_mensaje[] = {"•", "∆"};   // caracteres que simulan salto de línea y nuevo mensaje; ejemplo: "•"
char *GG_caracter_para_usar_como_enter_y_nuevo_mensaje_2[] = {"⛙", "⛚"}; // caracteres alternativos para enter y nuevo mensaje (set 2); ejemplo: "⛙"

char *GG_caracter_separacion_nom_parametro_de_valor[] = {"⊓", "⊔","⛪","⛩"};   // separadores entre nombre de parámetro y su valor; ejemplo: "⊓"
char *GG_caracter_separacion_nom_parametro_de_valor_2[] = {"⊑", "⊒","⛫","⛬"}; // separadores alternativos entre nombre de parámetro y valor (set 2); ejemplo: "⊑"

char *GG_caracter_guardado_para_confirmacion[] = {"⛞", "⛝"}; // caracteres reservados para confirmación de guardado; ejemplo: "⛞"

char *GG_id_programa = "SISTEMA_QU1R30N"; // identificador único del programa; ejemplo: "SISTEMA_QU1R30N"

#ifdef _WIN32
char *GG_archivos[][3] = {{"espacios\\", "ID|Usuario|Contraseña|Directorio_Archivo_permisos_usuarios|Nivel", "archivo_espacios.txt"},
                          {"conexion_arc\\",
                           "ID_DESTINO■ID_ORIGEN┴COMANDO┴INFORMACION_ESPEJO_NO_SE_MODIFICA", // la informacion espejo regresa esa misma informacion tal y como esta
                           "archivo_entrada.txt"},
                          {"conexion_arc\\",
                           "ID_DESTINO■ID_ORIGEN┴COMANDO┴INFORMACION_ESPEJO_NO_SE_MODIFICA", // la informacion espejo regresa esa misma informacion tal y como esta
                           "archivo_salida.txt"},
                          {"conexion_arc\\", 
                            NULL, 
                            "banderas.txt"},
                          {"conexion_arc\\",
                             "ID_DESTINO■ID_ORIGEN┴COMANDO┴INFORMACION_ESPEJO_NO_SE_MODIFICA",
                             "errores_de_com.txt"},
                          {NULL, NULL, NULL}};
#else
char *GG_archivos[][3] = {{"espacios/", "ID|Usuario|Contraseña|Directorio_Archivo_permisos_usuarios|Nivel", "archivo_espacios.txt"},
                          {"conexion_arc/",
                           "ID_DESTINO■ID_ORIGEN┴COMANDO┴INFORMACION_ESPEJO_NO_SE_MODIFICA", // la informacion espejo regresa esa misma informacion tal y como esta
                           "archivo_entrada.txt"},
                          {"conexion_arc/",
                           "ID_DESTINO■ID_ORIGEN┴COMANDO┴INFORMACION_ESPEJO_NO_SE_MODIFICA", // la informacion espejo regresa esa misma informacion tal y como esta
                           "archivo_salida.txt"},
                          {"conexion_arc/", NULL, "banderas.txt"},
                          {"conexion_arc/", "ID_DESTINO■ID_ORIGEN┴COMANDO┴INFORMACION_ESPEJO_NO_SE_MODIFICA", "errores_de_com.txt"},
                          {NULL, NULL, NULL}};
#endif

char *GG_archivos_registros[][2] = {{"registros_ventas.txt", ""}, {"registros_compras.txt", ""}, {NULL, NULL}};

/* ===== VARIABLES STRING GLOBALES ===== */

char *GG_variables_string[MAX_VAR_STRING] = {
    "", /* [0] codbar */
    "", /* [1] prov_anterior */
    "", /* [2] provedores_txt */
    "", /* [3] impuesto anterior */
    "", /* [4] impuestos_txt */
    "", /* [5] tipo_medida_producto_anterior */
    ""  /* [6] tipo_medida_producto_txt */
};

/* ===== TABLAS DE CONFIGURACIÓN VENTANAS ===== */

/* Ventana: Datos de Configuración (2 campos) */
const char *GG_ventana_datos_conf[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "dato_de_configuracion", "", "", "TEXTO"},
    {"2", "descripcion_de_configuracion", "", "", "TEXTO"},
    {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Productos (33 campos) */
const char *GG_ventana_emergente_productos[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */

    {"2", "_00_ID", "", "-1", "ENTERO_DECIMAL"},
    {"1", "_01_PRODUCTO", "", "NOSE", "TEXTO"},
    {"1", "_02_CONTENIDO", "0°SOLO_NUMEROS", "-0", "ENTERO_DECIMAL"},
    {"4", "_03_TIPO_MEDIDA", "NOSE°TODAS_MAYUSCULAS", "NOSE", "TEXTO"},
    {"1", "_04_PRECIO_VENTA", "0°SOLO_NUMEROS", "-0", "ENTERO_DECIMAL"},
    {"2", "_05_COD_BARRAS", "", "NOSE", "TEXTO"},
    {"1", "_06_CANTIDAD", "1°SOLO_NUMEROS", "-0", "ENTERO_DECIMAL"},
    {"1", "_07_COSTO_COMP", "0°SOLO_NUMEROS", "-0", "ENTERO_DECIMAL"},
    {"4", "_08_PROVEDOR", "NOSE°TODAS_MAYUSCULAS", "NOSE¬0", "TEXTO"},
    {"4", "_09_GRUPO", "PRODUCTO_PIEZA", "PRODUCTO_PIEZA", "TEXTO"},
    {"1", "_10_CANT_X_PAQUET", "1°SOLO_NUMEROS", "-0", "ENTERO_DECIMAL"},
    {"4", "_11_ES_PAQUETE", "INDIVIDUAL", "INDIVIDUAL", "TEXTO"},
    {"1", "_12_CODBAR_PAQUETE_E_ID", "", "NOSE_2", "TEXTO"},
    {"1", "_13_COD_BAR_INDIVIDUAL_ES_PAQ_E_ID", "", "NOSE_3¬1", "TEXTO"},
    {"1", "_14_LIGAR_PROD_SAB", "", "NOSE", "TEXTO"},
    {"1", "_15_IMPUESTOS", "REYENO_TEXTBOX_VENTANA_IMPU", "NOSE", "TEXTO"},
    {"1", "_16_INGREDIENTES", "NO_VISIBLE¬PRODUCTO_ELABORADO", "NOSE", "TEXTO"},
    {"1", "_17_CADUCIDAD", "0°SOLO_NUMEROS", "-0", "TEXTO"},
    {"1", "_18_ULTIMO_MOV", "0°SOLO_NUMEROS", "-0", "TEXTO"},
    {"1", "_19_SUCUR_VENT", "", "NOSE¬0", "TEXTO"},
    {"1", "_20_CLAF_PROD", "", "-0", "ENTERO_DECIMAL"},
    {"1", "_21_DIR_IMG_INTER", "", "NOSE", "TEXTO"},
    {"1", "_22_DIR_IMG_COMP", "", "NOSE", "TEXTO"},
    {"1", "_23_INFO_EXTRA", "", "NOSE", "TEXTO"},
    {"1", "_24_PROCESO_CREAR", "NO_VISIBLE", "NOSE", "TEXTO"},
    {"1", "_25_DIR_VID_PROC_CREAR", "", "NOSE", "TEXTO"},
    {"1", "_26_TIEMPO_FABRICACION", "0", "-0", "ENTERO_DECIMAL"},
    {"2", "_27_INDICES_DIA_REGISTRO_PRODUC_VENDIDO", "", "0", "TEXTO"},
    {"2", "_28_INDICES_MES_REGISTRO_PRODUC_VENDIDO", "", "0", "TEXTO"},
    {"2", "_29_INDICES_AÑO_REGISTRO_PRODUC_VENDIDO", "", "0", "TEXTO"},
    {"2", "_30_ULTIMA_VENTA", "", "", "TEXTO"},
    {"2", "_31_INDICES_TOTAL_REGISTRO_PRODUC_VENDIDO", "", "0", "TEXTO"},
    {"2", "_32_NO_PONER_NADA", "", "", "TEXTO"},
    {NULL, NULL, NULL, NULL, NULL}};
/* Ventana: Cosas No Estaban en Inventario (3 campos) */
const char *GG_ventana_COSAS_NO_ESTABAN_INVENTARIO[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID", "", "-1", "ENTERO_DECIMAL"},
    {"1", "_01_COD_BAR", "", "NOSE", "TEXTO"},
    {"1", "_02_NOMBRE", "", "NOSE", "TEXTO"},
    {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Proveedores (24 campos) */
const char *GG_ventana_provedor[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID_EMPRESA", "", "-0", "TEXTO"},
    {"1", "_01_NOMBRE_EMPRESA", "", "NOSE", "TEXTO"},
    {"1", "_02_NOMBRE_ENCARGADO", "", "NOSE", "TEXTO"},
    {"1", "_03_DIRECCIÓN_EMPRESA", "", "NOSE", "TEXTO"},
    {"1", "_04_CIUDAD_EMPRESA", "", "NOSE", "TEXTO"},
    {"1", "_05_ESTADO_EMPRESA", "", "NOSE", "TEXTO"},
    {"1", "_06_CÓDIGO_POSTAL", "", "NOSE", "TEXTO"},
    {"1", "_07_PAÍS", "", "NOSE", "TEXTO"},
    {"1", "_08_CORREO_ELECTRÓNICO", "", "NOSE", "TEXTO"},
    {"1", "_09_TELÉFONO_ENCARGADO", "", "NOSE", "TEXTO"},
    {"1", "_10_TELEFONO_EMPRESA", "", "NOSE", "TEXTO"},
    {"1", "_11_TIPO_DE_PROVEEDOR", "", "NOSE", "TEXTO"},
    {"1", "_12_PRODUCTOS_SERVICIOS_SUMINISTRADOS", "", "NOSE", "TEXTO"},
    {"1", "_13_CUENTA_BANCO", "", "NOSE", "TEXTO"},
    {"1", "_14_UBICACIÓN_(GPS)", "", "-0", "TEXTO"},
    {"1", "_15_NOTAS", "", "NOSE", "TEXTO"},
    {"1", "_16_RECORDATORIO", "", "", "TEXTO"},
    {"1", "_17_ACTIVO_O_NO_ACTIVO", "", "ACTIVO", "TEXTO"},
    {"1", "_18_CALIFICACIÓN", "", "0", "TEXTO"},
    {"1", "_19_COMENTARIOS", "", "NOSE", "TEXTO"},
    {"1", "_20_SUCURSALES_QUE_LE_COMPRAN", "", "NOSE", "TEXTO"},
    {"1", "_21_DINERO_A_COMPRARLE", "0°SOLO_NUMEROS", "0", "TEXTO"},
    {"1", "_22_DIAS_DE_PREVENTA", "", "NOSE¬NOSE", "TEXTO"},
    {"1", "_23_DIAS_DE_ENTREGA", "", "NOSE¬NOSE", "TEXTO"},
    {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Aprendices (32 campos) */
const char *GG_ventana_APRENDICES_E[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID", "", "-0", "TEXTO"},
    {"1", "_01_NOMBRE", "", "NOSE", "TEXTO"},
    {"1", "_02_APELLIDO_PATERNO", "", "NOSE", "TEXTO"},
    {"1", "_03_APELLIDO_MATERNO", "", "NOSE", "TEXTO"},
    {"1", "_04_FECHA_DE_NACIMIENTO", "", "NOSE", "TEXTO"},
    {"1", "_05_GÉNERO", "", "NOSE", "TEXTO"},
    {"1", "_06_DIRECCIÓN", "", "NOSE", "TEXTO"},
    {"1", "_07_CIUDAD", "", "NOSE", "TEXTO"},
    {"1", "_08_ESTADO_PROVINCIA", "", "NOSE", "TEXTO"},
    {"1", "_09_CÓDIGO_POSTAL", "", "NOSE", "TEXTO"},
    {"1", "_10_PAÍS", "", "NOSE", "TEXTO"},
    {"1", "_11_CORREO_ELECTRÓNICO", "", "NOSE", "TEXTO"},
    {"1", "_12_TELÉFONO", "", "NOSE", "TEXTO"},
    {"1", "_13_FECHA_DE_INGRESO", "", "NOSE", "TEXTO"},
    {"1", "_14_SUELDO", "", "NOSE", "TEXTO"},
    {"1", "_15_CARGO", "", "NOSE", "TEXTO"},
    {"1", "_16_ESTADO_DE_CURS_APRENDIS_E", "", "NOSE", "TEXTO"},
    {"1", "_17_SUPERVISOR", "", "NOSE", "TEXTO"},
    {"1", "_18_NOTAS", "", "", "TEXTO"},
    {"1", "_19_AFILIADO", "", "NOSE", "TEXTO"},
    {"1", "_20_FECHA_DE_TERMINACIÓN", "0°SOLO_NUMEROS", "0", "TEXTO"},
    {"1", "_21_MOTIVO_DE_TERMINACIÓN", "", "NOSE", "TEXTO"},
    {"1", "_22_HORAS_TRABAJADAS", "", "NOSE", "TEXTO"},
    {"1", "_23_EVALUACIONES_DE_DESEMPEÑO", "", "NOSE", "TEXTO"},
    {"1", "_24_HABILIDADES_Y_CERTIFICACIONES", "", "NOSE", "TEXTO"},
    {"1", "_25_IDIOMAS", "", "NOSE", "TEXTO"},
    {"1", "_26_FECHA_DE_ÚLTIMA_PROMOCIÓN", "", "NOSE", "TEXTO"},
    {"1", "_27_ID_DEPARTAMENTO", "", "NOSE", "TEXTO"},
    {"1", "_28_HISTORIAL_DE_CAPACITACIÓN", "", "NOSE", "TEXTO"},
    {"1", "_29_ÚLTIMO_AUMENTO_DE_SALARIO", "", "NOSE", "TEXTO"},
    {"1", "_30_TIPO_EMPLEADO", "", "NOSE", "TEXTO"},
    {"1", "_31_RANGO_CALIF", "", "-0", "TEXTO"},
    {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Afiliados Unificados (8 campos) */
const char *GG_ventana_afiliados_unificados[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"1", "_00_ID_USUARIO", "", "0", "TEXTO"}, {"1", "_01_IDP", "", "0╔0╦0¬0╔1╦1", "TEXTO"}, {"1", "_02_PUNTOS_D_R", "", "0╦0¬0╦0", "TEXTO"}, {"1", "_03_PUNTOS_D_R_TOTALES", "", "0", "TEXTO"}, {"1", "_04_DATOS", "", "NOSE", "TEXTO"}, {"1", "_05_NIVEL", "", "0", "TEXTO"}, {"1", "_06_ID_HORIZONTAL", "", "0", "TEXTO"}, {"1", "_07_TIPO_AFILIADO", "", "NOSE", "TEXTO"}, {NULL, NULL, NULL, NULL, NULL}};
/* Ventana: Niveles Afiliados Unificado (4 campos) */
const char *GG_ventana_niv_afiliados_unificado[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID", "", "-1", "ENTERO_DECIMAL"},
    {"1", "_01_NIVEL", "", "0", "TEXTO"},
    {"1", "_02_ID_HORIZONTAL", "", "0", "TEXTO"},
    {"1", "_03_VACIOS", "", "", "TEXTO"},
    {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Sucursales (20 campos) */
const char *GG_ventana_SUCUR[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID", "", "-1", "ENTERO_DECIMAL"},
    {"1", "_01_NOM_ID_SUCUR", "", "-0", "TEXTO"},
    {"1", "_02_NOMBRE_SUCUR", "", "NOSE", "TEXTO"},
    {"1", "_03_NOMBRE_ENCARGADO", "", "NOSE", "TEXTO"},
    {"1", "_04_DIRECCIÓN_SUCUR", "", "NOSE", "TEXTO"},
    {"1", "_05_CIUDAD_SUCUR", "", "NOSE", "TEXTO"},
    {"1", "_06_ESTADO_SUCUR", "", "NOSE", "TEXTO"},
    {"1", "_07_CÓDIGO_POSTAL", "", "NOSE", "TEXTO"},
    {"1", "_08_PAÍS", "", "NOSE", "TEXTO"},
    {"1", "_09_CORREO_ELECTRÓNICO", "", "NOSE", "TEXTO"},
    {"1", "_10_TELÉFONO_ENCARGADO", "", "NOSE", "TEXTO"},
    {"1", "_11_TELEFONO_SUCUR", "", "NOSE", "TEXTO"},
    {"1", "_12_TIPO_DE_SUCUR", "", "NOSE", "TEXTO"},
    {"1", "_13_PRODUCTOS_SERVICIOS", "", "NOSE", "TEXTO"},
    {"1", "_14_CUENTA_BANCO", "", "NOSE", "TEXTO"},
    {"1", "_15_UBICACIÓN_(GPS)", "", "-0", "TEXTO"},
    {"1", "_16_NOTAS", "", "", "TEXTO"},
    {"1", "_17_RECORDATORIO", "", "NOSE", "TEXTO"},
    {"1", "_18_ACTIVO_O_NO_ACTIVO", "", "NOSE", "TEXTO"},
    {"1", "_19_HORA_ABRIR_CERRAR", "", "NOSE", "TEXTO"},
    {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Registro Día (11 campos) */
const char *GG_ventana_reg_dia[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID", "", "-1", "ENTERO_DECIMAL"}, {"1", "_01_HORA", "", "0", "TEXTO"}, {"1", "_02_OPERACION", "", "NOSE", "TEXTO"}, {"1", "_03_TOTAL_IMPUESTO", "", "NOSE╦0", "TEXTO"}, {"1", "_04_CÓDIGO_PRECIO", "", "NOSE", "TEXTO"}, {"1", "_05_COMENTARIO", "", "", "TEXTO"}, {"1", "_06_TOTAL_VENTA", "", "0", "TEXTO"}, {"1", "_07_TOTAL_COSTO_COMP", "", "0", "TEXTO"}, {"1", "_08_TOTAL_IMPUESTOS", "", "0", "TEXTO"}, {"1", "_09_TOTAL_DEDUSIBLES", "", "0", "TEXTO"}, {"1", "_10_PLATAFORMA", "", "NOSE", "TEXTO"}, {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Registro Mes (9 campos) */
const char *GG_ventana_reg_mes[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID", "", "-1", "ENTERO_DECIMAL"}, {"1", "_01_DIA", "", "0", "TEXTO"}, {"1", "_02_OPERACION", "", "NOSE", "TEXTO"}, {"1", "_03_TOTAL_IMPUESTO", "", "NOSE╦0", "TEXTO"}, {"1", "_04_COMENTARIO", "", "", "TEXTO"}, {"1", "_05_TOTAL_VENTA", "", "0", "TEXTO"}, {"1", "_06_TOTAL_COSTO_COMP", "", "0", "TEXTO"}, {"1", "_07_TOTAL_IMPUESTOS", "", "0", "TEXTO"}, {"1", "_08_TOTAL_GANANCIA", "", "0", "TEXTO"}, {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Registro Año (9 campos) */
const char *GG_ventana_reg_año[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID", "", "-1", "ENTERO_DECIMAL"}, {"1", "_01_MES", "", "0", "TEXTO"}, {"1", "_02_OPERACION", "", "NOSE", "TEXTO"}, {"1", "_03_TOTAL_IMPUESTO", "", "NOSE╦0", "TEXTO"}, {"1", "_04_COMENTARIO", "", "", "TEXTO"}, {"1", "_05_TOTAL_VENTA", "", "0", "TEXTO"}, {"1", "_06_TOTAL_COSTO_COMP", "", "0", "TEXTO"}, {"1", "_07_TOTAL_IMPUESTOS", "", "0", "TEXTO"}, {"1", "_08_TOTAL_GANANCIA", "", "0", "TEXTO"}, {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Registro Total (9 campos) */
const char *GG_ventana_reg_total[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID", "", "-1", "ENTERO_DECIMAL"}, {"1", "_01_AÑO", "", "0", "TEXTO"}, {"1", "_02_OPERACION", "", "NOSE", "TEXTO"}, {"1", "_03_TOTAL_IMPUESTO", "", "NOSE╦0", "TEXTO"}, {"1", "_04_COMENTARIO", "", "", "TEXTO"}, {"1", "_05_TOTAL_VENTA", "", "0", "TEXTO"}, {"1", "_06_TOTAL_COSTO_COMP", "", "0", "TEXTO"}, {"1", "_07_TOTAL_IMPUESTOS", "", "0", "TEXTO"}, {"1", "_08_TOTAL_GANANCIA", "", "0", "TEXTO"}, {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Registro Productos Día (12 campos) */
const char *GG_ventana_reg_prod_dia[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID", "", "-1", "ENTERO_DECIMAL"}, {"1", "_01_NOMBRE_PRODUCTO", "", "NOSE", "TEXTO"}, {"1", "_02_CANTIDAD", "", "0", "TEXTO"}, {"1", "_03_COD_BAR", "", "NOSE", "TEXTO"}, {"1", "_04_PROVEDORES", "", "NOSE", "TEXTO"}, {"1", "_05_HISTORIAL", "", "0", "TEXTO"}, {"1", "_06_RANKING", "", "0", "TEXTO"}, {"1", "_07_PROMEDIO", "", "0", "TEXTO"}, {"1", "_08_VECES_SUPERA_PROMEDIO", "", "7", "TEXTO"}, {"1", "_09_USO_MULTIPLE", "", "", "TEXTO"}, {"1", "_10_TIPO_PRODUCTO", "", "", "TEXTO"}, {"1", "_11_NIVEL_NECESIDAD", "", "0", "TEXTO"}, {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Registro Productos Mes (12 campos) */
const char *GG_ventana_reg_prod_mes[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID", "", "-1", "ENTERO_DECIMAL"}, {"1", "_01_NOMBRE_PRODUCTO", "", "NOSE", "TEXTO"}, {"1", "_02_CANTIDAD", "", "0", "TEXTO"}, {"1", "_03_COD_BAR", "", "NOSE", "TEXTO"}, {"1", "_04_PROVEDORES", "", "NOSE", "TEXTO"}, {"1", "_05_HISTORIAL", "", "0", "TEXTO"}, {"1", "_06_RANKING", "", "0", "TEXTO"}, {"1", "_07_PROMEDIO", "", "0", "TEXTO"}, {"1", "_08_VECES_SUPERA_PROMEDIO", "", "7", "TEXTO"}, {"1", "_09_USO_MULTIPLE", "", "", "TEXTO"}, {"1", "_10_TIPO_PRODUCTO", "", "", "TEXTO"}, {"1", "_11_NIVEL_NECESIDAD", "", "0", "TEXTO"}, {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Registro Productos Año (12 campos) */
const char *GG_ventana_reg_prod_año[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID", "", "-1", "ENTERO_DECIMAL"}, {"1", "_01_NOMBRE_PRODUCTO", "", "NOSE", "TEXTO"}, {"1", "_02_CANTIDAD", "", "0", "TEXTO"}, {"1", "_03_COD_BAR", "", "NOSE", "TEXTO"}, {"1", "_04_PROVEDORES", "", "NOSE", "TEXTO"}, {"1", "_05_HISTORIAL", "", "0", "TEXTO"}, {"1", "_06_RANKING", "", "0", "TEXTO"}, {"1", "_07_PROMEDIO", "", "0", "TEXTO"}, {"1", "_08_VECES_SUPERA_PROMEDIO", "", "7", "TEXTO"}, {"1", "_09_USO_MULTIPLE", "", "", "TEXTO"}, {"1", "_10_TIPO_PRODUCTO", "", "", "TEXTO"}, {"1", "_11_NIVEL_NECESIDAD", "", "0", "TEXTO"}, {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Registro Productos Total (12 campos) */
const char *GG_ventana_reg_prod_total[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID", "", "-1", "ENTERO_DECIMAL"}, {"1", "_01_NOMBRE_PRODUCTO", "", "NOSE", "TEXTO"}, {"1", "_02_CANTIDAD", "", "0", "TEXTO"}, {"1", "_03_COD_BAR", "", "NOSE", "TEXTO"}, {"1", "_04_PROVEDORES", "", "NOSE", "TEXTO"}, {"1", "_05_HISTORIAL", "", "0", "TEXTO"}, {"1", "_06_RANKING", "", "0", "TEXTO"}, {"1", "_07_PROMEDIO", "", "0", "TEXTO"}, {"1", "_08_VECES_SUPERA_PROMEDIO", "", "7", "TEXTO"}, {"1", "_09_USO_MULTIPLE", "", "", "TEXTO"}, {"1", "_10_TIPO_PRODUCTO", "", "", "TEXTO"}, {"1", "_11_NIVEL_NECESIDAD", "", "0", "TEXTO"}, {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Impuestos (6 campos) */
const char *GG_ventana_IMPUESTOS[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID", "", "-1", "ENTERO_DECIMAL"}, {"1", "_01_IMPUESTO", "", "0", "TEXTO"}, {"1", "_02_PORCENTAGE", "", "0", "TEXTO"}, {"1", "_03_DESCRIPCION", "", "NOSE", "TEXTO"}, {"1", "_04_INFO_EXTRA", "", "NOSE", "TEXTO"}, {"1", "_05_TIPO_IMPUESTO", "", "3", "TEXTO"}, {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Deducibles (7 campos) */
const char *GG_ventana_DEDUSIBLES[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID", "", "-1", "ENTERO_DECIMAL"}, {"1", "_01_FECHA", "", "0", "TEXTO"}, {"1", "_02_MONTO", "", "0", "TEXTO"}, {"1", "_03_DESCRIPCION", "", "NOSE", "TEXTO"}, {"1", "_04_PROVEDOR", "", "NOSE", "TEXTO"}, {"1", "_05_ARCHIVO_FACTURA", "", "NOSE", "TEXTO"}, {"1", "_06_FOLIO", "", "NOSE", "TEXTO"}, {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Herramientas (2 campos) */
const char *GG_ventana_HERRAMIENTAS[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID", "", "-1", "ENTERO_DECIMAL"},
    {"1", "_01_COD_BAR", "", "", "TEXTO"},
    {NULL, NULL, NULL, NULL, NULL}};

/* Ventana: Trabajos por Día (8 campos) */
const char *GG_trabajos_dia[][5] = {
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    {"2", "_00_ID", "", "-1", "ENTERO_DECIMAL"}, {"1", "_01_DIA", "", "", "TEXTO"}, {"1", "_02_ID_TRABAJADOR", "", "", "TEXTO"}, {"1", "_03_HECHO_O_NO", "", "", "TEXTO"}, {"1", "_04_CANTIDAD", "", "", "TEXTO"}, {"1", "_05_COSTO_COMP", "", "", "TEXTO"}, {"1", "_06_ID_QUIENLOISO", "", "", "TEXTO"}, {"1", "_07_ID_PROGRAMA", "", "", "TEXTO"}, {NULL, NULL, NULL, NULL, NULL}};

/*
 * Uso: Ejecuta RecargarVentanaEmergenteDatosConfiguracion de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteDatosConfiguracion(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteDatosConfiguracion(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergenteProductos de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteProductos(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteProductos(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergente_Cosas_que_no_estaban de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergente_Cosas_que_no_estaban(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergente_Cosas_que_no_estaban(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergenteProvedor de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteProvedor(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteProvedor(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergenteAPRENDICES_E de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteAPRENDICES_E(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteAPRENDICES_E(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergenteAfiliados_simples de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteAfiliados_simples(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteAfiliados_simples(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergenteAfiliados_complejos de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteAfiliados_complejos(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteAfiliados_complejos(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergente_niv_afiliados_simples de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergente_niv_afiliados_simples(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergente_niv_afiliados_simples(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergente_niv_afiliados_comp de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergente_niv_afiliados_comp(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergente_niv_afiliados_comp(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergenteAfiliados_unificados de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteAfiliados_unificados(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteAfiliados_unificados(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergente_niv_afiliados_unificado de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergente_niv_afiliados_unificado(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergente_niv_afiliados_unificado(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergenteSUCUR de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteSUCUR(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteSUCUR(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergenteRegDia de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteRegDia(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteRegDia(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergenteRegMes de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteRegMes(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteRegMes(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

void RecargarVentanaEmergenteRegAño(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergenteRegTotal de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteRegTotal(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteRegTotal(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergenteReg_prod_Dia de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteReg_prod_Dia(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteReg_prod_Dia(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergenteReg_prod_Mes de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteReg_prod_Mes(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteReg_prod_Mes(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

void RecargarVentanaEmergenteReg_prod_Año(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergenteReg_prod_total de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteReg_prod_total(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteReg_prod_total(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergenteImpuestos de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteImpuestos(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteImpuestos(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergenteDedusibles de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergenteDedusibles(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergenteDedusibles(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergente_HERRAMIENTAS de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergente_HERRAMIENTAS(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergente_HERRAMIENTAS(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/*
 * Uso: Ejecuta RecargarVentanaEmergente_TRABAJOS_DIA de forma segura.
 * Entrada ejemplo: RecargarVentanaEmergente_TRABAJOS_DIA(al_finalizar_que_borrar)
 */
void RecargarVentanaEmergente_TRABAJOS_DIA(const char *al_finalizar_que_borrar)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (al_finalizar_que_borrar != NULL && strcmp(al_finalizar_que_borrar, "TODO") == 0) // verifica que el parámetro no sea NULL y sea "TODO"; ejemplo: "TODO"
    {
        for (int i = 0; i < MAX_VAR_STRING; i++) // recorre todas las variables string globales
        {
            if (GG_variables_string[i] != NULL) // verifica que la posición no sea NULL antes de limpiar
            {
                strcpy(GG_variables_string[i], ""); // limpia la variable string en la posición i; ejemplo: ""
            }
        }
    }
}

/* Función auxiliar para concatenar valores de columnas
id_columna se pasa para futuras extensiones; actualmente no se usa. */
/*
 * Uso: Ejecuta columnas_concatenadas de forma segura.
 * Entrada ejemplo: columnas_concatenadas(arreglo, filas, id_columna, caracter_separacion)
 */
char *columnas_concatenadas(const char *arreglo[][5], int filas, int id_columna, const char *caracter_separacion)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    static char resultado[4096];             // buffer est\u00e1tico donde se acumula el resultado concatenado; ejemplo: "col1°col2°col3"
    memset(resultado, 0, sizeof(resultado)); // limpia el buffer antes de usarlo para evitar basura de llamadas anteriores

    if (!arreglo) // valida que el arreglo de entrada no sea NULL
    {
        return resultado; // retorna buffer vac\u00edo si no hay arreglo v\u00e1lido
    }

    if (caracter_separacion == NULL) // usa separador por defecto si no se proporcion\u00f3 uno
    {
        caracter_separacion = GG_caracter_separacion[1]; // asigna el separador "°" como valor por defecto; ejemplo: "°"
    }

    if (id_columna < 0 || id_columna >= 5) // valida que el \u00edndice de columna est\u00e9 dentro del rango [0,4]
    {
        id_columna = 1; // restablece al \u00edndice de columna 1 (nombre) si el \u00edndice es inv\u00e1lido; ejemplo: 1
    }

    int total_filas = 0; // contador de filas del arreglo; ejemplo: 33
    if (filas > 0)       // si se proporcion\u00f3 cantidad de filas expl\u00edcita, la usa directamente
    {
        total_filas = filas; // asigna el valor de filas pasado como par\u00e1metro; ejemplo: 33
    }
    else
    {
        while (arreglo[total_filas][0] != NULL) // detecta autom\u00e1ticamente el fin del arreglo por NULL sentinel
        {
            total_filas++; // incrementa el contador hasta encontrar la fila terminadora NULL
        }
    }

    for (int i = 0; i < total_filas; i++) // itera sobre cada fila v\u00e1lida del arreglo
    {
        const char *valor = arreglo[i][id_columna]; // obtiene el valor de la columna solicitada en la fila i; ejemplo: "_01_PRODUCTO"

        if (!valor) // omite filas donde el valor de la columna sea NULL
        {
            continue; // salta a la siguiente iteraci\u00f3n sin agregar nada
        }

        if (resultado[0] != '\0') // agrega separador solo si ya hay contenido previo en el resultado
        {
            strcat(resultado, caracter_separacion); // concatena el separador entre columnas; ejemplo: "°"
        }

        strcat(resultado, valor); // agrega el valor de la columna al resultado; ejemplo: "_01_PRODUCTO"
    }

    return resultado; // retorna el string con todas las columnas concatenadas; ejemplo: "_00_ID°_01_PRODUCTO°..."
}

char *GG_direccion_carpetas_base[] = {"", NULL};

GG_ArchivoBaseNegocio *GG_dir_nom_archivos = NULL; // puntero dinámico al arreglo de archivos base del negocio; ejemplo: NULL al inicio

GG_ArchivoInventarioPendiente *GG_direccion_hacer_inventarios = NULL; // puntero dinámico al arreglo de archivos de inventario pendiente; ejemplo: NULL al inicio

/*
 * Uso: Ejecuta duplicar_texto de forma segura.
 * Entrada ejemplo: duplicar_texto(txt)
 */
static char *duplicar_texto(const char *txt)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!txt)        // valida que el puntero de entrada no sea NULL
        return NULL; // retorna NULL si no hay texto que duplicar

    char *salida = (char *)malloc(strlen(txt) + 1); // reserva memoria para la copia incluyendo el terminador nulo; ejemplo: malloc(6)
    if (!salida)                                    // verifica que la asignación de memoria fue exitosa
        return NULL;                                // retorna NULL si malloc falló por falta de memoria

    strcpy(salida, txt); // copia el texto fuente en el nuevo bloque de memoria; ejemplo: "hola"
    return salida;       // retorna el puntero a la copia del texto
}

/*
 * Uso: Ejecuta crear_metadata_archivo_base de forma segura.
 * Entrada ejemplo: crear_metadata_archivo_base(columnas)
 */
static char *crear_metadata_archivo_base(const char *columnas)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    const char *columnas_seguras = columnas ? columnas : "";                                                                             // usa string vacío si columnas es NULL para evitar crash; ejemplo: "_00_ID|_01_PRODUCTO"
    const char *cantidad_por_archivo = GG_cantidado_por_archivo ? GG_cantidado_por_archivo : "0";                                        // usa "0" si la global es NULL; ejemplo: "100"
    size_t longitud_total = strlen("ID_TOT|0\nCOLUMNAS|\nCANT_POR_ARCH|") + strlen(columnas_seguras) + strlen(cantidad_por_archivo) + 1; // calcula el tamaño exacto del string de metadata; ejemplo: 60
    char *metadata = (char *)malloc(longitud_total);                                                                                     // reserva memoria para el string de metadata del archivo

    if (!metadata) // verifica que malloc tuvo éxito
    {
        return NULL; // retorna NULL si no hay memoria disponible
    }

    snprintf(metadata, longitud_total, "ID_TOT|0\nCOLUMNAS|%s\nCANT_POR_ARCH|%s", columnas_seguras, cantidad_por_archivo); // formatea el string de metadata con las columnas y la cantidad; ejemplo: "ID_TOT|0\nCOLUMNAS|_00_ID°...\nCANT_POR_ARCH|100"
    return metadata;                                                                                                       // retorna el string de metadata listo para usarse como cabecera
}

/*
 * Uso: Ejecuta agregar_archivo_base_negocio de forma segura.
 * Entrada ejemplo: agregar_archivo_base_negocio(ruta, cabecera, extra)
 */
static int agregar_archivo_base_negocio(const char *ruta, const char *cabecera, const char *extra)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    int cantidad = 0;                  // contador de entradas existentes en el arreglo; ejemplo: 3
    GG_ArchivoBaseNegocio *tmp = NULL; // puntero temporal para realloc seguro; ejemplo: NULL

    if (GG_dir_nom_archivos) // verifica si ya hay entradas previas en el arreglo
    {
        while (GG_dir_nom_archivos[cantidad].ruta || GG_dir_nom_archivos[cantidad].cabecera || GG_dir_nom_archivos[cantidad].extra) // recorre hasta encontrar el terminador NULL
        {
            cantidad++; // incrementa el \u00edndice de conteo de entradas v\u00e1lidas
        }
    }

    tmp = (GG_ArchivoBaseNegocio *)realloc(GG_dir_nom_archivos, sizeof(GG_ArchivoBaseNegocio) * (cantidad + 2)); // expande el arreglo para alojar la nueva entrada m\u00e1s el terminador
    if (!tmp)                                                                                                    // verifica que realloc tuvo \u00e9xito
    {
        RETORNAR_PROCESO_ESTANDAR(-1); // retorna error si no se pudo reasignar memoria
    }

    GG_dir_nom_archivos = tmp;                         // actualiza el puntero global con la nueva memoria asignada
    GG_dir_nom_archivos[cantidad].ruta = NULL;         // inicializa el nuevo slot con NULL antes de asignar
    GG_dir_nom_archivos[cantidad].cabecera = NULL;     // inicializa campo cabecera del nuevo slot
    GG_dir_nom_archivos[cantidad].extra = NULL;        // inicializa campo extra del nuevo slot
    GG_dir_nom_archivos[cantidad + 1].ruta = NULL;     // inicializa el slot terminador NULL
    GG_dir_nom_archivos[cantidad + 1].cabecera = NULL; // inicializa cabecera del terminador
    GG_dir_nom_archivos[cantidad + 1].extra = NULL;    // inicializa extra del terminador

    GG_dir_nom_archivos[cantidad].ruta = duplicar_texto(ruta ? ruta : "");             // duplica y asigna la ruta; ejemplo: "CONFIG\\INF\\INVENTARIO\\INVENTARIO.TXT"
    GG_dir_nom_archivos[cantidad].cabecera = duplicar_texto(cabecera ? cabecera : ""); // duplica y asigna la cabecera con metadata del archivo
    GG_dir_nom_archivos[cantidad].extra = duplicar_texto(extra ? extra : "");          // duplica y asigna informaci\u00f3n extra del archivo; ejemplo: ""

    if (!GG_dir_nom_archivos[cantidad].ruta || !GG_dir_nom_archivos[cantidad].cabecera || !GG_dir_nom_archivos[cantidad].extra) // verifica que todos los campos se duplicaron correctamente
    {
        free(GG_dir_nom_archivos[cantidad].ruta);      // libera ruta si fall\u00f3 alguna asignaci\u00f3n
        free(GG_dir_nom_archivos[cantidad].cabecera);  // libera cabecera para evitar memory leak
        free(GG_dir_nom_archivos[cantidad].extra);     // libera extra para evitar memory leak
        GG_dir_nom_archivos[cantidad].ruta = NULL;     // resetea puntero a NULL tras liberar
        GG_dir_nom_archivos[cantidad].cabecera = NULL; // resetea puntero a NULL tras liberar
        GG_dir_nom_archivos[cantidad].extra = NULL;    // resetea puntero a NULL tras liberar
        RETORNAR_PROCESO_ESTANDAR(-1);                 // retorna error indicando falla en la duplicaci\u00f3n de campos
    }

    RETORNAR_PROCESO_ESTANDAR(0); // retorna \u00e9xito al haber agregado la entrada correctamente
}
/*
 * Uso: Ejecuta agregar_archivo_base_negocio_con_columnas de forma segura.
 * Entrada ejemplo: agregar_archivo_base_negocio_con_columnas(ruta, columnas, extra)
 */
static int agregar_archivo_base_negocio_con_columnas(const char *ruta, const char *columnas, const char *extra)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    char *metadata = crear_metadata_archivo_base(columnas); // genera el string de metadata con columnas y cantidad por archivo
    int resultado;                                          // almacena el código de retorno de agregar_archivo_base_negocio; ejemplo: 0

    if (!metadata) // verifica que la creación de metadata fue exitosa
    {
        RETORNAR_PROCESO_ESTANDAR(-1); // retorna error si no se pudo crear el string de metadata
    }

    resultado = agregar_archivo_base_negocio(ruta, metadata, extra); // agrega la entrada al arreglo usando la metadata generada
    free(metadata);                                                  // libera la memoria de metadata ya que fue copiada internamente
    RETORNAR_PROCESO_ESTANDAR(resultado);                            // retorna el resultado de la operación de agregado; ejemplo: 0
}

/*
 * Uso: Ejecuta agregar_archivo_inventario de forma segura.
 * Entrada ejemplo: agregar_archivo_inventario(ruta, cabecera)
 */
static int agregar_archivo_inventario(const char *ruta, const char *cabecera)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    int cantidad = 0;                          // contador de entradas existentes en el arreglo de inventarios; ejemplo: 2
    GG_ArchivoInventarioPendiente *tmp = NULL; // puntero temporal para realloc seguro; ejemplo: NULL

    if (GG_direccion_hacer_inventarios) // verifica si ya hay entradas previas en el arreglo
    {
        while (GG_direccion_hacer_inventarios[cantidad].ruta || GG_direccion_hacer_inventarios[cantidad].cabecera) // recorre hasta encontrar el terminador NULL
        {
            cantidad++; // incrementa el contador de entradas v\u00e1lidas
        }
    }

    tmp = (GG_ArchivoInventarioPendiente *)realloc(GG_direccion_hacer_inventarios, sizeof(GG_ArchivoInventarioPendiente) * (cantidad + 2)); // expande el arreglo para la nueva entrada m\u00e1s terminador
    if (!tmp)                                                                                                                               // verifica que realloc tuvo \u00e9xito
    {
        RETORNAR_PROCESO_ESTANDAR(-1); // retorna error si no se pudo reasignar memoria
    }

    GG_direccion_hacer_inventarios = tmp;                         // actualiza el puntero global con la nueva memoria asignada
    GG_direccion_hacer_inventarios[cantidad].ruta = NULL;         // inicializa el nuevo slot con NULL antes de asignar
    GG_direccion_hacer_inventarios[cantidad].cabecera = NULL;     // inicializa cabecera del nuevo slot
    GG_direccion_hacer_inventarios[cantidad + 1].ruta = NULL;     // inicializa el slot terminador NULL
    GG_direccion_hacer_inventarios[cantidad + 1].cabecera = NULL; // inicializa cabecera del terminador

    GG_direccion_hacer_inventarios[cantidad].ruta = duplicar_texto(ruta ? ruta : "");             // duplica y asigna la ruta del archivo de inventario; ejemplo: "CONFIG\\INF\\INVENTARIO\\HACER_INVENTARIO\\..."
    GG_direccion_hacer_inventarios[cantidad].cabecera = duplicar_texto(cabecera ? cabecera : ""); // duplica y asigna la cabecera del archivo de inventario

    if (!GG_direccion_hacer_inventarios[cantidad].ruta || !GG_direccion_hacer_inventarios[cantidad].cabecera) // verifica que ambos campos se duplicaron correctamente
    {
        free(GG_direccion_hacer_inventarios[cantidad].ruta);      // libera ruta para evitar memory leak
        free(GG_direccion_hacer_inventarios[cantidad].cabecera);  // libera cabecera para evitar memory leak
        GG_direccion_hacer_inventarios[cantidad].ruta = NULL;     // resetea puntero a NULL tras liberar
        GG_direccion_hacer_inventarios[cantidad].cabecera = NULL; // resetea puntero a NULL tras liberar
        RETORNAR_PROCESO_ESTANDAR(-1);                            // retorna error indicando falla en la duplicaci\u00f3n de campos
    }

    RETORNAR_PROCESO_ESTANDAR(0); // retorna \u00e9xito al haber agregado la entrada de inventario
}

/*
 * Uso: Ejecuta liberar_arreglo_dir_nom_archivos de forma segura.
 * Entrada ejemplo: liberar_arreglo_dir_nom_archivos()
 */
static void liberar_arreglo_dir_nom_archivos(void)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!GG_dir_nom_archivos) // verifica que haya memoria que liberar antes de intentar liberarla
    {
        return; // sale sin hacer nada si el puntero ya es NULL
    }

    for (int i = 0; GG_dir_nom_archivos[i].ruta || GG_dir_nom_archivos[i].cabecera || GG_dir_nom_archivos[i].extra; i++) // itera sobre cada entrada hasta el terminador NULL
    {
        free(GG_dir_nom_archivos[i].ruta);     // libera la ruta del archivo en la posición i
        free(GG_dir_nom_archivos[i].cabecera); // libera la cabecera del archivo en la posición i
        free(GG_dir_nom_archivos[i].extra);    // libera el campo extra del archivo en la posición i
    }

    free(GG_dir_nom_archivos);  // libera el arreglo completo de punteros
    GG_dir_nom_archivos = NULL; // resetea el puntero global a NULL para evitar uso tras liberación
}

/*
 * Uso: Ejecuta liberar_arreglo_direccion_inventarios de forma segura.
 * Entrada ejemplo: liberar_arreglo_direccion_inventarios()
 */
static void liberar_arreglo_direccion_inventarios(void)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    if (!GG_direccion_hacer_inventarios) // verifica que haya memoria que liberar antes de intentar liberarla
    {
        return; // sale sin hacer nada si el puntero ya es NULL
    }

    for (int i = 0; GG_direccion_hacer_inventarios[i].ruta || GG_direccion_hacer_inventarios[i].cabecera; i++) // itera sobre cada entrada hasta el terminador NULL
    {
        free(GG_direccion_hacer_inventarios[i].ruta);     // libera la ruta del archivo de inventario en la posición i
        free(GG_direccion_hacer_inventarios[i].cabecera); // libera la cabecera del archivo de inventario en la posición i
    }

    free(GG_direccion_hacer_inventarios);  // libera el arreglo completo de punteros
    GG_direccion_hacer_inventarios = NULL; // resetea el puntero global a NULL para evitar uso tras liberación
}

/*
 * Uso: Ejecuta RecargarArregloArchivos_dir_nom_archivos de forma segura.
 * Entrada ejemplo: RecargarArregloArchivos_dir_nom_archivos()
 */
void RecargarArregloArchivos_dir_nom_archivos(void)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    liberar_arreglo_dir_nom_archivos(); // libera el arreglo previo antes de reconstruirlo desde cero

    time_t ahora = time(NULL);        // obtiene el tiempo actual del sistema en segundos desde epoch
    struct tm *t = localtime(&ahora); // convierte el tiempo a estructura local con año, mes, día, etc.
    char yyyy[5] = "0000";            // buffer para el año en formato YYYY; ejemplo: "2026"
    char yyyymm[7] = "000000";        // buffer para año+mes en formato YYYYMM; ejemplo: "202605"
    char yyyymmdd[9] = "00000000";    // buffer para año+mes+día en formato YYYYMMDD; ejemplo: "20260505"

    if (t) // verifica que localtime retornó una estructura válida
    {
        strftime(yyyy, sizeof(yyyy), "%Y", t);             // formatea el año de 4 dígitos; ejemplo: "2026"
        strftime(yyyymm, sizeof(yyyymm), "%Y%m", t);       // formatea año y mes de 6 dígitos; ejemplo: "202605"
        strftime(yyyymmdd, sizeof(yyyymmdd), "%Y%m%d", t); // formatea fecha completa de 8 dígitos; ejemplo: "20260505"
    }

                                                                             
    const char *base = GG_direccion_carpetas_base[0] ? GG_direccion_carpetas_base[0] : ""; // prefijo de ruta base del espacio activo; ejemplo: "espacios\20260406224536_ferreteria_dan\"

    char tmp[512];// buffer temporal para construir rutas de archivos; ejemplo: "CONFIG\INF\INVENTARIO\INVENTARIO.TXT"
    
    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\INVENTARIO\\INVENTARIO.TXT", base);                                                                      // construye ruta al archivo de inventario principal
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_emergente_productos, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el archivo de inventario al arreglo global
        return;                                                                                                                                         // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\DAT\\PROVEDORES.TXT", base);                                                                  // construye ruta al archivo de proveedores
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_provedor, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el archivo de proveedores al arreglo global
        return;                                                                                                                              // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\DAT\\APRENDICES_E.TXT", base);                                                                    // construye ruta al archivo de aprendices
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_APRENDICES_E, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el archivo de aprendices al arreglo global
        return;                                                                                                                                  // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\AFILIADOS\\AFILIADOS_UNIFICADO.TXT", base);                                                                    // construye ruta al archivo de afiliados unificado
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_afiliados_unificados, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el archivo de afiliados al arreglo global
        return;                                                                                                                                          // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\REG_A_CONFIRMAR_.TXT", base);                                                                             // construye ruta al archivo de registros a confirmar
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_afiliados_unificados, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el archivo de registros a confirmar al arreglo global
        return;                                                                                                                                          // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\AFILIADOS\\NIVELES_E_ID_HORISONTAL_AFILIADOS_UNIFICADO.TXT", base);                                               // construye ruta al archivo de niveles de afiliados
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_niv_afiliados_unificado, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el archivo de niveles de afiliados al arreglo global
        return;                                                                                                                                             // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\AFILIADOS\\NIVELES_E_ID_HORISONTAL_AFILIADOS_UNIFICADO.TXT", base);                                               // construye ruta al archivo de niveles de afiliados (segunda referencia)
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_niv_afiliados_unificado, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega segunda referencia al archivo de niveles de afiliados
        return;                                                                                                                                             // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\AFILIADOS\\AFILIADOS_UNIFICADO.TXT", base);                                                                    // construye ruta al archivo de afiliados unificado (segunda referencia)
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_afiliados_unificados, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega segunda referencia al archivo de afiliados
        return;                                                                                                                                          // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\AFILIADOS\\NIVELES_E_ID_HORISONTAL_AFILIADOS_UNIFICADO.TXT", base);                                               // construye ruta al archivo de niveles de afiliados (tercera referencia)
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_niv_afiliados_unificado, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega tercera referencia al archivo de niveles de afiliados
        return;                                                                                                                                             // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\DAT\\SUCUR.TXT", base);                                                                    // construye ruta al archivo de sucursales
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_SUCUR, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el archivo de sucursales al arreglo global
        return;                                                                                                                           // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\REGISTROS\\FECHAS\\%s\\%s\\%s_REGISTRO.TXT", base, yyyy, yyyymm, yyyymmdd);                  // construye ruta al registro diario con la fecha actual
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_reg_dia, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el registro diario al arreglo global
        return;                                                                                                                             // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\REGISTROS\\FECHAS\\%s\\%s_REGISTRO.TXT", base, yyyy, yyyymm);                                // construye ruta al registro mensual con año y mes actuales
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_reg_mes, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el registro mensual al arreglo global
        return;                                                                                                                             // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\REGISTROS\\FECHAS\\%s_REGISTRO.TXT", base, yyyy);                                            // construye ruta al registro anual con el año actual
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_reg_año, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el registro anual al arreglo global
        return;                                                                                                                             // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\REGISTROS\\ACUMULADO_REGISTRO.TXT", base);                                                     // construye ruta al registro acumulado total
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_reg_total, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el registro acumulado total al arreglo global
        return;                                                                                                                               // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\REGISTROS\\FECHAS\\%s\\%s\\%s_PRODUC_REGISTRO.TXT", base, yyyy, yyyymm, yyyymmdd);                // construye ruta al registro de productos diario con la fecha actual
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_reg_prod_dia, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el registro de productos diario al arreglo global
        return;                                                                                                                                  // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\REGISTROS\\FECHAS\\%s\\%s_PRODUC_REGISTRO.TXT", base, yyyy, yyyymm);                              // construye ruta al registro de productos mensual
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_reg_prod_mes, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el registro de productos mensual al arreglo global
        return;                                                                                                                                  // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\REGISTROS\\FECHAS\\%s_PRODUC_REGISTRO.TXT", base, yyyy);                                          // construye ruta al registro de productos anual
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_reg_prod_año, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el registro de productos anual al arreglo global
        return;                                                                                                                                  // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\REGISTROS\\ACUMULADO_PRODUC_REGISTRO.TXT", base);                                                   // construye ruta al registro acumulado de productos
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_reg_prod_total, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el registro acumulado de productos al arreglo global
        return;                                                                                                                                    // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\IMPUESTOS\\IMPUESTOS.TXT", base);                                                              // construye ruta al archivo de impuestos
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_IMPUESTOS, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el archivo de impuestos al arreglo global
        return;                                                                                                                               // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\IMPUESTOS\\DEDUSIBLES.TXT", base);                                                              // construye ruta al archivo de deducibles
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_DEDUSIBLES, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el archivo de deducibles al arreglo global
        return;                                                                                                                                // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\INVENTARIO\\COSAS_NO_ESTABAN.TXT", base);                                                                        // construye ruta al archivo de cosas no encontradas en inventario
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_COSAS_NO_ESTABAN_INVENTARIO, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el archivo de cosas no encontradas al arreglo global
        return;                                                                                                                                                 // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\INVENTARIO\\TIPOS_DE_MEDIDA.TXT", base);                                                          // construye ruta al archivo de tipos de medida (herramientas)
    if (agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_ventana_HERRAMIENTAS, 0, 1, GG_caracter_separacion[1]), "") < 0) // agrega el archivo de tipos de medida al arreglo global
        return;                                                                                                                                  // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "%sCONFIG\\INF\\DAT\\TRABAJOS_POR_DIA.TXT", base);                                                // construye ruta al archivo de trabajos por día
    agregar_archivo_base_negocio_con_columnas(tmp, columnas_concatenadas(GG_trabajos_dia, 0, 1, GG_caracter_separacion[1]), ""); // agrega el archivo de trabajos por día (último, sin verificar retorno)
}

/*
 * Uso: Ejecuta RecargarArregloDireccionInventarios de forma segura.
 * Entrada ejemplo: RecargarArregloDireccionInventarios()
 */
void RecargarArregloDireccionInventarios(void)
{
    /* Paso a paso: validar entradas, procesar y manejar errores. */
    liberar_arreglo_direccion_inventarios(); // libera el arreglo previo antes de reconstruirlo

    time_t ahora = time(NULL);        // obtiene el tiempo actual del sistema
    struct tm *t = localtime(&ahora); // convierte a estructura de tiempo local
    char yyyymmdd[9] = "00000000";    // buffer para la fecha en formato YYYYMMDD; ejemplo: "20260505"

    if (t)                                                 // verifica que localtime retornó una estructura válida
        strftime(yyyymmdd, sizeof(yyyymmdd), "%Y%m%d", t); // formatea la fecha de 8 dígitos; ejemplo: "20260505"

    char tmp[512]; // buffer temporal para construir rutas de archivos de inventario

    snprintf(tmp, sizeof(tmp), "CONFIG\\INF\\INVENTARIO\\HACER_INVENTARIO\\%s_VENTAS_DURANTE_INV.TXT", yyyymmdd); // construye ruta al archivo de ventas durante inventario
    if (agregar_archivo_inventario(tmp, "CODBAR°nombre_producto°CANTIDA°ULTIMO_MOVIMIENTO") < 0)                  // agrega el archivo de ventas durante inventario al arreglo
        return;                                                                                                   // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "CONFIG\\INF\\INVENTARIO\\HACER_INVENTARIO\\%s_SOBRANTES.TXT", yyyymmdd); // construye ruta al archivo de productos sobrantes en inventario
    if (agregar_archivo_inventario(tmp, "CODBAR°nombre_producto°CANTIDA°ULTIMO_MOVIMIENTO") < 0)         // agrega el archivo de sobrantes al arreglo
        return;                                                                                          // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "CONFIG\\INF\\INVENTARIO\\HACER_INVENTARIO\\%s_FALTANTES.TXT", yyyymmdd); // construye ruta al archivo de productos faltantes en inventario
    if (agregar_archivo_inventario(tmp, "CODBAR°nombre_producto°CANTIDA°ULTIMO_MOVIMIENTO") < 0)         // agrega el archivo de faltantes al arreglo
        return;                                                                                          // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "CONFIG\\INF\\INVENTARIO\\HACER_INVENTARIO\\%s_NO_ESTAN_EN_EL_FISICO.TXT", yyyymmdd); // construye ruta al archivo de productos no encontrados en físico
    if (agregar_archivo_inventario(tmp, "CODBAR°nombre_producto°CANTIDA°ULTIMO_MOVIMIENTO") < 0)                     // agrega el archivo de no encontrados en físico al arreglo
        return;                                                                                                      // aborta si no se pudo agregar por falta de memoria

    snprintf(tmp, sizeof(tmp), "CONFIG\\INF\\INVENTARIO\\HACER_INVENTARIO\\%s_NO_ESTAN_EN_EL_FISICO_PERO_PUEDE_QUE_FALTEN.TXT", yyyymmdd); // construye ruta al archivo de posibles faltantes en físico
    agregar_archivo_inventario(tmp, "CODBAR°nombre_producto°CANTIDA°ULTIMO_MOVIMIENTO");                                                   // agrega el último archivo (sin verificar retorno porque es el final de la función)
}
