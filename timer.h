#pragma once
#include <chrono>

class Timer
{
public:
	void Start(double durationSeconds, double remainderLeeway = 0.0333)
	{
		start = std::chrono::high_resolution_clock::now();
		duration = durationSeconds;
		remainder = 0;
		this->remainderLeeway = remainderLeeway;
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
		if (remainder < -remainderLeeway)
		{
			remainder = -remainderLeeway;
		}
		start = std::chrono::high_resolution_clock::now();
	}

	double GetProgress()
	{
		std::chrono::duration<double>  delta = std::chrono::high_resolution_clock::now() - start;
		return std::min((delta.count() + remainder) / duration, 1.0);
	}
	
	double GetTimeLeft()
	{
		std::chrono::duration<double>  delta = std::chrono::high_resolution_clock::now() - start;
		return duration - (delta.count() + remainder);
	}
private:
	std::chrono::high_resolution_clock::time_point start;
	double remainder;
	double duration;
	double remainderLeeway;
};