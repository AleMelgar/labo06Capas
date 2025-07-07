#include <bits/stdc++.h>
using namespace std;

vector<int> dijkstra_quad(vector<vector<pair<int, int>>> &AL, int start) {
    int n = AL.size();
    vector<int> dist(n, INT_MAX);      // distancias mínimas desde el nodo start
    vector<bool> visited(n, false);    // nodos ya finalizados

    dist[start] = 0;

    for (int i = 0; i < n; i++) {
        // Buscar el nodo no visitado con menor distancia
        int u = -1;
        for (int j = 0; j < n; j++) {
            if (!visited[j] && (u == -1 || dist[j] < dist[u])) {
                u = j;
            }
        }

        // Si no queda ningún nodo alcanzable, salimos
        if (dist[u] == INT_MAX) break;

        visited[u] = true;

        // Relajar vecinos
        for (auto [v, peso] : AL[u]) {
            if (dist[v] > dist[u] + peso) {
                dist[v] = dist[u] + peso;
            }
        }
    }

    return dist;
}

int main() {
    // Grafo dirigido y ponderado
    // AL[i] = vector de {vecino, peso}
    vector<vector<pair<int, int>>> AL = {
        {{1, 4}},                   // 0 → 1 (4)
        {},                         // 1
        {{0, 9}, {1, 11}},          // 2 → 0 (9), 2 → 1 (11)
        {{2, 14}, {4, 18}},         // 3 → 2 (14), 3 → 4 (18)
        {{1, 14}, {3, 18}}          // 4 → 1 (14), 4 → 3 (18)
    };

    int start = 3; // nodo origen
    vector<int> dist = dijkstra_quad(AL, start);

    cout << "Distancias más cortas desde el nodo " << start << ":\n";
    for (int i = 0; i < dist.size(); i++) {
        if (dist[i] == INT_MAX)
            cout << i << ": INF\n";
        else
            cout << i << ": " << dist[i] << "\n";
    }

    return 0;
}
