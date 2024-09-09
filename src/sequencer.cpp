/**************************************************************************/
/*  sequencer.cpp                                                         */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GDSynthesizer                              */
/**************************************************************************/
/* Copyright (c) 2023-2024 Soyo Kuyo.                                     */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/


#include "sequencer.hpp"
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
#include <godot_cpp/variant/utility_functions.hpp> // for "UtilityFunctions::print()".
#endif // DEBUG_ENABLED && WINDOWS_ENABLED

#include "instrument.hpp"

const char* scale[] = {" C", "C#", " D", "D#", " E", " F", "F#", " G", "G#", " A", "A#", " B"};

PinkNoise::PinkNoise() {
    for (int32_t i = 0; i < tapNum ; i++) z[i] = 0;
    k[tapNum - 1] = 0.5f;
    for (int32_t i = tapNum - 1; i > 0; i--)
        k[i- 1] = k[i] * 0.25f;
}

PinkNoise::~PinkNoise() {
}

float PinkNoise::makeNoise(float in) {
    float q = in;
    for (int32_t i = 0; i < tapNum; i++) {
        z[i] = (q*k[i] + z[i] * (1.0f - k[i]));
        q = (q + z[i]) * 0.5f;
    }
    t = 0.75f * q + 0.25f * t;
    return t;    
}

Sequencer::Sequencer() {
    rand = memnew(godot::RandomNumberGenerator);
}

Sequencer::~Sequencer(){
    for (int32_t i = 0; i < std::size(toneInstances); i++) {
        delete [] toneInstances[i].delayBuffer;
        toneInstances[i].delayBuffer = nullptr;
    }

    if (rand) {
        memdelete(rand);
    }
}

float Sequencer::noteFrequency(int8_t note) {
    return (powf(2.0f, ((float)note - 69.0f) / 12.0f)) * 440.0f;
}

float Sequencer::centFrequency(float freq, float cent) {
    static const float h = (powf(2.0f,  120.0f/1200.0f)-1.0f)/120.0f;
    static const float l = (powf(2.0f, -120.0f/1200.0f)-1.0f)/120.0f;
    static const float t =  (float(pow2_x_1200LUT_size/2));
    static const float b = -(float(pow2_x_1200LUT_size/2));

    float result;
    if      (cent <=        b) result = freq * pow2_x_1200LUT[0];
    else if (cent <   -120.0f) result = freq * pow2_x_1200LUT[pow2_x_1200LUT_size/2+(int32_t)cent];
    else if (cent <      0.0f) result = freq * (1.0f - cent*l);
    else if (cent <=   120.0f) result = freq * (1.0f + cent*h);
    else if (cent <         t) result = freq * pow2_x_1200LUT[pow2_x_1200LUT_size/2+(int32_t)cent];
    else                       result = freq * pow2_x_1200LUT[pow2_x_1200LUT_size-1];
    if (result > samplingRate*0.47f) result = samplingRate*0.47f; // 0.47 is upper limit.
    return result;
}

godot::Array Sequencer::getInstruments(void) {
    godot::Array array;
    for (int32_t i = 0; i < 256; i++) {
        godot::Dictionary dic;

        dic["totalGain"]          = instruments[i].totalGain;

        dic["atackSlopeTime"]     = instruments[i].atackSlopeTime;
        dic["decayHalfLifeTime"]  = instruments[i].decayHalfLifeTime;
        dic["sustainRate"]        = instruments[i].sustainRate;
        dic["releaseSlopeTime"]   = instruments[i].releaseSlopeTime;

        dic["baseVsOthersRatio"]  = instruments[i].baseVsOthersRatio;
        dic["side1VsSide2Ratio"]  = instruments[i].side1VsSide2Ratio;
        dic["baseOffsetCent1"]    = instruments[i].baseOffsetCent1;
        dic["baseWave1"]          = static_cast<int32_t>(instruments[i].baseWave1);
        dic["baseOffsetCent2"]    = instruments[i].baseOffsetCent2;
        dic["baseWave2"]          = static_cast<int32_t>(instruments[i].baseWave2);
        dic["baseOffsetCent3"]    = instruments[i].baseOffsetCent3;
        dic["baseWave3"]          = static_cast<int32_t>(instruments[i].baseWave3);

        dic["noiseRatio"]         = instruments[i].noiseRatio;
        dic["noiseColorType"]     = static_cast<int32_t>(instruments[i].noiseColorType);

        dic["delay0Time"]         = instruments[i].delay0Time;
        dic["delay1Time"]         = instruments[i].delay1Time;
        dic["delay2Time"]         = instruments[i].delay2Time;
        dic["delay0Ratio"]        = instruments[i].delay0Ratio;
        dic["delay1Ratio"]        = instruments[i].delay1Ratio;
        dic["delay2Ratio"]        = instruments[i].delay2Ratio;

        dic["freqNoiseCentRange"] = instruments[i].freqNoiseCentRange;
        dic["freqNoiseType"]      = static_cast<int32_t>(instruments[i].freqNoiseType);

        dic["fmCentRange"]        = instruments[i].fmCentRange;
        dic["fmFreq"]             = instruments[i].fmFreq;
        dic["fmPhaseOffset"]      = instruments[i].fmPhaseOffset;
        dic["fmSync"]             = instruments[i].fmSync;
        dic["fmWave"]             = static_cast<int32_t>(instruments[i].fmWave);

        
        dic["amLevel"]            = instruments[i].amLevel;
        dic["amFreq"]             = instruments[i].amFreq;
        dic["amPhaseOffset"]      = instruments[i].amPhaseOffset;
        dic["amSync"]             = instruments[i].amSync;
        dic["amWave"]             = static_cast<int32_t>(instruments[i].amWave);

        array.push_back(dic);
    }
    return array;
}


