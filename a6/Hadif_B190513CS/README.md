
# FTAP (File Transfer Application Program)

## PROCEDURE:

1. From directory `FTAPServer`, compile and run the server program

   ```console
   $ gcc FTAPServer.c -o FTAPServer -pthread
   $ ./FTAPServer
   ```

2. From directory `FTAPClient`,compile and run the client program

    ```console
    $ gcc FTAPClient.c -o FTAPClient -pthread
    $ ./FTAPClient
    ```

3. Use `START` command in the client to establish a TCP connection with the server.
4. Next, use the `USERN` command to send the username to the server. Subsequently use the `PASSWD` command to send the password to the server.

   ```console
   START
   200 OK Connection is set-up
   USERN admin
   300 Correct Username; Need password
   PASSWD @dministr@tor
   305 User Authenticated with password
   ```

5. After successfully logging in you can use the commands provided by the FTAP, namely
    * `CREATEFILE <filename.txt>`
    * `LISTDIR`
    * `STOREFILE <filename>`
    * `GETFILE <filename>`

6. Use `QUIT` command to quit the application.
