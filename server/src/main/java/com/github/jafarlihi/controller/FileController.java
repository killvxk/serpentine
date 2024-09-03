package com.github.jafarlihi.controller;

import com.github.jafarlihi.service.ClientListenerService;
import com.github.jafarlihi.util.Client;
import com.github.jafarlihi.util.RequestType;
import org.json.JSONObject;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.context.request.async.DeferredResult;

import java.io.IOException;
import java.io.PrintWriter;

@RestController
public class FileController {

    @PostMapping("/file/{client}")
    public DeferredResult<String> getFile(@PathVariable("client") String clientName, @RequestBody String body) {
        DeferredResult<String> result = new DeferredResult<>();
        Client client = ClientListenerService.getClient(clientName);
        if (client == null) {
            result.setResult(new JSONObject().put("error", "Requested client was not found").toString());
            return result;
        }
        Long requestID = client.getRequestID();
        JSONObject request = new JSONObject();
        request.put("id", requestID);
        request.put("type", RequestType.GET_FILE.getValue());
        request.put("filename", new JSONObject(body).getString("filename"));

        Runnable task = () -> {
            client.observable.subscribe((response) -> {
                JSONObject responseJson = new JSONObject(response);
                if (responseJson.has("id") && responseJson.getLong("id") == requestID) {
                    result.setResult(response);
                    Thread.currentThread().interrupt();
                }
            }, (error) -> {});
        };
        Thread thread = new Thread(task);
        thread.start();

        try {
            PrintWriter out = new PrintWriter(client.socket.getOutputStream(), true);
            out.println(request.toString());
           } catch (IOException ex) {
            result.setResult(new JSONObject().put("error", "Failed to write to the socket, exception: " + ex).toString());
            return result;
        }

        return result;
    }

    @PutMapping("/file/{client}")
    public DeferredResult<String> putFile(@PathVariable("client") String clientName, @RequestBody String body) {
        DeferredResult<String> result = new DeferredResult<>();
        Client client = ClientListenerService.getClient(clientName);
        if (client == null) {
            result.setResult(new JSONObject().put("error", "Requested client was not found").toString());
            return result;
        }
        Long requestID = client.getRequestID();
        JSONObject request = new JSONObject();
        request.put("id", requestID);
        request.put("type", RequestType.PUT_FILE.getValue());
        request.put("filename", new JSONObject(body).getString("filename"));
        request.put("file", new JSONObject(body).getString("file"));
        try {
            PrintWriter out = new PrintWriter(client.socket.getOutputStream(), true);
            out.println(request.toString());
        } catch (IOException ex) {
            result.setResult(new JSONObject().put("error", "Failed to write to the socket, exception: " + ex).toString());
            return result;
        }
        client.observable.subscribe((response) -> {
            JSONObject responseJson = new JSONObject(response);
            if (responseJson.has("id") && responseJson.getLong("id") == requestID) {
                result.setResult(response);
                Thread.currentThread().interrupt();
            }
        }, (error) -> {});
        return result;
    }
}
