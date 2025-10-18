#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <climits>  
#include <random>
#include <utility>
#include <algorithm>
#include <unordered_set>
#include <cmath>
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


pair<vector<int>, vector<int>> misp_heuristic_with_contador(const vector<vector<int>>& adjList) {
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

    return make_pair(solution, contador);
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
            if (contador[i] < 0) continue; // nodo no valido

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


pair<vector<int>, vector<int>> misp_heuristic_randomized_with_counter(const vector<vector<int>>& adjList, float UMBRAL_IN, int k) {
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
            if (contador[i] < 0) continue; // nodo no valido

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

    return make_pair(solution, contador);
}














vector<int> misp_local_search(const vector<vector<int>>& adjList, const vector<int>& solucion_bad, vector<int>& contador) {
    vector<int> solution = solucion_bad;
    bool improved = true;

    while (improved) {
        improved = false;
        vector<int> solution_nueva = solution;

        // Probar retirar un nodo de la solución
        for (int i = 0; i < solution.size(); ++i) {
            int node_to_remove = solution[i];


            vector<int> temp_solution = solution;
            vector<int> contador_temp = contador;

            temp_solution.erase(temp_solution.begin() + i);
            contador_temp[node_to_remove] = -3;  // nodo temporalmente retirado

            // Liberar vecinos bloqueados
            for (int neighbor : adjList[node_to_remove]) {
                if (contador_temp[neighbor] == -2) {
                    contador_temp[neighbor] = adjList[neighbor].size(); // desbloquear
                }
            }

            // intentar agregar vecinos
            for (int neighbor : adjList[node_to_remove]) {
                if (contador_temp[neighbor] >= 0) {
                    bool can_add = true;
                    for (int temp_node : temp_solution) {
                        if (find(adjList[neighbor].begin(), adjList[neighbor].end(), temp_node) != adjList[neighbor].end()) {
                            can_add = false;
                            break;
                        }
                    }
                    if (can_add) {
                        temp_solution.push_back(neighbor);
                        remove_node_and_update(neighbor, adjList, contador_temp);
                    }
                }
            }

            // condición de aceptación
            if (temp_solution.size() > solution_nueva.size()) {
                solution_nueva = temp_solution;
                contador = contador_temp;
                improved = true;
                break;  // reiniciamos búsqueda
            }
        }

        solution = solution_nueva;
    }

    return solution;
}





pair<vector<int>, vector<int>> misp_local_search_opti(const vector<vector<int>>& adjList,
                              const vector<int>& solucion_bad,
                              vector<int>& contador) {
    vector<int> solution = solucion_bad;
    bool improved = true;


    unordered_set<int> solution_set(solution.begin(), solution.end());

    while (improved) {
        improved = false;
        vector<int> best_solution = solution;
        unordered_set<int> best_solution_set = solution_set;

        for (int i = 0; i < (int)solution.size(); ++i) {
            int node_to_remove = solution[i];

            vector<int> temp_solution = solution;
            unordered_set<int> temp_solution_set = solution_set;
            vector<int> contador_temp = contador;

            // Retiramos nodo
            temp_solution.erase(temp_solution.begin() + i);
            temp_solution_set.erase(node_to_remove);
            contador_temp[node_to_remove] = -3;

            for (int neighbor : adjList[node_to_remove]) {
                if (contador_temp[neighbor] == -2) {
                    contador_temp[neighbor] = adjList[neighbor].size();
                }
            }

            for (int neighbor : adjList[node_to_remove]) {
                if (contador_temp[neighbor] >= 0 && !temp_solution_set.count(neighbor)) {
                    bool can_add = true;

                    for (int n2 : adjList[neighbor]) {
                        if (temp_solution_set.count(n2)) {
                            can_add = false;
                            break;
                        }
                    }

                    if (can_add) {
                        temp_solution.push_back(neighbor);
                        temp_solution_set.insert(neighbor);
                        remove_node_and_update(neighbor, adjList, contador_temp);
                    }
                }
            }

            // condición de aceptación
            if (temp_solution.size() > best_solution.size()) {
                best_solution = temp_solution;
                best_solution_set = temp_solution_set;
                contador = contador_temp;
                improved = true;
                break;  // reiniciar búsqueda
            }
        }

        solution = best_solution;
        solution_set = best_solution_set;
    }

    return make_pair(solution, contador);
}



//Destruir solución obtenida y reparar
pair<vector<int>, vector<int>> destroy_repair(const vector<vector<int>>& adjList,const vector<int>& solucion_1, vector<int>& contador, float umbral=0.3)  {

    

    int N = adjList.size();
    vector<int> contador_nuevo = contador;
    vector<int> solution;


    random_device rd;
    mt19937 gen(rd());

    int len = solucion_1.size();
    uniform_real_distribution<float> dis(0,1);
    


    for(int i=0;i<len;i++){
        float rng=dis(gen);
        if (rng > umbral) {
            solution.push_back(solucion_1[i]);
            remove_node_and_update(solucion_1[i], adjList, contador_nuevo);
        }  
    }





    while (true) {
        int min_deg = INT_MAX;
        int min_node = -1;

        // Buscar nodo activo con menor grado
        for (int i = 0; i < N; ++i) {
            if (contador_nuevo[i] >= 0 && contador_nuevo[i] < min_deg) {
                min_deg = contador_nuevo[i];
                min_node = i;
            }
        }

        if (min_node == -1) break;

        solution.push_back(min_node);
        remove_node_and_update(min_node, adjList, contador_nuevo);
    }

    return make_pair(solution, contador_nuevo);
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



pair<vector<int>, float> IG(const vector<vector<int>>& adjList, float time_limit, float umbral=0.2) {

    auto start_time = high_resolution_clock::now(); 

    bool time_limit_reached = false;
    
    auto resultado = misp_heuristic_with_contador(adjList);

    int best_solution_value = resultado.first .size();

    float best_time = 0.0;

    pair<vector<int>, vector<int>> best_solution = resultado;
    while (!time_limit_reached) {

        
        auto solution_destroy_recreate  = destroy_repair(adjList,best_solution.first, best_solution.second, umbral);
        auto solution_localsearch = misp_local_search_opti(adjList, solution_destroy_recreate.first, solution_destroy_recreate.second);

        if (solution_localsearch.first.size() > best_solution_value) {
            best_solution_value = solution_localsearch.first.size();
            best_solution = solution_localsearch;
            best_time = duration_cast<milliseconds>(high_resolution_clock::now() - start_time).count();
            cout << "Best solution actual: " << best_solution.first.size() << endl;
            cout << "Tiempo transcurrido: " << best_time << " milisegundos" << endl;
        }
        //computo de tiempo
        auto current_time = high_resolution_clock::now(); 
        
        float elapsed_time = duration_cast<milliseconds>(current_time - start_time).count();
        if (elapsed_time >= time_limit) {
            time_limit_reached = true;
        }
    }
    return make_pair(best_solution.first, best_time);

}


pair<vector<int>,float> Grasp(const vector<vector<int>>& adjList, float time_limit, float UMBRAL_IN=0.7, int k=4) {

    auto start_time = high_resolution_clock::now();
    pair<vector<int>, vector<int>> best_solution;
    int best_size = 0;
    float best_time = 0.0;
    bool time_limit_reached = false;
    while (!time_limit_reached){
        auto resultado = misp_heuristic_randomized_with_counter(adjList,UMBRAL_IN,k);
        auto solution_localsearch = misp_local_search_opti(adjList, resultado.first, resultado.second);

        if (solution_localsearch.first.size() > best_size) {
            best_size = solution_localsearch.first.size();
            best_solution = solution_localsearch;
            best_time = duration_cast<milliseconds>(high_resolution_clock::now() - start_time).count();
            cout << "Best solution actual: " << best_solution.first.size() << endl;
            cout << "Tiempo transcurrido: " << best_time << " milisegundos" << endl;
        }
        
        auto current_time = high_resolution_clock::now();
        float elapsed_time = duration_cast<milliseconds>(current_time - start_time).count();
        if (elapsed_time >= time_limit) {
            time_limit_reached = true;
        }
    }

    return make_pair(best_solution.first, best_time);
}






void RunInstance(const string& instance_path) {
    // Leer grafo desde archivo
    vector<vector<int>> adjList = read_file(instance_path);
    // Ejecutar algoritmo y medir tiempo
    auto start = high_resolution_clock::now();
    auto solution = misp_heuristic(adjList);
    auto end = high_resolution_clock::now();

    //por si se quiere imprimir para verificar
    bool validate_console = validate_independent_set(adjList, solution);

    // Resultados
    float result = solution.size();
    float time_ms = duration_cast<milliseconds>(end - start).count();

    cout << "Tiempo: " << time_ms << " ms" << endl;
    cout << "Tamano del conjunto independiente: " << result << endl;
}





void RunTxtFile(string txt, float time_limit = 10000.0, float UMBRAL_IN = 0.7, int k = 4) {
    
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


    // Contador global de bloque (1, 2, 3…)
    int bloque = 1;

    for (int i = 0; i < file_paths.size(); i++) {
        cout << "Grafo "<< i << endl;
        vector<vector<int>> adjList = read_file(file_paths[i]);

        auto start = high_resolution_clock::now();
        auto grasp_result = Grasp(adjList, time_limit, UMBRAL_IN, k);
        result[i % 30] = grasp_result.first.size();
        auto end = high_resolution_clock::now();

        //bool validate_console = validate_independent_set(adjList, grasp_result.first);
        //cout << "Validacion de la solucion: " << validate_console << endl;

        time[i % 30] = grasp_result.second; // tiempo ya en ms

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

            // std
            float result_std = 0;
            for (int j = 0; j < 30; j++) {
                result_std += pow(result[j] - result_med, 2);
            }
            result_std = sqrt(result_std / 30);

            //guardar
            ofstream log("log_trayectoria_" + txt + "_" + to_string(UMBRAL_IN) + "_" + to_string(k) + ".txt", ios::app);
            log << bloque << " | Densidad: " << density[i_density]
                << " | Time MED: " << time_med << " ms"
                << " | Result MED: " << result_med
                << " | Result STD: " << result_std << endl;
            log.close();

            cout << "Bloque " << bloque << " completado." << endl;

            bloque++;
            i_density = min(i_density + 1, (int)density.size() - 1); // evitar overflow
        }
    }
}

void RunTxtFileIG(string txt, float time_limit = 10000.0, float UMBRAL_IN = 0.7) {
    
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


    // Contador global de bloque (1, 2, 3…)
    int bloque = 1;

    for (int i = 0; i < file_paths.size(); i++) {
        cout << "Grafo "<< i << endl;
        vector<vector<int>> adjList = read_file(file_paths[i]);

        auto start = high_resolution_clock::now();
        auto IG_result = IG(adjList, time_limit, UMBRAL_IN);
        result[i % 30] = IG_result.first.size();
        auto end = high_resolution_clock::now();

        //bool validate_console = validate_independent_set(adjList, IG_result.first);
        //cout << "Validacion de la solucion: " << validate_console << endl;

        time[i % 30] = IG_result.second; // tiempo ya en ms

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

            // std
            float result_std = 0;
            for (int j = 0; j < 30; j++) {
                result_std += pow(result[j] - result_med, 2);
            }
            result_std = sqrt(result_std / 30);

            //guardar
            ofstream log("log_trayectoria_" + txt + "_IG_" + to_string(UMBRAL_IN) + ".txt", ios::app);
            log << bloque << " | Densidad: " << density[i_density]
                << " | Time MED: " << time_med << " ms"
                << " | Result MED: " << result_med
                << " | Result STD: " << result_std << endl;
            log.close();

            cout << "Bloque " << bloque << " completado." << endl;

            bloque++;
            i_density = min(i_density + 1, (int)density.size() - 1); // evitar overflow
        }
    }
}








