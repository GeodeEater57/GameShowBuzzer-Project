// Player A: Green | Player B: Blue | Player C: Red
// Host Button (pin 13): Enables buzzing system
// Reset Button (pin 9): Resets LEDs only
// Note for grader: Original code was 3 pages long. I only added roughly 1 page, and I am still sorry.

// LED Pins
const int aLED = 12;  // Player A (Green)
const int bLED = 11;  // Player B (Blue)
const int cLED = 10;  // Player C (Red)

// Button Pins
const int aBuz = 4;    // Player A
const int bBuz = 3;    // Player B
const int cBuz = 2;    // Player C
const int hostButton = 13;
const int resetButton = 9;

// Game State
bool buzzingEnabled = false;
bool aLocked = false;
bool bLocked = false;
bool cLocked = false;
bool winA = false;
bool winB = false;
bool winC = false;
bool someoneWon = false;  // NEW: Track if someone has won

// Timing Control
unsigned long lockoutStartTime = 0;
const unsigned long buzzingWindow = 3000;
unsigned long punishmentStart = 0;
const unsigned long punishmentTime = 2000;

// Punishment System Variables
unsigned long punishEndTime = 0;
int punishLED = -1;
int blinkState = 0;
unsigned long lastBlinkTime = 0;

// Button State Tracking
int prevA = 0;
int prevB = 0;
int prevC = 0;
int prevHost = 0;
int prevReset = 0;

void setup() {
  pinMode(aLED, OUTPUT);
  pinMode(bLED, OUTPUT);
  pinMode(cLED, OUTPUT);
  pinMode(aBuz, INPUT);
  pinMode(bBuz, INPUT);
  pinMode(cBuz, INPUT);
  pinMode(hostButton, INPUT);
  pinMode(resetButton, INPUT);
  digitalWrite(aLED, LOW);
  digitalWrite(bLED, LOW);
  digitalWrite(cLED, LOW);
}

//  ===== NEW: Punishment =====
void startPunishment(int playerLED) {
  punishLED = playerLED;
  punishEndTime = millis() + 1200; // 3 blinks × 400ms
  blinkState = 0;
  lastBlinkTime = millis();
}

//  ===== NEW: Punishment Implementation =====
void handlePunishment() {
  if (punishLED == -1) return;
  
  unsigned long now = millis();
  if (now >= punishEndTime) {
    digitalWrite(punishLED, LOW);
    punishLED = -1;
    return;
  }

  if (now - lastBlinkTime >= 200) {
    lastBlinkTime = now;
    blinkState = !blinkState;
    digitalWrite(punishLED, blinkState ? HIGH : LOW);
  }
}

void resetGame() {
  digitalWrite(aLED, LOW);
  digitalWrite(bLED, LOW);
  digitalWrite(cLED, LOW);
  winA = false;
  winB = false;
  winC = false;
  someoneWon = false;  // NEW: Reset the win state
  aLocked = false;
  bLocked = false;
  cLocked = false;
}

