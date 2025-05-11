#include <iostream>
#include <fstream>
#include <cstdlib>
#include <windows.h>
#include <chrono>
#include <iomanip>

#define MAX_CLAUZE 10000
#define MAX_LITERALI 100
#define MAX_LUNGIME_CLAUZA 50

using namespace std;
using namespace std::chrono;

int clauze[MAX_CLAUZE][MAX_LUNGIME_CLAUZA];
int lungime[MAX_CLAUZE];
int nrClauze = 0;
int nrVariabile = 0;  // numărul de variabile (vom citi din header-ul p cnf)

double memorieFolositaGB() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    DWORDLONG memTotal = statex.ullTotalPhys;
    DWORDLONG memUsed = memTotal - statex.ullAvailPhys;
    return memUsed / (1024.0 * 1024.0 * 1024.0);
}

void normalizeazaClauza(int clauza[], int &lungime) {
    // Elimina duplicate si sorteaza (fara STL)
    for (int i = 0; i < lungime - 1; i++) {
        for (int j = i + 1; j < lungime; j++) {
            if (clauza[i] > clauza[j]) {
                int temp = clauza[i];
                clauza[i] = clauza[j];
                clauza[j] = temp;
            }
        }
    }
    int nouaLungime = 0;
    for (int i = 0; i < lungime; i++) {
        if (i == 0 || clauza[i] != clauza[i - 1]) {
            clauza[nouaLungime++] = clauza[i];
        }
    }
    lungime = nouaLungime;
}

void citireClauze(const char* numeFisier) {
    ifstream fin(numeFisier);
    if (!fin) {
        cout << "Eroare la deschiderea fisierului!\n";
        exit(1);
    }

    char c;
    string linie;
    while (fin >> c) {
        if (c == 'c') {
            getline(fin, linie);
            continue;
        }
        if (c == 'p') {
            fin >> linie;
            fin >> nrVariabile;
            int clauses;
            fin >> clauses;
            continue;
        }

        fin.putback(c);
        int literal, index = 0;
        while (fin >> literal) {
            if (literal == 0) break;
            if (index < MAX_LUNGIME_CLAUZA) {
                clauze[nrClauze][index++] = literal;
            }
        }
        lungime[nrClauze] = index;
        normalizeazaClauza(clauze[nrClauze], lungime[nrClauze]);
        nrClauze++;
        if (nrClauze >= MAX_CLAUZE) {
            cout << "Eroare: S-a depasit limita maxima de clauze!" << endl;
            exit(1);
        }
    }

    cout << "Clauze citite: " << nrClauze << endl;
    cout << "Numar variabile: " << nrVariabile << endl;
}

bool verificaFormula(bool valori[]) {
    for (int i = 0; i < nrClauze; i++) {
        bool satisfacut = false;
        for (int j = 0; j < lungime[i]; j++) {
            int literal = clauze[i][j];
            bool valoare = (literal > 0) ? valori[literal - 1] : !valori[-literal - 1];
            if (valoare) {
                satisfacut = true;
                break;
            }
        }
        if (!satisfacut) {
            return false;
        }
    }
    return true;
}

bool dpSatisfiabil(int idx, bool valori[]) {
    if (idx == nrVariabile) {
        return verificaFormula(valori);
    }

    valori[idx] = false;
    if (dpSatisfiabil(idx + 1, valori)) return true;

    valori[idx] = true;
    if (dpSatisfiabil(idx + 1, valori)) return true;

    return false;
}

int main() {
    const char* numeFisier = "formula.cnf";
    citireClauze(numeFisier);

    bool valori[MAX_LITERALI] = {0};  // initializez cu false

    auto start = steady_clock::now();

    bool rezultat = dpSatisfiabil(0, valori);

    auto end = steady_clock::now();
    auto total_time = duration_cast<milliseconds>(end - start).count();

    double memGB = memorieFolositaGB();

    cout << endl << "Rezultatul: " << (rezultat ? "Satisfiabil" : "Nesatisfiabil") << endl;
    cout << "Timp total de executie: " << total_time / 1000.0 << " secunde" << endl;
    cout << "Memorie folosita la final: " << fixed << setprecision(2) << memGB << " GB" << endl;

    return 0;
}
