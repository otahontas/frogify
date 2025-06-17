#pragma once

namespace Utils {
float msToSamples(float timeMs, double sampleRate);
float calculateSmoothingCoefficient(float ms, double sampleRate);
float calculateOnePoleSmoothedOutput(float currentValue, float targetValue,
                                     float coefficient);
} // namespace Utils
