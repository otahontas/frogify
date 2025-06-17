#include "LevelDetector.h"
#include "Utils.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include <algorithm>
#include <cmath>

LevelDetector::LevelDetector() {}
LevelDetector::~LevelDetector() {}

void LevelDetector::prepare(double sampleRate) {
  currentSampleRate = sampleRate;
  attackDetectorCoefficient =
      Utils::calculateSmoothingCoefficient(attackDetectionTimeMs, sampleRate);
  releaseDetectorCoefficient =
      Utils::calculateSmoothingCoefficient(releaseDetectionTimeMs, sampleRate);
  reset();
}

float LevelDetector::process(float sample) {
  auto inputLevelDb =
      std::max(juce::Decibels::gainToDecibels(std::abs(sample)), silenceFloor);

  auto smoothedAttackOutput = Utils::calculateOnePoleSmoothedOutput(
      previousSmoothedAttackOutput, inputLevelDb, attackDetectorCoefficient);

  auto smoothedReleaseOutput = Utils::calculateOnePoleSmoothedOutput(
      previousSmoothedReleaseOutput, inputLevelDb, releaseDetectorCoefficient);

  currentEnvelopeDb = (inputLevelDb > currentEnvelopeDb)
                          ? smoothedAttackOutput
                          : smoothedReleaseOutput;

  previousSmoothedAttackOutput = smoothedAttackOutput;
  previousSmoothedReleaseOutput = smoothedReleaseOutput;

  return currentEnvelopeDb;
}

void LevelDetector::reset() {
  currentEnvelopeDb = silenceFloor;
  previousSmoothedAttackOutput = silenceFloor;
  previousSmoothedReleaseOutput = silenceFloor;
}
