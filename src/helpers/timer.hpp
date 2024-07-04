//
// Created by admin on 6/20/2024.
//

#ifndef OPENGLDEMO_TIMER_HPP
#define OPENGLDEMO_TIMER_HPP

#include <chrono>

namespace Engine
{
    class Timer
    {
    public:
        Timer() = default;

        /// Retrieve the current offset from the start time.
        float getElapsedTime();
        /// Retrieve the time since the getTimeSpan was last called.
        float getTimeSpan();
        /// Retrieve the current frames per second.
        float getFPS();
        /// Start a stop watch.
        void startStopWatch();
        /// Get the current elapsed time of the stop watch (This is what apple calls it on the clock app idk).
        float lapStopWatch();
        /// Increment the number of frames elapsed.
        void incFrames();
        /// Reset the time and frames.
        void resetTimer();
    private:
        /// The startTime of the given timer. Only set on initialization and when resetTimer is called.
        std::chrono::steady_clock::time_point startTime{std::chrono::steady_clock::now()};
        /// The prevTime the timer has called getTimeSpan.
        std::chrono::steady_clock::time_point prevTime{startTime};
        /// The time the stop watch was started.
        std::chrono::steady_clock::time_point stopWatchStart{};
        /// The number of frames.
        float frames{0};
    };
}

#endif //OPENGLDEMO_TIMER_HPP
