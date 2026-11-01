#ifndef TWEENER_H_INCLUDED
#define TWEENER_H_INCLUDED

//Definiciones compartidas por generador.c y simulador.c: los dos tienen que ver
//exactamente los mismos registros, por eso viven en un solo lugar.

#define TAM_TEXTO       141     //char[141] segun el enunciado (el texto usa hasta TAM_TEXTO - 2 caracteres)
#define TAM_USUARIO     15      //char[15] segun el enunciado
#define LARGO_FEED      20      //int[20] segun el enunciado

#define CANT_POSTEOS    50000   //el id de cada posteo coincide con su numero de registro (0 .. CANT_POSTEOS - 1)
#define CANT_USUARIOS   2500    //valor por defecto del generador (el enunciado pide no menos de 200)
#define MIN_USUARIOS    200

typedef struct
{
    int  id;
    char texto[TAM_TEXTO];
} tPosteo;

typedef struct
{
    char usuario[TAM_USUARIO];
    int  feed[LARGO_FEED];
} tUsuario;

#endif // TWEENER_H_INCLUDED
