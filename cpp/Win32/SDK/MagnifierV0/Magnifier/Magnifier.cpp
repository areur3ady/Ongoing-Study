#include "stdafx.h"
#include "MagFocus.h"
#include "Magnifier.h"
#include "MagRegion.h"
#include "Common/Canvas.h"
#include "Common/Utility.h"


#define MAX_FACTOR (5.0)
#define MIN_FACTOR (0.5)


namespace
{
void CalcMagParamImpl(RECT &rcMagWnd, RECT &rcMagArea, POINT &ptFocus, 
    HWND hwnd, const POINT &ptCur, int width, int height, double factor, 
    Mag::MAG_MODE mode);
} // namespace -


namespace Mag
{
Magnifier::Magnifier()
{
    Options::LoadEx(m_Options);

    switch (m_Options.focus)
    {
    case MAGF_CROSS:
        m_Focus.reset(new CrossFocus(m_Options.nFocusSize, m_Options.clrFocus));
        break;
    case MAGF_X:
        m_Focus.reset(new XFocus(m_Options.nFocusSize, m_Options.clrFocus));
        break;
    case MAGF_RECT:
        m_Focus.reset(new RectFocus(m_Options.nFocusSize, m_Options.clrFocus));
        break;
    default: ASSERT(FALSE); break;
    }

    if (m_Options.style != MAGS_TRACKCUR && MAGM_DYNAMIC == m_Options.mode)
    {
        m_Options.mode = MAGM_SNAPSHOT;
    }

    switch (m_Options.mode)
    {
    case MAGM_DYNAMIC: m_Region.reset(new DynamicRegion); break;
    case MAGM_SNAPSHOT: m_Region.reset(new SnapshotRegion); break;
    default: ASSERT(FALSE); break;
    }
}


Magnifier::~Magnifier()
{
}


BOOL Magnifier::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    ASSERT_VALID(hwnd);
    UNREFERENCED_PARAMETER(lpCreateStruct);

    SetWindowLongPtr(hwnd, GWL_STYLE, m_Options.dwStyle);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, m_Options.dwExStyle);

    if (m_Options.bIsClientSize)
    {
        Utility::SetClientSize(hwnd, m_Options.nWidth, m_Options.nHeight);
    }
    else
    {
        SetWindowPos(hwnd, HWND_TOPMOST, 
            0, 0, m_Options.nWidth, m_Options.nHeight, 
            SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
    }

    return TRUE;
}


void Magnifier::OnPaint(HWND hwnd)
{
    ASSERT_VALID(hwnd);

    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    Update(hwnd, hdc);

    EndPaint(hwnd, &ps);
}


UINT Magnifier::OnNCHitTest(HWND hwnd, int x, int y)
{
    ASSERT_VALID(hwnd);
    
    POINT pt = { x, y };
    ScreenToClient(hwnd, &pt);

    RECT rcClient = Utility::SIZEToRECT(m_szClient);

    if (MAGS_STATIC == m_Options.style && PtInRect(&rcClient, pt))
    {
        return HTCAPTION;
    }
    else
    {
        return FORWARD_WM_NCHITTEST(hwnd, x, y, DefWindowProc);
    }
}


void Magnifier::OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    ASSERT_VALID(hwnd);

    m_szClient.cx = cx;
    m_szClient.cy = cy;

    if (cx > 0 && cy > 0)
    {
        HDC hdc = GetDC(hwnd);
        m_Canvas.reset(new Utility::Canvas(hdc, cx, cy));
        ReleaseDC(hwnd, hdc);

        m_Graphics.reset(Gdiplus::Graphics::FromHDC(m_Canvas->GetHDC()));
    }
}


void Magnifier::OnTimer(HWND hwnd, UINT id)
{
    ASSERT_VALID(hwnd);

    OnMagnify(hwnd);
}

void Magnifier::OnMagnify(HWND hwnd)
{
    ASSERT_VALID(hwnd);
    
    HDC hdc = GetDC(hwnd);
    ASSERT_VALID(hdc);

    Update(hwnd, hdc);

    ReleaseDC(hwnd, hdc);
}

void Magnifier::CalcMagParam(RECT &rcMagWnd, RECT &rcMagArea, POINT &ptFocus, HWND hwnd)
{
    ASSERT_VALID(hwnd);

    POINT pt;
    GetCursorPos(&pt);

    CalcMagParamImpl(rcMagWnd, rcMagArea, ptFocus, hwnd, pt, 
        m_szClient.cx, m_szClient.cy, m_Options.dFactor, m_Options.mode);
}

void Magnifier::Update(HWND hwnd, HDC hdc)
{
    ASSERT_VALID(hwnd);
    ASSERT_VALID(hdc);
    ASSERT(m_Canvas != nullptr);
    ASSERT(m_Graphics != nullptr);

    RECT rcMagWnd;
    RECT rcMagArea;
    POINT ptFocus;

    CalcMagParam(rcMagWnd, rcMagArea, ptFocus, hwnd);

    if (MAGS_TRACKCUR == m_Options.style)
    {
        SetWindowPos(hwnd, HWND_TOPMOST, 
            rcMagWnd.left, rcMagWnd.top, rcMagWnd.right - rcMagWnd.left, rcMagWnd.bottom - rcMagWnd.top, 
            SWP_NOACTIVATE | SWP_NOSIZE);
    }

    m_Region->Draw(m_Canvas->GetHDC(), Utility::SIZEToRECT(m_szClient), rcMagArea);
    m_Focus->Draw(*m_Graphics, Utility::POINTToPointF(ptFocus));

    BitBlt(hdc, 0, 0, m_szClient.cx, m_szClient.cy, m_Canvas->GetHDC(), 0, 0, SRCCOPY);
}
} // namespace Mag


