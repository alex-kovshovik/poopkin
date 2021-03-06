#pragma once

#include "client.h"

// �����, ����������� ������ ������ � ����������� �� ��� ������ ���������
class CFileList
{
   PUSHKIN_CLIENT::CPushkinClient *m_client_p;  // ��������� �� �������
   vector<SFileInfo> m_list;  // ������ �� ������� ������

public:
   SFileInfo **fi;             // ������ ���������� �� �������� ������

   // ������������ � ����������
   CFileList(void);
   ~CFileList(void);

   int Init(PUSHKIN_CLIENT::CPushkinClient *client_p);
   int Load(const char *path);
   int Sort(int sort_mode);  // ����������

   int GetCount() {return (int)m_list.size();}
};
