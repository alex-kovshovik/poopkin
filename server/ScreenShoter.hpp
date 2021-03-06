#pragma once

#include "jpeg.h"

// �������� ������, ����������� �������� ����������
class CScreenShoter
{
   HBITMAP hBitmap;      // ���������� ������������ BITMAP'a
   HBITMAP hOldBitmap;   // BITMAP, ������� ��� ������ ������ � ��������
   HDC     hdcBitmap;    // �������� ������������ BITMAP'a
   HDC     hdcDesktop;   // �������� ������
   int     cx;           // ������ ��������
   int     cy;           // ������ ��������
   int     bpp;          // ���������� ����� �� �������
   DWORD   n_pixels;     // ���������� �������� � ��������
   bool    b_conv_24bpp; // ������� ����, ��� ����� �������� ����� ���������� � 24 ����

   // �������, ������� ����� �������� ���� ���
   LPBITMAPFILEHEADER bf; // ��������� �� ������ ���� ��������
   LPBITMAPINFO bi;       // � ���� ��������� �������� ��� �������� � ���������� �� ��� (� �. �. � �������) (��������� �� ������ ���� ������)

   DWORD bitmap_len;      // ����� ������ ��� ��������

   // ��������������� � Jpeg
   int     jpeg_len;      // ����� ������ � ��������� Jpeg
   Jpeg    jpeg;          // ��������� BMP->JPG
   int     line_size;     // ����� ������ � ������
   LPBYTE  lpLineBuf;     // �����, ����������������� ��� �������� ����� ������ ��������
   LPBYTE  lpJpegBuf;     // ����� ��� �������� Jpeg

   int AllocateResources();  // ��������� ����������� ��������

public:
   // ������������ � ����������
   CScreenShoter();          // ����������� �� ���������
   ~CScreenShoter();         // ����������

   void Reset();              // ����� ����� � ������������ ������
   int MakeScreenshot(bool b_conv_24bpp_in = false); // �������� ���������
   int MakeJpeg(int quality=65); // ������� �������� Jpeg �� �������� Bitmap'a (� �������� ��������� �� 0 �� 100)

   // ��������� ����������������� ������
   LPBYTE GetBitmapBuffer() { return (LPBYTE)bf; }
   LPBYTE GetJpegBuffer()   { return lpJpegBuf;  }

   DWORD  GetBitmapLength() { return bitmap_len; }
   int    GetJpegLength()   { return jpeg_len;   }
};
