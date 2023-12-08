
long mmOrInchToAbsolutePos(Axis* a, float mmOrInch) {
  long scaleToDu = measure == MEASURE_METRIC ? 10000 : 254000;
  long part1 = a->gcodeRelativePos;
  long part2 = round(mmOrInch * scaleToDu / a->screwPitch * a->motorSteps);
  return part1 + part2;
}

void stepToRelativeMmOrInch(Axis* a, float mmOrInch) {
  stepTo(a, mmOrInchToAbsolutePos(a, mmOrInch));
}