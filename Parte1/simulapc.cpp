#include <iostream>
#include <queue>
#include <thread>
#include <condition_variable>
#include <fstream>
#include <vector>
#include <atomic>
#include <chrono>
#include <cstring>

class MonitorQueue {
private:
    std::queue<int> queue;
    size_t capacity;
    std::mutex mtx;
    std::condition_variable cv_full;
    std::condition_variable cv_empty;
    std::ofstream logFile;

public:
    MonitorQueue(size_t initial_capacity) : capacity(initial_capacity) {
        logFile.open("log.txt", std::ios::out | std::ios::trunc);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file. Check directory permissions." << std::endl; exit(1);
        }
    }

    ~MonitorQueue() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void push(int value) {
        std::unique_lock<std::mutex> lock(mtx);
        cv_full.wait(lock, [this]() { return queue.size() < capacity; });

        queue.push(value);
        logFile << "Pushed: " << value << " | Queue size: " << queue.size() << "\n";
        logFile.flush();

        if (queue.size() == capacity) {
            capacity *= 2;
            logFile << "Queue size doubled to: " << capacity << "\n";
            logFile.flush();
        }

        cv_empty.notify_one();
    }

    int pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv_empty.wait(lock, [this]() { return !queue.empty(); });

        int value = queue.front();
        queue.pop();
        logFile << "Popped: " << value << " | Queue size: " << queue.size() << "\n";
        logFile.flush();

        if (queue.size() <= capacity / 4 && capacity > 1) {
            capacity /= 2;
            logFile << "Queue size reduced to: " << capacity << "\n";
            logFile.flush();
        }

        cv_full.notify_one();
        return value;
    }
};

void producer(MonitorQueue* queue, int id, int num_items) {
    for (int i = 0; i < num_items; ++i) {
        queue->push(id * 100 + i);
        std::cout << "Producer " << id << " pushed: " << (id * 100 + i) << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void consumer(MonitorQueue* queue, int max_wait_time) {
    auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(max_wait_time);
    while (std::chrono::steady_clock::now() < end_time) {
        int value = queue->pop();
        std::cout << "Consumer popped: " << value << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

int main(int argc, char *argv[]) {
    int num_producers = 0;
    int num_consumers = 0;
    size_t initial_queue_size = 0;
    int max_wait_time = 0;

    // Parsing command-line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-p") == 0) {
            num_producers = std::stoi(argv[++i]);
        } else if (strcmp(argv[i], "-c") == 0) {
            num_consumers = std::stoi(argv[++i]);
        } else if (strcmp(argv[i], "-s") == 0) {
            initial_queue_size = std::stoi(argv[++i]);
        } else if (strcmp(argv[i], "-t") == 0) {
            max_wait_time = std::stoi(argv[++i]);
        }
    }

    if (num_producers <= 0 || num_consumers <= 0 || initial_queue_size <= 0 || max_wait_time <= 0) {
        std::cerr << "Invalid arguments. Usage: ./simulapc -p <num_producers> -c <num_consumers> -s <initial_queue_size> -t <max_wait_time>\n";
        return 1;
    }

    MonitorQueue queue(initial_queue_size);
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    // Start producer threads
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back(producer, &queue, i + 1, 10);
    }

    // Start consumer threads
    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back(consumer, &queue, max_wait_time);
    }

    // Join all threads
    for (auto &producer : producers) {
        producer.join();
    }

    for (auto &consumer : consumers) {
        consumer.join();
    }

    std::cout << "Simulation complete." << std::endl;
    return 0;
}