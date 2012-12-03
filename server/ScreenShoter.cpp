#include <windows.h>
#include <iostream>
#include "ScreenShoter.hpp"

using namespace std;

////////////////////////////////////////////////////////////////////////
// Конструкторы и деструктор
////////////////////////////////////////////////////////////////////////
CScreenShoter::CScreenShoter()
{
   hBitmap      = NULL;   // Контекст виртуального BITAMP'a
   hOldBitmap   = NULL;   // BITMAP, который был раньше выбран в контекст
   hdcBitmap    = NULL;   // Контекст виртуального BITMAP'a
   hdcDesktop   = NULL;   // Контекст экрана
   cx           = 0;      // Ширина картинки
   cy           = 0;      // Высота картинки
   bpp          = 0;      // Количество битов на пиксель
   n_pixels     = 0;      // Количество пикселей
   b_conv_24bpp = false;  // Признак того, что любую картинку нужно конвертить в 24 бита

   bf           = NULL;   // Указатель на начало всей картинки
   bi           = NULL;   // В этой структуре хранится вся картинка и информация по ней (в т. ч. и палитра)
   lpLineBuf    = NULL;   // Буфер, зарезервированный для хранения одной строки картинки
   line_size    = 0;
   lpJpegBuf    = NULL;   // Буфер для картинки Jpeg

   bitmap_len   = 0;      // Длина буфера для картинки Bitmap
   jpeg_len     = 0;      // Длина данных с картинкой Jpeg
}
CScreenShoter::~CScreenShoter()
{
   Reset();
}

////////////////////////////////////////////////////////////////////////
// Прочие функции
////////////////////////////////////////////////////////////////////////
void CScreenShoter::Reset()
{
   cx          = 0;      // Ширина картинки
   cy          = 0;      // Высота картинки
   bpp         = 0;      // Количество битов на пиксель
   n_pixels    = 0;      // Количество пикселей
   line_size   = 0;

   // Освобождение контекстов и виртуального BITMAP'a
   if (hOldBitmap && hdcBitmap)
   {
      DeleteObject(SelectObject(hdcBitmap,hOldBitmap));    // Удаляем созданный BITMAP
      DeleteDC(hdcBitmap);  // Удаляем созданный контекст
   }

   // Освобождаем контекст экрана
   if (hdcDesktop)
      ReleaseDC(GetDesktopWindow(),hdcDesktop);

   hBitmap    = NULL;
   hOldBitmap = NULL;
   hdcBitmap  = NULL;
   hdcDesktop = NULL;

   bi         = NULL;  // Под это память мы не выделяли, только присваивали значение на уже выделенную память

   if (bf) {free(bf); bf = NULL; }
   if (lpLineBuf) {free(lpLineBuf); lpLineBuf=NULL;}
   if (lpJpegBuf) {free(lpJpegBuf); lpJpegBuf=NULL;}
}

// Выделение ресурсов для создания кадра
int CScreenShoter::AllocateResources()
{
   // Если ресурсы уже были выделены и разрешение экрана и глубина цвета не поменялись, то просто выходим
   if (bi && bf && hBitmap && hOldBitmap && hdcBitmap && hdcDesktop)
   {
      int cx_new  = GetSystemMetrics(SM_CXSCREEN);         // Ширина экрана в пикселях
      int cy_new  = GetSystemMetrics(SM_CYSCREEN);         // Высота экрана в пикселях
      int bpp_new = GetDeviceCaps(hdcDesktop, BITSPIXEL);  // Количество битов на пиксель

      if (cx_new==cx && cy_new==cy && bpp_new==bpp)
         return 1; // Ничего не делаем - и так выделено все в достаточном количестве
   }

   // В остальных случаях мы просто все освобождаем
   Reset();

   // Затем все выделяем заново
   hdcDesktop  = GetDC(GetDesktopWindow());            // Получаем контекст всего экрана
   hdcBitmap   = CreateCompatibleDC(hdcDesktop);       // Создаем совместимый контекст для картинки
   cx          = GetSystemMetrics(SM_CXSCREEN);        // Ширина экрана в пикселях
   cy          = GetSystemMetrics(SM_CYSCREEN);        // Высота экрана в пикселях
   bpp         = GetDeviceCaps(hdcDesktop, BITSPIXEL); // Количество битов на пиксель
   n_pixels    = cx*cy;                                // Количество пикслей

   // Автоматически преобразовываем все в формат, удобный для конвертирования в JPEG
   if (b_conv_24bpp) bpp = 24;

   // Создаем совместимый BITMAP
   hBitmap     = CreateCompatibleBitmap(hdcDesktop, cx, cy);
   
   // "Выбираем" созданный BITMAP в совместимый контекст
   hOldBitmap  = (HBITMAP)SelectObject(hdcBitmap, hBitmap);

   // Выделяем память для хранения всей картинки и информации по ней (в т. ч. палитры - пока не реализовано)
   bitmap_len  = sizeof(BITMAPFILEHEADER) +
                 sizeof(BITMAPINFOHEADER) +
                 n_pixels*bpp/8;  // Вместо деления на 8 - так быстрей работает
                 // Потом добавить еще память под палитру

   bf = (LPBITMAPFILEHEADER)malloc(bitmap_len);
   bi = LPBITMAPINFO((LPBYTE)bf+sizeof(BITMAPFILEHEADER));

   BITMAPINFOHEADER &bih = bi->bmiHeader;

   // Сразу заполняем структуру информации о картинке правильными значениями
   bih.biSize          = sizeof(BITMAPINFOHEADER);
   bih.biWidth         = cx;
   bih.biHeight        = cy;
   bih.biPlanes        = 1;
   bih.biBitCount      = bpp;
   bih.biCompression   = BI_RGB;
   bih.biSizeImage     = n_pixels*bpp/8;
   bih.biXPelsPerMeter = 0;
   bih.biYPelsPerMeter = 0;
   bih.biClrUsed       = 0;  // Использованных цветов в таблице цветов (RGBQUAD)
   bih.biClrImportant  = 0;  // Все цвета в ней важны

   // Потом заполняем заголовок файла правильными значениями
   bf->bfReserved1 = 0;
   bf->bfReserved2 = 0;
   bf->bfSize      = sizeof(BITMAPFILEHEADER);
   bf->bfType      = ((WORD) ('M' << 8) | 'B');// is always "BM";
   bf->bfOffBits   = sizeof(BITMAPFILEHEADER) + bih.biSize;  // Потом добавить сюда длину данных палитры

   // Буфер для хранения одной строки данных не выделяем - выделим его прямо перед первой конвертацией в JPG

   return 1;
}

