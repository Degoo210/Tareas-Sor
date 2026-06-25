# Tarea 4 - Sistemas Operativos y Redes  

# Caso 1: Servidor UDP

Durante la actividad se capturaron paquetes enviados por el servidor de le profesor.

## 1. Filtro utilizado en Wireshark

Para filtrar únicamente los paquetes UDP cuya IP de destino sea `255.255.255.255`, se utilizó el siguiente filtro en Wireshark:

```
udp && ip.dst == 255.255.255.255
```

Este filtro permite visualizar solamente los paquetes UDP enviados hacia la dirección broadcast limitada `255.255.255.255`.

## 2. Captura del paquete en Wireshark

Al aplicar el filtro anterior, se identificaron paquetes UDP enviados desde la IP del servidor hacia la dirección broadcast.

En particular, se observó un paquete con las siguientes características:

```text
IP origen: 192.168.0.245
IP destino: 255.255.255.255
Protocolo: UDP
Puerto origen: 40910
Puerto destino: 8888
```

**Imagen 1: Paquete UDP filtrado en Wireshark**
![Captura paquete UDP](images/imagen1.jpeg)

## 3. Tamaño del paquete completo y tamaño de la capa UDP

El paquete completo tiene un tamaño de:

```text
74 bytes
```

De estos, la capa UDP corresponde a:

```text
40 bytes
```

La diferencia entre el tamaño total del paquete y el tamaño de la capa UDP se debe a que el paquete completo incluye información de distintas capas de la red, no solamente UDP.

En este caso:

```text
Ethernet: 14 bytes
IP:       20 bytes
UDP:      40 bytes
Total:    74 bytes
```

Por lo tanto, el tamaño total del paquete incluye las cabeceras de Ethernet e IP, además del segmento UDP.

Dentro de la capa UDP, los 40 bytes se dividen en:

```text
Header UDP: 8 bytes
Datos:      32 bytes
```

**Imagen 2: Detalle del tamaño del paquete y capa UDP**

![Captura tamaño paquete UDP](images/imagen2.jpeg)

## 4. Mensaje emitido por el servidor

El mensaje emitido por el servidor UDP es:

```text
Mi numero de la suerte es: 960
```

En el paquete, el mensaje aparece dentro del campo de datos UDP. Su largo es de:

```text
32 bytes
```

Este largo considera el texto del mensaje y el salto de línea final incluido en los datos transmitidos.

**Imagen 3: Mensaje dentro del paquete UDP**

![Captura mensaje UDP](images/imagen3.jpeg)

## 5. Dirección broadcast para la red 192.168.0.0/24

Si se quisiera enviar un mensaje broadcast a todos los usuarios de la red:

```text
192.168.0.0/24
```

la IP de destino debería ser:

```text
192.168.0.255
```

Esto se debe a que una máscara `/24` indica que los primeros 24 bits corresponden a la red y los últimos 8 bits corresponden a los hosts. Para obtener la dirección broadcast, todos los bits de host se colocan en `1`.

Por lo tanto:

```text
Red:        192.168.0.0
Máscara:    255.255.255.0
Broadcast: 192.168.0.255
```

# Caso 2:

2.1 User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/149.0.0.0 Safari/537.36\r\n

![User agent](images/imagen7.png)

2.2 Código de respuesta de /donuts.txt: 200 y significa que está todo "OK", es decir, es un código de aceptación a la solicitud del cliente.
    Código de respuesta de /missing.txt: 404 y representa el estado "Not found", es decir, el recurso solicitado por el cliente no existe.

![Reponse code](images/imagen8.png)
![Response code](images/imagen9.png)

2.3 Request URI: /84729098/c85hmnowu48n/x8472nm983/homero.jpg

![Direccion](images/imagen10.png)

