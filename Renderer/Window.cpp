#include <windows.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

bool g_drawing = false;
unsigned char* g_pBits = nullptr;
int g_rowSize = 0;

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

  int width = 0, height = 0, maxval = 0;
  while (file >> width) {
    if (file >> height && file >> maxval) break;
  }

  if (width <= 0 || height <= 0 || maxval != 255) return false;

  img.width = width;
  img.height = height;
  img.pixels.resize(width * height * 3);

  int val;
  size_t i = 0;
  while (file >> val && i < img.pixels.size()) {
    if (val < 0 || val > 255) return false; 
    img.pixels[i++] = static_cast<unsigned char>(val);
  }

  return i == img.pixels.size();
}


HBITMAP CreateBitmapFromPPM(const PPMImage& img) {
  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = img.width;
  bmi.bmiHeader.biHeight = -img.height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 24;
  bmi.bmiHeader.biCompression = BI_RGB;

  HDC hdc = GetDC(nullptr);
  HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&g_pBits), nullptr, 0);
  ReleaseDC(nullptr, hdc);

  if (hBitmap && g_pBits) {
    unsigned char* dest = static_cast<unsigned char*>(g_pBits);
    g_rowSize = ((img.width * 3 + 3) & ~3);

    for (int y = 0; y < img.height; ++y) {
      for (int x = 0; x < img.width; ++x) {
        int srcIdx = (y * img.width + x) * 3;
        int dstIdx = y * g_rowSize + x * 3;

        dest[dstIdx + 0] = img.pixels[srcIdx + 2];
        dest[dstIdx + 1] = img.pixels[srcIdx + 1];
        dest[dstIdx + 2] = img.pixels[srcIdx + 0];
      }
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
  case WM_LBUTTONDOWN:
    g_drawing = true;
    SetCapture(hwnd);
    return 0;
  case WM_LBUTTONUP:
    g_drawing = false;
    ReleaseCapture();
    return 0;

  case WM_MOUSEMOVE:
    if (g_drawing && g_pBits) {
      int x = LOWORD(lParam);
      int y = HIWORD(lParam);

      RECT rect;
      GetClientRect(hwnd, &rect);
      int width = rect.right;
      int height = rect.bottom;

      int imgX = x * g_image.width / width;
      int imgY = y * g_image.height / height;

      if (imgX >= 0 && imgX < g_image.width && imgY >= 0 && imgY < g_image.height) {
        int index = imgY * g_rowSize + imgX * 3;
        g_pBits[index + 0] = 0;
        g_pBits[index + 1] = 0;
        g_pBits[index + 2] = 255;
        InvalidateRect(hwnd, nullptr, FALSE);
      }
    }
    return 0;
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