// para instancia random
void RunInstanceProb(const string& instance_path, float UMBRAL_IN, int k) {
    // Leer grafo desde archivo
    vector<vector<int>> adjList = read_file(instance_path);

    // Ejecutar algoritmo y medir tiempo
    auto start = high_resolution_clock::now();
    auto solution = misp_heuristic_randomized(adjList, UMBRAL_IN, k);
    auto end = high_resolution_clock::now();
    
    //por si se quiere imprimir para verificar
    bool validate_console = validate_independent_set(adjList, solution);

    float result = solution.size();
    float time_ms = duration_cast<milliseconds>(end - start).count();

    cout << "Tiempo: " << time_ms << " ms" << endl;
    cout << "Tamano del conjunto independiente: " << result << endl;
}














void RunInstanceIG(const string& instance_path, float time_limit, float umbral=0.2) {

    vector<vector<int>> adjList = read_file(instance_path);

    auto start = high_resolution_clock::now();
    auto best_solution = IG(adjList, time_limit, umbral);
    auto end = high_resolution_clock::now();

    float time_ms = duration_cast<milliseconds>(end - start).count();

    cout << "Tiempo total IG: " << time_ms << " ms" << endl;
    cout << "Tamano del conjunto independiente IG: " << best_solution.first.size() << endl;
    bool validate_console = validate_independent_set(adjList, best_solution.first);
    cout << "Validez de la solucion: " << validate_console << endl;
}

