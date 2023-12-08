
void buttonPlusMinusPress(bool plus) {
  // Mutex is aquired in setDupr() and setStarts().
  bool minus = !plus;
  if (mode == MODE_THREAD && setupIndex == 2) {
    if (minus && starts > 2) {
      setStarts(starts - 1);
    } else if (plus && starts < STARTS_MAX) {
      setStarts(starts + 1);
    }
  } else if (isPassMode() && setupIndex == 1 && getNumpadResult() == 0) {
    if (minus && turnPasses > 1) {
      setTurnPasses(turnPasses - 1);
    } else if (plus && turnPasses < PASSES_MAX) {
      setTurnPasses(turnPasses + 1);
    }
  } else if (measure != MEASURE_TPI) {
    bool isMetric = measure == MEASURE_METRIC;
    int delta = isMetric ? MOVE_STEP_3 : MOVE_STEP_IMP_3;
    if (moveStep == MOVE_STEP_4) {
      // Don't speed up scrolling when on smallest step.
      delta = MOVE_STEP_4;
    }
    // Switching between mm/inch/tpi often results in getting non-0 3rd and 4th
    // precision points that can't be easily controlled. Remove them.
    long normalizedDupr = normalizePitch(dupr);
    if (minus) {
      if (dupr > -DUPR_MAX) {
        setDupr(max(-DUPR_MAX, normalizedDupr - delta));
      }
    } else if (plus) {
      if (dupr < DUPR_MAX) {
        setDupr(min(DUPR_MAX, normalizedDupr + delta));
      }
    }
  } else { // TPI
    if (dupr == 0) {
      setDupr(plus ? 1 : -1);
    } else {
      long currentTpi = round(254000.0 / dupr);
      long tpi = currentTpi + (plus ? 1 : -1);
      long newDupr = newDupr = round(254000.0 / tpi);
      // Happens for small pitches like 0.01mm.
      if (newDupr == dupr) {
        newDupr += plus ? -1 : 1;
      }
      if (newDupr != dupr && newDupr < DUPR_MAX && newDupr > -DUPR_MAX) {
        setDupr(newDupr);
      }
    }
  }
}

void beep() {
  tone(BUZZ, 1000, 500);
}

void buttonOnOffPress(bool on) {
  resetMillis = millis();
  if (on && isPassMode() && (needZStops() && (z.leftStop == LONG_MAX || z.rightStop == LONG_MIN) || x.leftStop == LONG_MAX || x.rightStop == LONG_MIN)) {
    beep();
  } else if (on && getLastSetupIndex() != 0 && setupIndex < getLastSetupIndex()) {
    // Move to the next setup step.
    setupIndex++;
  } else {
    setIsOn(on);
  }
}

void buttonOffRelease() {
  if (millis() - resetMillis > 4000) {
    reset();
    splashScreen();
  }
}



void buttonLeftStopPress() {
  setLeftStop(&z, z.leftStop == LONG_MAX ? z.pos : LONG_MAX);
}

void buttonRightStopPress() {
  setRightStop(&z, z.rightStop == LONG_MIN ? z.pos : LONG_MIN);
}

void buttonUpStopPress() {
  setLeftStop(&x, x.leftStop == LONG_MAX ? x.pos : LONG_MAX);
}

void buttonDownStopPress() {
  setRightStop(&x, x.rightStop == LONG_MIN ? x.pos : LONG_MIN);
}


void buttonDisplayPress() {
  if (!showAngle && !showTacho) {
    showAngle = true;
  } else if (showAngle) {
    showAngle = false;
    showTacho = true;
  } else {
    showTacho = false;
  }
}

void buttonMoveStepPress() {
  if (measure == MEASURE_METRIC) {
    if (moveStep == MOVE_STEP_1) {
      moveStep = MOVE_STEP_2;
    } else if (moveStep == MOVE_STEP_2) {
      moveStep = MOVE_STEP_3;
    } else if (moveStep == MOVE_STEP_3) {
      moveStep = MOVE_STEP_4;
    } else {
      moveStep = MOVE_STEP_1;
    }
  } else {
    if (moveStep == MOVE_STEP_IMP_1) {
      moveStep = MOVE_STEP_IMP_2;
    } else if (moveStep == MOVE_STEP_IMP_2) {
      moveStep = MOVE_STEP_IMP_3;
    } else {
      moveStep = MOVE_STEP_IMP_1;
    }
  }
}



