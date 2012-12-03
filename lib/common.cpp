#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include "commands.h"
#include "common.h"
#include "strlist.h"

using namespace std;

#pragma warning (disable: 4996)

String g_err_msg;  // Буфер для сообщений об ошибках для глобальных функций
int    g_err_code; // Для хранения кода ошибки при выполнении глобальных функций

// Отсылка ответа клиенту
int SendReply(SOCKET s, UINT code, UINT err_code)
{
   // Отсылаем код подтверждения
   if (SendUINT(s, code) != P_YES) return P_ERROR;

   // И код ошибки, если ошибка была
   if (code == CC_ERROR)
      if (SendUINT(s, err_code) != P_YES) return P_ERROR;

   return P_YES;
}

// Получение подтверждения и кода ошибки, если нет подтверждения
int IsConfirmed(SOCKET s, UINT &err_code)
{
   err_code = 0;  // По умолчанию ошибки нет

   UINT code;
   if (ReceiveUINT(s, code) != P_YES) return P_ERROR;

   if (code == CC_CONFIRM) return P_YES;

   // Если сервер вернул код ошибки, то пытаемся его получить тоже
   if (ReceiveUINT(s, code) != P_YES) return P_ERROR;
   err_code = code;
   return P_NO; // Запрос не подтвержден
}

// Чтение беззнакового целого с клиента
int ReceiveUINT(SOCKET s, UINT &val)
{
   char buf[4];  // Будем все получать прямо в строковый буфер
   
   UINT len;
   UINT len_in = sizeof(val);
   UINT total = 0;  // Всего получено байт

   while (len_in>0)
   {
      len = recv(s, buf+total, len_in, 0);
      if (len == SOCKET_ERROR) return P_ERROR;

      total += len;
      len_in -= len;
   }

   // Когда все прочитали, надо запихнуть все в число
   memcpy(&val, buf, sizeof(val));

   return P_YES;
}

// Чтение 64-битного целого
int ReceiveINT64(SOCKET s, __int64 &val)
{
   char buf[8];  // Будем все получать прямо в строковый буфер
   
   UINT len;
   UINT len_in = sizeof(val);
   UINT total = 0;  // Всего получено байт

   while (len_in>0)
   {
      len = recv(s, buf+total, len_in, 0);
      if (len == SOCKET_ERROR) return P_ERROR;

      total += len;
      len_in -= len;
   }

   // Когда все прочитали, надо запихнуть все в число
   memcpy(&val, buf, sizeof(val));

   return P_YES;
}

// Чтение строки данных определенного размера
int ReceiveSTR(SOCKET s, char *buf, int len_in)
{
   if (!buf) return P_ERROR;

   // Будем получать данные пока не получим все len_in байт и ниибет :)
   UINT len;
   UINT total = 0;  // Всего получено байт
   while (len_in>0)
   {
      len = recv(s, buf+total, len_in, 0);
      if (len == SOCKET_ERROR) return P_ERROR;

      total += len;
      len_in -= len;
   }

   return P_YES;
}

// Отослать целое число
int SendUINT(SOCKET s, UINT val)
{
   int len = send(s, (char*)&val, sizeof(val), 0);
   if (len != sizeof(val)) return P_ERROR;

   return P_YES;
}

// Отослать целое 64-битное число
int SendINT64(SOCKET s, __int64 val)
{
   int len = send(s, (char*)&val, sizeof(val), 0);
   if (len != sizeof(val)) return P_ERROR;

   return P_YES;
}


// Отослать строку данных
int SendSTR(SOCKET s, const char *buf, int len_in)
{
   if (!buf) return P_ERROR;
   int len = send(s, buf, len_in, 0);
   if (len != len_in) return P_ERROR;

   return P_YES;
}

// Отослать 8 байт - длину файла
int SendFPOST(SOCKET s, fpos_t pos)
{
   int len = send(s, (char*)&pos, sizeof(pos), 0);
   if (len != sizeof(pos)) return P_ERROR;

   return P_YES;
}

// Принять 8 байт - длину файла
int ReceiveFPOST(SOCKET s, fpos_t &pos)
{
   int len = recv(s, (char*)&pos, sizeof(pos), 0);
   if (len != sizeof(pos)) return P_ERROR;

   return P_YES;
}


