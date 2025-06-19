#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include <vector>

static const std::vector<mrta::ParameterInfo> Parameters{
    {
        Param::ID::Enabled,
        Param::Name::Enabled,
        Param::Ranges::EnabledOff,
        Param::Ranges::EnabledOn,
        Param::Defaults::ProcessorEnabledDefault,
    },
    {
        Param::ID::OutputGain,
        Param::Name::OutputGain,
        Param::Units::Db,
        Param::Defaults::OutputGainDefault,
        Param::Ranges::OutputGainMin,
        Param::Ranges::OutputGainMax,
        Param::Ranges::OutputGainInc,
        Param::Ranges::OutputGainSkw,
    },
    {
        Param::ID::FroginessLevel,
        Param::Name::FroginessLevel,
        Param::Units::Percentage,
        Param::Defaults::FroginessDefault,
        Param::Ranges::FroginessMin,
        Param::Ranges::FroginessMax,
        Param::Ranges::FroginessInc,
        Param::Ranges::FroginessSkw,
    }};

DynamicsAudioProcessor::DynamicsAudioProcessor()
    : parameterManager(*this, ProjectInfo::projectName, Parameters) {
  parameterManager.registerParameterCallback(
      Param::ID::Enabled,
      [this](float newValue, bool) { processorEnabled = (newValue > 0.5f); });

  parameterManager.registerParameterCallback(
      Param::ID::OutputGain, [this](float newValueDb, bool forced) {
        auto linearGain = juce::Decibels::decibelsToGain(newValueDb);
        if (forced) {
          outputGainSmoother.setCurrentAndTargetValue(linearGain);
        } else {
          outputGainSmoother.setTargetValue(linearGain);
        }
      });

  parameterManager.registerParameterCallback(
      Param::ID::FroginessLevel, [this](float newValue, bool forced) {
        float scaledValue = newValue / 100.0f;
        if (forced) {
          frogginessSmoother.setCurrentAndTargetValue(scaledValue);
        } else {
          frogginessSmoother.setTargetValue(scaledValue);
        }
      });
}

DynamicsAudioProcessor::~DynamicsAudioProcessor() {}

void DynamicsAudioProcessor::prepareToPlay(double sampleRate,
                                           int samplesPerBlock) {
  currentSampleRate = sampleRate;
  juce::dsp::ProcessSpec spec{
      sampleRate, static_cast<juce::uint32>(samplesPerBlock),
      static_cast<juce::uint32>(getMainBusNumOutputChannels())};

  processorChain.prepare(spec);

  // Configure the static properties of the formant filters once
  processorChain.get<0>().get<0>().setType(
      juce::dsp::StateVariableTPTFilterType::bandpass);
  processorChain.get<0>().get<0>().setCutoffFrequency(200.0f);
  processorChain.get<1>().get<0>().setType(
      juce::dsp::StateVariableTPTFilterType::bandpass);
  processorChain.get<1>().get<0>().setCutoffFrequency(700.0f);
  processorChain.get<2>().get<0>().setType(
      juce::dsp::StateVariableTPTFilterType::bandpass);
  processorChain.get<2>().get<0>().setCutoffFrequency(1400.0f);

  lfo.prepare(spec);
  lfo.setFrequency(8.0f);

  outputGainSmoother.reset(sampleRate, 0.05);
  frogginessSmoother.reset(sampleRate, 0.05);

  parameterManager.updateParameters(true);
}

void DynamicsAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                          juce::MidiBuffer &) {
  juce::ScopedNoDenormals noDenormals;
  parameterManager.updateParameters();

  if (!processorEnabled) {
    return;
  }

  float currentFrogginess = frogginessSmoother.getNextValue();
  float outputGain = outputGainSmoother.getNextValue();

  if (currentFrogginess < 0.001f) {
    buffer.applyGain(outputGain);
    return;
  }

  // --- REAL-TIME SAFE PARAMETER UPDATES ---
  float formantGainDb = juce::jmap(currentFrogginess, 0.0f, 1.0f, 0.0f, 18.0f);
  float formantQ = juce::jmap(currentFrogginess, 0.0f, 1.0f, 1.0f, 5.0f);

  processorChain.get<0>().get<0>().setResonance(formantQ);
  processorChain.get<0>().get<1>().setGainDecibels(formantGainDb);

  processorChain.get<1>().get<0>().setResonance(formantQ);
  processorChain.get<1>().get<1>().setGainDecibels(formantGainDb);

  processorChain.get<2>().get<0>().setResonance(formantQ);
  processorChain.get<2>().get<1>().setGainDecibels(formantGainDb);

  processorChain.get<3>().frogginess = currentFrogginess;

  // --- DSP ---

  // 1. Generate LFO data in a block-based, safe way
  juce::AudioBuffer<float> lfoBuffer(buffer.getNumChannels(),
                                     buffer.getNumSamples());
  // lfoBuffer.clear();
  // juce::dsp::AudioBlock<float> lfoBlock(lfoBuffer);
  // lfo.process(juce::dsp::ProcessContextReplacing<float>(lfoBlock));

  // 2. Apply LFO as a correct tremolo gain (THE FIX)
  // This is the stable way to apply the gain modulation.
  // float lfoDepth = currentFrogginess;
  // for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
  //   auto *audioSamples = buffer.getWritePointer(channel);
  //   auto *lfoSamples = lfoBuffer.getReadPointer(channel);
  //
  //   for (int i = 0; i < buffer.getNumSamples(); ++i) {
  //     // Map LFO from [-1, 1] range to a [0, 1] range
  //     float lfoValue = (lfoSamples[i] + 1.0f) * 0.5f;
  //     // Calculate gain, ensuring it's always positive and max 1.0
  //     float tremoloGain = 1.0f - lfoDepth * lfoValue;
  //     audioSamples[i] *= tremoloGain;
  //   }
  // }

  // 3. Process through the formant and shaper chain
  juce::dsp::AudioBlock<float> audioBlock(buffer);
  processorChain.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));

  // 4. Apply final output gain
  buffer.applyGain(outputGain);
}

void DynamicsAudioProcessor::releaseResources() { processorChain.reset(); }

void DynamicsAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
  parameterManager.getStateInformation(destData);
}

void DynamicsAudioProcessor::setStateInformation(const void *data,
                                                 int sizeInBytes) {
  parameterManager.setStateInformation(data, sizeInBytes);
}

juce::AudioProcessorEditor *DynamicsAudioProcessor::createEditor() {
  return new DynamicsAudioProcessorEditor(*this);
}

const juce::String DynamicsAudioProcessor::getName() const {
  return JucePlugin_Name;
}
bool DynamicsAudioProcessor::acceptsMidi() const { return false; }
bool DynamicsAudioProcessor::producesMidi() const { return false; }
bool DynamicsAudioProcessor::isMidiEffect() const { return false; }
double DynamicsAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int DynamicsAudioProcessor::getNumPrograms() { return 1; }
int DynamicsAudioProcessor::getCurrentProgram() { return 0; }
void DynamicsAudioProcessor::setCurrentProgram(int) {}
const juce::String DynamicsAudioProcessor::getProgramName(int) { return {}; }
void DynamicsAudioProcessor::changeProgramName(int, const juce::String &) {}
bool DynamicsAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new DynamicsAudioProcessor();
}
