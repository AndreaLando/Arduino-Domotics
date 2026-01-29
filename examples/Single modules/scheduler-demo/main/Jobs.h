#ifndef Jobs_H
#define Jobs_H

#pragma once
#include <Arduino.h>
#include <vector>
#include <algorithm>

/* ============================================================
   AsyncScheduler - Generic Non‑Blocking Step Scheduler
   ------------------------------------------------------------
   USAGE INSTRUCTIONS
   ------------------------------------------------------------
   This scheduler allows you to build asynchronous, non‑blocking
   workflows composed of steps. Each step is a function that
   receives a generic `void* context` pointer and returns true
   when the step is complete.

   HOW TO USE:

   1) Create a context object (any type you want):
        MyContext ctx;

   2) Create an AsyncScheduler instance:
        AsyncScheduler scheduler;
        scheduler.setContext(&ctx);

   3) Define step functions:
        bool stepA(void* ctx);
        bool stepB(void* ctx);

      Each function:
        - receives your context pointer
        - returns true when the step is finished
        - returns false to run again on next loop cycle

   4) Create a Job and add steps:
        AsyncScheduler::Job job;

        AsyncScheduler::Step s;
        s.type = AsyncScheduler::NORMAL_STEP;
        s.fnc = stepA;
        s.delayAfterMs = 500;
        s.description = "Example step";
        job.steps.push_back(s);

   5) (Optional) Add BRANCH steps:
        s.type = AsyncScheduler::BRANCH_STEP;
        s.condition = myCondition;
        s.thenStep = 2;
        s.elseStep = 3;

   6) (Optional) Add SKIP conditions:
        s.skipIf = mySkipCondition;

   7) Add job to scheduler:
        int jobIndex = scheduler.addJob(job);

   8) Start the job:
        scheduler.startJob(jobIndex);

   9) Call run() inside loop():
        void loop() {
            scheduler.run();
        }

   FEATURES:
     - Non‑blocking execution
     - Normal steps
     - Branch steps (if/else logic)
     - Skip conditions
     - Step delays
     - Job priorities
     - Completion callbacks
     - Generic context pointer (no dependency on any class)

   This file contains ONLY the generic scheduler.
   You may create derived classes (e.g., DomoScheduler) that
   inject a typed context for convenience.
   ============================================================ */


/* ============================================================
   AsyncScheduler (Generic Version)
   ============================================================ */
class AsyncScheduler {
public:
    typedef bool (*FunctionPointer)(void* ctx);
    typedef bool (*ConditionFunction)(void* ctx);
    typedef void (*CompletionCallback)();

    enum StepType { NORMAL_STEP, BRANCH_STEP };

    struct Step {
        StepType type = NORMAL_STEP;

        FunctionPointer fnc = nullptr;
        unsigned long delayAfterMs = 0;

        ConditionFunction condition = nullptr;
        int thenStep = -1;
        int elseStep = -1;

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

protected:
    void* context = nullptr;
    std::vector<Job> jobs;

public:
    AsyncScheduler() = default;
    
    void setContext(void* ctx) {
        context = ctx;
    }

    int addJob(const Job& job) {
        jobs.push_back(job);
        sortJobsByPriority();
        return jobs.size() - 1;
    }

    bool startJob(size_t index) {
        if (index >= jobs.size()) return false;

        Job& job = jobs[index];

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

    void cancelJob(size_t index) {
        if (index >= jobs.size()) return;
        jobs[index].cancelled = true;
        jobs[index].active = false;
    }

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

            if (step.type == BRANCH_STEP) {
                bool result = step.condition ? step.condition(context) : false;

                Serial.print(F("Branch step: "));
                Serial.println(step.description);

                auto valid = [&](int idx) {
                    return idx >= 0 && idx < (int)job.steps.size();
                };

                if (result) {
                    job.currentStep = valid(step.thenStep) ? step.thenStep : job.currentStep + 1;
                } else {
                    job.currentStep = valid(step.elseStep) ? step.elseStep : job.currentStep + 1;
                }

                job.nextRunTime = now;
                continue;
            }

            if (step.skipIf && step.skipIf(context)) {
                Serial.print(F("Skipping step: "));
                Serial.println(step.description);
                job.currentStep++;
                job.nextRunTime = now;
                continue;
            }

            if (!step.fnc) {
                Serial.println(F("Null function pointer, skipping step"));
                job.currentStep++;
                job.nextRunTime = now;
                continue;
            }

            Serial.print(F("Running step: "));
            Serial.println(step.description);

            bool done = step.fnc(context);

            if (done) {
                job.nextRunTime = now + step.delayAfterMs;
                job.currentStep++;
            } else {
                job.nextRunTime = now + 1;
            }
        }
    }

protected:
    void sortJobsByPriority() {
        std::sort(jobs.begin(), jobs.end(),
            [](const Job& a, const Job& b) {
                return a.priority > b.priority;
            }
        );
    }
};

#endif