void Sequencer::setInstruments(const godot::Array array) {
    if (array.size() != 256) {
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
        godot::UtilityFunctions::print("Error in setInstruments(): array size error, ", array.size());
#endif // DEBUG_ENABLED
    }
    int32_t WAVE_TAIL = static_cast<int32_t>(BaseWave::WAVE_TAIL)-1;
    int32_t NOISECTYPE_TAIL = static_cast<int32_t>(NoiseColorType::NOISECTYPE_TAIL)-1;
    int32_t NOISEDTYPE_TAIL = static_cast<int32_t>(NoiseDistributType::NOISEDTYPE_TAIL)-1;
    for (int32_t i = 0; i < 256; i++) {
        godot::Dictionary dic = array[i];

        instruments[i].totalGain          = (float)(godot::Math::clamp((double)(dic["totalGain"]), 0.0, 1.0));
        instruments[i].atackSlopeTime     = (float)(godot::Math::clamp((double)(dic["atackSlopeTime"]), 0.0, 5000.0));
        instruments[i].decayHalfLifeTime  = (float)(godot::Math::clamp((double)(dic["decayHalfLifeTime"]), 0.0, 5000.0));
        instruments[i].sustainRate        = (float)(godot::Math::clamp((double)(dic["sustainRate"]), 0.0, 1.0));
        instruments[i].releaseSlopeTime   = (float)(godot::Math::clamp((double)(dic["releaseSlopeTime"]), 0.0, 5000.0));

        instruments[i].baseVsOthersRatio  = (float)(godot::Math::clamp((double)(dic["baseVsOthersRatio"]), 0.0, 1.0));
        instruments[i].side1VsSide2Ratio  = (float)(godot::Math::clamp((double)(dic["side1VsSide2Ratio"]), 0.0, 1.0));
        instruments[i].baseOffsetCent1    = (float)(godot::Math::clamp((double)(dic["baseOffsetCent1"]), -8400.0, 8400.0));
        instruments[i].baseWave1          = static_cast<BaseWave>(std::clamp((int32_t)dic["baseWave1"], 0, WAVE_TAIL));
        instruments[i].baseOffsetCent2    = (float)(godot::Math::clamp((double)(dic["baseOffsetCent2"]), -8400.0, 8400.0));
        instruments[i].baseWave2          = static_cast<BaseWave>(std::clamp((int32_t)dic["baseWave2"], 0, WAVE_TAIL));
        instruments[i].baseOffsetCent3    = (float)(godot::Math::clamp((double)(dic["baseOffsetCent3"]), -8400.0, 8400.0));
        instruments[i].baseWave3          = static_cast<BaseWave>(std::clamp((int32_t)dic["baseWave3"], 0, WAVE_TAIL));

        instruments[i].noiseRatio         = (float)(godot::Math::clamp((double)(dic["noiseRatio"]), 0.0, 1.0));
        instruments[i].noiseColorType     = static_cast<NoiseColorType>(std::clamp((int32_t)dic["noiseColorType"], 0, NOISECTYPE_TAIL));

        instruments[i].delay0Time         = (float)(godot::Math::clamp((double)(dic["delay0Time"]), 0.0, 500.0));
        instruments[i].delay1Time         = (float)(godot::Math::clamp((double)(dic["delay1Time"]), 0.0, 500.0));
        instruments[i].delay2Time         = (float)(godot::Math::clamp((double)(dic["delay2Time"]), 0.0, 500.0));
        instruments[i].delay0Ratio        = (float)(godot::Math::clamp((double)(dic["delay0Ratio"]), 0.2, 0.2));
        instruments[i].delay1Ratio        = (float)(godot::Math::clamp((double)(dic["delay1Ratio"]), 0.2, 0.2));
        instruments[i].delay2Ratio        = (float)(godot::Math::clamp((double)(dic["delay2Ratio"]), 0.2, 0.2));

        instruments[i].freqNoiseCentRange = (float)(godot::Math::clamp((double)(dic["freqNoiseCentRange"]), -8400.0, 8400.0));
        instruments[i].freqNoiseType      = static_cast<NoiseDistributType>(std::clamp((int32_t)dic["freqNoiseType"], 0, NOISEDTYPE_TAIL));

        instruments[i].fmCentRange        = (float)(godot::Math::clamp((double)(dic["fmCentRange"]), -8400.0, 8400.0));
        instruments[i].fmFreq             = (float)(godot::Math::clamp((double)(dic["fmFreq"]), 0.0, 7040.0));
        instruments[i].fmPhaseOffset      = (float)(godot::Math::clamp((double)(dic["fmPhaseOffset"]), 0.0, 2.0));
        if (instruments[i].fmPhaseOffset == 2.0f) instruments[i].fmPhaseOffset = 0.0f;
        
        instruments[i].fmSync             = (int32_t)(std::clamp((int32_t)dic["fmSync"], 0, 1));
        instruments[i].fmWave             = static_cast<BaseWave>(std::clamp((int32_t)dic["fmWave"], 0, WAVE_TAIL));

        instruments[i].amLevel            = (float)(godot::Math::clamp((double)(dic["amLevel"]), 0.0, 1.0));
        instruments[i].amFreq             = (float)(godot::Math::clamp((double)(dic["amFreq"]), 0.0, 7040.0));
        instruments[i].amPhaseOffset      = (float)(godot::Math::clamp((double)(dic["amPhaseOffset"]), 0.0, 2.0));
        if (instruments[i].amPhaseOffset == 2.0f) instruments[i].amPhaseOffset = 0.0f;

        instruments[i].amSync             = (int32_t)(std::clamp((int32_t)dic["amSync"], 0, 1));
        instruments[i].amWave             = static_cast<BaseWave>(std::clamp((int32_t)dic["amWave"], 0, WAVE_TAIL));
    }
}


godot::Array Sequencer::getPercussions(void) {
    godot::Array array;
    for (int32_t i = 0; i < 128; i++) {
        godot::Dictionary dic;

        dic["program"]  = percussions[i].program;
        dic["key"]      = percussions[i].key;
        array.push_back(dic);
    }
    return array;
}


void Sequencer::setPercussions(const godot::Array array) {
    if (array.size() != 128) {
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
        godot::UtilityFunctions::print("Error in setPercussions(): array size error, ", array.size());
#endif // DEBUG_ENABLED
    }
    for (int32_t i = 0; i < 128; i++) {
        godot::Dictionary dic = array[i];

        percussions[i].program = (int32_t)(std::clamp((int32_t)dic["program"], 0, 255));
        percussions[i].key     = (int32_t)(std::clamp((int32_t)dic["key"], 0, 127));
    }
}


void Sequencer::setControlParams(const godot::Dictionary dic){
    asumedConcurrentTone = (float)(godot::Math::clamp((double)(dic["divisionNum"]), 0.1, 64.0));
    logLevel = (int32_t)(std::clamp((int32_t)dic["logLevel"], 0, 10));
    maxValue = 0.0;
}

