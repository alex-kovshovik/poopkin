#pragma once

#include <winsock2.h>
#include <vector>
#include "pstring.hpp"
#include "strlist.h"

namespace PUSHKIN_CLIENT
{
   // ����� - ������ (������������ ����������)
   class CPushkinClient
   {
      SOCKET       m_socket;         // ���������� �����
      sockaddr_in  m_server_addr;    // ����� �������
      String       m_err_msg;        // ����� ��� ��������� �� �������
      int          m_err_code;       // ��� ��������� ������
      int          m_server_version; // ������ �������

      // ������ ��� ����������������� � ��������
      char         m_connect_addr[32];
      int          m_connect_port;
      char         m_prev_login[21];
      char         m_prev_pwd[21];

      void InitStructures();

      // ������ � ��������
      int ReceiveGreeting();           // ��������� ����������� �� �������

   public:
      CPushkinClient();                   // ����������� �� ���������
      CPushkinClient(CPushkinClient &cl); // ����������� �����������
      ~CPushkinClient();                  // ����������

      // ��������� ���������� � ��������� ������
      const char *GetErrorMessage() {return m_err_msg.c_str();}
      int         GetErrorCode() {return m_err_code;}
      int         GetVersion() {return m_server_version;}

      int Connect(const char *IP, int port);  // ����������� � �������
      void Disconnect();                      // ���������� �� �������

      // ���������� ��������
      int SendCommand(UINT code);                    // �������� �������
      int Login(const char *login, const char *pwd); // ����������� �� �������
      int ShutdownServer();                          // ���������� ������� �, ��������������, �������������� ����������
      int GetFile(const char *fname, const char *fname_dest); // ��������� ����� � �������
      int GetServerVersion(UINT &ver);               // ��������� ������ �������
      int UpdateServer(const char *new_serv_name);   // ���������� ������� �� ����� ����� ������
      int Reconnect();                               // ����������������� �� ������� ������
      int Relogin();                                 // ��������������� �� ������� ������ � ������� ������
      int GetFileList(const char *path, std::vector<SFileInfo> &list);         // ��������� ������ ������ � �������
      int GetDrivesList(UINT &drives);               // ��������� ������ ���������� ������
      int SendMessage(const char*,const char*,int,int b_async=0);  // ������� ��������� ������� (����� �� ������)
      int GetProcessList(std::vector<SFileInfo> &list); // ��������� ������ ���������
      int OpenCDDoor(UINT b_open);                   // ������� ������ CD-ROM
      int GetScreenShot(const char *file_name);      // �������� ���������� ������
      int SendFile(const char *path, const char *dest_path); // �������� ���� �� ������
      int ServCreateProcess(const char *cmd_line, const char *working_dir); // ��������� ������� �� �������
      int MaximizeWinampVolume();                    // ��������� ���� Winamp'a �� ���������
      int GetFilesTree(const char *path, const char *add_s, std::vector<SFileInfo> &list, __int64 &total_size); // ��������� ������ ������ � ���������
      int GetFilesList(const char *add_s, vector<SFileInfo> &list_in, const char *dest_path);  // ��������� ������ �������
      int ServShellExecute(const char *operation, const char *file, const char *dir);  // ���������� ������� ShellExecute �� �������
      int GetServFreeSpace(const char *path, __int64 &free_space);         // ��������� ���������� ����� �� �������
      int SendFileDirect(const char *src, const char *dest); // ������ ����������� ������ - ����� �������
      int GetFileEx(const char *src, const char *dest);  // ������� �����

      // ����������� ��������� ������
      int (*BlockReadCallback)(DWORD bytes);            // ������� �������. ������������ � GetFile()
      int (*ReceivingFileCallback)(const char *fname);  // ������� �������. ������������ � GetFilesList();
      int (*ErrReceivingFileCallback)(const char *msg, const char *fname);  // ������� �������, ������������ ������ ���������� �����
   };
}
