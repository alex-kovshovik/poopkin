// Реализация класса CPushkinClient
#include <iostream>
#include "client.h"
#include "commands.h"
#include "common.h"
#include "strlist.h"
#include "pint.hpp"

#pragma warning (disable: 4996)

using namespace PUSHKIN_CLIENT;
using namespace std;

// Получить фотографию экрана
int CPushkinClient::GetScreenShot(const char *file_name)
{
   if (!file_name) return P_ERROR;

   // Посылаем команду
   if (SendCommand(SC_GET_SCREENSHOT) != P_YES) return P_ERROR;

   // Посылаем формат картинки и размер ее
   if (SendUINT(m_socket, 2) != P_YES) return P_ERROR;
   if (SendUINT(m_socket, 80) != P_YES) return P_ERROR;

   // Получаем подтверждение
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      cout << "Error getting screenshot. Error code=" << err_code << endl;
      return P_ERROR;
   }

   // Получаем данные кадра
   CBinaryBuffer bin;

   if (bin.Receive(m_socket) != P_YES) return P_ERROR;

   // Записываем буфер в файл
   const char *buf_p = bin.GetBuffer();

   // Размер файла
   UINT size_in;
   if (bin.GetUINT(buf_p, size_in) != P_YES) return P_ERROR;

   // Пишем все в файл
   FILE *pFile = fopen(file_name, "wb");
   if (pFile)
   {
      fwrite(buf_p, 1, size_in, pFile);
      fclose(pFile);
   }
   else
      return P_ERROR;

   return P_YES;
}
