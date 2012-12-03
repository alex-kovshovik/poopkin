#pragma once

#include "jpeg.h"

// Описание класса, упрощающего создание скриншотов
class CScreenShoter
{
   HBITMAP hBitmap;      // Десктиптор виртуального BITMAP'a
   HBITMAP hOldBitmap;   // BITMAP, который был раньше выбран в контекст
   HDC     hdcBitmap;    // Контекст виртуального BITMAP'a
   HDC     hdcDesktop;   // Контекст экрана
   int     cx;           // Ширина картинки
   int     cy;           // Высота картинки
   int     bpp;          // Количество битов на пиксель
   DWORD   n_pixels;     // Количество пикселей в картинке
   bool    b_conv_24bpp; // Признак того, что любую картинку нужно конвертить в 24 бита

   // Объекты, которые можно выделять один раз
   LPBITMAPFILEHEADER bf; // Указатель на начало всей картинки
   LPBITMAPINFO bi;       // В этой структуре хранится вся картинка и информация по ней (в т. ч. и палитра) (указатель на начало этих данных)

   DWORD bitmap_len;      // Длина буфера для картинки

   // Конвертирование в Jpeg
   int     jpeg_len;      // Длина данных с картинкой Jpeg
   Jpeg    jpeg;          // Конвертор BMP->JPG
   int     line_size;     // Длина строки в байтах
   LPBYTE  lpLineBuf;     // Буфер, зарезервированный для хранения одной строки картинки
   LPBYTE  lpJpegBuf;     // Буфер для картинки Jpeg

   int AllocateResources();  // Выделение необходимых ресурсов

public:
   // Конструкторы и деструктор
   CScreenShoter();          // Конструктор по умолчанию
   ~CScreenShoter();         // Деструктор

   void Reset();              // Сброс всего и освобождение памяти
   int MakeScreenshot(bool b_conv_24bpp_in = false); // Создание скриншота
   int MakeJpeg(int quality=65); // Создать картинку Jpeg из готового Bitmap'a (с заданным качеством от 0 до 100)

   // Получение сконвертированных данных
   LPBYTE GetBitmapBuffer() { return (LPBYTE)bf; }
   LPBYTE GetJpegBuffer()   { return lpJpegBuf;  }

   DWORD  GetBitmapLength() { return bitmap_len; }
   int    GetJpegLength()   { return jpeg_len;   }
};