godot::Dictionary Sequencer::getControlParams(void) {
    godot::Dictionary dic;
    dic["divisionNum"] = asumedConcurrentTone;
    dic["logLevel"] = logLevel;
    return dic;
}

bool Sequencer::initParam(double rate, double time, int32_t samples) {
    samplingRate = (float)rate;
    bufferingTime = (float)time;
    bufferSamples = samples;
    currentTime = 0;
    frameCount = 0;
    noiseBufSize = (int32_t)(rate/(double)bufferSamples);
    noiseBuffer = bufferSamples*noiseBufSize;
    freeTones.clear();
    activeTones.clear();

    {
        numAtackSlopeLUT = (int32_t)((1.0f/atackSlopeHz/2.0f)*samplingRate);
        atackSlopeLUT = std::make_unique<float[]>(numAtackSlopeLUT);
        atackSlopeTime = 1.0f/atackSlopeHz/2.0f*1000.0f;
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
        godot::UtilityFunctions::print("numAtackSlopeLUT ", numAtackSlopeLUT);
        godot::UtilityFunctions::print("atackSlopeTime ", atackSlopeTime);
#endif // DEBUG_ENABLED

        for (int32_t i = 0; i < numAtackSlopeLUT; i++){
            atackSlopeLUT[i] = (1.0f-cosf(PI*(float)i/(float)numAtackSlopeLUT))/2.0f;
        }
    }
    {
        numReleaseSlopeLUT = (int32_t)((1.0f/releaseSlopeHz/2.0f)*samplingRate);
        releaseSlopeLUT = std::make_unique<float[]>(numReleaseSlopeLUT);
        releaseSlopeTime = 1.0f/releaseSlopeHz/2.0f*1000.0f;
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
        godot::UtilityFunctions::print("numReleaseSlopeLUT ", numReleaseSlopeLUT);
        godot::UtilityFunctions::print("releaseSlopeTime ", releaseSlopeTime);
#endif // DEBUG_ENABLED
        for (int32_t i = 0; i < numReleaseSlopeLUT; i++){
            releaseSlopeLUT[i] = (1.0f+cosf(PI*(float)i/(float)numReleaseSlopeLUT))/2.0f;
        }
    }
    {
        numDecaySlopeLUT = (int32_t)((1.0f/decaySlopeHz/2.0f)*samplingRate);
        decaySlopeLUT = std::make_unique<float[]>(numDecaySlopeLUT);
        decaySlopeTime = 1.0f/decaySlopeHz/2.0f*1000.0f;
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
        godot::UtilityFunctions::print("numDecaySlopeLUT ", numDecaySlopeLUT);
        godot::UtilityFunctions::print("decaySlopeTime ", decaySlopeTime);
#endif // DEBUG_ENABLED
        for (int32_t i = 0; i < numDecaySlopeLUT; i++){
            decaySlopeLUT[i] = 0.5f+tanhf(log10f(decayHalfLifeTime/(decaySlopeTime*(float)i/(float)numDecaySlopeLUT)))/2.0f;
        }
    }
    {
        float offset = decaySlopeLUT[numDecaySlopeLUT-1];
        float range = 1.0f - offset;
        for (int32_t i = 0; i < numDecaySlopeLUT; i++){
            decaySlopeLUT[i] = (decaySlopeLUT[i]-offset)/range;
        }
    }

    // make base wave look-up tables.
    int32_t s = waveLUTSize;
    { // sin wave
        int32_t j = static_cast<int32_t>(BaseWave::WAVE_SIN);
        for (int32_t i = 0; i < s; i++){
            waveLUT[j][i] = sinf(2.0f*PI*(float)i/(float)s);
        }
    }
    { // square wave
        int32_t j = static_cast<int32_t>(BaseWave::WAVE_SQUARE);
        for (int32_t i = 0; i < s; i++){
            waveLUT[j][i] = (i < s/2) ? 1.0f : -1.0f;
        }
    }
    { // triangle wave
        int32_t j = static_cast<int32_t>(BaseWave::WAVE_TRIANGLE);
        for (int32_t i = 0; i < s; i++){
            waveLUT[j][(i+3*s/4)%s] = (i < s/2)?((float)i*4.0f)/((float)s)-1.0f:3.0f-((float)i*4.0f)/((float)s);
        }
    }
    { // sawtooth wave
        int32_t j = static_cast<int32_t>(BaseWave::WAVE_SAWTOOTH);
        for (int32_t i = 0; i < s; i++){
            waveLUT[j][(i+3*s/4)%s] = ((float)i*2.0f)/((float)s)-1.0f;
        }
    }
    { // sin on sawtooth2 wave
        int32_t l = static_cast<int32_t>(BaseWave::WAVE_SAWTOOTH);
        int32_t j = static_cast<int32_t>(BaseWave::WAVE_SIN);
        int32_t k = static_cast<int32_t>(BaseWave::WAVE_SINSAWx2);
        for (int32_t i = 0; i < s; i++){
            waveLUT[k][i] = ((waveLUT[j][i]+1.0f)+(waveLUT[l][(i*2)%s]+1.0f))/2.0f -1.0f;
        }
    }

    // make delay ring buffers
    delayBufferSize = (int32_t)((float)rate*(delayBufferDuration/1000.0f));

    for (int32_t i = 0; i < std::size(toneInstances); i++) {
        toneInstances[i].delayBuffer = new float[delayBufferSize];
        freeTones.push_back(toneInstances[i]);
    }

    { // make look-up table for white-noise and noise distributions.
        whiteNoiseLUT             = std::make_unique<float[]>(noiseBuffer);
        triangularDistributionLUT = std::make_unique<float[]>(noiseBuffer);
        cos4thPowDistributionLUT  = std::make_unique<float[]>(noiseBuffer);
        for (int32_t i = 0; i < noiseBuffer ; i++){
            whiteNoiseLUT[i] = (float)rand->randf_range(-1.0, 1.0); // randf_range() returns double.
            double r = godot::Math::absf((double)whiteNoiseLUT[i]);
            double c = godot::Math::absf(1-pow(cos(Math_PI*(r*0.5-0.5)), 4.0));
            triangularDistributionLUT[i] = (float)rand->randf_range(-r, r);
            cos4thPowDistributionLUT[i]  = (float)rand->randf_range(-c, c);
        }
    }

    { // make look-up table for pink-noise
        pinkNoiseLUT              = std::make_unique<float[]>(noiseBuffer);
        PinkNoise pinkNoise = PinkNoise();
        for (int32_t i = 0; i < noiseBuffer ; i++){
            pinkNoiseLUT[i] = pinkNoise.makeNoise(whiteNoiseLUT[i]);
        }
        float head = pinkNoiseLUT[0];
        float tail = pinkNoise.makeNoise(whiteNoiseLUT[0]);
        float diff = (tail- head)/(float)noiseBuffer;
        float max = -1.0f;
        float min =  1.0f;

        for (int32_t i = 0; i < noiseBuffer ; i++){
            pinkNoiseLUT[i] += diff*(float)i;
            if (pinkNoiseLUT[i] > max) max = pinkNoiseLUT[i];
            if (pinkNoiseLUT[i] < min) min = pinkNoiseLUT[i];
        }
        for (int32_t i = 0; i < noiseBuffer ; i++){
            pinkNoiseLUT[i] = (pinkNoiseLUT[i] - min)/(max - min)*2.0f - 1.0f;
        }
    }

    { // make look-up table for centFrequency function.
        pow2_x_1200LUT  = std::make_unique<float[]>(pow2_x_1200LUT_size);
        for (int32_t i = 0; i < pow2_x_1200LUT_size ; i++){
            pow2_x_1200LUT[i] = powf(2.0f, (float)(i-pow2_x_1200LUT_size/2)/1200.0f);
        }
    }
    { //make look-up table to convert velocity value to output power.
        velocity2powerLUT = std::make_unique<float[]>(128);
        {
            for (int32_t i = 0; i < 128; i++){
                velocity2powerLUT[i] = powf((float)(i+1)/128.0f, 2.2f);
            }
        }
    }

    instruments = defaultInstruments;
    percussions = defaultPercussions;

    isSet = true;
    return true;
}


