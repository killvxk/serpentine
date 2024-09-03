# serpentine

serpentine is a Windows RAT (Remote Administration Tool) that lets you interact with the clients using a multiplatform RESTful C2 server.

Functionalities:

- Startup

- Get files

- Put files

- Keylogger (Just get `%APPDATA%/svchost/log` file)

- Reverse shell (Using `netcat` as a client, listen on a port `nc -l -p 5555` and request a reverse shell connection to that port)

- Reverse proxy (Using [`revp`](https://github.com/jafarlihi/revp), upload `revp` to the client and execute it with a reverse shell)

- Take screenshots

- Remote desktop (Using `qtserpentine` frontend) [Work in progress, sending input is still in development]

### Dissection
`client` directory holds the Windows portion of the RAT made with C++ and Boost.

`server` holds the RESTful server portion made with Java and Spring.

`frontend` can hold any number of frontends that consume the RESTful API, currently holds `goserpentine` terminal client and `qtserpentine` GUI client.

#### Building & running
Build `server` with `mvn package` and then run the JAR with 2 arguments, first being the client-listening port and second the port RESTful API will be served at. Example: `java -jar target/serpentine-0.1.0.jar 2222 8080`

Build `client` with Visual Studio and also include Boost libraries: https://www.boost.org/doc/libs/1_73_0/more/getting_started/windows.html
Change `client` settings in `config.h`.

Build `goserpentine` `frontend` with `go build` and run the resulting executable to see the list of options.

Build `qtserpentine` by installing Qt Creator and building through that. Provide `QTSERPENTINE_API_ADDRESS` environment variable that points to the API server.

#### API
(To learn more about the endpoints and input parameters check the controllers in server code)

`/client` GET --> Returns list of currently connected clients (clients that pinged in the last 10 seconds)

`/client` POST --> Changes name associated with a client

`/file/{client}` POST --> Fetches a file from a client

`/file/{client}` PUT --> Uploads a file to a client

`/shell/{client}` POST --> Makes a client initiate a reverse shell connection

`/desktop/{client}` GET --> Takes and fetches a screenshot
