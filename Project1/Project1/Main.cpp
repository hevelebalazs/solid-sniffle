#include <Windows.h>

#include <math.h>

struct Buffer {
	unsigned int *colors;
	int width;
	int height;
};

static Buffer global_buffer;
static bool global_running;

static void
ResizeBuffer(Buffer *buffer, int width, int height) {
	delete buffer->colors;

	buffer->width = width;
	buffer->height = height;

	buffer->colors = new unsigned int[width * height];
}

static LRESULT CALLBACK
WinCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	LRESULT result = 0;

	switch(message) {
		case WM_SIZE: {
			RECT rect = {};
			GetClientRect(window, &rect);
			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;

			ResizeBuffer(&global_buffer, width, height);

			break;
		}
		case WM_DESTROY:
		case WM_CLOSE: {
			global_running = false;
			break;
		}
		default: {
			result = DefWindowProc(window, message, wparam, lparam);
			break;
		}
	}

	return result;
}

static void
SetPixelColor(Buffer *buffer, int row, int col, unsigned int color) {
	buffer->colors[row * buffer->width + col] = color;
}

static void
DrawScene(Buffer *buffer) {
	unsigned int background_color = 0xAA8888;
	unsigned int *pixel = buffer->colors;
	for(int row = 0; row < buffer->height; row++) {
		for(int col = 0; col < buffer->width; col++) {
			SetPixelColor(buffer, row, col, background_color);
		}
	}
}

int CALLBACK
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show) {
	WNDCLASSA win_class = {};
	win_class.style = CS_OWNDC;
	win_class.lpfnWndProc = WinCallback;
	win_class.hInstance = instance;
	win_class.lpszClassName = (LPCSTR)L"WWC";

	RegisterClassA(&win_class);
	HWND window = CreateWindowExA(
		0,
		win_class.lpszClassName,
		(LPCSTR)L"Window",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		instance,
		0
	);

	Buffer *buffer = &global_buffer;
	global_running = true;
	while(global_running) {
		MSG message = {};
		while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		HDC context = GetDC(window);
		BITMAPINFO bitmap_info = {};
		BITMAPINFOHEADER *header = &bitmap_info.bmiHeader;
		header->biSize = sizeof(*header);
		header->biWidth = buffer->width;
		header->biHeight = buffer->height;
		header->biPlanes = 1;
		header->biBitCount = 32;
		header->biCompression = BI_RGB;

		RECT rect = {};
		GetClientRect(window, &rect);

		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;

		DrawScene(buffer);

		StretchDIBits(context,
					  0, 0, buffer->width, buffer->height,
					  0, 0, width, height,
					  buffer->colors,
					  &bitmap_info,
					  DIB_RGB_COLORS,
					  SRCCOPY
		);
		ReleaseDC(window, context);
	}

	return 0;
}