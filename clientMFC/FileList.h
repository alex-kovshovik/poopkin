#pragma once

#include "client.h"

// Класс, загружающий список файлов и сортирующий их при помощи итератора
class CFileList
{
   PUSHKIN_CLIENT::CPushkinClient *m_client_p;  // Указатель на клиента
   vector<SFileInfo> m_list;  // Массив со списком файлов

public:
   SFileInfo **fi;             // Массив указателей на элементы списка

   // Конструкторы и деструктор
   CFileList(void);
   ~CFileList(void);

   int Init(PUSHKIN_CLIENT::CPushkinClient *client_p);
   int Load(const char *path);
   int Sort(int sort_mode);  // Сортировка

   int GetCount() {return (int)m_list.size();}
};