void buttonModePress() {
  if (mode == MODE_NORMAL) {
    setMode(MODE_ELLIPSE);
  } else if (mode == MODE_ELLIPSE) {
    setMode(MODE_GCODE);
  } else if (mode == MODE_GCODE) {
    setMode(MODE_ASYNC);
  } else {
    setMode(MODE_NORMAL);
  }
}

void buttonMeasurePress() {
  if (measure == MEASURE_METRIC) {
    setMeasure(MEASURE_INCH);
  } else if (measure == MEASURE_INCH) {
    setMeasure(MEASURE_TPI);
  } else {
    setMeasure(MEASURE_METRIC);
  }
}

void buttonReversePress() {
  setDupr(-dupr);
}

void numpadPress(int digit) {
  if (!inNumpad) {
    numpadIndex = 0;
  }
  numpadDigits[numpadIndex] = digit;
  if (numpadIndex < 7) {
    numpadIndex++;
  } else {
    numpadIndex = 0;
  }
}

void numpadBackspace() {
  if (inNumpad && numpadIndex > 0) {
    numpadIndex--;
  }
}

void resetNumpad() {
  numpadIndex = 0;
}

long getNumpadResult() {
  long result = 0;
  for (int i = 0; i < numpadIndex; i++) {
    result += numpadDigits[i] * pow(10, numpadIndex - 1 - i);
  }
  return result;
}

void numpadPlusMinus(bool plus) {
  if (numpadDigits[numpadIndex - 1] < 9 && plus) {
    numpadDigits[numpadIndex - 1]++;
  } else if (numpadDigits[numpadIndex - 1] > 1 && !plus) {
    numpadDigits[numpadIndex - 1]--;
  }
  // TODO: implement going over 9 and below 1.
}

long numpadToDeciMicrons() {
  long result = getNumpadResult();
  if (result == 0) {
    return 0;
  }
  if (measure == MEASURE_INCH) {
    result = result * 254;
  } else if (measure == MEASURE_TPI) {
    result = round(254000.0 / result);
  } else { // Metric
    result = result * 10;
  }
  return result;
}

float numpadToConeRatio() {
  return getNumpadResult() / 100000.0;
}

bool processNumpad(int keyCode) {
  if (keyCode == B_0) {
    numpadPress(0);
    inNumpad = true;
  } else if (keyCode == B_1) {
    numpadPress(1);
    inNumpad = true;
  } else if (keyCode == B_2) {
    numpadPress(2);
    inNumpad = true;
  } else if (keyCode == B_3) {
    numpadPress(3);
    inNumpad = true;
  } else if (keyCode == B_4) {
    numpadPress(4);
    inNumpad = true;
  } else if (keyCode == B_5) {
    numpadPress(5);
    inNumpad = true;
  } else if (keyCode == B_6) {
    numpadPress(6);
    inNumpad = true;
  } else if (keyCode == B_7) {
    numpadPress(7);
    inNumpad = true;
  } else if (keyCode == B_8) {
    numpadPress(8);
    inNumpad = true;
  } else if (keyCode == B_9) {
    numpadPress(9);
    inNumpad = true;
  } else if (keyCode == B_BACKSPACE) {
    numpadBackspace();
    inNumpad = true;
  } else if (inNumpad && (keyCode == B_PLUS || keyCode == B_MINUS)) {
    numpadPlusMinus(keyCode == B_PLUS);
    return true;
  } else if (inNumpad) {
    inNumpad = false;
    return processNumpadResult(keyCode);
  }
  return inNumpad;
}

