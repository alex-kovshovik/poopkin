#include <windows.h>
#include <iostream>
#include "ScreenShoter.hpp"

using namespace std;

////////////////////////////////////////////////////////////////////////
// ������������ � ����������
////////////////////////////////////////////////////////////////////////
CScreenShoter::CScreenShoter()
{
   hBitmap      = NULL;   // �������� ������������ BITAMP'a
   hOldBitmap   = NULL;   // BITMAP, ������� ��� ������ ������ � ��������
   hdcBitmap    = NULL;   // �������� ������������ BITMAP'a
   hdcDesktop   = NULL;   // �������� ������
   cx           = 0;      // ������ ��������
   cy           = 0;      // ������ ��������
   bpp          = 0;      // ���������� ����� �� �������
   n_pixels     = 0;      // ���������� ��������
   b_conv_24bpp = false;  // ������� ����, ��� ����� �������� ����� ���������� � 24 ����

   bf           = NULL;   // ��������� �� ������ ���� ��������
   bi           = NULL;   // � ���� ��������� �������� ��� �������� � ���������� �� ��� (� �. �. � �������)
   lpLineBuf    = NULL;   // �����, ����������������� ��� �������� ����� ������ ��������
   line_size    = 0;
   lpJpegBuf    = NULL;   // ����� ��� �������� Jpeg

   bitmap_len   = 0;      // ����� ������ ��� �������� Bitmap
   jpeg_len     = 0;      // ����� ������ � ��������� Jpeg
}
CScreenShoter::~CScreenShoter()
{
   Reset();
}

////////////////////////////////////////////////////////////////////////
// ������ �������
////////////////////////////////////////////////////////////////////////
void CScreenShoter::Reset()
{
   cx          = 0;      // ������ ��������
   cy          = 0;      // ������ ��������
   bpp         = 0;      // ���������� ����� �� �������
   n_pixels    = 0;      // ���������� ��������
   line_size   = 0;

   // ������������ ���������� � ������������ BITMAP'a
   if (hOldBitmap && hdcBitmap)
   {
      DeleteObject(SelectObject(hdcBitmap,hOldBitmap));    // ������� ��������� BITMAP
      DeleteDC(hdcBitmap);  // ������� ��������� ��������
   }

   // ����������� �������� ������
   if (hdcDesktop)
      ReleaseDC(GetDesktopWindow(),hdcDesktop);

   hBitmap    = NULL;
   hOldBitmap = NULL;
   hdcBitmap  = NULL;
   hdcDesktop = NULL;

   bi         = NULL;  // ��� ��� ������ �� �� ��������, ������ ����������� �������� �� ��� ���������� ������

   if (bf) {free(bf); bf = NULL; }
   if (lpLineBuf) {free(lpLineBuf); lpLineBuf=NULL;}
   if (lpJpegBuf) {free(lpJpegBuf); lpJpegBuf=NULL;}
}

// ��������� �������� ��� �������� �����
int CScreenShoter::AllocateResources()
{
   // ���� ������� ��� ���� �������� � ���������� ������ � ������� ����� �� ����������, �� ������ �������
   if (bi && bf && hBitmap && hOldBitmap && hdcBitmap && hdcDesktop)
   {
      int cx_new  = GetSystemMetrics(SM_CXSCREEN);         // ������ ������ � ��������
      int cy_new  = GetSystemMetrics(SM_CYSCREEN);         // ������ ������ � ��������
      int bpp_new = GetDeviceCaps(hdcDesktop, BITSPIXEL);  // ���������� ����� �� �������

      if (cx_new==cx && cy_new==cy && bpp_new==bpp)
         return 1; // ������ �� ������ - � ��� �������� ��� � ����������� ����������
   }

   // � ��������� ������� �� ������ ��� �����������
   Reset();

   // ����� ��� �������� ������
   hdcDesktop  = GetDC(GetDesktopWindow());            // �������� �������� ����� ������
   hdcBitmap   = CreateCompatibleDC(hdcDesktop);       // ������� ����������� �������� ��� ��������
   cx          = GetSystemMetrics(SM_CXSCREEN);        // ������ ������ � ��������
   cy          = GetSystemMetrics(SM_CYSCREEN);        // ������ ������ � ��������
   bpp         = GetDeviceCaps(hdcDesktop, BITSPIXEL); // ���������� ����� �� �������
   n_pixels    = cx*cy;                                // ���������� �������

   // ������������� ��������������� ��� � ������, ������� ��� ��������������� � JPEG
   if (b_conv_24bpp) bpp = 24;

   // ������� ����������� BITMAP
   hBitmap     = CreateCompatibleBitmap(hdcDesktop, cx, cy);
   
   // "��������" ��������� BITMAP � ����������� ��������
   hOldBitmap  = (HBITMAP)SelectObject(hdcBitmap, hBitmap);

   // �������� ������ ��� �������� ���� �������� � ���������� �� ��� (� �. �. ������� - ���� �� �����������)
   bitmap_len  = sizeof(BITMAPFILEHEADER) +
                 sizeof(BITMAPINFOHEADER) +
                 n_pixels*bpp/8;  // ������ ������� �� 8 - ��� ������� ��������
                 // ����� �������� ��� ������ ��� �������

   bf = (LPBITMAPFILEHEADER)malloc(bitmap_len);
   bi = LPBITMAPINFO((LPBYTE)bf+sizeof(BITMAPFILEHEADER));

   BITMAPINFOHEADER &bih = bi->bmiHeader;

   // ����� ��������� ��������� ���������� � �������� ����������� ����������
   bih.biSize          = sizeof(BITMAPINFOHEADER);
   bih.biWidth         = cx;
   bih.biHeight        = cy;
   bih.biPlanes        = 1;
   bih.biBitCount      = bpp;
   bih.biCompression   = BI_RGB;
   bih.biSizeImage     = n_pixels*bpp/8;
   bih.biXPelsPerMeter = 0;
   bih.biYPelsPerMeter = 0;
   bih.biClrUsed       = 0;  // �������������� ������ � ������� ������ (RGBQUAD)
   bih.biClrImportant  = 0;  // ��� ����� � ��� �����

   // ����� ��������� ��������� ����� ����������� ����������
   bf->bfReserved1 = 0;
   bf->bfReserved2 = 0;
   bf->bfSize      = sizeof(BITMAPFILEHEADER);
   bf->bfType      = ((WORD) ('M' << 8) | 'B');// is always "BM";
   bf->bfOffBits   = sizeof(BITMAPFILEHEADER) + bih.biSize;  // ����� �������� ���� ����� ������ �������

   // ����� ��� �������� ����� ������ ������ �� �������� - ������� ��� ����� ����� ������ ������������ � JPG

   return 1;
}

