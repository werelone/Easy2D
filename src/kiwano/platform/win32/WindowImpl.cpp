// Copyright (c) 2016-2020 Kiwano - Nomango
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <kiwano/platform/Window.h>

#if defined(KGE_PLATFORM_WINDOWS)

#include <memory>
#include <array>
#include <kiwano/core/Keys.h>
#include <kiwano/core/Exception.h>
#include <kiwano/core/Logger.h>
#include <kiwano/core/event/KeyEvent.h>
#include <kiwano/core/event/MouseEvent.h>
#include <kiwano/core/event/WindowEvent.h>
#include <kiwano/platform/Application.h>
#include <Windowsx.h>  // GET_X_LPARAM, GET_Y_LPARAM
#include <imm.h>       // ImmAssociateContext
#pragma comment(lib, "imm32.lib")

namespace kiwano
{

KGE_DECLARE_SMART_PTR(WindowWin32Impl);

class KGE_API WindowWin32Impl : public Window
{
public:
    WindowWin32Impl();

    virtual ~WindowWin32Impl();

    void Init(const String& title, uint32_t width, uint32_t height, uint32_t icon, bool resizable, bool fullscreen);

    void SetTitle(const String& title) override;

    void SetIcon(uint32_t icon_resource) override;

    void Resize(uint32_t width, uint32_t height) override;

    void SetFullscreenState(bool fullscreen) override;

    void SetCursor(CursorType cursor) override;

    void PumpEvents() override;

    DWORD GetStyle() const;

    void UpdateCursor();

    void SetActive(bool actived);

    LRESULT MessageProc(HWND, UINT32, WPARAM, LPARAM);

    static LRESULT CALLBACK StaticWndProc(HWND, UINT32, WPARAM, LPARAM);

private:
    bool       resizable_;
    CursorType mouse_cursor_;
    String     device_name_;

    std::array<KeyCode, 256> key_map_;
};

WindowPtr Window::Create(const String& title, uint32_t width, uint32_t height, uint32_t icon, bool resizable,
                         bool fullscreen)
{
    WindowWin32ImplPtr ptr = memory::New<WindowWin32Impl>();
    if (ptr)
    {
        ptr->Init(title, width, height, icon, resizable, fullscreen);
    }
    return ptr;
}

}  // namespace kiwano