bool processNumpadResult(int keyCode) {
  long newDu = numpadToDeciMicrons();
  float newConeRatio = numpadToConeRatio();
  long numpadResult = getNumpadResult();
  resetNumpad();
  // Ignore numpad input unless confirmed with ON.
  if (keyCode == B_ON) {
    if (isPassMode() && setupIndex == 1) {
      setTurnPasses(int(min(long(PASSES_MAX), numpadResult)));
    } else if (mode == MODE_CONE && setupIndex == 1) {
      setConeRatio(newConeRatio);
    } else {
      if (abs(newDu) <= DUPR_MAX) {
        setDupr(newDu);
      }
    }
    // Don't use this ON press for starting the motion.
    return true;
  }

  // Shared piece for stops and moves.
  Axis* a = (keyCode == B_STOPL || keyCode == B_STOPR || keyCode == B_LEFT || keyCode == B_RIGHT) ? &z : &x;
  long pos = a->pos + newDu / a->screwPitch * a->motorSteps * ((keyCode == B_STOPL || keyCode == B_STOPU || keyCode == B_LEFT || keyCode == B_UP) ? 1 : -1);

  // Potentially assign a new value to a limit. Treat newDu as a relative distance from current position.
  if (keyCode == B_STOPL) {
    setLeftStop(&z, pos);
    return true;
  } else if (keyCode == B_STOPR) {
    setRightStop(&z, pos);
    return true;
  } else if (keyCode == B_STOPU) {
    setLeftStop(&x, pos);
    return true;
  } else if (keyCode == B_STOPD) {
    setRightStop(&x, pos);
    return true;
  }

  // Potentially move by newDu in the given direction.
  // We don't support precision manual moves when ON yet. Can't stay in the thread for most modes.
  if (!isOn && (keyCode == B_LEFT || keyCode == B_RIGHT || keyCode == B_UP || keyCode == B_DOWN)) {
    if (pos < a->rightStop) {
      pos = a->rightStop;
      beep();
    } else if (pos > a->leftStop) {
      pos = a->leftStop;
      beep();
    } else if (abs(pos - a->pos) > a->estopSteps) {
      beep();
      return true;
    }
    a->speedMax = a->speedManualMove;
    stepTo(a, pos);
    return true;
  }

  if (keyCode == B_STEP) {
    if (newDu > 0) {
      moveStep = newDu;
    } else {
      beep();
    }
    return true;
  }

  return false;
}

void processKeypadEvents() {
  while (keypad.available() > 0) {
    int event = keypad.getEvent();
    int keyCode = event;
    bitWrite(keyCode, 7, 0);
    bool isPress = bitRead(event, 7) == 1; // 1 - press, 0 - release

    // Off button always gets handled.
    if (keyCode == B_OFF) {
      buttonOffPressed = isPress;
      isPress ? buttonOnOffPress(false) : buttonOffRelease();
    }

    if (mode == MODE_GCODE && isOn) {
      // Not allowed to interfere other than turn off.
      if (isPress && keyCode != B_OFF) beep();
      return;
    }

    // Releases don't matter in numpad but it has to run before LRUD since it might handle those keys.
    if (isPress && processNumpad(keyCode)) {
      continue;
    }

    // Setup wizard navigation.
    if ((setupIndex == 2) && (keyCode == B_LEFT || keyCode == B_RIGHT)) {
      if (isPress) {
        auxForward = !auxForward;
      }
    } else if (keyCode == B_LEFT) {
      buttonLeftPressed = isPress;
    } else if (keyCode == B_RIGHT) {
      buttonRightPressed = isPress;
    } else if (keyCode == B_UP) {
      buttonUpPressed = isPress;
    } else if (keyCode == B_DOWN) {
      buttonDownPressed = isPress;
    }

    // For all other keys we have no "release" logic.
    if (!isPress) {
      continue;
    }

    // Rest of the buttons.
    if (keyCode == B_PLUS) {
      buttonPlusMinusPress(true);
    } else if (keyCode == B_MINUS) {
      buttonPlusMinusPress(false);
    } else if (keyCode == B_ON) {
      buttonOnOffPress(true);
    } else if (keyCode == B_STOPL) {
      buttonLeftStopPress();
    } else if (keyCode == B_STOPR) {
      buttonRightStopPress();
    } else if (keyCode == B_STOPU) {
      buttonUpStopPress();
    } else if (keyCode == B_STOPD) {
      buttonDownStopPress();
    } else if (keyCode == B_MODE_OTHER) {
      buttonModePress();
    } else if (keyCode == B_DISPL) {
      buttonDisplayPress();
    } else if (keyCode == B_X) {
      markX0();
    } else if (keyCode == B_Z) {
      markZ0();
    } else if (keyCode == B_A) {
      x.isManuallyDisabled = !x.isManuallyDisabled;
      updateEnable(&x);
    } else if (keyCode == B_B) {
      z.isManuallyDisabled = !z.isManuallyDisabled;
      updateEnable(&z);
    } else if (keyCode == B_STEP) {
      buttonMoveStepPress();
    } else if (keyCode == B_SETTINGS) {
      // TODO.
    } else if (keyCode == B_REVERSE) {
      buttonReversePress();
    } else if (keyCode == B_MEASURE) {
      buttonMeasurePress();
    } else if (keyCode == B_MODE_GEARS) {
      setMode(MODE_NORMAL);
    } else if (keyCode == B_MODE_TURN) {
      setMode(MODE_TURN);
    } else if (keyCode == B_MODE_FACE) {
      setMode(MODE_FACE);
    } else if (keyCode == B_MODE_CONE) {
      setMode(MODE_CONE);
    } else if (keyCode == B_MODE_CUT) {
      setMode(MODE_CUT);
    } else if (keyCode == B_MODE_THREAD) {
      setMode(MODE_THREAD);
    }
  }
}


