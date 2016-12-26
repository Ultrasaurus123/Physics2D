#include <Windows.h>
#include <memory>
#include <assert.h>
#include "game.h"

static HWND WindowInit(HINSTANCE instance, const wchar_t* class_name, int32_t width, int32_t height);
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
	HWND window = WindowInit(instance, L"SimplePhysics", 1280, 720);

	if (!window)
	{
		return -1;
	}

	std::shared_ptr<Game> game(new Game());
	if (!game->Init(window))
	{
		return -2;
	}

	ShowWindow(window, SW_SHOW);
	UpdateWindow(window);

	LARGE_INTEGER now, lastFrame, frequency;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&lastFrame);

	MSG msg{};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			QueryPerformanceCounter(&now);
			float dt = (float)(now.QuadPart - lastFrame.QuadPart) / frequency.QuadPart;
			if (IsDebuggerPresent()) 
			{
				dt = (dt > 0.01666f) ? 0.01666f : dt;
			}

			lastFrame = now;
			if (!game->Update(dt))
			{
				break;
			}
			game->Draw();
		}
	}

	DestroyWindow(window);
	return 0;
}

HWND WindowInit(HINSTANCE instance, const wchar_t* class_name, int32_t width, int32_t height)
{
	WNDCLASSEX wcx{};
	wcx.cbSize = sizeof(wcx);
	wcx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcx.hInstance = instance;
	wcx.lpfnWndProc = WindowProc;
	wcx.lpszClassName = class_name;
	if (RegisterClassEx(&wcx) == INVALID_ATOM)
	{
		assert(false);
		return nullptr;
	}

	DWORD style = WS_OVERLAPPEDWINDOW;

	RECT rc{};
	rc.right = width;
	rc.bottom = height;
	AdjustWindowRect(&rc, style, FALSE);

	HWND hwnd = CreateWindow(wcx.lpszClassName, wcx.lpszClassName, style, CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, instance, nullptr);
	if (!hwnd)
	{
		assert(false);
		return nullptr;
	}

	return hwnd;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
