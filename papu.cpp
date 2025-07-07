#include <bits/stdc++.h>

using namespace std;

void dfs_topo(
    vector<vector<int>> &AL, 
    int u, 
    vector<bool> &visited, 
    vector<int> &orders, 
    int &currLabel
){
    visited[u] = true;
    
    for(auto v: AL[u]){
        if(visited[v]){
            continue;
        }
        dfs_topo(AL, v, visited, orders, currLabel);
    }
    
    orders[u] = currLabel--;
}

vector<int> topo_sort(
    vector<vector<int>> &AL
){
    vector<int> orders(AL.size(), -1);
    vector<bool> visited(AL.size(), false);
    
    int currLabel = AL.size();
    
    for(int i = 0; i < AL.size(); i++){
        if(visited[i]){
            continue;
        }
        dfs_topo(AL, i, visited, orders, currLabel);
    }
    return orders;
}

int main() {

    vector<vector<int>> AL = {
        {1, 5},
        {},
        {4, 5, 8},
        {4, 5, 6, 7, 8},
        {5, 7,  8},
        {},
        {},
        {},
        {}
    };
    
    vector<int> orders = topo_sort(AL);
    
    
    cout << "\nOrden topologico:\n";
    for (int i = 0; i < orders.size(); i++) {
        cout << i << "=>" << orders[i] << "\n";
    }

    return 0;
}
