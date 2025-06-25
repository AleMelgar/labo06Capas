package com.example.labo06.config.keycloak;

import com.example.labo06.client.keycloak.iKeycloakAuthClient;
import com.example.labo06.domain.dto.KeycloakTokenResponse;
import feign.RequestInterceptor;
import lombok.RequiredArgsConstructor;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;

@Configuration
@RequiredArgsConstructor
public class KeycloakFeignInterceptorConfig {
    private final iKeycloakAuthClient keycloakAuthClient;
    private final KeycloakProperties keycloakProperties;

    @Bean
    public RequestInterceptor getKeycloakAuthInterceptor() {
        return requestTemplate -> {
            MultiValueMap<String, String> form = new LinkedMultiValueMap<>();
            form.add("client_id", keycloakProperties.getClientId());
            form.add("client_secret", keycloakProperties.getClientSecret());
            form.add("grant_type", "client_credentials");
            KeycloakTokenResponse token = keycloakAuthClient.getToken(form);
            requestTemplate.header("Authorization", "Bearer " + token.getAccessToken());
        };
    }
}