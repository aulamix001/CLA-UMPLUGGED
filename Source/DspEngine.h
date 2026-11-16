#pragma once
#include <array>
#include <cstddef>
#include <algorithm>
#include <cmath>
#include <vector>

namespace unplugged
{
// XML indices/default and manual endpoints; intermediate divisions ordered by duration.
inline constexpr std::array<const char*,14> preDelayNames {
    "64T","64","32T","64D","32","16T","32D","16","8T","16D","8","4T","8D","4" };
inline constexpr std::array<double,14> preDelayBeats {
    1.0/24,1.0/16,1.0/12,3.0/32,1.0/8,1.0/6,3.0/16,1.0/4,1.0/3,3.0/8,1.0/2,2.0/3,3.0/4,1.0 };
inline float preDelayMilliseconds(int index,double bpm) noexcept {
    const double tempo=std::clamp(std::isfinite(bpm)?bpm:120.0,20.0,400.0);
    return static_cast<float>(60000.0/tempo*preDelayBeats[static_cast<std::size_t>(std::clamp(index,0,13))]);
}
inline int nearestPreDelay(float ms,double bpm=120) noexcept {
    if(!std::isfinite(ms)) return 7;
    int result=0;
    for(int i=1;i<14;++i)
        if(std::abs(preDelayMilliseconds(i,bpm)-ms)<std::abs(preDelayMilliseconds(result,bpm)-ms)) result=i;
    return result;
}
struct Settings
{
    float inputDb = 0, outputDb = 0;
    std::array<float, 6> amount { 0, 0, 0, 0, 0, 0 };
    // 0 = bypass, 1 = green, 2 = blue, 3 = red.
    std::array<int, 6> colour { 1, 1, 1, 1, 1, 1 };
    std::array<int, 2> preDelayNote { 7, 7 };
    std::array<bool, 2> preDelayOn { false, false };
    float balance = 0;
    bool direct = true, bypass = false;
    double bpm = 120;
};

class Engine
{
public:
    void prepare(double sampleRate);
    void reset() noexcept;
    void setSettings(const Settings&) noexcept;
    // In-place stereo. Buffers must have samples valid elements and must not alias.
    void process(float* left, float* right, int samples) noexcept;
    std::array<float, 2> inputPeak {}, outputPeak {};
private:
    struct Smooth
    {
        float value = 0, target = 0, coefficient = 0;
        float next() noexcept { return value = target + coefficient * (value - target); }
    };
    struct Biquad
    {
        double b0=1, b1=0, b2=0, a1=0, a2=0;
        std::array<double, 2> z1 {}, z2 {};
        void shelf(double sr, double hz, double db, bool high) noexcept;
        float tick(float x, int ch) noexcept;
        void clear() noexcept { z1 = {}; z2 = {}; }
    };
    struct Delay
    {
        std::vector<float> data;
        std::size_t write = 0;
        void prepare(std::size_t size);
        void clear() noexcept;
        float read(double samples) const noexcept;
        void push(float value) noexcept;
    };
    struct Reverb
    {
        std::array<Delay, 8> lines;
        std::array<float, 8> damp {};
        Delay pre;
        double sr = 48000;
        void prepare(double sampleRate);
        void clear() noexcept;
        std::array<float,2> tick(float x, int colour, int variant, float predelay) noexcept;
    };
    Settings settings;
    double sr = 48000;
    Biquad bass, treble;
    std::array<Reverb, 2> reverbs;
    std::array<float, 2> preDelayTimes {};
    std::array<Delay, 2> echo;
    Smooth inGain, outGain, directGain, balance;
    std::array<Smooth, 3> wet;
    Smooth echoTime;
    float envelope = 0, attack = 0, release = 0;
    static float gain(float db) noexcept;
};
}
