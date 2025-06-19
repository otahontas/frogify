#pragma once

#include <JuceHeader.h>

// This struct holds our stateful waveshaper logic
struct FrogWaveShaper {
  float frogginess = 0.0f;
  void prepare(const juce::dsp::ProcessSpec &) {}
  void reset() { frogginess = 0.0f; }

  template <typename ProcessContext>
  void process(const ProcessContext &context) noexcept {
    auto &inputBlock = context.getInputBlock();
    auto &outputBlock = context.getOutputBlock();
    if (context.isBypassed) {
      outputBlock.copyFrom(inputBlock);
      return;
    }
    for (size_t ch = 0; ch < outputBlock.getNumChannels(); ++ch) {
      auto *in = inputBlock.getChannelPointer(ch);
      auto *out = outputBlock.getChannelPointer(ch);
      for (size_t i = 0; i < outputBlock.getNumSamples(); ++i) {
        auto distorted = std::tanh(in[i] * (1.0f + frogginess * 4.0f));
        out[i] = (in[i] * (1.0f - frogginess)) + (distorted * frogginess);
      }
    }
  }
};

// Use the StateVariableTPTFilter which is safe for real-time parameter changes
using Filter = juce::dsp::StateVariableTPTFilter<float>;
// A FormantBand now contains the safe filter and a gain stage for the boost
using FormantBand = juce::dsp::ProcessorChain<Filter, juce::dsp::Gain<float>>;

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

  // DSP Objects
  juce::dsp::Oscillator<float> lfo;
  juce::dsp::ProcessorChain<FormantBand, FormantBand, FormantBand,
                            FrogWaveShaper>
      processorChain;

  // Parameters
  bool processorEnabled = Param::Defaults::ProcessorEnabledDefault;
  juce::SmoothedValue<float> frogginessSmoother;
  juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>
      outputGainSmoother;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DynamicsAudioProcessor)
};