void RunInstanceGrasp(const string& instance_path, float time_limit, float UMBRAL_IN=0.7, int k=4) {

    vector<vector<int>> adjList = read_file(instance_path);

    auto start = high_resolution_clock::now();
    auto best_solution = Grasp(adjList, time_limit, UMBRAL_IN, k);
    auto end = high_resolution_clock::now();

    float time_ms = duration_cast<milliseconds>(end - start).count();

    cout << "Tiempo total Grasp: " << time_ms << " ms" << endl;
    cout << "Tamano del conjunto independiente Grasp: " << best_solution.first.size() << endl;
    bool validate_console = validate_independent_set(adjList, best_solution.first);
    cout << "Validez de la solucion: " << validate_console << endl;
}




void RunMasterFile(string master_txt, float time_limit = 10000.0, float UMBRAL_IN = 0.7, int k = 4) {
    ifstream master(master_txt);
    string subfile;
    vector<string> subfiles;

    while (getline(master, subfile)) {
        if (!subfile.empty())
            subfiles.push_back(subfile);
    }
    master.close();

    for (auto &txt : subfiles) {
        cout << "Ejecutando archivo de grafos: " << txt << endl;
        RunTxtFile(txt, time_limit, UMBRAL_IN, k);
        cout << "------------------------------------" << endl;
    }
}

