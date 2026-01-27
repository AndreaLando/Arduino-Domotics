#ifndef Jobs_H
#define Jobs_H

#pragma once
#include <Arduino.h>
#include "Domo.h"
#include <vector>
#include <algorithm>

class AsyncScheduler {
  private:
    DomoManager* domo = nullptr;

  public:
    void setManager(DomoManager* mgr) { domo = mgr; }

    enum StepType { NORMAL_STEP, BRANCH_STEP };

    typedef bool (*FunctionPointer)(DomoManager*);
    typedef void (*CompletionCallback)();
    typedef bool (*ConditionFunction)(DomoManager*);
    
    struct Step {
      StepType type = NORMAL_STEP;

      // For NORMAL_STEP
      FunctionPointer fnc=nullptr; // returns true when done 
      unsigned long delayAfterMs;

      // For BRANCH_STEP
      ConditionFunction condition=nullptr; // NEW: optional condition
      int thenStep = -1; // index of next step if condition true 
      int elseStep = -1; // index of next step if condition false

      // NEW: skip condition 
      ConditionFunction skipIf = nullptr;

      String description;
    };

    struct Job {
      std::vector<Step> steps;
      int currentStep = 0;
      bool active = false;
      bool cancelled = false;
      unsigned long nextRunTime = 0;
      int priority = 0;
      CompletionCallback onComplete = nullptr;
    };

    // Add a job and return its index
    int addJob(const Job& job) {
      jobs.push_back(job);
      sortJobsByPriority();
      return static_cast<int>(jobs.size()) - 1;
    }

    // Start a job
    bool startJob(size_t index) {
      if (index >= jobs.size()) return false;

      Job& job = jobs[index];

      // NEW: prevent restarting an active job
      if (job.active && !job.cancelled) {
        Serial.println(F("startJob failed: job already running"));
        return false;
      }

      job.active = true;
      job.cancelled = false;
      job.currentStep = 0;
      job.nextRunTime = millis();

      return true;
    }

    // Cancel a job
    void cancelJob(size_t index) {
      if (index >= jobs.size()) return;
      jobs[index].cancelled = true;
      jobs[index].active = false;
    }

    // Main scheduler loop
    void run() {
      unsigned long now = millis();

      for (auto& job : jobs) {
        if (!job.active || job.cancelled) continue;
        if (now < job.nextRunTime) continue;

        if (job.currentStep >= (int)job.steps.size()) {
          job.active = false;
          if (job.onComplete) job.onComplete();
          continue;
        }

        Step& step = job.steps[job.currentStep];

        // BRANCH_STEP handling 
        if (step.type == BRANCH_STEP) { 
          bool result = step.condition ? step.condition(domo) : false; 
          Serial.print(F("Branch step: ")); 
          Serial.println(step.description); 
          
          auto isValidIndex = [&](int idx) { 
            return idx >= 0 && idx < static_cast<int>(job.steps.size()); 
          }; 
            
          if (result) { 
            if (isValidIndex(step.thenStep)) { 
              job.currentStep = step.thenStep; 
            } else { 
              Serial.println(F("Invalid THEN index, skipping branch")); 
              job.currentStep++; 
            } 
          } else { 
            if (isValidIndex(step.elseStep)) { 
              job.currentStep = step.elseStep; 
            } else { 
              Serial.println(F("Invalid ELSE index, skipping branch")); 
              job.currentStep++; 
            } 
          } 
          job.nextRunTime = now; // evaluate next step immediately 
          continue; 
        } 
          
        // NEW: SKIP CONDITION handling 
        if (step.skipIf && step.skipIf(domo)) { 
          Serial.print(F("Skipping step: ")); 
          Serial.println(step.description); 
          job.currentStep++; 
          job.nextRunTime = now; 
          continue; 
        }

        // NORMAL_STEP handling (non-blocking) 
        if (!step.fnc) { 
          Serial.println(F("Null function pointer, skipping step")); 
          job.currentStep++; 
          job.nextRunTime = now; 
          continue; 
        }

        Serial.print("Running step: ");
        Serial.println(step.description);

        bool done = step.fnc(domo); 
        if (done) { 
          job.nextRunTime = now + step.delayAfterMs; job.currentStep++; 
        } else { 
          job.nextRunTime = now + 1; // retry soon 
        }
      }
    }

  private:
    std::vector<Job> jobs;
    
    void sortJobsByPriority() {
      std::sort(jobs.begin(), jobs.end(),
        [](const Job& a, const Job& b) {
            return a.priority > b.priority; // higher first
        }
      );
    }
};

#endif
