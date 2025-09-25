#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <climits>  
#include <random>
using namespace std;
using namespace std::chrono;

// Leer archivo y construir lista de adyacencia
vector<vector<int>> read_file(const string& file_path) {
    ifstream file(file_path);
    string line;
    int N = 0;

    if (getline(file, line)) {
        N = stoi(line);
    }

    vector<vector<int>> adjList(N);
    //cout << "N: " << N << endl;

    while (getline(file, line)) {
        int x, y;
        istringstream iss(line);
        iss >> x >> y;
        adjList[x].push_back(y);
        adjList[y].push_back(x); // grafo no dirigido
    }

    file.close();
    return adjList;
}

// Crear vector de grados iniciales
vector<int> create_contador(const vector<vector<int>>& adjList) {
    int N = adjList.size();
    vector<int> contador(N);
    for (int i = 0; i < N; ++i) {
        contador[i] = adjList[i].size();
    }
    return contador;
}

// Eliminar nodo y actualizar contadores
void remove_node_and_update(int node, const vector<vector<int>>& adjList, vector<int>& contador) {
    contador[node] = -1; // Nodo retirado

    // Bloquear vecinos del nodo
    for (int neighbor : adjList[node]) {
        if (contador[neighbor] >= 0) {
            contador[neighbor] = -2;
        }
    }

    // Actualizar contadores
    for (int i = 0; i < adjList.size(); ++i) {
        if (contador[i] >= 0) {
            int decrement = 0;
            for (int neighbor : adjList[i]) {
                if (contador[neighbor] == -1 || contador[neighbor] == -2) {
                    ++decrement;
                }
            }
            contador[i] -= decrement;
            if (contador[i] < 0) contador[i] = 0;
        }
    }
}

// Heurística para MISP
vector<int> misp_heuristic(const vector<vector<int>>& adjList) {
    int N = adjList.size();
    vector<int> contador = create_contador(adjList);
    vector<int> solution;

    while (true) {
        int min_deg = INT_MAX;
        int min_node = -1;

        // Buscar nodo activo con menor grado
        for (int i = 0; i < N; ++i) {
            if (contador[i] >= 0 && contador[i] < min_deg) {
                min_deg = contador[i];
                min_node = i;
            }
        }

        if (min_node == -1) break;

        solution.push_back(min_node);
        remove_node_and_update(min_node, adjList, contador);
    }

    return solution;
}


// Heurística para MISP RANDOM
vector<int> misp_heuristic_randomized(const vector<vector<int>>& adjList, float UMBRAL_IN, int k) {
    int N = adjList.size();

    random_device rd;
    mt19937 gen(rd());

    float UMBRAL = UMBRAL_IN;

    vector<int> contador = create_contador(adjList);
    vector<int> solution;

    while (true) {
        // encontrar los k nodos de menor grado
        vector<int> k_nodes(k, -1);     // guarda nodos
        vector<int> k_degs(k, INT_MAX); // guarda grados de esos nodos

        for (int i = 0; i < N; ++i) {
            if (contador[i] < 0) continue; // nodo eliminado

            int d = contador[i];

            // ver si este nodo entra en el "top k"
            for (int j = 0; j < k; ++j) {
                if (d < k_degs[j]) {
                    for (int m = k - 1; m > j; --m) {
                        k_degs[m] = k_degs[m - 1];
                        k_nodes[m] = k_nodes[m - 1];
                    }
                    k_degs[j] = d;
                    k_nodes[j] = i;
                    break;
                }
            }
        }

        if (k_nodes[0] == -1) break; // no quedan nodos

        // factor aleatorio
        double delta = (double) gen() / gen.max();

        int chosen;
        if (delta > UMBRAL) {
            int idx = gen() % k;
            while (k_nodes[idx] == -1) { // por si hay menos de k nodos disponibles
                idx = gen() % k;
            }
            chosen = k_nodes[idx];
        } else {
            chosen = k_nodes[0]; // decision greedy
        }

        solution.push_back(chosen);

        
        remove_node_and_update(chosen, adjList, contador);
    }

    return solution;
}

























// Validar que la solución es un conjunto independiente
bool validate_independent_set(const vector<vector<int>>& adjList, const vector<int>& solution) {
    vector<bool> in_solution(adjList.size(), false);
    for (int node : solution) {
        in_solution[node] = true;
    }

    // Revisar que ningún par de nodos de la solución esté conectado
    for (int node : solution) {
        for (int neighbor : adjList[node]) {
            if (in_solution[neighbor]) {
                cout << "Error: nodo " << node << " y vecino " << neighbor 
                     << " estan en solución." << endl;
                return false;
            }
        }
    }

    return true;
}

void RunTxtFile(string txt) {
    vector<float> density = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
    int i_density = 0;

    vector<string> file_paths;
    ifstream file(txt);
    string path;
    while (getline(file, path)) {
        if (!path.empty())
            file_paths.push_back(path);
    }
    file.close();

    vector<float> time(30);
    vector<int> result(30);

    float time_med = 0;
    float result_med = 0;

    for (int i = 0; i < file_paths.size(); i++) {
        vector<vector<int>> adjList = read_file(file_paths[i]);

        auto start = high_resolution_clock::now();
        result[i % 30] = misp_heuristic(adjList).size();
        auto end = high_resolution_clock::now();

        time[i % 30] = duration_cast<milliseconds>(end - start).count();

        
        // Cada 30 ejecuciones
        if ((i + 1) % 30 == 0) {
            

            time_med = 0;
            result_med = 0;
            for (int j = 0; j < 30; j++) {
                time_med += time[j];
                result_med += result[j];
            }
            time_med /= 30;
            result_med /= 30;

            cout << "== Promedio de 30 ejecuciones == " << density[i_density] << endl;
            cout << "Time MED: " << time_med << " ms" << endl;
            cout << "Result MED: " << result_med << endl;

            ofstream log("log_" + txt + ".txt", ios::app);
            log << density[i_density] << " | Time MED: " << time_med << " ms | Result MED: " << result_med << endl;
            log.close();
            i_density++;
        }
    }
}
int main() {

    
    RunTxtFile("grafos_1000.txt");
    return 0;
}
