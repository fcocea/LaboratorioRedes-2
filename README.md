# Servidor HTTP simple 
## Introducción
El proyecto consiste en un servidor HTTP simple que permite la descarga de archivos, como en cualquier servidor web. Soportando únicamente el método `GET`, el servidor es capaz de responder a las solicitudes de los clientes, enviando el archivo solicitado en caso de que exista, o un mensaje de error en caso contrario.

Donde las cabeceras de las solicitudes no son de importancia, el servidor no las procesa, y simplemente las ignora. Por otro lado, las cabeceras de las respuestas son generadas automáticamente por el servidor, entregando prioritariamente el tipo de contenido del archivo solicitado, y el largo del contenido en el caso que corresponda.

> Si la ruta del archivo solicitado no cumple de cierta forma la siguiente expresión regular: `.[a-zA-Z0-9]+$`, es decir, si no tiene una extensión, el servidor entenderá que se trata de un directorio, y buscará el archivo `index.html` dentro de dicho directorio. En caso de no encontrarlo, responderá con un mensaje de error. <details> <summary>Extra</summary>
En caso de considerarse la ruta un directorio y la solicitud no termina con `/`, el servidor realizará la redirección (302) a la misma ruta, pero con `/` al final.
</details>

El protocolo HTTPS es soportado por el servidor, permitiendo la comunicación segura entre el cliente y el servidor. Para ello, es necesario especificar el certificado y la clave privada del servidor al momento de iniciar el programa. En caso de no especificarlos, el servidor solamente aceptará conexiones HTTP.

## Requisitos
Los requisitos para poder ejecutar el proyecto son los siguientes:
* CMake 3.10
* Make
* C++ 11
* [OpenSSL](https://www.openssl.org/)

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
$ ./server -p <puerto> [-c <certificado> -k <clave>]
```
> Si el programa no detecta la carpeta `www` en el directorio actual, se creará la respectiva carpeta.

<details><summary> Generar certificado SSL </summary>

Se puede generar un certificado SSL para utilizarlo con el servidor. Para ello, es necesario tener instalado [OpenSSL](https://www.openssl.org/). Luego, se deben realizar los siguientes pasos:

- Generar el certificado de la Autoridad de Certificación (CA):

```bash
$ openssl genrsa -out CA.key -des3 2048
$ openssl req -x509 -sha256 -new -nodes \
    -days 3650 -key CA.key -out CA.pem
```
Estos comandos generarán una clave privada (`CA.key`) y un certificado de la CA (`CA.pem`) que se utilizará para firmar el certificado del servidor.

Una vez generado el certificado de la `CA`, se crea el certificado para el servidor y se firma con el certificado de la `CA`.

* Generar el certificado del servidor:
```bash
$ openssl genrsa -out <nombre>.key -des3 2048
$ openssl req -new -key <nombre>.key -out <nombre>.csr
$ openssl x509 -req -in <nombre>.csr -CA CA.pem \ 
    -CAkey CA.key -CAcreateserial -days 3650 -sha256 \
    -extfile <(echo -e "authorityKeyIdentifier = keyid,issuer\nbasicConstraints = CA:FALSE\nkeyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment\nsubjectAltName = @alt_names\n\n[alt_names]\nDNS.1 = localhost\nIP.1 = 127.0.0.1") \
    -out <nombre>.crt

```
Por último, es necesario importar el certificado `CA.pem` en el navegador web que se utilizará para acceder al servidor. Esto permitirá que el certificado sea reconocido como válido y no se muestre un mensaje de advertencia en el navegador. (Opcional)

Una vez realizado lo anterior, se puede iniciar el servidor con el certificado y su respectiva clave privada:
```bash
$ ./server -p <puerto> -c <nombre>.crt -k <nombre>.key
```

---
</details>

Una vez iniciado el servidor, se puede acceder a los archivos que se encuentran en la carpeta `www` desde un navegador web o utilizando algún servicio como [Postman](https://www.postman.com/), [Hoppscotch](https://hoppscotch.io/) o [Netcat](https://en.wikipedia.org/wiki/Netcat). Por ejemplo, si el servidor se está ejecutando en el puerto `8080`, se puede acceder a la página `index.html` desde la siguiente URL: [http://localhost:8080/](http://localhost:8080/).

### Créditos
Proyecto realizado por:
* Francisco Cea
* Oscar Castillo
* Matías Gayoso