// Returns number of letters printed.
int printDeciMicrons(long deciMicrons, int precisionPointsMax) {
  if (deciMicrons == 0) {
    return lcd.print("0");
  }
  bool imperial = measure != MEASURE_METRIC;
  long v = imperial ? round(deciMicrons / 25.4) : deciMicrons;
  int points = 0;
  if (v == 0) {
    points = 5;
  } else if ((v % 10) != 0) {
    points = 4;
  } else if ((v % 100) != 0) {
    points = 3;
  } else if ((v % 1000) != 0) {
    points = 2;
  } else if ((v % 10000) != 0) {
    points = 1;
  }
  int count = lcd.print(deciMicrons / (imperial ? 254000.0 : 10000.0), min(precisionPointsMax, points));
  count += lcd.print(imperial ? "\"" : "mm");
  return count;
}

int printDupr(long value) {
  int count = 0;
  if (measure != MEASURE_TPI) {
    count += printDeciMicrons(value, 5);
  } else {
    float tpi = 254000.0 / value;
    if (abs(tpi - round(tpi)) < TPI_ROUND_EPSILON) {
      count += lcd.print(int(round(tpi)));
    } else {
      int tpi100 = round(tpi * 100);
      int points = 0;
      if ((tpi100 % 10) != 0) {
        points = 2;
      } else if ((tpi100 % 100) != 0) {
        points = 1;
      }
      count += lcd.print(tpi, points);
    }
    count += lcd.print("tpi");
  }
  return count;
}

void printLcdSpaces(int charIndex) {
  // Our screen has width 20.
  for (; charIndex < 20; charIndex++) {
    lcd.print(" ");
  }
}


int printNoTrailing0(float value) {
  long v = round(value * 100000);
  int points = 0;
  if ((v % 10) != 0) {
    points = 5;
  } else if ((v % 100) != 0) {
    points = 4;
  } else if ((v % 1000) != 0) {
    points = 3;
  } else if ((v % 10000) != 0) {
    points = 2;
  } else if ((v % 100000) != 0) {
    points = 1;
  }
  return lcd.print(value, points);
}


