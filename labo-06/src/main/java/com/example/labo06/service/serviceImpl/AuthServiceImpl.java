package com.example.labo06.service.serviceImpl;

import com.example.labo06.client.keycloak.iKeycloakAdminClient;
import com.example.labo06.client.keycloak.iKeycloakAuthClient;
import com.example.labo06.config.keycloak.KeycloakProperties;
import com.example.labo06.domain.dto.CreateUserDTO;
import com.example.labo06.domain.dto.KeycloakTokenResponse;
import com.example.labo06.service.iAuthService;
import feign.Response;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;

import java.nio.charset.StandardCharsets;

import static com.example.labo06.utils.UserIdFromKeycloak.getUserIdFromKeycloakResponse;
import static com.example.labo06.utils.mappers.GeneralMappers.createUserDtoToMap;
import static com.example.labo06.utils.mappers.GeneralMappers.loginToFormData;

@Service
@RequiredArgsConstructor
public class AuthServiceImpl implements iAuthService {

    private final iKeycloakAdminClient keycloakAdminClient;
    private final iKeycloakAuthClient keycloakAuthClient;
    private final KeycloakProperties keycloakProperties;

    @Override
    public KeycloakTokenResponse register(CreateUserDTO user) throws Exception {
        Response response = keycloakAdminClient.createUser(createUserDtoToMap(user));
        if (response.status() != 201) throw new Exception("Failed to create user: " + new String(response.body().asInputStream().readAllBytes(), StandardCharsets.UTF_8));
        String userId = getUserIdFromKeycloakResponse(response);
        return login(user.getUserName(), user.getPassword());
    }

    @Override
    public KeycloakTokenResponse login(String username, String password) {
        return keycloakAuthClient.getToken(loginToFormData(username, password, keycloakProperties.getClientId(), keycloakProperties.getClientSecret()));
    }
}