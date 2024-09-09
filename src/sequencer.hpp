/**************************************************************************/
/*  sequencer.hpp                                                         */
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

#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <filesystem>

#include <cmath>
#include "smfparser.hpp"
#include <list>
#include <array>
#include <functional>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/json.hpp>

#define PI (float)Math_PI
#define FLOAT_LONGTIME 36000000.0f

enum class BaseWave {
    WAVE_SIN,         //  0
    WAVE_SQUARE,      //  1
    WAVE_TRIANGLE,    //  2
    WAVE_SAWTOOTH,    //  3
    WAVE_SINSAWx2,    //  4

    WAVE_TAIL
};


enum class NoiseDistributType {
    NOISEDTYPE_FLAT,       //  0
    NOISEDTYPE_TRIANGULAR, //  1
    NOISEDTYPE_COS4ThPOW,  //  2

    NOISEDTYPE_TAIL
};

enum class NoiseColorType {
    NOISECTYPE_WHITE,   //  0
    NOISECTYPE_PINK,    //  1

    NOISECTYPE_TAIL
};

struct Instrument{
    float totalGain;
    
    float atackSlopeTime;
    float decayHalfLifeTime;
    float sustainRate;
    float releaseSlopeTime;

    float baseVsOthersRatio;
    float side1VsSide2Ratio;

    float baseOffsetCent1;
    BaseWave baseWave1;
    float baseOffsetCent2;
    BaseWave baseWave2;
    float baseOffsetCent3;
    BaseWave baseWave3;

    float noiseRatio;
    NoiseColorType noiseColorType;
    
    float delay0Time;
    float delay1Time;
    float delay2Time;
    float delay0Ratio;
    float delay1Ratio;
    float delay2Ratio;
    
    float freqNoiseCentRange;
    NoiseDistributType freqNoiseType;
    
    float fmCentRange;
    float fmFreq;
    float fmPhaseOffset;
    int32_t fmSync;
    BaseWave fmWave;

    float amLevel;
    float amFreq;
    float amPhaseOffset;
    int32_t amSync;
    BaseWave amWave;
};

struct Percussion{
    int32_t program;
    int32_t key;
};

class PinkNoise {
private:
    static constexpr int32_t tapNum  = 16;
    float z[tapNum];
    float k[tapNum];
    float t = 0.0;
public:
    PinkNoise();
    ~PinkNoise();
    float makeNoise (float);    
};


class Sequencer {
public:
    // constant control params.
    static constexpr int32_t numinstruments = 256;
    static constexpr int32_t numPercussions = 128;

private:
    // constant control params.
    static constexpr int32_t numTone = 64;
//    static constexpr int32_t waveLUTSize = 8192;
    static constexpr int32_t waveLUTSize = 32768;
    static constexpr float delayBufferDuration = 500.0;// msec

    struct Tone {
        // from smf
        Note note;
        int32_t velocity;
        float velocity_f;
        int32_t tempo;
        float tempo_f;
        int32_t restartVelocity;
        float restartVelocity_f;
        int32_t restartTempo;
        float restartTempo_f;

        // envelope factor
        float strength = 0.0;
        float atackedStrength = 0.0f;
        float decayedStrength = 0.0f;
        float atackedStrengthfloor = 0.0f;
        float atackSlopeRatio;
        float decaySlopeRatio;
        float releaseSlopeRatio;

        // signal cont
        float phase1;
        float phase2;
        float phase3;
        float baseIncrement1 = 0.0;
        float baseIncrement2 = 0.0;
        float baseIncrement3 = 0.0;

        float base1ratio;
        float base2ratio;
        float base3ratio;
        float frequency;
        int32_t passed;
        float waitDuration;
        float restartWaitDuration;
        float mainteinDuration;

        float freqNoiseCentharfRange;
        
        Instrument instrument;
        
        // for delay
        float* delayBuffer = nullptr;
        int32_t delayBufferIndex;
        int32_t delay0Index;
        int32_t delay1Index;
        int32_t delay2Index;
        float delay0Ratio;
        float delay1Ratio;
        float delay2Ratio;
        float mainRatio;
        
        //fm moduration
        float fmPhase;
        float fmIncrement;
        
        //am moduration
        float amPhase;
        float amIncrement;
        
        int32_t program;
        int32_t key;
        int32_t realKey1;
        int32_t realKey2;
        int32_t realKey3;
        float maxDelayTime;
    };
    SMFParser midi;
    int32_t delayBufferSize = 0;
    float unitOfTime = 60000.0;
    std::array<Tone, numTone> toneInstances;
    std::list<Tone> activeTones;
    std::list<Tone> freeTones;
    std::array<Instrument, numinstruments> instruments;
    std::array<Percussion, numPercussions> percussions;

    float samplingRate = 44100.0f;
    float bufferingTime = 0.05f;
    int32_t bufferSamples;

    int32_t currentTime = 0;
    int32_t frameCount = 0;
    int32_t noiseBufSize;
    int32_t noiseBuffer;
    bool isSet = false;
    std::array<std::array<float, waveLUTSize>, static_cast<int32_t>(BaseWave::WAVE_TAIL)> waveLUT;

    float atackSlopeHz = 25.0;
    std::unique_ptr<float []> atackSlopeLUT;
    float atackSlopeTime;
    int32_t numAtackSlopeLUT;
    
    float releaseSlopeHz = 25.0;
    std::unique_ptr<float []> releaseSlopeLUT;
    float releaseSlopeTime;
    int32_t numReleaseSlopeLUT;
    
    float decaySlopeHz = 1.0;
    float decayHalfLifeTime = 50.0;
    std::unique_ptr<float []> decaySlopeLUT;
    float decaySlopeTime;
    int32_t numDecaySlopeLUT;
    
    float sustainRate = 0.0;
    godot::RandomNumberGenerator *rand;

    std::unique_ptr<float []> velocity2powerLUT;
    std::unique_ptr<float []> whiteNoiseLUT;
    std::unique_ptr<float []> pinkNoiseLUT;
    std::unique_ptr<float []> triangularDistributionLUT;
    std::unique_ptr<float []> cos4thPowDistributionLUT;
    static constexpr int32_t pow2_x_1200LUT_size = 7200;
    std::unique_ptr<float []> pow2_x_1200LUT;
    
    float asumedConcurrentTone = 4.0f;
    bool checkNewNote(Note);
    int32_t logLevel = 1;
public:
    double maxValue = 0.0;
    float noteFrequency(int8_t);
    float centFrequency(float, float);
    bool initParam(double, double, int32_t);
    godot::Array getInstruments(void);
    void setInstruments(const godot::Array);
    void setControlParams(const godot::Dictionary);
    godot::Dictionary getControlParams(void);
    void setPercussions(const godot::Array);
    godot::Array getPercussions(void);
    void incertNoteOn(const godot::Dictionary);
    void incertNoteOff(const godot::Dictionary);
    godot::Ref<godot::Image> getMiniWavePicture(const godot::Dictionary);
    bool feed(double*);
    bool smfLoad(const char*, double);
    bool smfLoad(const godot::String &, double);
    bool smfUnload(void);
    std::function<void(const godot::Dictionary dic)> emitSignal;
    Sequencer();
    ~Sequencer();
};

#endif // SEQUENCER_H

