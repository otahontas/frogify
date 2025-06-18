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
      Param::ID::Enabled, [this](float newValue, bool /*forced*/) {
        processorEnabled = (newValue > 0.5f);
      });
  parameterManager.registerParameterCallback(
      Param::ID::OutputGain, [this](float newValue, bool forced) {
        float gainLinear = juce::Decibels::decibelsToGain(newValue);
        if (forced) {
          outputGainSmoother.setCurrentAndTargetValue(gainLinear);
        } else {
          outputGainSmoother.setTargetValue(gainLinear);
        }
      });
  parameterManager.registerParameterCallback(
      Param::ID::FroginessLevel, [this](float newValue, bool forced) {
        if (forced) {
          froginessSmoother.setCurrentAndTargetValue(newValue);
        } else {
          froginessSmoother.setTargetValue(newValue);
        }
      });
}

DynamicsAudioProcessor::~DynamicsAudioProcessor() {}

void DynamicsAudioProcessor::prepareToPlay(double sampleRate,
                                           int samplesPerBlock) {
  currentSampleRate = sampleRate;
  // int numChannels = getTotalNumInputChannels();
  // TODO:
}

void DynamicsAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                          juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  parameterManager.updateParameters();

  if (!processorEnabled) {
    return;
  }

  // TODO:
}

void DynamicsAudioProcessor::releaseResources() {
  // TODO
}

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
