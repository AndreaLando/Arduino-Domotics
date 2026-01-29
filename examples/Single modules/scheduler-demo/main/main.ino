
/* ============================================================
   DEMO: AsyncScheduler (Generic Version, No DomoManager)
   ------------------------------------------------------------
   This example shows:
     - How to define your own context struct
     - How to write step functions using void* ctx
     - How to write condition functions
     - How to build a job with normal, branch, and skip steps
     - How to run the scheduler in a non-blocking loop
   ============================================================ */

#include <Arduino.h>
#include "Jobs.h"

/* ============================================================
   USER CONTEXT (replace with whatever you need)
   ============================================================ */
struct MyContext {
    int counter = 0;
    bool flag = false;
};

MyContext ctx;

/* ============================================================
   STEP FUNCTIONS (NORMAL STEPS)
   ============================================================ */

// Step 1: increment counter until it reaches 5
bool step_increment(void* c) {
    MyContext* ctx = static_cast<MyContext*>(c);
    ctx->counter++;

    Serial.print("[STEP] Increment counter: ");
    Serial.println(ctx->counter);

    return (ctx->counter >= 5);
}

// Step 2: toggle flag once
bool step_toggleFlag(void* c) {
    MyContext* ctx = static_cast<MyContext*>(c);
    ctx->flag = !ctx->flag;

    Serial.print("[STEP] Toggle flag -> ");
    Serial.println(ctx->flag);

    return true; // completes immediately
}

// Step 3: wait until counter reaches 10
bool step_waitCounter10(void* c) {
    MyContext* ctx = static_cast<MyContext*>(c);

    Serial.print("[STEP] Waiting for counter >= 10 (current: ");
    Serial.print(ctx->counter);
    Serial.println(")");

    ctx->counter++;
    return (ctx->counter >= 10);
}

/* ============================================================
   CONDITION FUNCTIONS
   ============================================================ */

// Branch condition: is counter even?
bool cond_isEven(void* c) {
    MyContext* ctx = static_cast<MyContext*>(c);
    return (ctx->counter % 2 == 0);
}

// Skip condition: skip step if flag is true
bool cond_skipIfFlag(void* c) {
    MyContext* ctx = static_cast<MyContext*>(c);
    return ctx->flag;
}

/* ============================================================
   COMPLETION CALLBACK
   ============================================================ */
void onJobComplete() {
    Serial.println("=== JOB COMPLETED SUCCESSFULLY ===");
}

/* ============================================================
   SCHEDULER INSTANCE
   ============================================================ */
AsyncScheduler scheduler;

/* ============================================================
   SETUP
   ============================================================ */
void setup() {
    Serial.begin(115200);
    delay(300);

    Serial.println("=== AsyncScheduler DEMO (No DomoManager) ===");

    // Attach context
    scheduler.setContext(&ctx);

    // Create a job
    AsyncScheduler::Job job;

    /* --------------------------------------------------------
       STEP 0: Increment counter to 5
       -------------------------------------------------------- */
    {
        AsyncScheduler::Step s;
        s.type = AsyncScheduler::NORMAL_STEP;
        s.fnc = step_increment;
        s.delayAfterMs = 500;
        s.description = "Increment counter to 5";
        job.steps.push_back(s);
    }

    /* --------------------------------------------------------
       STEP 1: Branch based on counter parity
       -------------------------------------------------------- */
    {
        AsyncScheduler::Step s;
        s.type = AsyncScheduler::BRANCH_STEP;
        s.condition = cond_isEven;
        s.thenStep = 2; // go to step 2 if even
        s.elseStep = 3; // go to step 3 if odd
        s.description = "Branch: is counter even?";
        job.steps.push_back(s);
    }

    /* --------------------------------------------------------
       STEP 2: Toggle flag (branch THEN)
       -------------------------------------------------------- */
    {
        AsyncScheduler::Step s;
        s.type = AsyncScheduler::NORMAL_STEP;
        s.fnc = step_toggleFlag;
        s.delayAfterMs = 300;
        s.description = "Toggle flag (branch THEN)";
        job.steps.push_back(s);
    }

    /* --------------------------------------------------------
       STEP 3: Wait until counter >= 10 (skip if flag is true)
       -------------------------------------------------------- */
    {
        AsyncScheduler::Step s;
        s.type = AsyncScheduler::NORMAL_STEP;
        s.fnc = step_waitCounter10;
        s.delayAfterMs = 200;
        s.skipIf = cond_skipIfFlag;
        s.description = "Wait until counter >= 10";
        job.steps.push_back(s);
    }

    /* --------------------------------------------------------
       STEP 4: Final flag toggle
       -------------------------------------------------------- */
    {
        AsyncScheduler::Step s;
        s.type = AsyncScheduler::NORMAL_STEP;
        s.fnc = step_toggleFlag;
        s.delayAfterMs = 0;
        s.description = "Final flag toggle";
        job.steps.push_back(s);
    }

    // Set priority and completion callback
    job.priority = 1;
    job.onComplete = onJobComplete;

    // Add job to scheduler
    int jobIndex = scheduler.addJob(job);

    // Start job
    scheduler.startJob(jobIndex);
}

/* ============================================================
   LOOP
   ============================================================ */
void loop() {
    scheduler.run();

    // Optional: print context state every second
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 1000) {
        lastPrint = millis();
        Serial.print("State -> Counter: ");
        Serial.print(ctx.counter);
        Serial.print(" | Flag: ");
        Serial.println(ctx.flag);
    }
}