////////////////////////////////////////////////////////////////////////////////////
// Прием/передача списков строк
////////////////////////////////////////////////////////////////////////////////////
// Отсылает список строк (файлов)
int SendStrList(SOCKET s, vector<SFileInfo> &list)
{
   // Будем сначала передавать общее количество строк в сеансе, затем
   // длину строки и строку. После отсылки каждой строки будем получать подтверждение
   // для продолжения

   // Новый алгоритм - будем все посылать как файлы.
   // Сначала формировать двоичный файл в памяти или так,
   // затем его отсылать
   CBinaryBuffer bin;

   // Заполняем двоичный буфер данными
   bin.AddUINT((UINT)list.size());

   // Цикл по всем строкам
   for (int i=0; i<(int)list.size(); i++)
   {
      UINT len = (UINT)strlen(list[i].name.c_str());
      
      // Запоминаем атрибуты файла и его размер
      bin.AddUINT(list[i].attrs);
      bin.AddUINT(list[i].size_high);
      bin.AddUINT(list[i].size_low);
      
      // Запоминаем длину строки и саму строку
      bin.AddUINT(len);
      bin.AddSTR(list[i].name.c_str(), len);
   }

//   bin.Dump("from_server.bin");

   // Отсылаем двоичный буфер
   if (bin.Send(s) != P_YES) return P_ERROR;

   return P_YES;
}

