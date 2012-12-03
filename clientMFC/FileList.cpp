#include "StdAfx.h"
#include "filelist.h"
#include "commands.h"
#include "common.h"

using namespace PUSHKIN_CLIENT;

// Конструкторы и деструктор
CFileList::CFileList(void)
{
   m_client_p = NULL;
   fi = NULL;
}

CFileList::~CFileList(void)
{
   if (fi) free(fi);
}

int CFileList::Init(CPushkinClient *client_p)
{
   m_client_p = client_p;
   return P_YES;
}

// Загрузка списка файлов
int CFileList::Load(const char *path)
{
   if (m_client_p->GetFileList(path, m_list) != P_YES)
      return P_ERROR;

   return P_YES;
}

// Сортировка
int CFileList::Sort(int sort_mode)
{
   if (fi) {free(fi); fi=NULL;}

   // Создаем итератор (массив указателей на элементы списка)
   if (m_list.size() == 0) return P_ERROR;
   fi = (SFileInfo**)malloc(sizeof(SFileInfo*)*m_list.size());

   // Заполняем этот массив
   int size = (int)m_list.size();
   for (int i=0; i<(int)size; i++)
      fi[i] = &m_list[i];

   SFileInfo *fi1;
   SFileInfo *fi2;
   SFileInfo *z;

   // Сортируем список (перетасовываем только указатели) - метод пузырька
   for (int k=0; k<size-1; k++)
      for (int i=0; i<size-1; i++)
      {
         fi1 = fi[i];
         fi2 = fi[i+1];

         if ( !(fi1->attrs & FILE_ATTRIBUTE_DIRECTORY) &&
               (fi2->attrs & FILE_ATTRIBUTE_DIRECTORY))
         {
            z       = fi[i];
            fi[i]   = fi[i+1];
            fi[i+1] = z;
         }
         else
         if ( !(fi1->attrs & FILE_ATTRIBUTE_DIRECTORY) &&
              !(fi2->attrs & FILE_ATTRIBUTE_DIRECTORY) &&
              
              ((sort_mode == 0 && strcmpi(fi1->name, fi2->name) > 0) ||
               (sort_mode == 1 && fi1->size_low < fi2->size_low))
            )
         {
            z       = fi[i];
            fi[i]   = fi[i+1];
            fi[i+1] = z;
         }
         else
         if ( (fi1->attrs & FILE_ATTRIBUTE_DIRECTORY) &&
              (fi2->attrs & FILE_ATTRIBUTE_DIRECTORY) &&
              strcmpi(fi1->name, fi2->name)>0 )
         {
            z       = fi[i];
            fi[i]   = fi[i+1];
            fi[i+1] = z;
         }
      }

   return P_YES;
}