bool Sequencer::smfUnload(void) {
    unitOfTime = 60000.0;
    midi.setUnitOfTime(unitOfTime); // milliseconds
    freeTones.clear();
    activeTones.clear();
    for (int32_t i = 0; i < std::size(toneInstances); i++) {
        freeTones.push_back(toneInstances[i]);
    }

    midi.unload();
    
    return true;
}


bool Sequencer::smfLoad(const char *name, double givenUnitOfTime) {
    currentTime = 0;
    unitOfTime = (float)givenUnitOfTime;
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
    godot::UtilityFunctions::print("unitOfTime ", unitOfTime);
#endif // DEBUG_ENABLED

    midi.setUnitOfTime(unitOfTime); // milliseconds
    if (midi.load(name) == false) {
        return false;
    }
    
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
        godot::UtilityFunctions::print("smf file size: ", midi.filesize);
#endif // DEBUG_ENABLED
    return true;
}


bool Sequencer::smfLoad(const godot::String &name, double givenUnitOfTime) {
    currentTime = 0;
    unitOfTime = (float)givenUnitOfTime;
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
    godot::UtilityFunctions::print("unitOfTime ", unitOfTime);
#endif // DEBUG_ENABLED

    midi.setUnitOfTime(unitOfTime); // milliseconds
    if (midi.load(name) == false) {
        return false;
    }
    
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
        godot::UtilityFunctions::print("smf file size: ", midi.filesize);
#endif // DEBUG_ENABLED
    return true;
}


void Sequencer::incertNoteOn(const godot::Dictionary dic){
    Note oneNote;
    oneNote.state     = NState::NS_ON_FOREVER;
    oneNote.trackNum  = 0;
    oneNote.channel   = (int32_t)(std::clamp((int32_t)dic["channel"], 0, 31));
    oneNote.key       = (int32_t)(std::clamp((int32_t)dic["key"], 0, 127));
    oneNote.velocity  = (int32_t)(std::clamp((int32_t)dic["velocity"], 0, 127));
    oneNote.program   = (int32_t)(std::clamp((int32_t)dic["program"], 0, 255));
    oneNote.startTick = 0;
    oneNote.startTime = currentTime;
    oneNote.tempo     = (int32_t)(std::clamp((int32_t)dic["tempo"], 1, 999));

    checkNewNote(oneNote);
};


void Sequencer::incertNoteOff(const godot::Dictionary dic){
    Note oneNote;
    oneNote.state     = NState::NS_OFF;
    oneNote.trackNum  = 0;
    oneNote.channel   = (int32_t)(std::clamp((int32_t)dic["channel"], 0, 31));
    oneNote.key       = (int32_t)(std::clamp((int32_t)dic["key"], 0, 127));
    oneNote.velocity  = (int32_t)(std::clamp((int32_t)dic["velocity"], 0, 127));
    oneNote.program   = (int32_t)(std::clamp((int32_t)dic["program"], 0, 255));
    oneNote.startTick = 0;
    oneNote.startTime = currentTime;
    oneNote.tempo     = (int32_t)(std::clamp((int32_t)dic["tempo"], 1, 999));

    checkNewNote(oneNote);
};


godot::Ref<godot::Image> Sequencer::getMiniWavePicture(const godot::Dictionary dic){
    int32_t size_x = dic["size_x"];
    int32_t size_y = dic["size_y"];
    int32_t type = dic["type"];
    int32_t phase = dic["phase"];
    if (size_x > 400) size_x = 400;
    if (size_x < 16) size_x = 16;
    if (size_y > 400) size_y = 400;
    if (size_y < 16) size_y = 16;
    float invert = 1.0f;
    if (type == static_cast<int32_t>(BaseWave::WAVE_TAIL)){
        type = static_cast<int32_t>(BaseWave::WAVE_SAWTOOTH);
        invert = -1.0f;
    }
    if (type > static_cast<int32_t>(BaseWave::WAVE_TAIL)) type = 0;
    if (type < 0) type = 0;
    if (phase > 360) phase = 360;
    if (phase == 360) phase = 0;
    if (phase < 0) phase = 0;
    phase = (int32_t)((double)phase/360.0*(double)size_x);

    godot::Ref<godot::Image> miniWaveImage;

    miniWaveImage = godot::Image::create(size_x, size_y, false, godot::Image::FORMAT_RGBA8);
    miniWaveImage->fill(godot::Color(0.2, 0.2, 0.2, 1.0));
    int32_t s = waveLUTSize;
    
    int32_t pre_y;
    for (int32_t i = 0; i < size_x; i++){
        int32_t x = (int32_t)(double((i+phase)%size_x)/double(size_x)*double(s));
        float fy = (1.0f-waveLUT[type][x]*invert)/2.0f;
        if (fy >= 1.0f) fy = 0.99f;
        if (fy <= 0.0f) fy = 0.01f;
        int32_t y = (int32_t)(fy*(float)size_y);
        if (i == 0) pre_y = y;
        if (y < pre_y){
            for (int32_t j = y; j <= pre_y; j++){
                miniWaveImage->set_pixel(i, j, godot::Color(1.0, 0.5, 0.0, 1.0));
            }
        }
        else{
            for (int32_t j = pre_y; j <= y; j++){
                miniWaveImage->set_pixel(i, j, godot::Color(1.0, 0.5, 0.0 , 1.0));
            }
        }
        
        pre_y = y;
    }
    return miniWaveImage;
}


