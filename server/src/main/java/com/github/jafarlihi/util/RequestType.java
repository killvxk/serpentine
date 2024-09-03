package com.github.jafarlihi.util;

public enum RequestType {
    PING(0),
    GET_FILE(1),
    PUT_FILE(2),
    CREATE_REVERSE_SHELL_SESSION(3),
    GET_SCREENSHOT(4);

    private final int value;
    RequestType(int value) {
        this.value = value;
    }
    public int getValue() {
        return value;
    }
}
