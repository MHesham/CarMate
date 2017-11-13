#include "common.h"

// the setup function runs once when you press reset or power the board
void setup() {

  vCoreInit();
  vCanInit();
  vWeatherInit();
  vShellInit();

  Serial << "CarMate RTOS is starting...\n";

  // Now the task scheduler, which takes over control of scheduling individual
  // tasks, is automatically started.
}

void loop() {
  // Empty. Things are done in Tasks.
}