namespace kiwano
{

#define WINDOW_FIXED_STYLE WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
#define WINDOW_RESIZABLE_STYLE WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX
#define WINDOW_FULLSCREEN_STYLE WS_CLIPCHILDREN | WS_POPUP

namespace
{
MONITORINFOEXA GetMoniterInfoEx(HWND hwnd)
{
    HMONITOR       monitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFOEXA monitor_info;

    memset(&monitor_info, 0, sizeof(monitor_info));
    monitor_info.cbSize = sizeof(monitor_info);
    ::GetMonitorInfoA(monitor, &monitor_info);

    return monitor_info;
}

void AdjustWindow(uint32_t width, uint32_t height, DWORD style, uint32_t* win_width, uint32_t* win_height)
{
    RECT rc;
    ::SetRect(&rc, 0, 0, (int)width, (int)height);
    ::AdjustWindowRect(&rc, style, false);

    *win_width  = rc.right - rc.left;
    *win_height = rc.bottom - rc.top;

    MONITORINFOEXA info = GetMoniterInfoEx(NULL);

    uint32_t screenw = info.rcWork.right - info.rcWork.left;
    uint32_t screenh = info.rcWork.bottom - info.rcWork.top;

    if (*win_width > screenw)
        *win_width = screenw;
    if (*win_height > screenh)
        *win_height = screenh;
}

}  // namespace

WindowWin32Impl::WindowWin32Impl()
    : resizable_(false)
    , mouse_cursor_(CursorType::Arrow)
    , key_map_{}
{
    // Keys
    key_map_[VK_UP]      = KeyCode::Up;
    key_map_[VK_LEFT]    = KeyCode::Left;
    key_map_[VK_RIGHT]   = KeyCode::Right;
    key_map_[VK_DOWN]    = KeyCode::Down;
    key_map_[VK_RETURN]  = KeyCode::Enter;
    key_map_[VK_SPACE]   = KeyCode::Space;
    key_map_[VK_ESCAPE]  = KeyCode::Esc;
    key_map_[VK_CONTROL] = KeyCode::Ctrl;
    key_map_[VK_SHIFT]   = KeyCode::Shift;
    key_map_[VK_MENU]    = KeyCode::Alt;
    key_map_[VK_TAB]     = KeyCode::Tab;
    key_map_[VK_DELETE]  = KeyCode::Delete;
    key_map_[VK_BACK]    = KeyCode::Back;

    // VK_L* and VK_R*
    key_map_[VK_LCONTROL] = KeyCode::Ctrl;
    key_map_[VK_RCONTROL] = KeyCode::Ctrl;
    key_map_[VK_LSHIFT]   = KeyCode::Shift;
    key_map_[VK_RSHIFT]   = KeyCode::Shift;
    key_map_[VK_LMENU]    = KeyCode::Alt;
    key_map_[VK_RMENU]    = KeyCode::Alt;
    key_map_[VK_LWIN]     = KeyCode::Super;
    key_map_[VK_RWIN]     = KeyCode::Super;

    // A - Z
    for (size_t i = 0, size = ('Z' - 'A'); i <= size; ++i)
        key_map_['A' + i] = KeyCode(size_t(KeyCode::A) + i);

    // Num 0 - 9
    for (size_t i = 0; i < 10; ++i)
        key_map_['0' + i] = KeyCode(size_t(KeyCode::Num0) + i);

    // Numpad 0 - 9
    for (size_t i = 0; i < 10; ++i)
        key_map_[VK_NUMPAD0 + i] = KeyCode(size_t(KeyCode::Numpad0) + i);

    // F1 - F12
    for (size_t i = 0; i < 12; ++i)
        key_map_[VK_F1 + i] = KeyCode(size_t(KeyCode::F1) + i);
}

WindowWin32Impl::~WindowWin32Impl()
{
    if (handle_)
    {
        ::DestroyWindow(handle_);
        handle_ = nullptr;
    }
}

void WindowWin32Impl::Init(const String& title, uint32_t width, uint32_t height, uint32_t icon, bool resizable,
                           bool fullscreen)
{
    HINSTANCE  hinst   = GetModuleHandle(nullptr);
    WNDCLASSEXA wcex   = { 0 };
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.lpszClassName = "KiwanoAppWnd";
    wcex.style         = CS_HREDRAW | CS_VREDRAW /* | CS_DBLCLKS */;
    wcex.lpfnWndProc   = WindowWin32Impl::StaticWndProc;
    wcex.hIcon         = nullptr;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = sizeof(LONG_PTR);
    wcex.hInstance     = hinst;
    wcex.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName  = nullptr;
    wcex.hCursor       = ::LoadCursor(hinst, IDC_ARROW);

    if (icon)
    {
        wcex.hIcon = (HICON)::LoadImage(hinst, MAKEINTRESOURCE(icon), IMAGE_ICON, 0, 0,
                                        LR_DEFAULTCOLOR | LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
    }

    ::RegisterClassExA(&wcex);

    // Get the nearest monitor to this window
    HMONITOR monitor = ::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);

    // Get the target monitor info
    MONITORINFOEXA monitor_info_ex;
    memset(&monitor_info_ex, 0, sizeof(MONITORINFOEXA));
    monitor_info_ex.cbSize = sizeof(MONITORINFOEXA);
    ::GetMonitorInfoA(monitor, &monitor_info_ex);

    // Save the device name
    device_name_ = monitor_info_ex.szDevice;

    int left = -1, top = -1;

    if (fullscreen)
    {
        top  = monitor_info_ex.rcMonitor.top;
        left = monitor_info_ex.rcMonitor.left;

        if (width > static_cast<uint32_t>(monitor_info_ex.rcWork.right - left))
            width = static_cast<uint32_t>(monitor_info_ex.rcWork.right - left);

        if (height > static_cast<uint32_t>(monitor_info_ex.rcWork.bottom - top))
            height = static_cast<uint32_t>(monitor_info_ex.rcWork.bottom - top);
    }
    else
    {
        uint32_t screenw = monitor_info_ex.rcWork.right - monitor_info_ex.rcWork.left;
        uint32_t screenh = monitor_info_ex.rcWork.bottom - monitor_info_ex.rcWork.top;

        uint32_t win_width, win_height;
        AdjustWindow(width, height, GetStyle(), &win_width, &win_height);

        left   = monitor_info_ex.rcWork.left + (screenw - win_width) / 2;
        top    = monitor_info_ex.rcWork.top + (screenh - win_height) / 2;
        width  = win_width;
        height = win_height;
    }

    handle_ = ::CreateWindowExA(fullscreen ? WS_EX_TOPMOST : 0, "KiwanoAppWnd", title.c_str(), GetStyle(),
                                left, top, width, height, nullptr, nullptr, hinst, nullptr);

    if (handle_ == nullptr)
    {
        ::UnregisterClassA("KiwanoAppWnd", hinst);
        KGE_THROW_SYSTEM_ERROR(HRESULT_FROM_WIN32(GetLastError()), "Create window failed");
    }

    width_     = width;
    height_    = height;
    resizable_ = resizable;

    // disable imm
    ::ImmAssociateContext(handle_, nullptr);

    // use Application instance in message loop
    ::SetWindowLongPtrA(handle_, GWLP_USERDATA, LONG_PTR(this));

    ::ShowWindow(handle_, SW_SHOWNORMAL);
    ::UpdateWindow(handle_);

    if (is_fullscreen_)
    {
        SetFullscreenState(true);
    }
}

void WindowWin32Impl::PumpEvents()
{
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
}

void WindowWin32Impl::SetTitle(const String& title)
{
    KGE_ASSERT(handle_);
    ::SetWindowTextA(handle_, title.c_str());
}

void WindowWin32Impl::SetIcon(uint32_t icon_resource)
{
    KGE_ASSERT(handle_);

    HINSTANCE hinstance = ::GetModuleHandle(nullptr);
    HICON     icon      = (HICON)::LoadImage(hinstance, MAKEINTRESOURCE(icon_resource), IMAGE_ICON, 0, 0,
                                    LR_DEFAULTCOLOR | LR_CREATEDIBSECTION | LR_DEFAULTSIZE);

    ::SendMessage(handle_, WM_SETICON, ICON_BIG, (LPARAM)icon);
    ::SendMessage(handle_, WM_SETICON, ICON_SMALL, (LPARAM)icon);
}

void WindowWin32Impl::Resize(uint32_t width, uint32_t height)
{
    KGE_ASSERT(handle_);
    if (!is_fullscreen_)
    {
        RECT rc = { 0, 0, LONG(width), LONG(height) };
        ::AdjustWindowRect(&rc, GetStyle(), false);

        width  = rc.right - rc.left;
        height = rc.bottom - rc.top;

        MONITORINFOEXA info    = GetMoniterInfoEx(handle_);
        uint32_t       screenw = uint32_t(info.rcWork.right - info.rcWork.left);
        uint32_t       screenh = uint32_t(info.rcWork.bottom - info.rcWork.top);
        int            left    = screenw > width ? ((screenw - width) / 2) : 0;
        int            top     = screenh > height ? ((screenh - height) / 2) : 0;

        ::SetWindowPos(handle_, 0, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
    }
    else
    {
        MONITORINFOEXA info = GetMoniterInfoEx(handle_);
        ::SetWindowPos(handle_, HWND_TOPMOST, info.rcMonitor.top, info.rcMonitor.left, width, height, SWP_NOACTIVATE);
    }
}

void WindowWin32Impl::SetFullscreenState(bool fullscreen)
{
    if (is_fullscreen_ != fullscreen)
    {
        is_fullscreen_ = fullscreen;

        // Adjust the rect of client area
        RECT rc = { 0, 0, LONG(width_), LONG(height_) };
        ::AdjustWindowRect(&rc, GetStyle(), false);

        uint32_t width  = uint32_t(rc.right - rc.left);
        uint32_t height = uint32_t(rc.bottom - rc.top);

        if (is_fullscreen_)
        {
            // Reset window style
            ::SetWindowLongPtrA(handle_, GWL_STYLE, GetStyle());

            // Top the window
            MONITORINFOEXA info = GetMoniterInfoEx(handle_);
            ::SetWindowPos(handle_, HWND_TOPMOST, info.rcMonitor.top, info.rcMonitor.left, width, height,
                           SWP_NOACTIVATE);
        }
        else
        {
            MONITORINFOEXA info    = GetMoniterInfoEx(handle_);
            uint32_t       screenw = uint32_t(info.rcWork.right - info.rcWork.left);
            uint32_t       screenh = uint32_t(info.rcWork.bottom - info.rcWork.top);
            int            left    = screenw > width ? ((screenw - width) / 2) : 0;
            int            top     = screenh > height ? ((screenh - height) / 2) : 0;

            // Reset window style
            ::SetWindowLongPtrA(handle_, GWL_STYLE, GetStyle());

            // Unpin the window
            ::SetWindowPos(handle_, HWND_NOTOPMOST, left, top, width, height, SWP_DRAWFRAME | SWP_FRAMECHANGED);
        }

        ::ShowWindow(handle_, SW_SHOWNORMAL);

        // Send event
        WindowFullscreenEventPtr evt = new WindowFullscreenEvent;
        evt->fullscreen              = is_fullscreen_;
        PushEvent(evt);
    }
}

void WindowWin32Impl::SetCursor(CursorType cursor)
{
    mouse_cursor_ = cursor;
}

void WindowWin32Impl::SetActive(bool actived)
{
    if (!handle_)
        return;

    if (is_fullscreen_)
    {
        // Hide window when it is not active
        if (actived)
        {
            MONITORINFOEXA info = GetMoniterInfoEx(handle_);
            ::SetWindowPos(handle_, HWND_TOPMOST, info.rcMonitor.top, info.rcMonitor.left, width_, height_,
                           SWP_NOACTIVATE);
        }
        else
        {
            ::SetWindowPos(handle_, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            ::ShowWindow(handle_, SW_MINIMIZE);
        }
    }
}

DWORD WindowWin32Impl::GetStyle() const
{
    return is_fullscreen_ ? (WINDOW_FULLSCREEN_STYLE) : (resizable_ ? (WINDOW_RESIZABLE_STYLE) : (WINDOW_FIXED_STYLE));
}

void WindowWin32Impl::UpdateCursor()
{
    LPTSTR win32_cursor = IDC_ARROW;
    switch (mouse_cursor_)
    {
    case CursorType::Arrow:
        win32_cursor = IDC_ARROW;
        break;
    case CursorType::TextInput:
        win32_cursor = IDC_IBEAM;
        break;
    case CursorType::SizeAll:
        win32_cursor = IDC_SIZEALL;
        break;
    case CursorType::SizeWE:
        win32_cursor = IDC_SIZEWE;
        break;
    case CursorType::SizeNS:
        win32_cursor = IDC_SIZENS;
        break;
    case CursorType::SizeNESW:
        win32_cursor = IDC_SIZENESW;
        break;
    case CursorType::SizeNWSE:
        win32_cursor = IDC_SIZENWSE;
        break;
    case CursorType::Hand:
        win32_cursor = IDC_HAND;
        break;
    }
    ::SetCursor(::LoadCursor(nullptr, win32_cursor));
}

LRESULT WindowWin32Impl::MessageProc(HWND hwnd, UINT32 msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        KeyCode key = this->key_map_[size_t(wparam)];
        if (key != KeyCode::Unknown)
        {
            KeyDownEventPtr evt = new KeyDownEvent;
            evt->code           = key;
            this->PushEvent(evt);
        }
    }
    break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        KeyCode key = this->key_map_[size_t(wparam)];
        if (key != KeyCode::Unknown)
        {
            KeyUpEventPtr evt = new KeyUpEvent;
            evt->code         = key;
            this->PushEvent(evt);
        }
    }
    break;

    case WM_CHAR:
    {
        KeyCharEventPtr evt = new KeyCharEvent;
        evt->value          = char(wparam);
        this->PushEvent(evt);
    }
    break;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
    {
        MouseDownEventPtr evt = new MouseDownEvent;
        evt->pos              = Point((float)GET_X_LPARAM(lparam), (float)GET_Y_LPARAM(lparam));

        if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK)
        {
            evt->button = MouseButton::Left;
        }
        else if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK)
        {
            evt->button = MouseButton::Right;
        }
        else if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK)
        {
            evt->button = MouseButton::Middle;
        }

        this->PushEvent(evt);
    }
    break;

    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
        MouseUpEventPtr evt = new MouseUpEvent;
        evt->pos            = Point((float)GET_X_LPARAM(lparam), (float)GET_Y_LPARAM(lparam));

        if (msg == WM_LBUTTONUP)
        {
            evt->button = MouseButton::Left;
        }
        else if (msg == WM_RBUTTONUP)
        {
            evt->button = MouseButton::Right;
        }
        else if (msg == WM_MBUTTONUP)
        {
            evt->button = MouseButton::Middle;
        }

        this->PushEvent(evt);
    }
    break;

    case WM_MOUSEMOVE:
    {
        MouseMoveEventPtr evt = new MouseMoveEvent;
        evt->pos              = Point((float)GET_X_LPARAM(lparam), (float)GET_Y_LPARAM(lparam));
        this->PushEvent(evt);
    }
    break;

    case WM_MOUSEWHEEL:
    {
        MouseWheelEventPtr evt = new MouseWheelEvent;
        evt->pos               = Point((float)GET_X_LPARAM(lparam), (float)GET_Y_LPARAM(lparam));
        evt->wheel             = GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA;
        this->PushEvent(evt);
    }
    break;

    case WM_SIZE:
    {
        if (SIZE_MAXHIDE == wparam || SIZE_MINIMIZED == wparam)
        {
            KGE_SYS_LOG("Window minimized");
        }
        else
        {
            // KGE_SYS_LOG("Window resized");

            this->width_  = ((uint32_t)(short)LOWORD(lparam));
            this->height_ = ((uint32_t)(short)HIWORD(lparam));

            WindowResizedEventPtr evt = new WindowResizedEvent;
            evt->width                = this->GetWidth();
            evt->height               = this->GetHeight();
            this->PushEvent(evt);
        }
    }
    break;

    case WM_MOVE:
    {
        WindowMovedEventPtr evt = new WindowMovedEvent;
        evt->x                  = GET_X_LPARAM(lparam);
        evt->y                  = GET_Y_LPARAM(lparam);
        this->PushEvent(evt);
    }
    break;

    case WM_ACTIVATE:
    {
        bool active = (LOWORD(wparam) != WA_INACTIVE);

        this->SetActive(active);

        WindowFocusChangedEventPtr evt = new WindowFocusChangedEvent;
        evt->focus                     = active;
        this->PushEvent(evt);
    }
    break;

    case WM_SETTEXT:
    {
        KGE_SYS_LOG("Window title changed");

        this->title_ = strings::WideToNarrow(reinterpret_cast<LPCWSTR>(lparam));

        WindowTitleChangedEventPtr evt = new WindowTitleChangedEvent;
        evt->title                     = this->title_;
        this->PushEvent(evt);
    }
    break;

    case WM_SETICON:
    {
        KGE_SYS_LOG("Window icon changed");
    }
    break;

    case WM_DISPLAYCHANGE:
    {
        KGE_SYS_LOG("The display resolution has changed");

        ::InvalidateRect(hwnd, nullptr, FALSE);
    }
    break;

    case WM_SETCURSOR:
    {
        this->UpdateCursor();
    }
    break;

    case WM_CLOSE:
    {
        KGE_SYS_LOG("Window is closing");

        WindowClosedEventPtr evt = new WindowClosedEvent;
        this->PushEvent(evt);
        this->SetShouldClose(true);
        return 0;
    }
    break;

    case WM_DESTROY:
    {
        KGE_SYS_LOG("Window was destroyed");

        ::PostQuitMessage(0);
        return 0;
    }
    break;
    }

    return ::DefWindowProcA(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK WindowWin32Impl::StaticWndProc(HWND hwnd, UINT32 msg, WPARAM wparam, LPARAM lparam)
{
    LONG_PTR ptr = static_cast<LONG_PTR>(::GetWindowLongPtrA(hwnd, GWLP_USERDATA));
    if (ptr)
    {
        WindowWin32Impl* window = reinterpret_cast<WindowWin32Impl*>(ptr);
        return window->MessageProc(hwnd, msg, wparam, lparam);
    }
    return ::DefWindowProcA(hwnd, msg, wparam, lparam);
}

}  // namespace kiwano

#endif
