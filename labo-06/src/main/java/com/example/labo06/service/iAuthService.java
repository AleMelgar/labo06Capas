package com.example.labo06.service;

import com.example.labo06.domain.dto.CreateUserDTO;
import com.example.labo06.domain.dto.KeycloakTokenResponse;

public interface iAuthService {
    KeycloakTokenResponse register(CreateUserDTO user) throws Exception;
    KeycloakTokenResponse login(String username, String password);
}
