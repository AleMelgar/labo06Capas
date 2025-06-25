package com.example.labo06.utils;

import feign.Response;

public class UserIdFromKeycloak {
    public static String getUserIdFromKeycloakResponse(Response response){
        String location = response.headers().get("Location").stream()
                .findFirst()
                .orElseThrow(() -> new RuntimeException("Location header not found in response"));
        return location.substring(location.lastIndexOf("/") + 1);
    }
}