void RunMasterFileIG(string master_txt, float time_limit = 10000.0, float UMBRAL_IN = 0.7) {
    ifstream master(master_txt);
    string subfile;
    vector<string> subfiles;

    while (getline(master, subfile)) {
        if (!subfile.empty())
            subfiles.push_back(subfile);
    }
    master.close();

    for (auto &txt : subfiles) {
        cout << "Ejecutando archivo de grafos: " << txt << endl;
        RunTxtFileIG(txt, time_limit, UMBRAL_IN);
        cout << "------------------------------------" << endl;
    }
}





int main(int argc, char* argv[]) {
    if (argc < 5) {
        cerr << "Uso: " << argv[0]
             << " -i <instancia-problema> -t <tiempo-limite> [-u <umbral>] [-k <k>]" << endl;
        return 1;
    }

    string instancia;
    double time = 0;
    //double umbral = 0.7; // valores por defecto
    //int k = 4;
    // para IG
    double umbral = 0.3; // valores por defecto
    //int k = 4;


    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        if (arg == "-i" && i + 1 < argc) {
            instancia = argv[++i];
        } else if (arg == "-t" && i + 1 < argc) {
            time = atof(argv[++i]);
        } else if (arg == "-u" && i + 1 < argc) {
            umbral = atof(argv[++i]);
        } else if (arg == "-k" && i + 1 < argc) {
            //k = atoi(argv[++i]);
        }
        else if (arg == "-i" || arg == "-t" || arg == "-u" || arg == "-k") {
            cerr << "Error: falta valor después de " << arg << endl;
            return 1;
        }
    }

    if (instancia.empty() || time <= 0) {
        cerr << "Error: los parámetros -i (instancia) y -t (tiempo) son obligatorios." << endl;
        return 1;
    }
    //cout << "Parametros: " << "time " << time << ", umbral " << umbral << ", k " << k << endl;
    cout << "Parametros: " << "time " << time << ", umbral " << umbral << endl;
    RunMasterFileIG(instancia.c_str(), time, umbral);
    return 0;
}