void updateDisplay() {
  if (emergencyStop != ESTOP_NONE) {
    return;
  }
  int rpm = showTacho ? getApproxRpm() : 0;
  int charIndex = 0;

  if (lcdHashLine0 == LCD_HASH_INITIAL) {
    // First run after reset.
    lcd.clear();
    lcdHashLine1 = LCD_HASH_INITIAL;
    lcdHashLine2 = LCD_HASH_INITIAL;
    lcdHashLine3 = LCD_HASH_INITIAL;
  }

  long newHashLine0 = isOn + (z.leftStop - z.rightStop) + (x.leftStop - x.rightStop) + spindlePosSync + moveStep + mode + measure;
  if (lcdHashLine0 != newHashLine0) {
    lcdHashLine0 = newHashLine0;
    charIndex = 0;
    lcd.setCursor(0, 0);
    if (mode == MODE_ASYNC) {
      charIndex += lcd.print("ASY ");
    } else if (mode == MODE_CONE) {
      charIndex += lcd.print("CONE ");
    } else if (mode == MODE_TURN) {
      charIndex += lcd.print("TURN ");
    } else if (mode == MODE_FACE) {
      charIndex += lcd.print("FACE ");
    } else if (mode == MODE_CUT) {
      charIndex += lcd.print("CUT ");
    } else if (mode == MODE_THREAD) {
      charIndex += lcd.print("THRD ");
    } else if (mode == MODE_ELLIPSE) {
      charIndex += lcd.print("ELLI ");
    } else if (mode == MODE_GCODE) {
      charIndex += lcd.print("GCODE ");
    }
    charIndex += lcd.print(isOn ? "ON " : "off ");
    int beforeStops = charIndex;
    if (z.leftStop != LONG_MAX) {
      charIndex += lcd.print("L");
    }
    if (z.rightStop != LONG_MIN) {
      charIndex += lcd.print("R");
    }
    if (x.leftStop != LONG_MAX) {
      charIndex += lcd.print("U");
    }
    if (x.rightStop != LONG_MIN) {
      charIndex += lcd.print("D");
    }
    if (beforeStops != charIndex) {
      charIndex += lcd.print(" ");
    }

    if (spindlePosSync && !isPassMode()) {
      charIndex += lcd.print("SYN ");
    }
    if (mode == MODE_NORMAL && !spindlePosSync) {
      charIndex += lcd.print("step ");
    }
    charIndex += printDeciMicrons(moveStep, 5);
    printLcdSpaces(charIndex);
  }

  long newHashLine1 = dupr + starts + mode + measure;
  if (lcdHashLine1 != newHashLine1) {
    lcdHashLine1 = newHashLine1;
    charIndex = 0;
    lcd.setCursor(0, 1);
    charIndex += lcd.print("Pitch ");
    charIndex += printDupr(dupr);
    if (starts != 1) {
      charIndex += lcd.print(" x");
      charIndex += lcd.print(starts);
    }
    printLcdSpaces(charIndex);
  }

  long zDisplayPos = z.pos + z.originPos;
  long xDisplayPos = x.pos + x.originPos;
  long newHashLine2 = zDisplayPos + xDisplayPos + measure + z.isManuallyDisabled + x.isManuallyDisabled;
  if (lcdHashLine2 != newHashLine2) {
    lcdHashLine2 = newHashLine2;
    charIndex = 0;
    lcd.setCursor(0, 2);
    if (z.isManuallyDisabled) {
      charIndex += lcd.print("Z disable");
    } else {
      if (xDisplayPos == 0 && !x.isManuallyDisabled) {
        charIndex += lcd.print("Position ");
      }
      charIndex += printAxisPos(&z);
    }
    while (charIndex < 10) {
      charIndex += lcd.print(" ");
    }
    if (x.isManuallyDisabled) {
      charIndex += lcd.print("X disable");
    } else if (xDisplayPos != 0) {
      charIndex += printAxisPos(&x);
    }
    printLcdSpaces(charIndex);
  }

  long numpadResult = getNumpadResult();
  long gcodeCommandHash = 0;
  for (int i = 0; i < gcodeCommand.length(); i++) {
    gcodeCommandHash += gcodeCommand.charAt(i);
  }
  long newHashLine3 = z.pos + (showAngle ? spindlePos : -1) + (showTacho ? rpm : -2) + measure + (numpadResult > 0 ? numpadResult : -1) + mode * 5 + dupr +
      (mode == MODE_CONE ? round(coneRatio * 10000) : 0) + turnPasses + opIndex + setupIndex + isOn * 4 + (inNumpad ? 10 : 0) + (auxForward ? 1 : 0) +
      (z.leftStop == LONG_MAX ? 123 : z.leftStop) + (z.rightStop == LONG_MIN ? 1234 : z.rightStop) +
      (x.leftStop == LONG_MAX ? 1235 : x.leftStop) + (x.rightStop == LONG_MIN ? 123456 : x.rightStop) + gcodeCommandHash;
  if (lcdHashLine3 != newHashLine3) {
    lcdHashLine3 = newHashLine3;
    charIndex = 0;
    lcd.setCursor(0, 3);
    if (mode == MODE_GCODE) {
      charIndex += lcd.print(gcodeCommand.substring(0, 20));
    } else if (isPassMode()) {
      bool missingStops = needZStops() && (z.leftStop == LONG_MAX || z.rightStop == LONG_MIN) || x.leftStop == LONG_MAX || x.rightStop == LONG_MIN;
      if (!inNumpad && missingStops) {
        charIndex += lcd.print(needZStops() ? "Set all stops" : "Set X stops");
      } else if (numpadResult != 0 && setupIndex == 1) {
        long passes = min(long(PASSES_MAX), numpadResult);
        charIndex += lcd.print(passes);
        charIndex += lcd.print(" passes?");
      } else if (!isOn && setupIndex == 1) {
        charIndex += lcd.print(turnPasses);
        charIndex += lcd.print(" passes?");
      } else if (!isOn && setupIndex == 2) {
        if (mode == MODE_FACE) {
          charIndex += lcd.print(auxForward ? "Right to left?" : "Left to right?");
        } else if (mode == MODE_CUT) {
          charIndex += lcd.print(dupr >= 0 ? "Pitch > 0, external" : "Pitch < 0, internal");
        } else {
          charIndex += lcd.print(auxForward ? "External?" : "Internal?");
        }
      } else if (!isOn && setupIndex == 3) {
        charIndex += lcd.print("Go?");
      } else if (isOn && numpadResult == 0) {
        charIndex += lcd.print("Operation ");
        charIndex += lcd.print(opIndex);
        charIndex += lcd.print(" of ");
        charIndex += lcd.print(turnPasses * starts);
      }
    } else if (mode == MODE_CONE) {
      if (numpadResult != 0 && setupIndex == 1) {
        charIndex += lcd.print("Use ratio ");
        charIndex += lcd.print(numpadToConeRatio(), 5);
        charIndex += lcd.print("?");
      } else if (!isOn && setupIndex == 1) {
        charIndex += lcd.print("Use ratio ");
        charIndex += printNoTrailing0(coneRatio);
        charIndex += lcd.print("?");
      } else if (!isOn && setupIndex == 2) {
        charIndex += lcd.print(auxForward ? "External?" : "Internal?");
      } else if (!isOn && setupIndex == 3) {
        charIndex += lcd.print("Go?");
      } else if (isOn && numpadResult == 0) {
        charIndex += lcd.print("Cone ratio ");
        charIndex += printNoTrailing0(coneRatio);
      }
    }

    if (charIndex == 0 && inNumpad) { // Also show for 0 input to allow setting limits to 0.
      charIndex += lcd.print("Use ");
      charIndex += printDupr(numpadToDeciMicrons());
      charIndex += lcd.print("?");
    }

    if (charIndex > 0) {
      // No space for shared RPM/angle text.
    } else if (showAngle) {
      charIndex += lcd.print("Angle ");
      charIndex += lcd.print(spindleModulo(spindlePos) * 360 / ENCODER_STEPS, 2);
      charIndex += lcd.print(char(223));
    } else if (showTacho) {
      charIndex += lcd.print("Tacho ");
      charIndex += lcd.print(rpm);
      if (shownRpm != rpm) {
        shownRpm = rpm;
        shownRpmTime = micros();
      }
      charIndex += lcd.print("rpm");
    }
    printLcdSpaces(charIndex);
  }
}


