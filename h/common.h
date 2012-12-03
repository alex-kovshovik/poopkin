#pragma once

#ifndef __PSTRING_HPP__
#include "pstring.hpp"
#endif

#ifndef P_YES
   #define P_YES     0
   #define P_NO      1
   #define P_ERROR  -1
#endif

// Общие функции для передачи/приема данных
int SendReply(SOCKET s, UINT code, UINT err_code=0);  // Отсылка ответа клиенту
int IsConfirmed(SOCKET s, UINT &err_code);            // Получение подтверждения и кода ошибки, если нет подтверждения

int ReceiveUINT(SOCKET s, UINT &val);                 // Чтение беззнакового целого с клиента
int ReceiveINT64(SOCKET s, __int64 &val);             // Чтение 64-битного целого
int ReceiveSTR(SOCKET s, char *buf, int len);         // Чтение строки данных определенного размера
int SendUINT(SOCKET s, UINT val);                     // Отослать целое число
int SendINT64(SOCKET s, __int64 val);                  // Отослать целое 64-битное число
int SendSTR(SOCKET s, const char *buf, int len);      // Отослать строку данных

int SendFPOST(SOCKET s, fpos_t pos);                  // Отослать 8 байт - длину файла
int ReceiveFPOST(SOCKET s, fpos_t &pos);              // Принять 8 байт - длину файла

// Отправка и получение файлов
int SendFile(SOCKET s, const char *fpath, __int64 off);   // Отправить файл
int ReceiveFile(SOCKET s, const char *fpath, int b_update, int (*BlockReadCallback)(DWORD)=NULL);         // Принять файл

// Отправка и получение строк в переменные PSQL_API::String
int ReceiveString(SOCKET s, PSQL_API::String &str_out);
int SendString(SOCKET s, PSQL_API::String &str_in);