namespace
{
void AdjustFactor(double &factor)
{
    if (factor < MIN_FACTOR)
    {
        factor = MIN_FACTOR;
    }
    else if (factor > MAX_FACTOR)
    {
        factor = MAX_FACTOR;
    }
}


void CalcMagAreaRect(RECT &rcMag, POINT &ptFocus, const POINT &ptCur, int width, int height, double factor)
{
    int w = (int)(width / factor);
    int h = (int)(height / factor);

    rcMag.left = ptCur.x - w / 2;
    rcMag.top = ptCur.y - h / 2;

    if (rcMag.left < 0)
    {
        rcMag.left = 0;
    }
    else if (rcMag.left > GetSystemMetrics(SM_CXSCREEN) - w)
    {
        rcMag.left = GetSystemMetrics(SM_CXSCREEN) - w;
    }
    rcMag.right = rcMag.left + w;

    if (rcMag.top < 0)
    {
        rcMag.top = 0;
    }
    else if (rcMag.top > GetSystemMetrics(SM_CYSCREEN) - h)
    {
        rcMag.top = GetSystemMetrics(SM_CYSCREEN) - h;
    }
    rcMag.bottom = rcMag.top + h;

    ptFocus.x = (LONG)((ptCur.x - rcMag.left) * factor);
    ptFocus.y = (LONG)((ptCur.y - rcMag.top) * factor);
}


enum ERelCorner
{
    ERC_LT,     // Left-Top Corner
    ERC_RT,     // Right-Top Corner
    ERC_CENTER, // Center
    ERC_LB,     // Left-Bottom Corner
    ERC_RB,     // Right-Bottom Corner
};


void CalcMagWndRect(HWND hwnd, RECT &rcMag, const POINT &ptCur,
    int cxOffset, int cyOffset, ERelCorner corner, int width, int height)
{
    int cxScreen = GetSystemMetrics(SM_CXSCREEN);
    int cyScreen = GetSystemMetrics(SM_CYSCREEN);

    ASSERT(width <= cxScreen && height <= cyScreen);

    SetRect(&rcMag, 0, 0, width, height);

    POINT ptTopLeftCorner = {};
    POINT ptCenter = { width / 2, height / 2 };
    ClientToScreen(hwnd, &ptTopLeftCorner);
    ClientToScreen(hwnd, &ptCenter);
    int cxHalf = (int)(ptCenter.x - ptTopLeftCorner.x);
    int cyHalf = (int)(ptCenter.y - ptTopLeftCorner.y);

    switch (corner)
    {
    case ERC_LT: break;
    case ERC_RT: cxOffset -= cxHalf * 2; break;
    case ERC_CENTER: cxOffset -= cxHalf; cyOffset -= cyHalf; break;
    case ERC_LB: cyOffset -= cyHalf * 2; break;
    case ERC_RB: cxOffset -= cxHalf * 2; cyOffset -= cyHalf * 2; break;
    default: ASSERT(FALSE); break;
    }

    OffsetRect(&rcMag, (int)ptCur.x + cxOffset, (int)ptCur.y + cyOffset);

    if (corner != ERC_CENTER)
    {
        cxOffset += cxHalf;
        cyOffset += cyHalf;
    }

    if (ptCur.x >= rcMag.left && ptCur.x <= rcMag.right)
    {
        if (rcMag.left < 0 || rcMag.right > cxScreen)
        {
            OffsetRect(&rcMag, rcMag.left < 0 ? -rcMag.left : cxScreen - rcMag.right, 0);
        }
    }
    else
    {
        if (rcMag.left < 0 || rcMag.right > cxScreen)
        {
            OffsetRect(&rcMag, -2 * cxOffset, 0);
        }
    }

    if (ptCur.y >= rcMag.top && ptCur.y <= rcMag.bottom)
    {
        if (rcMag.top < 0 || rcMag.bottom > cyScreen)
        {
            OffsetRect(&rcMag, 0, rcMag.top < 0 ? -rcMag.top : cyScreen - rcMag.bottom);
        }
    }
    else
    {
        if (rcMag.top < 0 || rcMag.bottom > cyScreen)
        {
            OffsetRect(&rcMag, 0, -2 * cyOffset);
        }
    }
}


void CalcMagParamImpl(RECT &rcMagWnd, RECT &rcMagArea, POINT &ptFocus, 
    HWND hwnd, const POINT &ptCur, int width, int height, double factor, 
    Mag::MAG_MODE mode)
{
    AdjustFactor(factor);

    CalcMagAreaRect(rcMagArea, ptFocus, ptCur, width, height, factor);

    if (Mag::MAGM_DYNAMIC == mode)
    {
        int cxOffset = (int)(max(width - ptFocus.x, ptFocus.x) / factor) + 2;
        int cyOffset = (int)(max(height - ptFocus.y, ptFocus.y) / factor) + 2;

        if (cxOffset < 16)
        {
            cxOffset = 16;
        }

        if (cyOffset < 16)
        {
            cyOffset = 16;
        }

        CalcMagWndRect(hwnd, rcMagWnd, ptCur, cxOffset, cyOffset, ERC_LT, width, height);
    }
    else
    {
        CalcMagWndRect(hwnd, rcMagWnd, ptCur, 32, 32, ERC_LT, width, height);
    }
}
} // namespace -