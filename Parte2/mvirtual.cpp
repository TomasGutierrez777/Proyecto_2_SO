#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <list>
#include <queue>
#include <algorithm> // Para std::find y std::distance

using namespace std;

class VirtualMemorySimulator {
private:
    int frameCount;
    string replacementAlgorithm;
    vector<int> pageReferences;
    unordered_map<int, bool> pageTable;  // Tabla de páginas
    list<int> frames;  // Marco de páginas (FIFO, LRU, etc.)
    unordered_map<int, list<int>::iterator> framePosition;  // Mapa de posición en los frames
    vector<bool> referenceBits; // Para LRU Reloj simple
    int faultCount;

public:
    VirtualMemorySimulator(int m, string a, const string& filePath)
        : frameCount(m), replacementAlgorithm(a), faultCount(0) {
        loadPageReferences(filePath);
    }

    void loadPageReferences(const string& filePath) {
        ifstream inputFile(filePath);
        if (!inputFile.is_open()) {
            cerr << "Error opening file: " << filePath << endl;
            exit(EXIT_FAILURE);
        }

        int page;
        while (inputFile >> page) {
            pageReferences.push_back(page);
        }
        inputFile.close();
    }

    void run() {
        if (replacementAlgorithm == "FIFO") {
            runFIFO();
        } else if (replacementAlgorithm == "LRU") {
            runLRU();
        } else if (replacementAlgorithm == "Optimal") {
            runOptimal();
        } else if (replacementAlgorithm == "Clock") {
            runClock();
        } else if (replacementAlgorithm == "LRU_Clock") {
            runLRUClock();
        } else {
            cerr << "Unknown replacement algorithm: " << replacementAlgorithm << endl;
            exit(EXIT_FAILURE);
        }

        cout << "Page Faults: " << faultCount << endl;
    }

private:
    void runFIFO() {
        queue<int> fifoQueue;
        unordered_map<int, bool> inFrame;  // Mapa de páginas cargadas

        for (int page : pageReferences) {
            if (inFrame.find(page) == inFrame.end()) {  // Si la página no está en memoria
                faultCount++;
                if (fifoQueue.size() == frameCount) {
                    int evictPage = fifoQueue.front();  // Sacamos la página más antigua
                    fifoQueue.pop();
                    inFrame.erase(evictPage);  // Borramos la página de la tabla
                }
                fifoQueue.push(page);
                inFrame[page] = true;  // Marcamos la página como cargada
            }
        }
    }

    void runLRU() {
        list<int> lruCache;  // Lista de páginas en memoria (más reciente al frente)
        unordered_map<int, list<int>::iterator> pageIterMap;  // Mapa para encontrar las páginas rápidamente

        for (int page : pageReferences) {
            if (pageIterMap.find(page) == pageIterMap.end()) {  // Si la página no está en memoria
                faultCount++;
                if (lruCache.size() == frameCount) {
                    int evictPage = lruCache.back();  // Sacamos la página menos recientemente utilizada
                    lruCache.pop_back();
                    pageIterMap.erase(evictPage);  // Borramos la página del mapa
                }
            } else {  // Si la página está en memoria, actualizamos su posición (a la más reciente)
                lruCache.erase(pageIterMap[page]);
            }

            lruCache.push_front(page);  // Añadimos la página al frente
            pageIterMap[page] = lruCache.begin();  // Actualizamos la posición de la página
        }
    }

    void runOptimal() {
        unordered_map<int, int> frameIndex;  // Mapa de páginas cargadas y su última posición
        for (size_t i = 0; i < pageReferences.size(); ++i) {
            int page = pageReferences[i];
            if (frameIndex.find(page) == frameIndex.end()) {  // Si la página no está en memoria
                faultCount++;
                if (frames.size() == frameCount) {
                    int farthest = -1, evictPage = -1;
                    // Encontramos la página que será usada más tarde
                    for (int p : frames) {
                        auto it = find(pageReferences.begin() + i + 1, pageReferences.end(), p);
                        if (it == pageReferences.end()) {
                            evictPage = p;
                            break;
                        } else {
                            int dist = distance(pageReferences.begin(), it);
                            if (dist > farthest) {
                                farthest = dist;
                                evictPage = p;
                            }
                        }
                    }
                    frames.remove(evictPage);
                    frameIndex.erase(evictPage);
                }
                frames.push_back(page);  // Cargamos la nueva página
                frameIndex[page] = i;
            }
        }
    }

    void runClock() {
        vector<pair<int, bool>> clockCache(frameCount, {-1, false});  // {Página, bit de referencia}
        int clockHand = 0;

        for (int page : pageReferences) {
            bool found = false;
            for (auto& frame : clockCache) {
                if (frame.first == page) {  // Página encontrada, se marca el bit de referencia
                    frame.second = true;
                    found = true;
                    break;
                }
            }
            if (!found) {  // Si no está en memoria, ocurre un fallo
                faultCount++;
                // Buscamos una página para reemplazar (si tiene bit de referencia en 0)
                while (clockCache[clockHand].second) {
                    clockCache[clockHand].second = false;  // Limpiamos el bit de referencia
                    clockHand = (clockHand + 1) % frameCount;  // Movemos la manecilla
                }
                clockCache[clockHand] = {page, true};  // Cargamos la nueva página
                clockHand = (clockHand + 1) % frameCount;
            }
        }
    }

    void runLRUClock() {
        vector<pair<int, bool>> clockCache(frameCount, {-1, false});  // {Página, bit de referencia}
        int clockHand = 0;

        for (int page : pageReferences) {
            bool found = false;
            for (auto& frame : clockCache) {
                if (frame.first == page) {  // Página encontrada, se marca el bit de referencia
                    frame.second = true;
                    found = true;
                    break;
                }
            }
            if (!found) {  // Si no está en memoria, ocurre un fallo
                faultCount++;
                // Buscamos una página para reemplazar (si tiene bit de referencia en 0)
                while (clockCache[clockHand].second) {
                    clockCache[clockHand].second = false;  // Limpiamos el bit de referencia
                    clockHand = (clockHand + 1) % frameCount;  // Movemos la manecilla
                }
                clockCache[clockHand] = {page, true};  // Cargamos la nueva página
                clockHand = (clockHand + 1) % frameCount;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 7) {
        cerr << "Usage: " << argv[0] << " -m <numFrames> -a <algorithm> -f <file>" << endl;
        return EXIT_FAILURE;
    }

    int frameCount = 0;
    string replacementAlgorithm;
    string filePath;

    for (int i = 1; i < argc; i += 2) {
        string arg = argv[i];
        if (arg == "-m") {
            frameCount = stoi(argv[i + 1]);
        } else if (arg == "-a") {
            replacementAlgorithm = argv[i + 1];
        } else if (arg == "-f") {
            filePath = argv[i + 1];
        } else {
            cerr << "Unknown argument: " << arg << endl;
            return EXIT_FAILURE;
        }
    }

    VirtualMemorySimulator simulator(frameCount, replacementAlgorithm, filePath);
    simulator.run();

    return 0;
}