// Принимает список строк
int ReceiveStrList(SOCKET s, vector<SFileInfo> &list)
{
   // Очищаем предыдущий список строк
   list.clear();

   UINT count_in;  // Для хранения количества принимаемых строк
   SFileInfo fi;   // Для хранения информации об одном файле
   UINT str_len;   // Для хранения длин строк
   
   // Атрибуты файла
   UINT attrs;
   UINT size_high;
   UINT size_low;

   CBinaryBuffer bin;

   // Принимаем двоичный буфер
   if (bin.Receive(s) != P_YES) return P_ERROR;

//   bin.Dump("from_client.bin");

   // Разбираемся, чего же нам прислали
   const char *buf_p = bin.GetBuffer();

   // Количество блоков данных в буфере
   if (bin.GetUINT(buf_p, count_in) != P_YES) return P_ERROR;

   // Получаем данные по файлам
   UINT allocated = 1024;  // Динамически выделяемые строки
   char *str_p = (char*)malloc(allocated);
   if (!str_p) return P_ERROR;

   for (int i=0; i<(int)count_in; i++)
   {
      // Получаем информацию о файле (атрибуты и размер)
      if (bin.GetUINT(buf_p, attrs) != P_YES) {free(str_p);return P_ERROR;}
      if (bin.GetUINT(buf_p, size_high) != P_YES)  {free(str_p);return P_ERROR;}
      if (bin.GetUINT(buf_p, size_low) != P_YES)  {free(str_p);return P_ERROR;}

      fi.attrs     = attrs;
      fi.size_high = size_high;
      fi.size_low  = size_low;

      // Получаем размер строки
      if (bin.GetUINT(buf_p, str_len) != P_YES) {free(str_p); return P_ERROR;}

      // Если размер передаваемой строки больше, чем размер буфера - довыделяем еще памяти
      if (str_len+1 > allocated)
      {
         str_p = (char*)realloc(str_p, str_len+1);
         allocated = str_len+1;
         if (!str_p) return P_ERROR;
      }
   
      // Получаем саму строку
      if (bin.GetSTR(buf_p, str_p, str_len) != P_YES)  { free(str_p); return P_ERROR; }
      str_p[str_len] = 0;  // Не забываем терминирующий нуль!
      
      // Запоминаем все в массиве
      fi.name = str_p;
      list.push_back(fi);
   }

   free(str_p);

   return P_YES;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// Отправка и получение файлов
////////////////////////////////////////////////////////////////////////////////////////////////

// Максимальный размер буфера для отсылки/приема файлов (мегабайт пока)
#define SF_MAX_BUF_SIZE 1024*1024

// Отправить файл
int SendFile(SOCKET s, const char *fpath, __int64 off)
{
   // Сначала отсылаем подтверждение отсылки файла   
   // затем отсылаем размер файла - только 4 байта, затем сам файл
   if (!fpath)  // Это крайне маловероятная причина проблем
   {
      SendReply(s, CC_ERROR, ERR_FPATH_PARAM_NULL);
      return P_ERROR;  // Это жестко - отключаем клиента
   }

   // Теперь будем открывать файл при помощи API винды
   HANDLE hFile = CreateFile(fpath, FILE_READ_DATA|FILE_READ_ATTRIBUTES,FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

   if (hFile == INVALID_HANDLE_VALUE)
      return SendReply(s, CC_ERROR, ERR_OPENING_FILE);  // !!! - это нормальная ситуация, нельзя за это отключать клиента - !!!

   // Определяем размер файла
   DWORD fSizeHigh = 0;
   DWORD fSizeLow = GetFileSize(hFile, &fSizeHigh);

   // Если нужно переместиться по файлу, то делаем это
   LARGE_INTEGER li;
   li.QuadPart = (LONGLONG)off;

   if (off != 0)
      if (!SetFilePointerEx(hFile, li, NULL, FILE_BEGIN));  // Пока не делаем ничего по этому поводу
   
   // Теперь будем отсылать размер реально отсылаемых данных (пока это не так)
   __int64 fSize = fSizeHigh;
   fSize <<=32;
   fSize |= fSizeLow;

//   fSize = fSize - off + 1;

   char *buf = (char*)malloc(SF_MAX_BUF_SIZE);
   if (!buf)
   {
      CloseHandle(hFile);
      return SendReply(s, CC_ERROR, ERR_OUT_OF_MEMORY);
   }

   // Отсылаем подтверждение того, что мы ща будем слать файл
   if (SendReply(s, CC_CONFIRM) != P_YES) {free(buf); CloseHandle(hFile); return P_ERROR;}

   // Отсылаем размер файла - теперь __int64
   if (SendINT64(s, fSize) != P_YES) {free(buf); CloseHandle(hFile); return P_ERROR;}

   DWORD bytes_read = 0;
   
   // Отсылаем файл блоками
   while (ReadFile(hFile, buf, SF_MAX_BUF_SIZE, &bytes_read, NULL))
   {
      // Отсылаем размер блока
      if (SendUINT(s, (UINT)bytes_read) != P_YES) {free(buf); CloseHandle(hFile); return P_ERROR;}

      // Если мы прочитали нулевой блок, то это конец файла
      if (bytes_read == 0) break;

      // Отсылаем сам блок
      if (SendSTR(s, buf, (UINT)bytes_read) != P_YES) {free(buf); CloseHandle(hFile); return P_ERROR;}
   }

   free(buf);
   CloseHandle(hFile);

   // Завершающий нулевой блок уже отослан

   return P_YES;
}

// Принять файл
int ReceiveFile(SOCKET s, const char *fpath_dest, int b_update, int (*BlockReadCallback)(DWORD))
{
   // b_update - признак того, что файл нужно дописать
   g_err_msg = "";
   g_err_code = 0;

   bool b_fopen_error = false;

   if (!fpath_dest)  // Это тоже очень маловероятно
   {
      g_err_msg = "Нулевой параметр fpath_dest в функции приема файла";
      return P_ERROR;
   }

   __int64 file_size; // Размер файла (посланный сервером)
   UINT block_size;   // Размер блока данных
   UINT total_size=0; // Размер реально полученных данных

   // Получаем подтверждение приема файла
   UINT err_code;
   int ret = IsConfirmed(s, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      g_err_code = err_code;
      g_err_msg = GetErrorText(err_code);
      return P_ERROR;
   }

   // Получаем размер файла
   if (ReceiveINT64(s, file_size) != P_YES)
   {
      g_err_msg = "Ошибка получения размера файла";
      return P_ERROR;
   }

   // Файл будем позже открывать теперь
   const char *mode;
   if (b_update) mode = "r+b";
   else mode = "wb";

   HANDLE hFile = CreateFile(fpath_dest, FILE_READ_DATA|FILE_READ_ATTRIBUTES|FILE_APPEND_DATA,FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
   if (hFile == INVALID_HANDLE_VALUE)
   {
      g_err_msg = "Ошибка открытия файла для записи";
      b_fopen_error = true;

      LPVOID lpMsgBuf;
      if (!FormatMessage( 
         FORMAT_MESSAGE_ALLOCATE_BUFFER | 
         FORMAT_MESSAGE_FROM_SYSTEM | 
         FORMAT_MESSAGE_IGNORE_INSERTS,
         NULL,
         GetLastError(),
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
         (LPTSTR) &lpMsgBuf,
         0,
         NULL ))
      {
         // Handle the error.
         return 0;
      }

      // Display the string.
      MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );

      // Free the buffer.
      LocalFree( lpMsgBuf );
   }
   /*
   FILE *pFile = fopen(fpath_dest, "wb");
   if (!pFile)  // А вот это случиться может в любой момент, поентому надо обработать такую ошибку
   {
      // Т. е., выходить мы тогда не будем, но и файл дальше принимать надо
      g_err_msg = "Ошибка открытия файла для записи";
      b_fopen_error = true;
   }
*/
   // Если файл нужно дописать, то перемещаемся к концу файла
   if (b_update)
   {
//      fseek(pFile, 0, SEEK_END);
      SetFilePointer(hFile, 0, NULL, FILE_END);
   }

   // Файлы с нулевым размером тоже бывают!!!
   if (file_size == 0)
   {
      // Закрываем файл      
      if (hFile) {CloseHandle(hFile);hFile=NULL;}
      
      // Получаем последний нулевой блок данных
      if (ReceiveUINT(s, block_size) != P_YES)
      {
         g_err_msg = "Ошибка получения завершающего блока для файла с нулевым размером";
         return P_ERROR;
      }

      // Проверяем возвращенное значение на правильность
      if (block_size != 0)
      {
         g_err_msg = "Ненулевой завершающий блок пустого файла";
         return P_ERROR;
      }

      // Выходим, файл получен. А если он даже не открыт, то такую ошибку нужно предусмотреть
      if (b_fopen_error) return P_ERROR;  // Ошибка открытия файла с нулевой длинной
      else return P_YES;  // Все нормально - файл имеет нулевую длину
   }

   // Тут мы, конечно, будем всегда выделять буфер максимальной длины, т. к. блок может придти любой
   char *buf = (char*)malloc(SF_MAX_BUF_SIZE);
   if (!buf) {if (hFile) CloseHandle(hFile); return P_ERROR;}  // Не достаточно памяти

   while (1)
   {
      // Получаем размер блока
      if (ReceiveUINT(s, block_size) != P_YES)
      {
         free(buf);
         if (hFile) {CloseHandle(hFile); hFile=NULL;}
         g_err_msg = "Ошибка получения размера блока данных";
         return P_ERROR;
      }

      if (block_size == 0) break;  // Это завершающий пустой блок

      // Получаем сам блок
      if (ReceiveSTR(s, buf, block_size) != P_YES)
      {
         free(buf);
         if (hFile) {CloseHandle(hFile); hFile=NULL;}
         g_err_msg = "Ошибка получения блока данных";
         return P_ERROR;
      }

      // Пишем блок в файл
      DWORD bytes_written = 0;
      if (hFile) WriteFile(hFile, buf, block_size, &bytes_written, NULL);

      // Вызываем функцию для демонстрации процесса копирования
      if (BlockReadCallback) 
         if (!BlockReadCallback(block_size))
         {
            // Т. к. это будет только в асинхронном режиме, то после отмены клиент сразу же отключится,
            // следовательно сервер сам перестанет посылать данные и закроет соединение
            free(buf);
            if (hFile) {CloseHandle(hFile); hFile=NULL;}
            return P_NO;
         }

      total_size += block_size;
   }

   free(buf);
   if (hFile) {CloseHandle(hFile); hFile=NULL;}

   if (total_size != file_size)
   {
      g_err_msg = "Файл получен не полностью";
      return P_ERROR;
   }

   if (b_fopen_error) return P_ERROR;  // Мы ничего не записали в файл, т. к. произошла ошибка

   // Во всех остальных случаях - все в порядке
   return P_YES;
}

// Получение и отсылка строк через PSQL_API::String
int ReceiveString(SOCKET s, String &str_out)
{
   // Сначала получаем длину строки
   UINT len;
   if (ReceiveUINT(s, len) != P_YES) return P_ERROR;

   // Если строка пустая, то ничего больше не принимаем, только обнуляем переданную строку
   if (len == 0)
   {
      str_out = "";
      return P_YES;
   }

   // Выделяем память под хранение всей строки
   char *buf = (char*)malloc(len+1);
   if (!buf) return P_ERROR;  // Не достаточно памяти

   // Получаем саму строку
   if (ReceiveSTR(s, buf, len) != P_YES) return P_ERROR;

   // Ставим туда терминирующий нуль
   buf[len] = 0;

   // Запоминаем строку в переданной переменной
   str_out = buf;

   // Освобождаем память под строку
   free(buf);
   return P_YES;
}

int SendString(SOCKET s, String &str_in)
{
   // Сначала получаем длину строки
   UINT len = (UINT)strlen(str_in);

   // Отсылаем длину передаваемой строки
   if (SendUINT(s, len) != P_YES) return P_ERROR;

   // Если длина нулевая, то не шлем саму строку
   if (len == 0) return P_YES;
   
   // Отсылаем саму строку
   if (SendSTR(s, str_in.c_str(), len) != P_YES) return P_ERROR;

   return P_YES;
}
