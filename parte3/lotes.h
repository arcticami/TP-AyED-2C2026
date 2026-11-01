#ifndef LOTES_H_INCLUDED
#define LOTES_H_INCLUDED

//Generacion de los lotes de prueba de Tweener. La usan generador.c (para generar un lote suelto)
//y simulador.c (que genera los tres lotes antes de simularlos), asi que vive en un solo lugar.

#define CANT_TIPOS_LOTE 3
#define TAM_RUTA        64
#define RUTA_POSTEOS    "posteos.bin"   //se escribe en la carpeta de trabajo del programa

extern const char *const TIPOS_LOTE[CANT_TIPOS_LOTE];   //"concentrado", "disperso", "ordenado"

int tipoLoteValido(const char *tipo);
void rutaUsuarios(char *ruta, const char *tipo);        //ruta debe tener lugar para TAM_RUTA caracteres

//escriben el archivo y devuelven 1 si salio bien, 0 si no
int generarPosteos(const char *ruta);
int generarUsuarios(const char *ruta, const char *tipo, unsigned semilla, unsigned cantUsuarios);

#endif // LOTES_H_INCLUDED