2.4 homepage: se efectuaron 35 GET, 1 GET inicial por el HTML, y luego 33 GET adicionales debido a que se encontraron referencias a otros recursos adicionales. Por último, otro cuando se pide que volvamos al homepage.
/donuts.txt: se efectuó un solo GET debido a que es un solo texto plano.
/missing.txt: se efectuó un único GET, pues después de la solicitud el HTML responde "Not Found".
/84729098/c85hmnowu48n/x8472nm983/homero.jpg: se efectuó un solo GET debido a que es un archivo de imagen, sin recursos adicionales.
/84729098/c85hmnowu48n/x8472nm983/urgent.mp3: se efectuó un solo GET debido a que es un archivo de audio, sin recursos adicionales.
/meme: se efectuaron 15 GET, 1 GET inicial por el HTML, y luego 14 adicionales debido a que se encontraron referencias a otros recursos adicionales.

![homepage](images/imagen11.png) ![](images/imagen12.png)
![donuts](images/imagen13.png)
![missing](images/imagen14.png)
![homero.jpg](images/imagen15.png)
![urgent.mp3](images/imagen16.png)
![meme](images/imagen17.png)

2.5
homepage: 73746 bytes
homepage (2da vez): 73746 bytes
/donuts.txt: 58619 bytes
/missing.txt: 1856 bytes
/84729098/c85hmnowu48n/x8472nm983/homero.jpg: 70520 bytes
/84729098/c85hmnowu48n/x8472nm983/urgent.mp3: 632 bytes
/meme: 3506 bytes
TOTAL: 282625 bytes

![homepage](images/imagen18.png) 
![homepage (2da vez)](images/imagen19.png)
![donuts](images/imagen20.png)
![missing](images/imagen21.png)
![homero.jpg](images/imagen22.png)
![urgent.mp3](images/imagen23.png)
![meme](images/imagen24.png)

# Caso 3: Protocolos de mensajes

En este caso se analizaron distintos protocolos en la red le profesor usando Wireshark. En particular, se observaron paquetes asociados a `ICMP`, `ARP` y `DHCP`.

## 3.1 Ping al default gateway

Para esta parte se realizó un `ping` desde el computador hacia un dispositivo de la red local. En la captura de Wireshark se observaron paquetes del protocolo:

```text
ICMP
```

En Wireshark, los paquetes generados por el comando `ping` aparecen como mensajes ICMP de tipo `Echo request` y `Echo reply`.

En la captura se observa comunicación ICMP entre:

```text
Origen: 192.168.0.110
Destino: 192.168.0.245
```

Los paquetes capturados corresponden a solicitudes y respuestas de ping. Por ejemplo:

```text
192.168.0.110 → 192.168.0.245    ICMP Echo request
192.168.0.245 → 192.168.0.110    ICMP Echo reply
```

El protocolo `ICMP`, o `Internet Control Message Protocol`, se utiliza principalmente para enviar mensajes de control y diagnóstico dentro de redes IP. No se utiliza para transportar datos de aplicaciones como HTTP o UDP, sino para informar estados, errores o verificar conectividad entre dispositivos.

El comando `ping` utiliza ICMP enviando un mensaje `Echo request` al destino. Si el destino está disponible y puede responder, devuelve un mensaje `Echo reply`. Esto permite verificar si existe conectividad entre dos dispositivos y medir el tiempo de respuesta.

**Imagen 4:** Captura de paquetes ICMP.

![Captura ICMP](imagen4.jpeg)

## 3.2 Protocolo ARP

Al observar la captura sin aplicar filtros, o aplicando el filtro:

```wireshark
arp
```

se identificaron paquetes del protocolo `ARP`.

ARP significa `Address Resolution Protocol`. Este protocolo se utiliza para relacionar una dirección IP con una dirección MAC dentro de una red local. Esto es necesario porque, aunque los dispositivos se comuniquen usando direcciones IP a nivel de red, para enviar una trama dentro de una red Ethernet se necesita conocer la dirección MAC de destino.

El funcionamiento general de ARP es el siguiente:

