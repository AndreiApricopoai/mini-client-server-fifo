# mini-client-server-fifo

These programs simulate a server/client protocol using fifo:

  -We send commands to the server.c from client.c using fifo channels.
  -The server reads the commands and sends a response back.
  -We make use of child processes to handle each request.