void taskDisplay(void *param) {
  while (emergencyStop == ESTOP_NONE) {
    updateDisplay();
    // Calling Preferences.commit() blocks all interrupts for 30ms, don't call saveIfChanged() if
    // encoder is likely to move soon.
    unsigned long now = micros();
    if (!stepperIsRunning(&z) && !stepperIsRunning(&x) && (now > spindleEncTime + SAVE_DELAY_US) && (now < saveTime || now > saveTime + SAVE_DELAY_US)) {
      if (saveIfChanged()) {
        saveTime = now;
      }
    }
    if (beepFlag) {
      beepFlag = false;
      beep();
    }
    if (abs(z.pendingPos) > z.estopSteps || abs(x.pendingPos) > x.estopSteps) {
      setEmergencyStop(ESTOP_POS);
    }
    taskYIELD();
  }
  reset();
  saveIfChanged();
}

void taskKeypad(void *param) {
  while (emergencyStop == ESTOP_NONE) {
    processKeypadEvents();
    taskYIELD();
  }
}

void waitForPendingPosNear0(Axis* a) {
  while (abs(a->pendingPos) > a->motorSteps / 10) {
    taskYIELD();
  }
}

void waitForPendingPos0(Axis* a) {
  while (a->pendingPos != 0) {
    taskYIELD();
  }
}

bool isContinuousStep() {
  return moveStep == (measure == MEASURE_METRIC ? MOVE_STEP_1 : MOVE_STEP_IMP_1);
}

long getStepMaxSpeed(Axis* a) {
  return isContinuousStep() ? a->speedManualMove : min(long(a->speedManualMove), abs(moveStep) * 1000 / STEP_TIME_MS);
}

int printAxisPos(Axis* a) {
  return printDeciMicrons(getAxisPosDu(a), 3);
}