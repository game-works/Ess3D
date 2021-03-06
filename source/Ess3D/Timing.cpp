#include "Timing.h"
#include <SDL.h>
#include <iostream>
#include <algorithm>

namespace Ess3D {

  FPSLimiter::FPSLimiter() { }

  void FPSLimiter::init(float maxFPS, bool limitFPS) {
    setMaxFPS(maxFPS);
    _limitFPS = limitFPS;
  }

  void FPSLimiter::begin() {
    _startTicks = std::chrono::high_resolution_clock::now();
  }

  float FPSLimiter::end() {
    _newTicks = std::chrono::high_resolution_clock::now();
    _frameTime = std::chrono::duration_cast<std::chrono::microseconds>(_newTicks - _startTicks).count() / 1000.0f; //in miliseconds

    //limit FPS
    if(_limitFPS && _maxFPS > 0 && 1000.0f / _maxFPS > _frameTime) {
      SDL_Delay((Uint32) (1000.0f / _maxFPS - _frameTime));
    }
     
    //calculate FPS
    calculateFPS();

    return _fps;
  }

  void FPSLimiter::setMaxFPS(float fps) {
    _maxFPS = fps;
  }

  void FPSLimiter::calculateFPS() {
    static const int NUM_SAMPLES = 10;
    static float frameTimes[NUM_SAMPLES];
    static int currentFrame = 0;
    static std::chrono::high_resolution_clock::time_point prevTicks = std::chrono::high_resolution_clock::now();

    std::chrono::high_resolution_clock::time_point currentTicks = std::chrono::high_resolution_clock::now();
    _frameTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTicks - prevTicks).count() / 1000.0f;

    //if it runs too fast, it puts the frame time as 0 so we just wanna approximate to 1
    if(_frameTime == 0) {
      _frameTime = 1;
    }

    frameTimes[currentFrame % NUM_SAMPLES] = _frameTime;

    prevTicks = currentTicks;
    currentFrame++;

    int sampleCount = NUM_SAMPLES;
    if(currentFrame < NUM_SAMPLES) {
      sampleCount = currentFrame;
    }

    float frameTimeAverage = 0;
    for(int i = 0; i < sampleCount; i++) {
      frameTimeAverage += frameTimes[i];
    }

    frameTimeAverage /= sampleCount;

    if(frameTimeAverage > 0) {
      _fps = 1000.0f / frameTimeAverage;
    } else {
      _fps = 0.0f;
    }
  }

  TimestepAccumulator::TimestepAccumulator() {}

  TimestepAccumulator::~TimestepAccumulator() {}

  void TimestepAccumulator::init() {
    _prevTicks = std::chrono::high_resolution_clock::now(); // microseconds
    _newTicks = _prevTicks; // microseconds
    _frametime = 0.0f;
  }

  int TimestepAccumulator::step() {
    _newTicks = std::chrono::high_resolution_clock::now(); // microseconds
    _frametime = std::chrono::duration_cast<std::chrono::microseconds>(_newTicks - _prevTicks).count() / 1000.0f; //in miliseconds
    _prevTicks = _newTicks;

    _accumulator += _frametime / 1000.0f;
    const int nSteps = static_cast<int> (std::floor(_accumulator / _timestep));
    if(nSteps > 0) {
      _accumulator -= nSteps * _timestep;
    }
    _accumulatorRatio = _accumulator / _timestep;

    return std::min(nSteps, _maxFramesSimulated);
  }

}