bool Sequencer::checkNewNote(Note oneNote){
    float durationTime = 0;
    if (oneNote.state == NState::NS_ON_FOREVER) durationTime = FLOAT_LONGTIME;
    auto ringingTone = std::find_if(activeTones.begin(), activeTones.end(), [&](const Tone &foundTone){ 
        return (   oneNote.key == foundTone.note.key 
                && oneNote.channel == foundTone.note.channel
                && foundTone.note.state != NState::NS_OFF);
    });
    if (oneNote.state == NState::NS_OFF) {
        if (ringingTone != activeTones.end()) {
            ringingTone->mainteinDuration = (float)(oneNote.startTime - ringingTone->note.startTime);
            ringingTone->note.state = NState::NS_OFF;

            {
                godot::Dictionary dic;

                dic["msg"]                = (int32_t)0;
                dic["onOff"]              = (int32_t)0;
                dic["trackNum"]           = ringingTone->note.trackNum;
                dic["channel"]            = ringingTone->note.channel;
                dic["velocity"]           = ringingTone->note.velocity;
                dic["program"]            = ringingTone->note.program;
                dic["key"]                = ringingTone->note.key;
                dic["instrumentNum"]      = ringingTone->program;
                dic["key2"]               = ringingTone->key;
                emitSignal(dic);
            }

#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
            if (logLevel > 1){
                godot::UtilityFunctions::print(
                        " state ", static_cast<int32_t>(ringingTone->note.state),
                        "  ch ", ringingTone->note.channel,
                        "  prog ", ringingTone->note.program,
                        "  key ", ringingTone->note.key,
                        "  scale ", scale[ringingTone->note.key%12],(uint16_t)(ringingTone->note.key / 12) - 1,
                        "  end(ms) ", ringingTone->note.startTime
                );
            }
#endif // DEBUG_ENABLED
        }
        else {
            return false;
        }
    }
    else if (freeTones.size() != 0){
        auto tone = freeTones.begin();

        tone->note = oneNote;

        tone->phase1 = tone->phase2 = tone->phase3 = 0.0f;
        tone->key = oneNote.key;
        tone->frequency = noteFrequency(oneNote.key);
        tone->passed = 0;
        tone->waitDuration = (float)(oneNote.startTime - currentTime);

        tone->mainteinDuration = durationTime;
        tone->restartWaitDuration = FLOAT_LONGTIME;
        tone->restartTempo_f = tone->tempo_f = (float)oneNote.tempo*1000.0f; // msec

        // select instrument
        if (tone->note.channel > 127 || tone->note.channel < 0) {
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
            godot::UtilityFunctions::print("invalid tone->note.channel ", tone->note.channel);
#endif // DEBUG_ENABLED
            tone->program = 0;
            tone->instrument = instruments[0];
            tone->note.velocity = 0;
        }
        else if (tone->note.channel == 9  || tone->note.channel == 25) { // 9 is ch10 that is reserved for Percussions.
            tone->program = percussions[tone->note.key].program;
            tone->instrument = instruments[percussions[tone->note.key].program];
            tone->key = percussions[tone->note.key].key;
            tone->frequency = noteFrequency(percussions[tone->note.key].key);
        }
        else {
            if (oneNote.program >= 0x70  && oneNote.program < 0x80){ // Percussives and Sound effects.
                tone->program = percussions[oneNote.program].program;
                tone->instrument = instruments[percussions[oneNote.program].program];
                tone->key = percussions[oneNote.program].key;
                tone->frequency = noteFrequency(percussions[oneNote.program].key);
            }
            else{
                tone->program = oneNote.program;
                tone->instrument = instruments[oneNote.program];
            }
        }
        {
            tone->realKey1 = tone->key + (int32_t)(tone->instrument.baseOffsetCent1/100.0f);
            tone->realKey2 = tone->key + (int32_t)(tone->instrument.baseOffsetCent2/100.0f);
            tone->realKey3 = tone->key + (int32_t)(tone->instrument.baseOffsetCent3/100.0f);
        }
        {
            godot::Dictionary dic;
            dic["msg"]                = (int32_t)0;
            dic["onOff"]              = (int32_t)1;
            dic["trackNum"]           = tone->note.trackNum;
            dic["channel"]            = tone->note.channel;
            dic["velocity"]           = tone->note.velocity;
            dic["program"]            = tone->note.program;
            dic["key"]                = tone->note.key;
            dic["instrumentNum"]      = tone->program;
            dic["key2"]               = tone->key;
            emitSignal(dic);
        }
        {
            tone->restartVelocity_f = tone->velocity_f = velocity2powerLUT[tone->note.velocity];
            tone->atackedStrengthfloor = 0.0f;
        }
        tone->base1ratio = tone->instrument.baseVsOthersRatio;
        tone->base2ratio = (1.0f-tone->instrument.baseVsOthersRatio)*tone->instrument.side1VsSide2Ratio;
        tone->base3ratio = (1.0f-tone->instrument.baseVsOthersRatio)*(1.0f-tone->instrument.side1VsSide2Ratio);
                
        // fm moduration related.
        tone->fmPhase= PI * tone->instrument.fmPhaseOffset;
        tone->fmIncrement = 0.0f;
        if (tone->instrument.fmFreq != 0.0f) {
            if (tone->instrument.fmSync == 0){
                tone->fmIncrement = (2.0f * PI * tone->instrument.fmFreq ) / samplingRate;
            }
            else{
                tone->fmIncrement = (2.0f * PI * tone->instrument.fmFreq * tone->tempo_f / unitOfTime) / samplingRate;
            }
        }

        // am moduration related.
        tone->amPhase= PI * tone->instrument.amPhaseOffset;
        tone->amIncrement = 0.0f;
        if (tone->instrument.amFreq != 0.0f) {
            if (tone->instrument.amSync == 0){
                tone->amIncrement = (2.0f * PI * tone->instrument.amFreq ) / samplingRate;
            }
            else{
                tone->amIncrement = (2.0f * PI * tone->instrument.amFreq * tone->tempo_f / unitOfTime) / samplingRate;
            }
        }

        // variable freqNoise related.
        {
            tone->freqNoiseCentharfRange = tone->instrument.freqNoiseCentRange*0.5f;
            float c1 = centFrequency(tone->frequency, tone->instrument.baseOffsetCent1);
            float l1 = centFrequency(c1, -(tone->freqNoiseCentharfRange));
            tone->baseIncrement1  = (2.0f * PI * l1) / samplingRate;

            float c2 = centFrequency(tone->frequency, tone->instrument.baseOffsetCent2);
            float l2 = centFrequency(c2, -(tone->freqNoiseCentharfRange));
            tone->baseIncrement2  = (2.0f * PI * l2) / samplingRate;

            float c3 = centFrequency(tone->frequency, tone->instrument.baseOffsetCent3);
            float l3 = centFrequency(c3, -(tone->freqNoiseCentharfRange));
            tone->baseIncrement3  = (2.0f * PI * l3) / samplingRate;
        }

        // init delay ring buffer
        tone->delayBufferIndex = 0;
        tone->delay0Index = tone->delay1Index = tone->delay2Index = 0;
        tone->delay0Ratio = tone->delay1Ratio = tone->delay2Ratio = 0.0f;
        tone->maxDelayTime = 0.0f;
        if (   tone->instrument.delay0Time > 0.0f
            && tone->instrument.delay0Time < delayBufferDuration
            && tone->instrument.delay0Ratio < 1.00f
            && tone->instrument.delay0Ratio > 0.0f)
        {
            tone->maxDelayTime = tone->instrument.delay0Time;
            tone->delay0Index = (uint32_t)((float)delayBufferSize/delayBufferDuration * tone->instrument.delay0Time);
            tone->delay0Ratio = tone->instrument.delay0Ratio;
        }
        if (   tone->instrument.delay1Time > 0.0f
            && tone->instrument.delay1Time < delayBufferDuration
            && tone->instrument.delay1Ratio < 1.00f
            && tone->instrument.delay1Ratio > 0.0f)
        {
            if (tone->instrument.delay1Time > tone->maxDelayTime) tone->maxDelayTime = tone->instrument.delay1Time;
            tone->delay1Index = (uint32_t)((float)delayBufferSize/delayBufferDuration * tone->instrument.delay1Time);
            tone->delay1Ratio = tone->instrument.delay1Ratio;
        }
        if (   tone->instrument.delay2Time > 0.0f
            && tone->instrument.delay2Time < delayBufferDuration
            && tone->instrument.delay2Ratio < 1.00f
            && tone->instrument.delay2Ratio > 0.0f)
        {
            if (tone->instrument.delay2Time > tone->maxDelayTime) tone->maxDelayTime = tone->instrument.delay2Time;
            tone->delay2Index = (uint32_t)((float)delayBufferSize/delayBufferDuration * tone->instrument.delay2Time);
            tone->delay2Ratio = tone->instrument.delay2Ratio;
        }
        tone->maxDelayTime *= 3.0f;

        tone->mainRatio = 1.0f - (tone->delay0Ratio+tone->delay1Ratio+tone->delay2Ratio);
        for (int32_t i = 0; i < delayBufferSize; i++) {
            tone->delayBuffer[i] = 0.0f;
        }

        tone->atackSlopeRatio = atackSlopeTime/tone->instrument.atackSlopeTime;
        tone->decaySlopeRatio = decayHalfLifeTime/tone->instrument.decayHalfLifeTime;
        tone->releaseSlopeRatio = releaseSlopeTime/tone->instrument.releaseSlopeTime;

        activeTones.insert(activeTones.end(), *freeTones.begin());
        freeTones.erase(freeTones.begin());

#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
        if (logLevel > 1){
            godot::UtilityFunctions::print(
                " state ", static_cast<int32_t>(tone->note.state),
                "  ch ", tone->note.channel,
                "  prog ", tone->note.program,
                "  velocity ", tone->note.velocity,
//                "  tempo ", tone->tempo_f,
                "  key ", tone->note.key,
                "  scale ", scale[tone->note.key%12],(uint16_t)(tone->note.key / 12) - 1,
                "  start(ms) ", tone->note.startTime,
                " ", activeTones.size(), ":", freeTones.size()
            );
        }
#endif // DEBUG_ENABLED
    }
    else {
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
        godot::UtilityFunctions::print("Error: no free tone.");
#endif // DEBUG_ENABLED
        return false;
    }
    return true;
}

