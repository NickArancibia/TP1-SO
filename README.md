# TP1 SO - G16
El trabajo consiste en tres programas que se comunican entre sí por medio de mecanismos de IPC para procesar múltiples archivos recibidos por argumento y devolver el md5 de cada uno, junto con el pid de cada uno de los procesos que lo computen.
A continuación se detallan instrucciones de compilación y ejecución para el proyecto.

## Compilación
Para la compilación del trabajo se requiere de un contenedor de docker. 
### Instalación
Para la instalación de docker consulta la [página oficial](https://www.docker.com/get-started/).
### Descargar la imágen
La imágen de docker que utilizaremos podemos descargarla con:
````
docker pull agodio/itba-so-multi-platform:3.0
````
Y corriendo el comando ```docker images``` deberíamos poder ser capaces de ver la imágen una vez descargada.
### Ejecución del contenedor
Para ejecutar el contenedor de docker utilizaremos el siguiente comando:
````
docker run -v "${PWD}:/root" --privileged -ti agodio/itba-so-multi-platform:3.0
````
Una vez dentro podemos acceder a los archivos del proyecto ejecutando ```cd root```.
### Compilación de los archivos
Utilizaremos makefile para compilar los archivos fuente (Los cuales se encuentran en la carpeta src). Para ello podemos ejecutar ```make``` o ```make all```. Una vez compilados y linkeditados, deberíamos ser capaces de ver los ejecutables en el actual directorio.
Para borrar los archivos ejecutables generados podemos utilizar ```make clean```.

## Ejecución
Todos los archivos son ejecutables, tanto slave como vista y md5. Utilizaremos los mismos para computar el md5 de diferentes archivos.
### Ejecución mediante slave
Únicamente corriendo ```./slave``` seremos capaces de ejecutar dicho programa. A continuación, se quedará esperando a que ingresemos por entrada estándar el path de los archivos a procesar. Podemos ingresar uno o varios separados entre sí por un espacio.
Al finalizar, se imprimirá en pantalla el resultado de md5sum por cada uno de los archivos.
### Ejecución de md5 y vista mediante piping
Ejecutando ```./md5 <files> | ./vista```, donde en _<files>_ debemos indicar el path de los archivos a procesar, podremos conectar ambos procesos y, a medida que cada uno de los archivos sea procesado, veremos en pantalla el resultado, junto al PID del proceso que se encargó de ello.
Además, al finalizar la ejecución, veremos en el directorio actual un archivo _'results.txt'_ con la información previamente visualizada en pantalla.
### Ejecución de md5 y vista separados
Para esto debemos abrir una nueva terminal y ejecutar ```docker ps```. Allí deberíamos ver un contenedor llamado SO que estamos ejecutando actualmente, junto al ID del mismo. Para ejecutar un nuevo contenedor conectado a éste corremos el siguiente comando:
````
docker exec -ti <container ID> bash
````
Donde en container ID indicaremos el ID de nuestro contenedor SO obtenido anteriormente con ```docker ps```. Luego podemos movernos a root y seguir con la ejecución.

Ahora desde una de las terminales activas ejecutaremos ```./md5 <files>``` con los archivos a procesar y veremos que sale un mensaje en pantalla que dice _sharedMemory_.
En la otra terminal ejecutamos ```./vista sharedMemory``` y, si se hace antes de que md5 termine de ejecutar, deberíamos ver que el programa vista se queda esperando hasta que comienza a imprimir la información que md5 va escribiendo en la memoria.
Adicionalmente, al finalizar la ejecución también encontraremos el archivo _'results.txt'_ en nuestro directorio.

La otra opción es correr el md5 en background, para esto utilizar el comando ```./md5 <files> &``` y una vez que se imprimió el nombre de la shared memory se puede correr vista ejecutando ```./vista sharedMemory``` siempre y cuando el proceso md5 no haya terminado.
## Consideraciones generales
* Tanto la cantidad de hijos como la cantidad inicial de archivos que cada uno recibe están parametrizados en el código. Por las funciones utilizadas, la cantidad máxima de hijos que se pueden crear es 255.
* El peso de los archivos recibidos como argumento no debe superar 2MB por una limitación de la shell.
* La longitud del path de los archivos envíados por parámetro no debe superar los 100 caracteres.