1. Un dispositivo quiere enviar datos a una IP dentro de la red local.
2. Si no conoce la dirección MAC asociada a esa IP, envía una solicitud ARP en broadcast.
3. La solicitud pregunta algo como: “¿Quién tiene esta dirección IP?”.
4. El dispositivo que posee esa IP responde indicando su dirección MAC.
5. El emisor guarda esa relación IP-MAC en su tabla ARP para futuras comunicaciones.

En la captura se observa, por ejemplo, una consulta ARP desde un equipo de la red preguntando por la dirección MAC asociada a otra IP:

```text
Who has 192.168.0.245? Tell 192.168.0.110
```

También se pueden observar respuestas ARP, donde el dispositivo consultado informa su dirección MAC correspondiente.

Un ejemplo de este proceso es:

```text
192.168.0.110 pregunta por 192.168.0.245
192.168.0.245 responde con su dirección MAC
```

Esto evidencia que ARP permite que los dispositivos de la red local puedan encontrar la dirección física necesaria para comunicarse mediante Ethernet.

**Imagen 5:** Captura de paquetes ARP en Wireshark. Se observa una solicitud ARP del tipo `Who has...? Tell...`, donde un dispositivo pregunta por la dirección MAC asociada a una dirección IP. También se observa la respuesta correspondiente, donde el dispositivo consultado informa su dirección MAC.

![Captura ARP](imagen5.jpeg)

## 3.3 Verificación de DHCP en la red local

Para esta parte se realizó una nueva captura en una red local conectada a internet, correspondiente a una red doméstica, con el fin de observar el proceso DHCP al renovar la configuración de red del equipo.

Para verificar si el servicio DHCP está habilitado en la red local, se realizó una captura en Wireshark mientras el equipo renovaba su configuración de red. Luego, se aplicó el siguiente filtro:

```wireshark
dhcp
```

En la captura se observaron paquetes correspondientes al proceso DHCP, incluyendo:

```text
DHCP Release
DHCP Discover
DHCP Offer
DHCP Request
DHCP ACK
```

Esto permite concluir que el servicio DHCP sí está habilitado y funcionando en la red local.

DHCP significa `Dynamic Host Configuration Protocol`. Este protocolo permite que un dispositivo obtenga automáticamente su configuración de red, como dirección IP, máscara de subred, gateway y servidores DNS, sin tener que configurar estos valores manualmente.

El proceso observado corresponde al intercambio típico de DHCP, conocido como DORA:

```text
Discover → Offer → Request → ACK
```

Este proceso funciona de la siguiente manera:

1. `DHCP Discover`: el cliente envía un mensaje en broadcast para buscar servidores DHCP disponibles.
2. `DHCP Offer`: el servidor DHCP responde ofreciendo una configuración de red al cliente.
3. `DHCP Request`: el cliente solicita formalmente la configuración ofrecida.
4. `DHCP ACK`: el servidor confirma la asignación de la configuración de red.

En la captura se observa que el cliente utiliza inicialmente la dirección `0.0.0.0` y envía mensajes hacia `255.255.255.255`, ya que todavía no tiene una dirección IP confirmada. Luego, el servidor DHCP de la red responde desde `192.168.1.1`, asignando la configuración correspondiente al cliente.

En particular, se observa que el servidor DHCP es:

```text
192.168.1.1
```

y que el cliente recibe la dirección:

```text
192.168.1.105
```

Por lo tanto, a partir de la presencia de los mensajes `DHCP Discover`, `DHCP Offer`, `DHCP Request` y `DHCP ACK`, se puede afirmar que DHCP está habilitado y funcionando correctamente en la red local.

**Imagen 6:** Captura de paquetes DHCP en Wireshark. Se observan mensajes `DHCP Discover`, `DHCP Offer`, `DHCP Request` y `DHCP ACK`, lo que confirma que el servicio DHCP está habilitado y funcionando en la red local.

![Captura DHCP](imagen6.jpeg)