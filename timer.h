#pragma once
#include <chrono>

class Timer
{
public:
	void Start(double durationSeconds)
	{
		start = std::chrono::high_resolution_clock::now();
		this->duration = durationSeconds;
	}

	bool HasFinished()
	{
		auto delta = std::chrono::high_resolution_clock::now() - start;
		return (delta.count() / ToSec) > duration;
	}
	
	void Restart()
	{
		start = std::chrono::high_resolution_clock::now();
	}
	
	double GetProgress()
	{
		auto delta = std::chrono::high_resolution_clock::now() - start;
		return std::min((delta.count() / ToSec) / duration, 1.0);
	}
	
	double GetTimeLeft()
	{
		auto delta = std::chrono::high_resolution_clock::now() - start;
		return duration - (delta.count() / ToSec);
	}
private:
	constexpr static long long ToSec = 1'000'000'000;
	std::chrono::high_resolution_clock::time_point start;
	double duration;

};