bool Sequencer::feed(double *frame){
    for (int i=0; i < bufferSamples; i++) frame[i] = 0.0;

    int32_t frameTime = (int32_t)(bufferingTime*1000.0f);
    Note oneNote;
    while(isSet) {
        oneNote = midi.parse(currentTime + frameTime);
        if (oneNote.state == NState::NS_END || oneNote.state == NState::NS_EMPTY) {
            break;
        }
        if (checkNewNote(oneNote) == false) break ;
    }
    currentTime += frameTime;
    int32_t noiseBufIndex = frameCount*bufferSamples;
    float period = (float)std::size(waveLUT[0])/(PI*2.0f);
    float delta = 1.0f/samplingRate*1000.0f;
    float div = 1.0f/asumedConcurrentTone; // to avoid saturation.

    for (auto tone = activeTones.begin(); tone != activeTones.end();) {
        float current = (float)tone->passed;
        bool isEnd = false;
        int32_t sinWave   = static_cast<int32_t>(BaseWave::WAVE_SIN);
        int32_t baseWave1 = static_cast<int32_t>(tone->instrument.baseWave1);
        int32_t baseWave2 = static_cast<int32_t>(tone->instrument.baseWave2);
        int32_t baseWave3 = static_cast<int32_t>(tone->instrument.baseWave3);
        int32_t fmWave = static_cast<int32_t>(tone->instrument.fmWave);
        float fmWaveInvert = 1.0f;
        if (tone->instrument.fmWave == BaseWave::WAVE_SINSAWx2){
            fmWave = static_cast<int32_t>(BaseWave::WAVE_SAWTOOTH);
            fmWaveInvert = -1.0f;
        }
        int32_t amWave = static_cast<int32_t>(tone->instrument.amWave);
        float amWaveInvert = 1.0f;
        if (tone->instrument.amWave == BaseWave::WAVE_SINSAWx2){
            amWave = static_cast<int32_t>(BaseWave::WAVE_SAWTOOTH);
            amWaveInvert = -1.0f;
        }
        double maxFrameValue = 0.0;
        for (int32_t i = 0; i < bufferSamples; i++){
            bool isTone = false;
            if (current > tone->restartWaitDuration) {
                tone->note.state = NState::NS_ON_FOREVER;
                tone->mainteinDuration = tone->restartWaitDuration = FLOAT_LONGTIME;
                tone->atackedStrengthfloor = tone->strength;
                tone->waitDuration = current;
                tone->tempo_f = tone->restartTempo_f;
                tone->velocity_f = tone->restartVelocity_f;
            }
            if (current > tone->waitDuration+tone->mainteinDuration+tone->instrument.releaseSlopeTime+tone->maxDelayTime){
                isEnd = true;
                break;
            }
            else if (current > tone->waitDuration+tone->mainteinDuration){ // release
                int32_t d = (int32_t)(((current-(tone->waitDuration+tone->mainteinDuration))*tone->releaseSlopeRatio)/delta);
                if (d >= numReleaseSlopeLUT) d = numReleaseSlopeLUT - 1;
                tone->atackedStrengthfloor = tone->strength = tone->decayedStrength*releaseSlopeLUT[d];
                isTone = true;
            }
            else if (current > tone->waitDuration+tone->instrument.atackSlopeTime){ // decay and sustain
                int32_t d = (int32_t)(((current-(tone->waitDuration+tone->instrument.atackSlopeTime))*tone->decaySlopeRatio)/delta);
                if (d >= numDecaySlopeLUT) d = numDecaySlopeLUT - 1;
                tone->strength = tone->atackedStrength*((decaySlopeLUT[d]*(1.0f-tone->instrument.sustainRate)+tone->instrument.sustainRate));
                tone->atackedStrengthfloor = tone->decayedStrength = tone->strength;
                isTone = true;
            }
            else if (current > tone->waitDuration){ // atack
                int32_t d = (int32_t)((current-tone->waitDuration)*tone->atackSlopeRatio/delta);
                if (d >= numAtackSlopeLUT) d = numAtackSlopeLUT - 1;
                tone->strength = atackSlopeLUT[d]*(1.0f-tone->atackedStrengthfloor)+tone->atackedStrengthfloor;
                tone->decayedStrength = tone->atackedStrength = tone->strength;
                isTone = true;
            }
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
            if (tone->atackedStrength > 1.0f) godot::UtilityFunctions::print("atackedStrength saturated! ", tone->atackedStrength);
            if (tone->decayedStrength > 1.0f) godot::UtilityFunctions::print("decayedStrength saturated! ", tone->decayedStrength);
            if (tone->strength > 1.0f)        godot::UtilityFunctions::print("strength saturated! ", tone->strength);
            if (tone->atackedStrength < 0.0f) godot::UtilityFunctions::print("atackedStrength underflowed! ", tone->atackedStrength);
            if (tone->decayedStrength < 0.0f) godot::UtilityFunctions::print("decayedStrength underflowed! ", tone->decayedStrength);
            if (tone->strength < 0.0f)        godot::UtilityFunctions::print("strength underflowed! ", tone->strength);
#endif // DEBUG_ENABLED
            if (isTone){
                float inc1, inc2, inc3;
                float cent;
                if (tone->instrument.freqNoiseType == NoiseDistributType::NOISEDTYPE_TRIANGULAR) {
                    cent = tone->freqNoiseCentharfRange*triangularDistributionLUT[noiseBufIndex+i];
                }
                else if (tone->instrument.freqNoiseType == NoiseDistributType::NOISEDTYPE_COS4ThPOW) {
                    cent = tone->freqNoiseCentharfRange*cos4thPowDistributionLUT[noiseBufIndex+i];
                }
                else {
                    cent = tone->freqNoiseCentharfRange*whiteNoiseLUT[noiseBufIndex+i];
                }
                if (current > tone->waitDuration){
                    tone->fmPhase += tone->fmIncrement;
                    if (tone->fmPhase > PI*2.0f) tone->fmPhase -= PI*2.0f;
                    cent += tone->instrument.fmCentRange*(waveLUT[fmWave][(int32_t)(tone->fmPhase*period)]*fmWaveInvert+1.0f)*0.5f;
                }
                
                inc1 = centFrequency(tone->baseIncrement1, cent);
                inc2 = centFrequency(tone->baseIncrement2, cent);
                inc3 = centFrequency(tone->baseIncrement3, cent);
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
                if (inc1 < 0.0f) godot::UtilityFunctions::print("inc1 is going backwards! ", inc1);
                if (inc2 < 0.0f) godot::UtilityFunctions::print("inc2 is going backwards! ", inc2);
                if (inc3 < 0.0f) godot::UtilityFunctions::print("inc3 is going backwards! ", inc3);
#endif // DEBUG_ENABLED
                
                tone->phase1 += inc1;
                if (tone->phase1 > PI*2.0f) tone->phase1 -= PI*2.0f;
                tone->phase2 += inc2;
                if (tone->phase2 > PI*2.0f) tone->phase2 -= PI*2.0f;
                tone->phase3 += inc3;
                if (tone->phase3 > PI*2.0f) tone->phase3 -= PI*2.0f;
                
                float level = 1.0f;
                if (current > tone->waitDuration){
                    tone->amPhase += tone->amIncrement;
                    if (tone->amPhase > PI*2.0f) tone->amPhase -= PI*2.0f;
                    level = (tone->instrument.amLevel)*(waveLUT[amWave][(int32_t)(tone->amPhase*period)]*amWaveInvert+1.0f)*0.5f;
                    level += 1.0f - tone->instrument.amLevel;
                }

#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
                if (level > 1.0f) godot::UtilityFunctions::print("level saturated! ", level);
#endif // DEBUG_ENABLED
                
                float tone1, tone2, tone3;
                {
                    double c = 1.0/120.0; // key 120 may be 8372.0Hz
                    double f1 = (double)waveLUT[sinWave][(int32_t)(tone->phase1*period)];
                    double f2 = (double)waveLUT[sinWave][(int32_t)(tone->phase2*period)];
                    double f3 = (double)waveLUT[sinWave][(int32_t)(tone->phase3*period)];

                    double g1 = (double)waveLUT[baseWave1][(int32_t)(tone->phase1*period)];
                    double g2 = (double)waveLUT[baseWave2][(int32_t)(tone->phase2*period)];
                    double g3 = (double)waveLUT[baseWave3][(int32_t)(tone->phase3*period)];

                    double r1 = godot::Math::clamp((double)(tone->realKey1)*c, 0.0, 1.0);
                    double r2 = godot::Math::clamp((double)(tone->realKey2)*c, 0.0, 1.0);
                    double r3 = godot::Math::clamp((double)(tone->realKey3)*c, 0.0, 1.0);

                    tone1 = (float)godot::Math::lerp(g1, f1, r1)*tone->base1ratio;
                    tone2 = (float)godot::Math::lerp(g2, f2, r2)*tone->base2ratio;
                    tone3 = (float)godot::Math::lerp(g3, f3, r3)*tone->base3ratio;
                }
                float data = tone1+tone2+tone3;
                
                if (tone->instrument.noiseColorType == NoiseColorType::NOISECTYPE_WHITE) {
                    data = data*(1.0f - tone->instrument.noiseRatio)+whiteNoiseLUT[noiseBufIndex+i]*tone->instrument.noiseRatio;
                }
                else if (tone->instrument.noiseColorType == NoiseColorType::NOISECTYPE_PINK) {
                    data = data*(1.0f - tone->instrument.noiseRatio)+pinkNoiseLUT[noiseBufIndex+i]*tone->instrument.noiseRatio;
                }

#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
                if (godot::Math::absf(data) > 1.0){
                    godot::UtilityFunctions::print("data 1 saturated! ", data);
                }
#endif // DEBUG_ENABLED
                data = godot::Math::clamp(data, -1.0f, 1.0f);

                data *= (tone->velocity_f*tone->strength*div*level)*tone->instrument.totalGain;
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
                if (godot::Math::absf(data) > 1.0){
                    godot::UtilityFunctions::print("data 2 saturated! ", data);
                }
#endif // DEBUG_ENABLED
                data = godot::Math::clamp(data, -1.0f, 1.0f);

                data = data * tone->mainRatio + tone->delayBuffer[tone->delayBufferIndex];
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
                if (godot::Math::absf(data) > 1.0){
                    godot::UtilityFunctions::print("data 3 saturated! ", data);
                }
#endif // DEBUG_ENABLED
                data = godot::Math::clamp(data, -1.0f, 1.0f);
                
                // delay
                float delayData;
                delayData = tone->delayBuffer[tone->delay0Index] + data * tone->delay0Ratio;
                if (delayData >  1.0) delayData =  1.0;
                if (delayData < -1.0) delayData = -1.0;
                tone->delayBuffer[tone->delay0Index] = delayData;
                delayData = tone->delayBuffer[tone->delay1Index] + data * tone->delay1Ratio;
                if (delayData >  1.0) delayData =  1.0;
                if (delayData < -1.0) delayData = -1.0;
                tone->delayBuffer[tone->delay1Index] = delayData;
                delayData = tone->delayBuffer[tone->delay2Index] + data * tone->delay2Ratio;
                if (delayData >  1.0) delayData =  1.0;
                if (delayData < -1.0) delayData = -1.0;
                tone->delayBuffer[tone->delay2Index] = delayData;

                tone->delay0Index += 1;
                if (tone->delay0Index == delayBufferSize) tone->delay0Index = 0;
                tone->delay1Index +=1;
                if (tone->delay1Index == delayBufferSize) tone->delay1Index = 0;
                tone->delay2Index += 1;
                if (tone->delay2Index == delayBufferSize) tone->delay2Index = 0;
                tone->delayBuffer[tone->delayBufferIndex] = 0.0f;
                tone->delayBufferIndex +=1;
                if (tone->delayBufferIndex == delayBufferSize) tone->delayBufferIndex = 0;

                frame[i] += (double)data;
                if (godot::Math::absf(frame[i]) > maxFrameValue) maxFrameValue = godot::Math::absf(frame[i]);
                frame[i] = godot::Math::clamp(frame[i], -1.0, 1.0);
            }
            current += delta;
        }
        if (maxFrameValue > 1.0){
#if defined(DEBUG_ENABLED) && defined(WINDOWS_ENABLED)
            godot::UtilityFunctions::print("saturated! ", maxFrameValue);
#endif // DEBUG_ENABLED
        }
        if (maxFrameValue > maxValue) maxValue = maxFrameValue;
        {
            godot::Dictionary dic;
            dic["msg"]                = (int32_t)1;
            dic["max_level"]          = (int32_t)(maxValue*1000.0);
            dic["frame_level"]        = (int32_t)(maxFrameValue*1000.0);
            emitSignal(dic);
        }

        maxFrameValue = 0.0;
        if (isEnd && tone->restartWaitDuration == FLOAT_LONGTIME){
            tone->phase1 = tone->phase2 = tone->phase3  = 0.0f;
            tone->strength = 0.0f;
            tone->atackedStrength = 0.0f;
            tone->decayedStrength = 0.0f;
            freeTones.insert(freeTones.end(), *tone);
            tone = activeTones.erase(tone);
            continue;
        }
        tone->passed += (int32_t)(delta * (float)bufferSamples);
        tone++;
    }
    frameCount += 1;
    frameCount %= noiseBufSize;

    if (oneNote.state == NState::NS_END && activeTones.size() == 0){
        midi.restart();
//        currentTime = -1000; // wait 1sec for repetition.
        currentTime = 0; // or executed immediately without waiting.
    }
    return true;
}
