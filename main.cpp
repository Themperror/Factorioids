#include <utility>
#include <memory>
#include <chrono>

#include "input.h"
#include "menu.h"
#include "game.h"
#include "renderer.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

bool shouldQuit = false;
Input input{};



LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//Retrieve our renderer if it exists
	LONG_PTR userData = GetWindowLongPtrA(hwnd,0);
	Renderer* renderer = nullptr;
	if (userData)
	{
		renderer = reinterpret_cast<Renderer*>(userData);
	}


	if (msg == WM_SIZE && renderer)
	{
		if (wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED)
		{
			RECT windowRect;
			GetClientRect(hwnd, &windowRect);
			renderer->Resize(windowRect.right, windowRect.bottom);
		}
	}

	switch (msg)
	{
		case WM_CLOSE:
		{
			shouldQuit = true;
		}
		break;
		case WM_DESTROY:
		{
			shouldQuit = true;
		}
		break;
		case WM_SIZE:
		{
			if (wParam == SIZE_MAXIMIZED)
			{
				//Themp::System::tSys->m_SVars[std::string("WindowPosX")] = 0;
				//Themp::System::tSys->m_SVars[std::string("WindowPosY")] = 0;
			}
			//I see no reason why we should resize DURING the sizing move, rather wait until we're done resizing and then actually change everything..
		}
		break;
		case WM_MOVING:
		{
			//GetWindowRect(Themp::System::tSys->m_Window, &windowRect);
			//Themp::System::tSys->m_SVars[std::string("WindowPosX")] = windowRect.left;
			//Themp::System::tSys->m_SVars[std::string("WindowPosY")] = windowRect.top;
		}
		break;
		case WM_EXITSIZEMOVE:
		{
			if (renderer)
			{
				RECT windowRect;
				GetClientRect(hwnd, &windowRect);
				renderer->Resize(windowRect.right, windowRect.bottom);
			}
		}
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


HWND window{};

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//Console for debug output
#if _DEBUG
	AllocConsole();
	FILE* conout = nullptr;
	freopen_s(&conout, "CONOUT$", "w", stdout);
#endif
	Util::SetLogFile("log.txt");

	//Set up Win32 window
	{
		WNDCLASSEX wndClass{};
		wndClass.cbSize = sizeof(WNDCLASSEXA);
		wndClass.style = CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc = &WinProc;
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.hInstance = hInstance;
		wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wndClass.lpszClassName = L"Factorioids";
		RegisterClassEx(&wndClass);

		HWND desktop = GetDesktopWindow();
		RECT bSize;
		GetWindowRect(desktop, &bSize);

		window = CreateWindowEx(NULL,
			L"Factorioids",
			L"Factorioids",
			WS_POPUP,
			bSize.left,
			bSize.top,
			bSize.right,
			bSize.bottom,
			NULL, NULL, hInstance, NULL);

		SetWindowLongPtrA(window, 0, 0);

		AdjustWindowRect(&bSize, WS_VISIBLE | WS_POPUP, true);
		ShowWindow(window, nShowCmd);
	}

	{
		std::chrono::high_resolution_clock::time_point old = std::chrono::high_resolution_clock::now();
		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		auto delta = now - old;

		enum class SceneState{ Menu, Game } sceneState = SceneState::Menu;
		std::unique_ptr<Menu> menu;
		std::unique_ptr<Game> game;
		std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>();

		RECT windowRect;
		GetWindowRect(window, &windowRect);

		if (!renderer->Init(window, windowRect.right, windowRect.bottom))
		{
			MessageBoxA(window, "Something went wrong initialising the renderer. Exiting!", "Error!", MB_OK);
			shouldQuit = true;
		}
		//so that we may access the renderer in our message callbacks
		SetWindowLongPtrA(window,0,static_cast<LONG_PTR>(reinterpret_cast<uintptr_t>(renderer.get())));

		while (!shouldQuit)
		{
			//////////////
			//Input handling
			input.Update();
			MSG msg{};
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (msg.message == WM_QUIT)
				{
					shouldQuit = true;
				}
				
				if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN)
				{
					input.SetKey(static_cast<VK_KEY>(msg.wParam), KeyState::JustDown);
				}
				if (msg.message == WM_KEYUP || msg.message == WM_SYSKEYUP)
				{
					input.SetKey(static_cast<VK_KEY>(msg.wParam), KeyState::JustUp);
				}
				//Util::Print("0x%x 0x%x 0x%x", msg.message, msg.wParam, msg.lParam);

			}
			//////////////


			//////////////
			//scene handling
			renderer->BeginDraw();
			old = now;
			now = std::chrono::high_resolution_clock::now();
			delta = now - old;
			double deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count() / 1000.0;
			switch (sceneState)
			{
				case SceneState::Menu:
				{
					if (!menu.get())
					{
						menu = std::make_unique<Menu>();
						menu->Init(*renderer);
					}

					Scene::Status status = menu->Update(deltaMs, input);
					if (status == Scene::Status::Running)
					{
						menu->Render(*renderer);
					}
					else
					{
						menu = nullptr;
					}

					if (status == Scene::Status::ToGame)
					{
						sceneState = SceneState::Game;
					}
					else if (status == Scene::Status::Quitting)
					{
						shouldQuit = true;
					}
				}
				break;
				case SceneState::Game:
				{
					if (!game.get())
					{
						game = std::make_unique<Game>();
						game->Init(*renderer);
					}

					Scene::Status status = game->Update(deltaMs, input);
					if (status == Scene::Status::Running)
					{
						game->Render(*renderer);
					}
					else
					{
						game = nullptr;
					}

					if (status == Scene::Status::ToMenu)
					{
						sceneState = SceneState::Menu;
					}
					else if (status == Scene::Status::Quitting)
					{
						shouldQuit = true;
					}
				}
				break;
			}
			renderer->Present();
			//////////////
		}
	}

#if _DEBUG
	fclose(conout);
#endif

	SetWindowLongPtrA(window, 0, 0);

	return 0;
}