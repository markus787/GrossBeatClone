#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class GrossBeatCloneProcessor : public juce::AudioProcessor
{
public:
    GrossBeatCloneProcessor();
    ~GrossBeatCloneProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "GrossBeatClone"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    std::atomic<float>* pMix   = nullptr;
    std::atomic<float>* pCycle = nullptr;
    std::atomic<float>* pTime  = nullptr;
    std::atomic<float>* pVol   = nullptr;
    std::atomic<float>* pOut   = nullptr;

    juce::AudioBuffer<float> delayBuf;
    int writeIndex = 0;
    juce::int64 writeCount = 0;
    double phase = 0.0;
    double currentSampleRate = 44100.0;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> gainSmooth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GrossBeatCloneProcessor)
};
