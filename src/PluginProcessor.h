#pragma once

#include <JuceHeader.h>

namespace Param {
namespace ID {
static const juce::String Enabled{"enabled"};
static const juce::String OutputGain{"output_gain"};
static const juce::String FroginessLevel{"froginess_level"};
} // namespace ID

namespace Name {
static const juce::String Enabled{"Enabled"};
static const juce::String OutputGain{"Output gain"};
static const juce::String FroginessLevel{"Froginess level"};
} // namespace Name

namespace Ranges {
static const juce::String EnabledOff{"Off"};
static const juce::String EnabledOn{"On"};
static constexpr float OutputGainMin{-24.0f};
static constexpr float OutputGainMax{6.0f};
static constexpr float OutputGainInc{0.1f};
static constexpr float OutputGainSkw{2.0f};
static constexpr float FroginessMin{0.0f};
static constexpr float FroginessMax{100.0f};
static constexpr float FroginessInc{1.0f};
static constexpr float FroginessSkw{1.0f};
} // namespace Ranges
namespace Defaults {
static constexpr bool ProcessorEnabledDefault{true};
static constexpr float OutputGainDefault{0.0f};
static constexpr float FroginessDefault{0.0f};
} // namespace Defaults

namespace Units {
static const juce::String Percentage{"%"};
static const juce::String Db{"dB"};
} // namespace Units
} // namespace Param

class DynamicsAudioProcessor : public juce::AudioProcessor {
public:
  DynamicsAudioProcessor();
  ~DynamicsAudioProcessor() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;
  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  mrta::ParameterManager &getParameterManager() { return parameterManager; }

  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;
  const juce::String getName() const override;
  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

private:
  mrta::ParameterManager parameterManager;
  double currentSampleRate = 0;
  bool processorEnabled = Param::Defaults::ProcessorEnabledDefault;
  juce::SmoothedValue<float> outputGainSmoother;
  juce::SmoothedValue<float> froginessSmoother;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DynamicsAudioProcessor)
};
