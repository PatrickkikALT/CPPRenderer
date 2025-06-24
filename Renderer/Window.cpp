#include <windows.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

struct PPMImage {
  int width, height;
  std::vector<unsigned char> pixels;
};

bool LoadPPM(const wchar_t* filename, PPMImage& img) {
  std::ifstream file(filename);
  if (!file) return false;

  std::string magic;
  file >> magic;
  if (magic != "P3") return false;

  while (file.peek() == '#') file.ignore(2048, '\n');

  file >> img.width >> img.height;
  int maxval;
  file >> maxval;
  if (maxval != 255) return false;

  img.pixels.resize(img.width * img.height * 3);
  for (size_t i = 0; i < img.pixels.size(); ++i) {
    int val;
    file >> val;
    img.pixels[i] = static_cast<unsigned char>(val);
  }

  return file.good();
}

HBITMAP CreateBitmapFromPPM(const PPMImage& img) {
  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = img.width;
  bmi.bmiHeader.biHeight = -img.height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 24;
  bmi.bmiHeader.biCompression = BI_RGB;

  void* pBits = nullptr;
  HDC hdc = GetDC(nullptr);
  HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
  ReleaseDC(nullptr, hdc);

  if (hBitmap && pBits) {
    unsigned char* dest = static_cast<unsigned char*>(pBits);
    for (int i = 0; i < img.width * img.height; ++i) {
      dest[i * 3 + 0] = img.pixels[i * 3 + 2];
      dest[i * 3 + 1] = img.pixels[i * 3 + 1];
      dest[i * 3 + 2] = img.pixels[i * 3 + 0];
    }
  }

  return hBitmap;
}

PPMImage g_image;
HBITMAP g_hBitmap = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    if (g_hBitmap) {
      RECT rect;
      GetClientRect(hwnd, &rect);

      HDC memDC = CreateCompatibleDC(hdc);
      HGDIOBJ oldBmp = SelectObject(memDC, g_hBitmap);

      StretchBlt(
        hdc,
        0, 0, rect.right, rect.bottom,
        memDC,
        0, 0, g_image.width, g_image.height,
        SRCCOPY
      );

      SelectObject(memDC, oldBmp);
      DeleteDC(memDC);
    }
    EndPaint(hwnd, &ps);
    return 0;
  }

  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
  if (!LoadPPM(L"image.ppm", g_image)) {
    MessageBoxW(nullptr, L"Failed to load PPM image", L"Error", MB_OK);
    return 1;
  }

  g_hBitmap = CreateBitmapFromPPM(g_image);
  if (!g_hBitmap) {
    MessageBoxW(nullptr, L"Failed to create bitmap", L"Error", MB_OK);
    return 1;
  }

  const wchar_t CLASS_NAME[] = L"PPMViewerClass";

  WNDCLASSW wc = {};
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;
  wc.style = CS_HREDRAW | CS_VREDRAW;
  RegisterClassW(&wc);

  HWND hwnd = CreateWindowExW(
    0,
    CLASS_NAME,
    L"PPM Viewer",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT,
    g_image.width + 300, g_image.height + 300,
    nullptr, nullptr, hInstance, nullptr
  );

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  MSG msg;
  while (GetMessageW(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  DeleteObject(g_hBitmap);
  return 0;
}
