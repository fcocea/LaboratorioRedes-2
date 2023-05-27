# Servidor HTTP simple 
## Introducción
El proyecto consiste en un servidor HTTP simple que permite la descarga de archivos, como en cualquier servidor web. Soportando únicamente el método `GET`, el servidor es capaz de responder a las solicitudes de los clientes, enviando el archivo solicitado en caso de que exista, o un mensaje de error en caso contrario.

Donde las cabeceras de las solicitudes no son de importancia, el servidor no las procesa, y simplemente las ignora. Por otro lado, las cabeceras de las respuestas son generadas automáticamente por el servidor, entregando prioritariamente el tipo de contenido del archivo solicitado, y el largo del contenido en el caso que corresponda.

## Requisitos
Los requisitos para poder ejecutar el proyecto son los siguientes:
* CMake 3.10
* Make
* C++ 11

## Compilación
Para instalar y compilar el proyecto es necesario ejecutar los siguientes comandos:
```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
```
## Uso
Para iniciar el servidor es necesario especificar el puerto en el que se va a escuchar. El servidor se ejecuta de la siguiente manera:
```bash
$ ./server -p <puerto>
```
> Si el programa no detecta la carpeta `www` en el directorio actual, se creará la respectiva carpeta.

Una vez iniciado el servidor, se puede acceder a los archivos que se encuentran en la carpeta `www` desde un navegador web o utilizando algún servicio como [Postman](https://www.postman.com/), [Hoppscotch](https://hoppscotch.io/) o [Netcat](https://en.wikipedia.org/wiki/Netcat). Por ejemplo, si el servidor se está ejecutando en el puerto `8080`, se puede acceder a la página `index.html` desde la siguiente URL: [http://localhost:8080/](http://localhost:8080/).

### Créditos
Proyecto realizado por:
* Francisco Cea
* Oscar Castillo
* Matías Gayoso