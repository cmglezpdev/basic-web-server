Indice
- [Descripción](#decripcion)
- [Detalles de la implementación](#detalles-de-implementación)
- [Manejo de solicitud](#manejo-de-la-solicitud)
- [Templates](#templates)


# Descripción

Este proyecto tiene como objetivo implementar un pequeño servidor web que permita la transferencia de archivos desde un servidor a un cliente. Al ejecutarlo se debe especificar el puerto y el directorio a mostrar en el servidor. Además el servidor puede manejar múltiples clientes de forma concurrente. Un cliente puede realizar las siguientes acciones:

1. Listar los archivos del directorio actual, con sus respectivos nombre, tamaño y fecha de modificación, y ordenarlos por cualquiera de estos campos.
2. Navegar por el file system del servidor.
3. Descargar los archivos.

<hr/>

## Detalles de Implementación

El flujo empieza en la función `main`, en donde se toma el puerto y el directorio raiz de los argumentos del programa. En caso de que falte uno de los dos muestra un error, sino crea un instancia del servidor y lo ejecuta. 

Aquí se abstrajo el servidor a un `struct Server` que tiene todos los parámetros para poder ejecutarse. El servidor se encuentra en el fichero `server.c` y hace uso del constructor `Server_constructor`.

Al ejecutar servidor este lanza la función  `launch` que tiene el flujo general para cuando se realiza una petición.

<hr />

## Manejo de la Solicitud

Al realizar una solicitud al servidor (en el método `launch`) se crea un proceso hijo el cual manejará la petición en background. Luego del socket del cliente se extrae y se analiza la petición.

De la petición se extrae el tipo de petición __(GET, POST, PUT, DELETE, ...)__ y la url. Se analiza la url y se asocia como ruta base (`'/'`) el directorio raíz dado. Luego se parsea la url eliminando los __"%20"__ y si no hay ningún error se llama a la función `http_handler` que realiza las acciones correspondientes con la petición. Si `http_handler` tiene algún error este devuelve un `Bad Request` al cliente. 

La función `http_handler` toma la __url__ y analiza si corresponde a un archivo o a una carpeta. 
Si la url corresponde a un archivo entonces usando el método `send_file` de nuestro módulo `server.c` y envía el archivo al cliente en forma de descarga.
Si la url corresponde a una carpeta entonces se lee de la carpeta `templates` los templates para construir el html de la páguina, y se envía esta al cliente con un status `OK 200`.

<hr />

## Templates

Hay dos templates:
1. `page.html`: contiene todo el html, css y javascript para la páguina. Este tiene un comentario __TABLE_HEAR__, el cual es remplazado por la lista de archivos y ficheros del directorio correspondiente a la url. El javascript permite poder ordenar los elementos por nombre, fecha de modificación y tamaño, ademas de poder hacer reverse al orden.
2. `item.html`: es una representación de un item en la tabla de los archivos y carpetas de un url. Para construir un archivo se usa el contenido de `item.html` y se sustituyen los `%s` que aparecen por la url, el tipo de archivo, nombre del archivo su tamaño y fecha de modificación respectivamente.

La paguina de forma general se construye dentro del módulo `build_page.c`, específicamente a partir de la función `build_page`.