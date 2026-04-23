#include <QtTest/QtTest>

// Deklaracje funkcji uruchamiających z poszczególnych plików testowych
int runTestCodeExecutor(int argc, char *argv[]);
int runTestFilterCalculator(int argc, char *argv[]);
int runTestOllamaClient(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    int status = 0;
    
    // Uruchomienie wszystkich testów
    status |= runTestCodeExecutor(argc, argv);
    status |= runTestFilterCalculator(argc, argv);
    status |= runTestOllamaClient(argc, argv);
    
    return status;
}