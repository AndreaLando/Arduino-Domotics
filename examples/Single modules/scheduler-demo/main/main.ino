#include <Arduino.h>
#include "Jobs.h"
#include "Domo.h"
#include "Initialization.h"

// Creiamo uno scheduler
AsyncScheduler scheduler;

// Funzioni non bloccanti simulate
bool stepAccensione(DomoManager* dm) {
  static unsigned long start = 0;

  if (start == 0) {
    start = millis();
    Serial.println("Step 1: Accensione in corso...");
  }

  if (millis() - start >= 3000) {
    Serial.println("Step 1 completato");
    start = 0;
    return true;
  }

  return false;
}

bool stepRiscaldamento(DomoManager* dm) {
  static unsigned long start = 0;

  if (start == 0) {
    start = millis();
    Serial.println("Step 2: Riscaldamento...");
  }

  if (millis() - start >= 5000) {
    Serial.println("Step 2 completato");
    start = 0;
    return true;
  }

  return false;
}

bool stepRaffrescamento(DomoManager* dm) {
  static unsigned long start = 0;

  if (start == 0) {
    start = millis();
    Serial.println("Step 3: Raffrescamento...");
  }

  if (millis() - start >= 4000) {
    Serial.println("Step 3 completato");
    start = 0;
    return true;
  }

  return false;
}

// Condizione di branching
bool isHot(DomoManager* dm) {
  // Simuliamo una temperatura interna
  float temp = 20 + sin(millis() / 3000.0);
  Serial.print("Temperatura simulata: ");
  Serial.println(temp);

  return temp > 21.0;
}

// Skip condition
bool skipCooling(DomoManager* dm) {
  return millis() % 15000 < 5000; // salta ogni tanto
}

// Callback finale
void jobFinito() {
  Serial.println(">>> JOB COMPLETATO!");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Scheduler demo avviato");

  // Lo scheduler ha bisogno del DomoManager
  scheduler.setManager(&Manager);

  // Creiamo un job
  AsyncScheduler::Job job;
  job.priority = 10;
  job.onComplete = jobFinito;

  // Step 0: Branch
  AsyncScheduler::Step branch;
  branch.type = AsyncScheduler::BRANCH_STEP;
  branch.description = "Verifica temperatura";
  branch.condition = isHot;
  branch.thenStep = 2; // se caldo → raffrescamento
  branch.elseStep = 1; // se freddo → riscaldamento
  job.steps.push_back(branch);

  // Step 1: Riscaldamento
  AsyncScheduler::Step heat;
  heat.type = AsyncScheduler::NORMAL_STEP;
  heat.fnc = stepRiscaldamento;
  heat.description = "Riscaldamento ambiente";
  heat.delayAfterMs = 1000;
  job.steps.push_back(heat);

  // Step 2: Raffrescamento
  AsyncScheduler::Step cool;
  cool.type = AsyncScheduler::NORMAL_STEP;
  cool.fnc = stepRaffrescamento;
  cool.description = "Raffrescamento ambiente";
  cool.delayAfterMs = 1000;
  cool.skipIf = skipCooling;
  job.steps.push_back(cool);

  // Step 3: Accensione finale
  AsyncScheduler::Step acc;
  acc.type = AsyncScheduler::NORMAL_STEP;
  acc.fnc = stepAccensione;
  acc.description = "Accensione finale";
  acc.delayAfterMs = 500;
  job.steps.push_back(acc);

  // Aggiungiamo il job allo scheduler
  int jobId = scheduler.addJob(job);

  // Avviamo il job
  scheduler.startJob(jobId);
}

void loop() {
  // Lo scheduler deve essere chiamato continuamente
  scheduler.run();
}