////////////////////////////////////////////////////////////////////////
// �������� ���������
////////////////////////////////////////////////////////////////////////
int CScreenShoter::MakeScreenshot(bool b_conv_24bpp_in)
{
   // ������� ����, ��� ����� �������� ����� ���������� � 24 ����   (� Reset �� ������������!)
   b_conv_24bpp = b_conv_24bpp_in;

   // ���� ��� ����� �� �������� �������, �������� ��
   if (!AllocateResources()) return 0; // �����-�� ������ ��������� ��������

   // ��� ������� �������� - ������ ������ ������
   // �������� ���������� ������ � ����������� BITMAP (� ������� ������������ ���������)
   BitBlt(hdcBitmap, 0, 0, cx, cy, hdcDesktop, 0, 0, SRCCOPY);

   // �������� ���� ��������
   // !!! ���� ������ 100 ������, �� 19 �� 21 ������ ������ �� ��� �����!!!
   // ��� ��� ���, ��� �� ������ 150 �������� ������ ������ 2 ������� - ����������
   if (!GetDIBits (hdcBitmap, hBitmap, 0, cy,
                   (LPBYTE)bi+sizeof(BITMAPINFOHEADER),  // ����� �������� �������� �� ����� �������!!!
                    bi, DIB_RGB_COLORS)) return 0;  // �� ������� ���� - �������

   return 1;
}

////////////////////////////////////////////////////////////////////////
// �������� ���������
////////////////////////////////////////////////////////////////////////
int CScreenShoter::MakeJpeg(int quality)
{
   if (quality<0 || quality>100) quality = 100;

   if (!bf || !bi)
      if (!MakeScreenshot(true)) return 0;

   if (lpJpegBuf) {free(lpJpegBuf); lpJpegBuf=NULL;}

   if (bpp != 24) return 0;  // �� ������������ ������ �������

   // ������������ ������ � �������� �������
   LPBYTE lpBytes = (LPBYTE)bf + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

   // �������� ������ ��� �������� ����� ������ ��������
   if (line_size < cx*3)
   {
      line_size = cx*3;
      lpLineBuf = (LPBYTE)realloc(lpLineBuf, line_size);
   }

   // ���� �� ������� (�� �������� �����)
   // ������������ �� � �������� �������
   for (int i=0; i<cy/2; i++)
   {
      LPBYTE lpLine1 = lpBytes + i*line_size;  // ��������� �� ������ ������ ������
      LPBYTE lpLine2 = lpBytes + (cy-i-1)*line_size;  // ��������� �� ������ ������ ������

      memcpy(lpLineBuf, lpLine1,   line_size);
      memcpy(lpLine1,   lpLine2,   line_size);
      memcpy(lpLine2,   lpLineBuf, line_size);
   }

   lpJpegBuf = (LPBYTE)jpeg.Compress(lpBytes, cx, cy, 3, jpeg_len, quality);

   if (!lpJpegBuf) return 0;
   if (!jpeg_len) return 0;

   return 1;
}
