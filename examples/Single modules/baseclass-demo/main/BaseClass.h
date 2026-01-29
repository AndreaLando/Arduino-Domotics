#ifndef BaseClass_H
#define BaseClass_H

#include <vector>

/*==============================================================================
   MODULE HELP — MEASUREMENT MANAGEMENT, MOVING AVERAGES, TREND ANALYSIS
==============================================================================

   INTRODUCTION TO THE MODULE
   This module provides a set of classes designed to manage numerical
   measurements, compute moving averages, analyze variations between
   consecutive values, and determine trends over time.

   Components:
     - Cell<T>:
         A lightweight container that stores a value and tracks whether it
         has changed since the last read.

     - Gruppo:
         A circular-buffer structure that stores measurements and their
         variations, computes averages, and evaluates trends.

     - CalcolatoreMedie:
         A manager class that handles multiple groups, allowing you to add
         measurements and retrieve aggregated statistics.

   The design emphasizes modularity, clarity, and suitability for embedded
   systems where predictable performance and low overhead are essential.

------------------------------------------------------------------------------

   DEVELOPER NOTES
   - Cell<T> tracks value changes using an internal flag.
     Use setIfDiff() to update the value only when it differs from the
     previous one.

   - Gruppo maintains two circular buffers:
         * one for measurements
         * one for variations between consecutive measurements
     The buffer size is configurable.

   - The trend() function returns:
         * CONSTANT
         * INCREASING
         * DECREASING
     based on the average variation and a configurable sensitivity threshold.

   - CalcolatoreMedie manages multiple groups identified by name.
     If a measurement is added to a non-existing group, the group is created
     automatically.

   - The module is designed for extensibility:
         * new analysis methods
         * new buffer types
         * more advanced trend logic

------------------------------------------------------------------------------

   USAGE EXAMPLES

   1. Creating a group and adding measurements:
        CalcolatoreMedie calc;
        calc.creaGruppo("Temperature", 10, 0.2f);
        calc.aggiungiMisura("Temperature", 22.5f);
        calc.aggiungiMisura("Temperature", 23.1f);

   2. Reading the average:
        float avg = calc.mediaGruppo("Temperature");

   3. Checking the trend:
        Gruppo::Trend t = calc.trendGruppo("Temperature");

   4. Using Cell<T>:
        Cell<int> c;
        c.setIfDiff(5);
        if (c.hasChanged()) {
            // handle change
        }
        int value = c.get();   // resets the 'changed' flag

==============================================================================*/


template<typename T>
class Cell {
  private:
    T value;
    bool changed;

  public:
    Cell() : value{}, changed(false) {}
    Cell(const T& initial) : value(initial), changed(false) {}

    inline void setIfDiff(const T& newValue) {
      // Set change flag only if old and new values are differents
      if (value != newValue) {
          value = newValue;
          changed = true;
      }
    }

    inline void set(const T& newValue) {
      value = newValue;
      changed = true;
    }

    inline T get() {
        changed = false;
        return value;
    }

    // Lettura che NON resetta il flag 
    inline T preserveGet() const { 
      return value; 
    }

    inline bool hasChanged() const {
        return changed;
    }
};

  //Calcolatore Medie
#define NUM_VARIAZIONI 5

class Gruppo {
public:
    String nome;

    // Buffer circolare misure
    std::vector<float> buffer;
    int maxSize;
    int head = 0;
    int count = 0;

    // Buffer circolare variazioni
    std::vector<float> variazioni;
    int headVar = 0;
    int countVar = 0;

    float soglia;

    enum Trend {
        COSTANTE,
        CRESCENTE,
        DECRESCENTE
    };

    Gruppo(String n, int size = 5, float sens = 0.1)
        : nome(n), maxSize(size), soglia(sens)
    {
        buffer.resize(maxSize, 0);
        variazioni.resize(NUM_VARIAZIONI, 0);
    }

    void aggiornaMisura(float valore) {
        // Calcolo variazione rispetto all'ultima misura
        if (count > 0) {
            float ultima = buffer[(head - 1 + maxSize) % maxSize];
            float diff = valore - ultima;
            aggiornaVariazione(diff);
        }

        // Scrittura nel buffer circolare
        buffer[head] = valore;
        head = (head + 1) % maxSize;

        if (count < maxSize) count++;
    }

    void aggiornaVariazione(float diff) {
        variazioni[headVar] = diff;
        headVar = (headVar + 1) % NUM_VARIAZIONI;

        if (countVar < NUM_VARIAZIONI) countVar++;
    }

    float media() const {
        if (count == 0) return 0;

        float somma = 0;
        for (int i = 0; i < count; i++) {
            somma += buffer[i];
        }
        return somma / count;
    }

    float mediaVariazioni() const {
        if (countVar == 0) return 0;

        float somma = 0;
        for (int i = 0; i < countVar; i++) {
            somma += variazioni[i];
        }
        return somma / countVar;
    }

    Trend trend() const {
        if (countVar == 0) return COSTANTE;

        float m = mediaVariazioni();

        if (fabs(m) < soglia) return COSTANTE;
        if (m > 0) return CRESCENTE;
        return DECRESCENTE;
    }
};


class CalcolatoreMedie {
public:
    std::vector<Gruppo> gruppi;

    void creaGruppo(const String& nome, int size = 5, float soglia = 0.1) {
        gruppi.emplace_back(nome, size, soglia);
    }

    Gruppo* trovaGruppo(const String& nome) {
        for (auto& g : gruppi) {
            if (g.nome == nome) return &g;
        }
        return nullptr;
    }

    void aggiungiMisura(const String& nomeGruppo, float valore, float soglia = 0.1) {
        Gruppo* g = trovaGruppo(nomeGruppo);
        if (!g) {
            creaGruppo(nomeGruppo, 5, soglia);
            g = trovaGruppo(nomeGruppo);
        }
        g->aggiornaMisura(valore);
    }

    float mediaGruppo(const String& nomeGruppo) {
        Gruppo* g = trovaGruppo(nomeGruppo);
        return g ? g->media() : 0;
    }

    Gruppo::Trend trendGruppo(const String& nomeGruppo) {
        Gruppo* g = trovaGruppo(nomeGruppo);
        return g ? g->trend() : Gruppo::COSTANTE;
    }
};

#endif
