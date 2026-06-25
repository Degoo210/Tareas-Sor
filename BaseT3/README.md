## RESPUESTAS

1.1) DNS, TCP y finalmente SMTP.

1.2) DNS: Dado que, antes de enviar un correo, la PC de Bart solo conoce el dominio mail.estado.com mas no la dirección ip del servidor de correo central, el DNS resuelve dicho dominio a la ip 10.0.10.4 que es la del servidor de correo central. Gracias a este protocolo el paquete podrá ser dirigido a la dirección especificada.

SMTP: Este protocolo es el encargado de indicar el emisor del mensaje (Bart) y receptor del mesaje (Milhouse) y de transferir el mesaje hacia el servidor de correo central.

2.1) No me aparece el tamaño exacto explícitamente, lo que sí aparece es: DATA (VARIABLE LENGTH). Pero si me baso en la medida que aparece justo arriba (lo que parece una regla) serían 4 bytes (32 bits).
DST IP: 10.0.10.3

2.2) DNS actúa primero. El navegador de Bart solo conoce el dominio www.ellimonero.com pero no sabe a qué ip conectarse. El DNS traduce dicho dominio a la ip del servidor web. Sin esta solución, el navegador no sabría a dónde enviar la petición.

TCP actúa segundo. Una vez que se conoce la ip del servidor web, TCP establece una conexión confiable con el servidor web. TCP garantiza que los datos lleguen completos y en orden. Sin esta conexión establecida, HTTP no puede operar.

HTTP actúa último. Con la ip de destino y una conexión establecida, HTTP envía la petición al servidor solicitando la página web. El servidor responde con el código HTML que el navegador muestra al PC de Bart.

En resumen, los tres protocolos actúan en cadena y cada uno es prerequisito del siguiente.

2.3) Ambos pc escriben el mismo dominio www.ellimonero.com, pero cada pc consulta un DNS distinto. Bart usa el DNS de Springfield (10.0.10.2) que resuelve el dominio a 10.0.10.3, mientras que el habitante de Shelbyville usa el DNS de Shelbyville (10.0.20.2) que resuelve el mismo dominio a 10.0.20.3. Así, el DNS local de cada usuario actúa como punto de control que deriva a cada usuario a un servidor físico distinto, en este caso a servidores web distintos.

2.4) El paquete es destruido en layer 3, es decir, en la capa de red del modelo OSI.