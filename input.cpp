#include "Input.h"

#include <assert.h>

KeyState Input::GetKeyState(VK_KEY key)
{
	return KeyState();
}

void Input::Update()
{
	for (size_t i = 0; i < data.size(); i++)
	{
		if (data[i] == KeyState::JustUp)
		{
			data[i] = KeyState::Up;
		}	
		else if (data[i] == KeyState::JustDown)
		{
			data[i] = KeyState::Down;
		}
	}
}

void Input::SetKey(VK_KEY key, KeyState state)
{
	assert(key < data.size());

	//avoid repeat inputs
	if(data[key] == KeyState::Down && state == KeyState::JustDown)
		return;
		
	data[key] = state;
}
