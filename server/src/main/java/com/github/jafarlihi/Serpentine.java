package com.github.jafarlihi;

import com.github.jafarlihi.service.ClientListenerService;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

@SpringBootApplication
public class Serpentine {

    private static final Logger logger = LogManager.getLogger();

    public static void main(String[] args) {
        if (args.length != 2) {
            logger.error("Two arguments are required, first one specifying client listening port, second one specifying controlling REST API port");
            System.exit(1);
        }
        ClientListenerService.listen(Integer.parseInt(args[0]));
        System.setProperty("server.port", args[1]);
        SpringApplication.run(Serpentine.class, args);
    }
}
