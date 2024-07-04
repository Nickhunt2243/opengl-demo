//
// Created by admin on 6/20/2024.
//

#include "timer.hpp"

namespace Engine
{
    float Timer::getTimeSpan()
    {
        std::chrono::steady_clock::time_point currTime = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(currTime - prevTime);
        prevTime = currTime;
        return (float) diff.count();
    }
    float Timer::getElapsedTime()
    {
        std::chrono::steady_clock::time_point currTime = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(currTime - startTime);
        return (float) diff.count();
    }
    void Timer::startStopWatch()
    {
        stopWatchStart = std::chrono::steady_clock::now();
    }
    float Timer::lapStopWatch()
    {
        std::chrono::steady_clock::time_point currTime = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(currTime - stopWatchStart);
        return (float) diff.count();
    }
    void Timer::incFrames()
    {
        frames++;
    }
    void Timer::resetTimer()
    {
        startTime = std::chrono::steady_clock::now();
        frames = 0;
    }
    float Timer::getFPS()
    {

        return frames / (getElapsedTime() / 1000);
    }
}