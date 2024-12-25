#pragma once
#include <chrono>

class Timer
{
public:
	void Start(double durationSeconds)
	{
		start = std::chrono::high_resolution_clock::now();
		this->duration = durationSeconds;
		remainder = 0;
	}

	bool HasFinished()
	{
		std::chrono::duration<double> delta = (std::chrono::high_resolution_clock::now() - start);
		return delta.count() + remainder > duration;
	}
	
	void Restart()
	{
		start = std::chrono::high_resolution_clock::now();
		remainder = 0;
	}
	
	void RestartWithRemainder()
	{
		remainder = GetTimeLeft();
		start = std::chrono::high_resolution_clock::now();
	}

	double GetProgress()
	{
		std::chrono::duration<double>  delta = std::chrono::high_resolution_clock::now() - start;
		return std::min((delta.count() ) / duration, 1.0);
	}
	
	double GetTimeLeft()
	{
		std::chrono::duration<double>  delta = std::chrono::high_resolution_clock::now() - start;
		return duration - (delta.count());
	}
private:
	constexpr static long long ToSec = 1'000'000'000'000;
	std::chrono::high_resolution_clock::time_point start;
	double remainder;
	double duration;

};