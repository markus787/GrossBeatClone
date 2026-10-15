#include "PluginProcessor.h"
#include "Curves.h"

namespace
{
    constexpr double kCycleBeats[] = { 0.5, 1.0, 2.0, 4.0, 8.0 };
}

GrossBeatCloneProcessor::GrossBeatCloneProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    pMix   = apvts.getRawParameterValue ("mix");
    pCycle = apvts.getRawParameterValue ("cycle");
    pTime  = apvts.getRawParameterValue ("time");
    pVol   = apvts.getRawParameterValue ("vol");
    pOut   = apvts.getRawParameterValue ("out");
}

juce::AudioProcessorValueTreeState::ParameterLayout GrossBeatCloneProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "mix", 1 }, "Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "cycle", 1 }, "Zyklus",
        juce::StringArray { "1/2 Beat", "1 Beat", "2 Beats", "1 Takt", "2 Takte" }, 3));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "time", 1 }, "Zeit-Kurve", gb::timeNames(), 0));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "vol", 1 }, "Lautstaerke-Kurve", gb::volumeNames(), 0));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "out", 1 }, "Ausgang (dB)",
        juce::NormalisableRange<float> (-24.0f, 12.0f, 0.1f), 0.0f));

    return layout;
}

void GrossBeatCloneProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    delayBuf.setSize (2, (int) (sampleRate * 60.0));   // 60 Sekunden Puffer
    delayBuf.clear();
    writeIndex = 0;
    writeCount = 0;
    phase = 0.0;
    gainSmooth.reset (sampleRate, 0.003);
    gainSmooth.setCurrentAndTargetValue (1.0f);
}

bool GrossBeatCloneProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return out == layouts.getMainInputChannelSet();
}

void GrossBeatCloneProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numCh = juce::jmin (buffer.getNumChannels(), 2);

    for (int ch = getTotalNumInputChannels(); ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);

    // Tempo und Position vom Host (FL Studio)
    double bpm = 120.0, ppq = 0.0;
    bool havePpq = false, playing = false;
    if (auto* head = getPlayHead())
        if (auto pos = head->getPosition())
        {
            if (auto b = pos->getBpm())         bpm = *b;
            if (auto p = pos->getPpqPosition()) { ppq = *p; havePpq = true; }
            playing = pos->getIsPlaying();
        }
    bpm = juce::jlimit (20.0, 400.0, bpm);

    const float mix      = pMix->load();
    const int   cycleIdx = juce::jlimit (0, 4, (int) pCycle->load());
    const int   timeIdx  = (int) pTime->load();
    const int   volIdx   = (int) pVol->load();
    const float outGain  = juce::Decibels::decibelsToGain (pOut->load());

    const int size = delayBuf.getNumSamples();
    const double cycleBeats = kCycleBeats[cycleIdx];
    const double cycleLen = juce::jmin (cycleBeats * 60.0 / bpm * currentSampleRate,
                                        (size - 8) * 0.5);
    const double phaseInc = 1.0 / cycleLen;

    if (havePpq && playing)
    {
        const double p = ppq / cycleBeats;
        phase = p - std::floor (p);
    }

    for (int i = 0; i < numSamples; ++i)
    {
        float dry[2] = { 0.0f, 0.0f };
        for (int ch = 0; ch < numCh; ++ch)
        {
            dry[ch] = buffer.getSample (ch, i);
            delayBuf.setSample (ch, writeIndex, dry[ch]);
        }

        // Leseposition aus der Zeit-Kurve (nie in der Zukunft)
        const float t = gb::timeCurve (timeIdx, (float) phase);
        double readAbs = (double) writeCount + ((double) t - phase) * cycleLen;
        readAbs = juce::jlimit ((double) writeCount - (size - 4), (double) writeCount, readAbs);

        const auto i0  = (juce::int64) std::floor (readAbs);
        const float fr = (float) (readAbs - (double) i0);
        const int idx0 = (int) (((i0 % size) + size) % size);
        const int idx1 = (i0 + 1 <= writeCount) ? (idx0 + 1) % size : idx0;

        gainSmooth.setTargetValue (gb::volumeCurve (volIdx, (float) phase));
        const float g = gainSmooth.getNextValue();

        for (int ch = 0; ch < numCh; ++ch)
        {
            const float* d = delayBuf.getReadPointer (ch);
            const float wet = d[idx0] + fr * (d[idx1] - d[idx0]);
            buffer.setSample (ch, i, (dry[ch] * (1.0f - mix) + wet * g * mix) * outGain);
        }

        writeIndex = (writeIndex + 1) % size;
        ++writeCount;
        phase += phaseInc;
        if (phase >= 1.0) phase -= 1.0;
    }
}

juce::AudioProcessorEditor* GrossBeatCloneProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

void GrossBeatCloneProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void GrossBeatCloneProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrossBeatCloneProcessor();
}
