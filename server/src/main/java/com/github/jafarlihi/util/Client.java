package com.github.jafarlihi.util;

import io.reactivex.rxjava3.core.Observable;

import java.net.Socket;
import java.time.LocalDateTime;

public class Client {

    public Socket socket;
    public Observable<String> observable;
    public String computerName;
    public String stubName;
    public String activeWindowTitle;
    public LocalDateTime lastSeen;
    private Long requestID;

    public Client(Socket socket, Observable<String> observable) {
        this.socket = socket;
        this.observable = observable;
        this.lastSeen = LocalDateTime.now();
        this.requestID = 0L;
    }

    public Long getRequestID() {
        return requestID++;
    }
}
