#include "Timer.h"
#include <iostream>

#ifdef max
#undef max
#endif

//------------------------------------------------------------------------

double Timer::s_ticksToSecsCoef    = -1.0;
long long int Timer::s_prevTicks   = 0;

//------------------------------------------------------------------------

float Timer::end(void)
{
    long long int elapsed = getElapsedTicks();
    m_startTicks += elapsed;
    m_totalTicks += elapsed;
    return ticksToSecs(elapsed);
}

//------------------------------------------------------------------------

inline long long int max(long long int a, long long int b) { return a > b ? a : b; }
inline double max(double a, double b) { return a > b ? a : b; }

long long int Timer::queryTicks(void)
{
  s_prevTicks = max(s_prevTicks, clock());
  return s_prevTicks;
}

//------------------------------------------------------------------------

float Timer::ticksToSecs(long long int ticks)
{
  return (float)(ticks) / (float)CLOCKS_PER_SEC;
}

//------------------------------------------------------------------------

long long int Timer::getElapsedTicks(void)
{
    long long int curr = queryTicks();
    if (m_startTicks == -1)
        m_startTicks = curr;
    return curr - m_startTicks;
}

//------------------------------------------------------------------------
