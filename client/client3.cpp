// Реализация класса CPushkinClient
#include <iostream>
#include "client.h"
#include "commands.h"
#include "common.h"

#pragma warning (disable: 4996)

using namespace PUSHKIN_CLIENT;
using namespace std;

extern String g_err_msg;

// Получение файла с сервера
int CPushkinClient::GetFile(const char *fname_src, const char *fname_dest)
{
   m_err_code = 0;
   if (!fname_src)
   {
      m_err_msg = "Нулевой параметр fname_src в функции получение файла с сервера";
      return P_ERROR;
   }

   // Отсылаем команду на получение файла
   if (SendCommand(SC_GET_FILE) != P_YES) return P_ERROR;  // Сообщение об ошибке уже заполнено

   // И сразу отсылаем параметры получения файла
   // (целое число - длину пути к файлу)
   if (SendString(m_socket, (String)fname_src) != P_YES)
   {
      m_err_msg = "Ошибка отсылки пути к файлу для загрузки с сервера";
      return P_ERROR;
   }

   int ret = ReceiveFile(m_socket, fname_dest, 0, BlockReadCallback);
   if (ret != P_YES)
   {
      m_err_msg = g_err_msg;
      return ret;
   }
   return P_YES;
}

// Получение файлов списком
int CPushkinClient::GetFilesList(const char *add_s, vector<SFileInfo> &list_in, const char *dest_path)
{
   // dest_path - путь, куда складывать файлы
   m_err_code = 0;

   // Шлем команду серверу
   if (SendCommand(SC_GET_FILES_LIST) != P_YES) return P_ERROR;  // Сообщение об ошибке уже заполнено

   // Проверяем ответ 
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg = "Ошибка получения списка файлов. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // Шлем параметры получения файлов
   if (SendString(m_socket, (String)add_s) != P_YES)
   {
      m_err_msg = "Ошибка отсылки параметров команды";
      return P_ERROR;
   }

   if (SendStrList(m_socket, list_in) != P_YES)
   {
      m_err_msg = "Ошибка отсылки параметров команды";
      return P_ERROR;
   }

   String fname;       // Имя файла, принятое с сервера
   String fname_res;   // Полное сформированное имя файла

   bool b_skip_err_files = false; // Признак того, что файлы с ошибками можно пропускать

   // Получаем файлы по одному
   while (1)
   {
      // Получаем имя принимаемого файла
      if (ReceiveString(m_socket, fname) != P_YES)
      {
         m_err_msg = "Ошибка получения имени принимаемого файла";
         return P_ERROR;
      }

      // Если имя - пустая строка, то это конец списка принимаемых файлов
      if (fname == "") break;

      fname_res = dest_path;
      fname_res += fname;

      // Передаем управляющей программе информацию о получении файла
      if (ReceivingFileCallback)
         if (!ReceivingFileCallback(fname_res))
            return P_NO;  // Процесс копирования прерван

      // Принимаем сам файл
      ret = ::ReceiveFile(m_socket, fname_res, 0, BlockReadCallback);
      if (ret == P_NO) return P_NO;  // Процесс копирования прерван
      if (ret == P_ERROR)
      {
         m_err_msg = g_err_msg;

         // Ищем в массиве файлов имя ошибочного файла и берем оттуда его размер
         for (int i=0; i<(int)list_in.size();i++)
         {
            if (list_in[i].name == fname)
            {
               if (BlockReadCallback)
                  if (!BlockReadCallback(list_in[i].size_low)) return P_NO;  // Прерывание процесса копирования
               break;
            }
         }

         // Если мы можем пропускать все файлы с ошибками, то просто пропускаем их
         if (b_skip_err_files) continue;

         // Ошибка копирования какого-то файла - ее надо бы показать
         if (ErrReceivingFileCallback)
            ret = ErrReceivingFileCallback(m_err_msg, fname_res);
         else
            ret = 2;  // Если обработчик не задан, то такие файлы будем просто пропускать

         if (ret == 2) b_skip_err_files = true;

         if (ret == 0) return P_NO;  // Прерывание процесса копирования пользователем
      }
   }

   return P_YES;
}
