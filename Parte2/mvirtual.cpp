#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <list>
#include <algorithm>
#include <queue>

using namespace std;

class VirtualMemorySimulator {
private:
    int numFrames;
    string algorithm;
    vector<int> references;
    unordered_map<int, list<int>> pageTable;
    list<int> frames;
    unordered_map<int, list<int>::iterator> framePosition;

    int pageFaults;

public:
    VirtualMemorySimulator(int m, string a, const string& filePath) : numFrames(m), algorithm(a), pageFaults(0) {
        loadReferences(filePath);
    }

    void loadReferences(const string& filePath) {
        ifstream inputFile(filePath);
        if (!inputFile.is_open()) {
            cerr << "Error opening file: " << filePath << endl;
            exit(EXIT_FAILURE);
        }

        int page;
        while (inputFile >> page) {
            references.push_back(page);
        }
        inputFile.close();
    }

    void run() {
        if (algorithm == "FIFO") {
            runFIFO();
        } else if (algorithm == "LRU") {
            runLRU();
        } else if (algorithm == "Optimal") {
            runOptimal();
        } else if (algorithm == "Clock") {
            runClock();
        } else {
            cerr << "Unknown replacement algorithm: " << algorithm << endl;
            exit(EXIT_FAILURE);
        }

        cout << "Page Faults: " << pageFaults << endl;
    }

private:
    void runFIFO() {
        queue<int> frameQueue;
        unordered_map<int, bool> inFrame;
        for (int page : references) {
            if (inFrame.find(page) == inFrame.end()) {
                pageFaults++;
                if (frameQueue.size() == numFrames) {
                    int evictPage = frameQueue.front();
                    frameQueue.pop();
                    inFrame.erase(evictPage);
                }
                frameQueue.push(page);
                inFrame[page] = true;
            }
        }
    }

    void runLRU() {
        list<int> lruFrames;
        unordered_map<int, list<int>::iterator> pageIterMap;
        for (int page : references) {
            if (pageIterMap.find(page) == pageIterMap.end()) {
                pageFaults++;
                if (lruFrames.size() == numFrames) {
                    int evictPage = lruFrames.back();
                    lruFrames.pop_back();
                    pageIterMap.erase(evictPage);
                }
            } else {
                lruFrames.erase(pageIterMap[page]);
            }
            lruFrames.push_front(page);
            pageIterMap[page] = lruFrames.begin();
        }
    }

    void runOptimal() {
        unordered_map<int, int> frameIndex;
        for (size_t i = 0; i < references.size(); ++i) {
            int page = references[i];
            if (frameIndex.find(page) == frameIndex.end()) {
                pageFaults++;
                if (frames.size() == numFrames) {
                    int farthest = -1, evictPage = -1;
                    for (int p : frames) {
                        auto it = find(references.begin() + i + 1, references.end(), p);
                        if (it == references.end()) {
                            evictPage = p;
                            break;
                        } else {
                            int dist = distance(references.begin(), it);
                            if (dist > farthest) {
                                farthest = dist;
                                evictPage = p;
                            }
                        }
                    }
                    frames.remove(evictPage);
                    frameIndex.erase(evictPage);
                }
                frames.push_back(page);
                frameIndex[page] = i;
            }
        }
    }

    void runClock() {
        vector<pair<int, bool>> clockFrames(numFrames, {-1, false});
        int clockHand = 0;

        for (int page : references) {
            bool found = false;
            for (auto& frame : clockFrames) {
                if (frame.first == page) {
                    frame.second = true;
                    found = true;
                    break;
                }
            }
            if (!found) {
                pageFaults++;
                while (clockFrames[clockHand].second) {
                    clockFrames[clockHand].second = false;
                    clockHand = (clockHand + 1) % numFrames;
                }
                clockFrames[clockHand] = {page, true};
                clockHand = (clockHand + 1) % numFrames;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 7) {
        cerr << "Usage: " << argv[0] << " -m <numFrames> -a <algorithm> -f <file>" << endl;
        return EXIT_FAILURE;
    }

    int numFrames = 0;
    string algorithm;
    string filePath;

    for (int i = 1; i < argc; i += 2) {
        string arg = argv[i];
        if (arg == "-m") {
            numFrames = stoi(argv[i + 1]);
        } else if (arg == "-a") {
            algorithm = argv[i + 1];
        } else if (arg == "-f") {
            filePath = argv[i + 1];
        } else {
            cerr << "Unknown argument: " << arg << endl;
            return EXIT_FAILURE;
        }
    }

    VirtualMemorySimulator simulator(numFrames, algorithm, filePath);
    simulator.run();

    return 0;
}
