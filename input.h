#pragma once
#include <array>

enum class KeyState : uint8_t
{
	Up = 0x0, 
	Down = 0x1,
	JustDown = 0x2,
	JustUp = 0x4
};

using VK_KEY = uint8_t;

class Input
{
	static constexpr size_t max_keys = 0xFF;
public:
	KeyState GetKeyState(VK_KEY key);
	void Update();
	void SetKey(VK_KEY key, KeyState state);
private:
	std::array<KeyState, max_keys> data;
};