void loop() {
  handlePunishment();  // Handle ongoing punishments first

  // Read inputs
  int stateA = digitalRead(aBuz);
  int stateB = digitalRead(bBuz);
  int stateC = digitalRead(cBuz);
  int hostState = digitalRead(hostButton);
  int resetState = digitalRead(resetButton);

  // Reset button (edge detection)
  if (resetState && !prevReset) {
    resetGame();
    delay(50);
  }
  prevReset = resetState;

  // Host button - enables buzzing system
  if (hostState && !prevHost) {
    buzzingEnabled = true;
    lockoutStartTime = millis();
    aLocked = bLocked = cLocked = false;
    winA = winB = winC = false;
    someoneWon = false;  // NEW: Reset win state when host enables
  }
  prevHost = hostState;

  // ===== NEW: Early button press detection (before host activates) =====
  if (!buzzingEnabled && !hostState) {  // Only check if host hasn't enabled yet
    if (stateA && !prevA && !aLocked) {
      aLocked = true;
      punishmentStart = millis();
      startPunishment(aLED);
    }
    if (stateB && !prevB && !bLocked) {
      bLocked = true;
      punishmentStart = millis();
      startPunishment(bLED);
    }
    if (stateC && !prevC && !cLocked) {
      cLocked = true;
      punishmentStart = millis();
      startPunishment(cLED);
    }
  }

  // Auto-disable buzzing after time window
  if (buzzingEnabled && (millis() - lockoutStartTime >= buzzingWindow)) {
    buzzingEnabled = false;
  }

  // Punishment timeout
  if (aLocked && (millis() - punishmentStart >= punishmentTime)) aLocked = false;
  if (bLocked && (millis() - punishmentStart >= punishmentTime)) bLocked = false;
  if (cLocked && (millis() - punishmentStart >= punishmentTime)) cLocked = false;

  // Old Buzzing logic (only when enabled and in time window)
  bool buzzingAllowed = buzzingEnabled && (millis() - lockoutStartTime < buzzingWindow);

  if (buzzingAllowed && !someoneWon) {  // NEW: Only allow buzzing if no one has won yet
    // Simultaneous press resolution
    if (stateA && stateB && !aLocked && !bLocked) {
      if (winA) {
        digitalWrite(bLED, HIGH); digitalWrite(aLED, LOW);
        winB = true; winA = false;
        someoneWon = true;  // NEW: Someone has won
      } else if (winB) {
        digitalWrite(aLED, HIGH); digitalWrite(bLED, LOW);
        winA = true; winB = false;
        someoneWon = true;  // NEW: Someone has won
      }
    }
    else if (stateA && stateC && !aLocked && !cLocked) {
      if (winA) {
        digitalWrite(cLED, HIGH); digitalWrite(aLED, LOW);
        winC = true; winA = false;
        someoneWon = true;  // NEW: Someone has won
      } else if (winC) {
        digitalWrite(aLED, HIGH); digitalWrite(cLED, LOW);
        winA = true; winC = false;
        someoneWon = true;  // NEW: Someone has won
      }
    }
    else if (stateB && stateC && !bLocked && !cLocked) {
      if (winB) {
        digitalWrite(cLED, HIGH); digitalWrite(bLED, LOW);
        winC = true; winB = false;
        someoneWon = true;  // NEW: Someone has won
      } else if (winC) {
        digitalWrite(bLED, HIGH); digitalWrite(cLED, LOW);
        winB = true; winC = false;
        someoneWon = true;  // NEW: Someone has won
      }
    }
    // Single presses
    else if (stateA && !prevA && !aLocked && !winB && !winC) {
      digitalWrite(aLED, HIGH); digitalWrite(bLED, LOW); digitalWrite(cLED, LOW);
      winA = true;
      someoneWon = true;  // NEW: Someone has won
    }
    else if (stateB && !prevB && !bLocked && !winA && !winC) {
      digitalWrite(bLED, HIGH); digitalWrite(aLED, LOW); digitalWrite(cLED, LOW);
      winB = true;
      someoneWon = true;  // NEW: Someone has won
    }
    else if (stateC && !prevC && !cLocked && !winA && !winB) {
      digitalWrite(cLED, HIGH); digitalWrite(aLED, LOW); digitalWrite(bLED, LOW);
      winC = true;
      someoneWon = true;  // NEW: Someone has won
    }
  } 
  // ====NEW: false start handling (after host enabled but outside time window)====
  else if (buzzingEnabled && !someoneWon) {  // NEW: Only punish if no one has won
    if (stateA && !prevA && !aLocked) {
      aLocked = true;
      punishmentStart = millis();
      startPunishment(aLED);
    }
    if (stateB && !prevB && !bLocked) {
      bLocked = true;
      punishmentStart = millis();
      startPunishment(bLED);
    }
    if (stateC && !prevC && !cLocked) {
      cLocked = true;
      punishmentStart = millis();
      startPunishment(cLED);
    }
  }

  // Update previous states
  prevA = stateA;
  prevB = stateB;
  prevC = stateC;
}