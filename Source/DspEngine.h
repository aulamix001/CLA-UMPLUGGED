#pragma once
#include <array>
#include <cstddef>
#include <vector>

namespace unplugged
{
struct Settings
{
    float inputDb = 0, outputDb = 0;
    std::array<float, 6> amount { 0, 0, 0, 0, 0, 0 };
    // 0 = bypass, 1 = green, 2 = blue, 3 = red.
    std::array<int, 6> colour { 1, 1, 1, 1, 1, 1 };
    std::array<float, 2> preDelayMs { 16, 16 };
    std::array<bool, 2> preDelayOn { true, true };
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
    std::array<Delay, 2> echo;
    Smooth inGain, outGain, directGain, balance;
    std::array<Smooth, 3> wet;
    Smooth echoTime;
    float envelope = 0, attack = 0, release = 0;
    static float gain(float db) noexcept;
};
}