////////////////////////////////////////////////////////////////////////
// Создание скриншота
////////////////////////////////////////////////////////////////////////
int CScreenShoter::MakeScreenshot(bool b_conv_24bpp_in)
{
   // Признак того, что любую картинку нужно конвертить в 24 бита   (в Reset не сбрасывается!)
   b_conv_24bpp = b_conv_24bpp_in;

   // Если для этого не выделены ресурсы, выделяем их
   if (!AllocateResources()) return 0; // Какая-то ошибка выделения ресурсов

   // Все ресурсы выделены - делаем снимок экрана
   // Копируем содержимое экрана в виртуальный BITMAP (с помощью совместимого контекста)
   BitBlt(hdcBitmap, 0, 0, cx, cy, hdcDesktop, 0, 0, SRCCOPY);

   // Получаем саму картинку
   // !!! Если делать 100 кадров, то 19 из 21 секунд уходит на это место!!!
   // Это при том, что на запись 150 мегабайт уходит только 2 секунды - невероятно
   if (!GetDIBits (hdcBitmap, hBitmap, 0, cy,
                   (LPBYTE)bi+sizeof(BITMAPINFOHEADER),  // Потом добавить смещение на длину палитры!!!
                    bi, DIB_RGB_COLORS)) return 0;  // Не следали кадр - хреново

   return 1;
}

////////////////////////////////////////////////////////////////////////
// Создание скриншота
////////////////////////////////////////////////////////////////////////
int CScreenShoter::MakeJpeg(int quality)
{
   if (quality<0 || quality>100) quality = 100;

   if (!bf || !bi)
      if (!MakeScreenshot(true)) return 0;

   if (lpJpegBuf) {free(lpJpegBuf); lpJpegBuf=NULL;}

   if (bpp != 24) return 0;  // Не конвертируем другие форматы

   // Переставляем строки в картинке местами
   LPBYTE lpBytes = (LPBYTE)bf + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

   // Выделяем память под хранение одной строки картинки
   if (line_size < cx*3)
   {
      line_size = cx*3;
      lpLineBuf = (LPBYTE)realloc(lpLineBuf, line_size);
   }

   // Цикл по строкам (до половины строк)
   // Переставляем их в обратном порядке
   for (int i=0; i<cy/2; i++)
   {
      LPBYTE lpLine1 = lpBytes + i*line_size;  // Указатель на начало первой строки
      LPBYTE lpLine2 = lpBytes + (cy-i-1)*line_size;  // Указатель на начало второй строки

      memcpy(lpLineBuf, lpLine1,   line_size);
      memcpy(lpLine1,   lpLine2,   line_size);
      memcpy(lpLine2,   lpLineBuf, line_size);
   }

   lpJpegBuf = (LPBYTE)jpeg.Compress(lpBytes, cx, cy, 3, jpeg_len, quality);

   if (!lpJpegBuf) return 0;
   if (!jpeg_len) return 0;

   return 1;
}
