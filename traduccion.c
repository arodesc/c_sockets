#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "traduccion.h"


/*
  Convierte una direccion de Internet y un puerto de servicio
  (ambos cadena de caracteres) a valores numericos para poder
  ser utilizados en otras funciones, como bind y connect.
  La informacion tambien se imprimira por pantalla.
  Parametros de entrada:
  - maquina - cadena de caracteres con la direccion de Internet
  - puerto - cadena de caracteres con el puerto de servicio
  - tipo - SOCKET_UDP, SOCKET_TCP o SOCKET_TCP_PASIVO (escucha)
  Parametros de salida:
  - info - estructura addrinfo con el primer valor encontrado
  Devuelve:
  - Verdadero, si ha tenido exito.
*/
int traduce_a_direccion(const char *maquina, const char *puerto,
                        int tipo, struct addrinfo *info)
{
  struct addrinfo hints;        /* Estructura utilizada para afinar la
                                   busqueda */
  struct addrinfo *result, *rp; /*rp, variable usada para recorrer
                                   la lista de direcciones 
                                   encontradas */
  int error = 0;

  /* Obtiene las direcciones que coincidan con maquina/puerto */

  /* Ponemos a 0 la estructura hints */
  memset(&hints, 0, sizeof(struct addrinfo));
  /*Inicializamos la estructura */
  hints.ai_family = AF_INET;    /* AF_UNSPEC Permite IPv4 o IPv6
                                   AF_INET solo IPv4 */
  if (SOCKET_UDP == tipo)
    hints.ai_socktype = SOCK_DGRAM;     /* Socket de datagramas */
  else
    hints.ai_socktype = SOCK_STREAM;    /* Socket de flujo */
  hints.ai_protocol = 0;        /* Cualquier protocolo */
  if (SOCKET_TCP_PASIVO == tipo)
    hints.ai_flags |= AI_PASSIVE;       /* Cualquier direccion IP */


  /*Llamamos a la funcion de busqueda de nombres */
  error = getaddrinfo(maquina, puerto, &hints, &result);
  if (error != 0)
    {
      //Mostramos informacion del error usando la funcion gai_strerror
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    }
  else
    {

      /* getaddrinfo() devuelve una lista de estructuras addrinfo.
         Vamos a imprimirlas todas, aunque solo devolveremos la primera */
      printf("Resultado de la resolucion de nombre:\n");
      for (rp = result; rp != NULL; rp = rp->ai_next)
        {
          printf("-> ");
          imprime_extremo_conexion(rp->ai_addr, rp->ai_addrlen, tipo);
          printf("\n");
        }

      if (result == NULL)
        {                       /* No se ha devuelto ninguna direccion */
          fprintf(stderr, "No se han encontrado direcciones.\n");
          error = 1;            //Hay error
        }
      else
        {
          //Copiamos solo los campos del primer resultado que interesan.
          info->ai_family = result->ai_family;
          info->ai_socktype = result->ai_socktype;
          info->ai_protocol = result->ai_protocol;
          *info->ai_addr = *result->ai_addr;    //Copiamos contenido del puntero
          info->ai_addrlen = result->ai_addrlen;
        }

      freeaddrinfo(result);     /* Liberamos los datos */

    }
  return !error;
}

/*
  Funcion que imprime el nombre de la maquina asociada a una
  direccion de internet y el puerto de una conexion.
  Hace uso de la funcion getnameinfo.
  Parametros de entrada:
  - direccion - estructura sockaddr con informacion de un extremo del socket.
  - len - longitud de la estructura direccion
  - tipo - SOCKET_UDP o SOCKET_TCP
  Devuelve:
  - Nada
*/
void imprime_extremo_conexion(const struct sockaddr *direccion, socklen_t len,
                              int tipo)
{
  char hbufnum[NI_MAXHOST];     //cadena de la maquina (numerico)
  char hbufnombre[NI_MAXHOST];  //cadena de la maquina (nombre)
  char sbuf[NI_MAXSERV];        //cadena del servicio
  int opciones = NI_NUMERICHOST | NI_NUMERICSERV;       //Opciones para getnameinfo
  int error = 0;

  if (tipo == SOCKET_UDP)
    {
      opciones |= NI_DGRAM;
    }
  //Convertimos a cadena de caracteres
  error = getnameinfo(direccion, len, hbufnum, sizeof(hbufnum), sbuf,
                      sizeof(sbuf), opciones);
  if (error == 0)
    {
      //Obtenemos tambien el nombre asociado a esa direccion IP
      if (tipo == SOCKET_TCP_PASIVO)
        printf("Escuchando en ");
      else if (getnameinfo(direccion, len, hbufnombre, sizeof(hbufnombre),
                           NULL, 0, NI_NAMEREQD))
        //Error obteniendo el nombre
        printf("Maquina=(desconocida) ");
      else
        printf("Maquina=%s ", hbufnombre);

      //Imprimimos valores numericos.
      printf("(%s), Puerto=%s", hbufnum, sbuf);
      if (tipo == SOCKET_UDP)
        printf(" UDP");
      else
        printf(" TCP");

    }
  else
    //Mostramos informacion del error usando la funcion gai_strerror
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
}
