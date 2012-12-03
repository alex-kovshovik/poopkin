#include "stdafx.h"
#include "LoadFilesDialog.h"
#include "CopyAskDialog.h"
#include <vector>
#include "client.h"
#include "load_files.hpp"
#include "common.h"
#include "shlwapi.h"

using namespace std;
using namespace PUSHKIN_CLIENT;

int b_load_files_ready;  // Глобальная переменная для синхронизации потоков
__int64 g_total_size;    // Глобальная переменная, в которой будет храниться полный объем скачиваемых файлов
__int64 g_bytes_read;    // Байт уже прочитано (для отображения процесса копирования)
HWND   g_hWndProgress;   // Дескриптор окна с процессом копирования

// Возвращает функции копирование 1, если нужно продолжать копирование
// (пока это не учитывается, а серверу пофиг - он будет слать данные без перерыва, пока все не пошлет)
int BlockReadCallback(DWORD bytes)
{
   if (g_total_size == 0) return 1;
   g_bytes_read += bytes;

   double percent = (double)g_bytes_read*1000.0/(double)g_total_size;

   PostMessage(g_hWndProgress, WM_SET_PROGRESS, (int)percent, 0);

   return (int)SendMessage(g_hWndProgress, WM_CONFIRM_CONTINUE, 0, 0);
}

// Это тоже для функции копирования
int ReceivingFileCallback(const char *fname)
{
   // Воизбежание слишком частого показа имен файлов будем это делать
   // только раз в 100 милисекунд как минимум
   static DWORD last_time = 0;

   DWORD time = GetTickCount();
   if (time - last_time > 100)
   {
      last_time = time;
      SendMessage(g_hWndProgress, WM_SET_FILENAME, 0, (LPARAM)fname);
      return (int)SendMessage(g_hWndProgress, WM_CONFIRM_CONTINUE, 0, 0);
   }
   
   // Спрашивать у диалога разрешение на продолжение тоже будем только раз в 100 милисекунд
   return 1;
}

// Вызывается, когда произошла ошибка загрузки файла
int ErrReceivingFileCallback(const char *msg, const char *fname)
{
   // Говорить пользователю, что в быстром режиме кнопка "Повторить" не доступна
   CCopyAskDialog dlg;
   dlg.m_filename = fname;
   dlg.m_caption  = msg;
   dlg.m_dlg_kind = CCopyAskDialog::KND_FAST_COPY;
   if (dlg.DoModal() != IDOK) return 0; // Прерывание процесса копирования

   if (dlg.m_button == CCopyAskDialog::BN_SKIP) return 1;  // Все нормально, пропустить этот файл
   if (dlg.m_button == CCopyAskDialog::BN_SKIPALL) return 2; // Все нормально, пропустить все файлы с ошибками

   return 1;  // В остальных случаях тоже все нормально - продолжаем копирование
}

UINT AsyncLoadFilesImpl(LPVOID pParam)
{
   // Снова копируем все переданное
   SLoadFilesInfo *lfi_p = (SLoadFilesInfo*)pParam;
   SLoadFilesInfo lfi(lfi_p->client);

   lfi.list        = lfi_p->list;
   lfi.dest_path   = lfi_p->dest_path;
   lfi.src_path    = lfi_p->src_path;
   lfi.hWnd        = lfi_p->hWnd;
   lfi.b_fast_copy = lfi_p->b_fast_copy;

   // Пытаемся выполнить второе соединение с сервером
   if (lfi.client.Reconnect() != P_YES)
   {
      MessageBox(lfi.hWnd, "Ошибка создания дополнительного соединения с сервером", "Error", MB_OK|MB_ICONSTOP);
      return 0;
   }

   // Пытаемся перелогиниться
   if (lfi.client.Relogin() != P_YES)
   {
      MessageBox(lfi.hWnd, "Ошибка повторной авторизации на сервере", "Error", MB_OK|MB_ICONSTOP);
      return 0;
   }

   const char *capt1 = "Подготовка списка копируемых файлов...";
   const char *capt2 = "Создание структуры каталогов...";
   const char *capt3 = NULL;
   
   if (lfi.b_fast_copy) capt3 = "Быстрое копирование файлов...";
   else capt3 = "Копирование файлов...";

   //===================================================================================
   // Подготавливаем список копируемых файлов (заодно считаем их суммарный размер)
   //===================================================================================
   SendMessage(lfi.hWnd, WM_SET_CAPTION, 0, (LPARAM)capt1);  // Устанавливаем новое название диалога

   g_total_size = 0;
   vector<SFileInfo> file_list;

   // Цикл по всем выделенным файлам и каталогам
   for (int i=0; i<(int)lfi.list.size(); i++)
   {
      // Если перед нами каталог, то будем просить сервер составить список его подкаталогов и файлов в них,
      // иначе будем сами добавлять файлы к списку
      if (lfi.list[i].attrs & FILE_ATTRIBUTE_DIRECTORY)
      {
         if (lfi.list[i].name == "." ||lfi.list[i].name == "..") continue;  // Такую хрень будем пропускать

         // Добавляем к общему списку по-любому
         SFileInfo fi;
         fi = lfi.list[i];
         fi.name = (String)"\\" + fi.name;
         file_list.push_back(fi);

         // Создаем дерево файлов и каталогов на сервере
         __int64 total_size_tmp = 0;
         if (lfi.client.GetFilesTree(lfi.src_path+"\\"+lfi.list[i].name,(String)"\\"+lfi.list[i].name+"\\", file_list, total_size_tmp) != P_YES)
         {
            MessageBox(lfi.hWnd, lfi.client.GetErrorMessage(), "Error", MB_OK|MB_ICONSTOP);
            goto Exit;
         }

         g_total_size += total_size_tmp;
      }
      else
      {
         // Иначе сами добавляем файл к списку копируемых
         SFileInfo fi;
         fi = lfi.list[i];
         fi.name = (String)"\\" + fi.name;
         file_list.push_back(fi);

         __int64 temp64;
         temp64 = fi.size_high;
         temp64 <<= 32;
         temp64 |= fi.size_low;

         g_total_size += temp64;
      }
   }

   // Записываем сформированный список в текстовый файл
   FILE *pFile = fopen("shit.txt", "wt");
   if (pFile)
   {
      for (int i=0; i<(int)file_list.size(); i++)
         fprintf(pFile, "%s\n", file_list[i].name.c_str());

      // В конце дописываем итоговый размер
      fprintf(pFile, "\n\nПОЛНЫЙ ОБЪЕМ: %0.0f\n", (double)g_total_size);
      fclose(pFile);
   }

   int cur_progress = 0;

   //===================================================================================
   // Подготавливаем структуру каталогов для копирования
   //===================================================================================
   SendMessage(lfi.hWnd, WM_SET_CAPTION, 0, (LPARAM)capt2);  // Устанавливаем новое название диалога
   SendMessage(lfi.hWnd, WM_SET_RANGE, 0, file_list.size());

   for (int i=0; i<(int)file_list.size(); i++)
   {
      // Показываем процесс
      PostMessage(lfi.hWnd, WM_SET_PROGRESS, i, 0);

      // Спрашиваем, продолжать или нет
      if (!SendMessage(lfi.hWnd, WM_CONFIRM_CONTINUE, 0, 0))
      {
         SendMessage(lfi.hWnd, WM_CLOSE_PROGRESS, 0, 0);
         return 0;
      }

      // Создаем каталог, если он не существует
      if (file_list[i].attrs & FILE_ATTRIBUTE_DIRECTORY)
         if (!PathFileExists(lfi.dest_path+file_list[i].name))
            CreateDirectory(lfi.dest_path+file_list[i].name, NULL);
   }
   
   //===================================================================================
   // Копируем файлы
   //===================================================================================
   SendMessage(lfi.hWnd, WM_SET_CAPTION, 0, (LPARAM)capt3);  // Устанавливаем новое название диалога
   SendMessage(lfi.hWnd, WM_SET_RANGE, 0, 1000);  // Будем использовать 1000%-ный график

   g_bytes_read = 0;       // Байт уже прочитано
   lfi.client.BlockReadCallback = BlockReadCallback;
   if (lfi.b_fast_copy)  // Только в режиме быстрого копирования это может понадобиться
   {
      lfi.client.ReceivingFileCallback = ReceivingFileCallback;
      lfi.client.ErrReceivingFileCallback = ErrReceivingFileCallback;
   }
   g_hWndProgress = lfi.hWnd;

   bool b_skip_err_files = false;  // Признак того, что надо пропускать все файлы с ошибками чтения
   int n_errors = 0;               // Количество ошибок копирования

   if (lfi.b_fast_copy)
   {
      int ret = lfi.client.GetFilesList(lfi.src_path, file_list, lfi.dest_path);
      if (ret == P_NO) goto Exit;     // Процесс копирования прерван
      if (ret != P_YES)
         MessageBox(lfi.hWnd, lfi.client.GetErrorMessage(), "Error", MB_OK|MB_ICONSTOP);
   }
   else  // Иначе стандартное копирование по одному файлу
      // Оно тоже быстрое, если файлы большие (больше 5 мег)
   for (int i=0; i<(int)file_list.size(); i++)
   {
      // Спрашиваем, продолжать или нет
      if (!SendMessage(lfi.hWnd, WM_CONFIRM_CONTINUE, 0, 0))
         break;

      // Копируем файл
      String src_path;
      String dest_path;

      src_path = lfi.src_path;
      src_path += file_list[i].name;

      // Показываем имя копируемого файла
      char buf[128];
      sprintf(buf, "%i", i);
      SendMessage(lfi.hWnd, WM_SET_FILENAME, 0, (LPARAM)src_path.c_str());

      dest_path = lfi.dest_path;
      dest_path += file_list[i].name;

      // Если это директория, то пытаемся ее создать, если она не существует
      if (file_list[i].attrs & FILE_ATTRIBUTE_DIRECTORY)
      {
         // Перед нами директория - пытаемся ее создать
         if (!PathFileExists(dest_path))
            CreateDirectory(dest_path, NULL);
      }
      else
      {
         // Перед нами файл - копируем его
         // Если файл уже существует, то предлагаем дописать его
         int ret;
         if (PathFileExists(dest_path))
         {
            if (MessageBox(lfi.hWnd, "Файл существует, дописать?", "Вопрос", MB_YESNO|MB_ICONQUESTION) == IDYES)
               ret = lfi.client.GetFileEx(src_path, dest_path);
            else
               goto Exit; // При отказе пока просто выходим
         }
         else
            ret = lfi.client.GetFile(src_path, dest_path);

         if (ret == P_NO) goto Exit;
         while (ret == P_ERROR)
         {
            // Пропускаем все файлы с ошибками чтения         
            if (b_skip_err_files)
            {
               BlockReadCallback(file_list[i].size_low);
               n_errors++;
               break;
            }

            CCopyAskDialog dlg;
            dlg.m_filename = src_path;
            dlg.m_caption  = lfi.client.GetErrorMessage();
            if (dlg.DoModal() != IDOK)
               goto Exit;  // Тогда прерываем процесс копирования

            if (dlg.m_button == CCopyAskDialog::BN_SKIP)
            {
               BlockReadCallback(file_list[i].size_low);
               n_errors++;
               break;
            }
            else
            if (dlg.m_button == CCopyAskDialog::BN_SKIPALL)
            {
               BlockReadCallback(file_list[i].size_low);
               n_errors++;
               b_skip_err_files = true;
               break;
            }

            ret = lfi.client.GetFile(src_path, dest_path);
            if (ret == P_NO) goto Exit;
         }

         SendMessage(lfi.hWnd, WM_SET_ERR_COUNT, 0, n_errors);
      }
   }
Exit:
   SendMessage(lfi.hWnd, WM_CLOSE_PROGRESS, 0, 0);
   return 0;
}

BOOL CLoadFilesDialog::OnInitDialog()
{
   CDialog::OnInitDialog();

   m_copy_progress.SetRange32(0,100);

   m_filename_edit.bkColor(RGB(220,220,220));
   m_filename_edit.textColor(RGB(50,0,0));

   return TRUE;  // return TRUE unless you set the focus to a control
}

// При показе окна копирования файлов
void CLoadFilesDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
   CDialog::OnShowWindow(bShow, nStatus);

   // При инициализации диалога мы начинаем процесс копирования файлов
   // Запускаем поток отсюда, т. к. это безопаснее в смысле сбоев программы
   SLoadFilesInfo *lfi = (SLoadFilesInfo*)m_pParam;
   lfi->hWnd = m_hWnd;
   CWinThread *t = AfxBeginThread(AsyncLoadFilesImpl, m_pParam);
}

void CLoadFilesDialog::OnBnClickedCancel()
{
   bWantStop = true;
//   OnCancel();
}

// Асинхронная загрузка файлов с сервера
UINT AsyncLoadFiles( LPVOID pParam )
{
   // Копируем себе все переданные данные и освобождаем основной поток приложения
   SLoadFilesInfo *lfi_p = (SLoadFilesInfo*)pParam;
   SLoadFilesInfo lfi(lfi_p->client);

   lfi.list        = lfi_p->list;
   lfi.dest_path   = lfi_p->dest_path;
   lfi.src_path    = lfi_p->src_path;
   lfi.b_fast_copy = lfi_p->b_fast_copy;

   b_load_files_ready = 1;  // Освобождаем основной поток приложения

   // Когда все прошло успешно, начинаем загружать файлы
   // Рисуем окно процесса копирования в отдельном потоке
   CLoadFilesDialog dlg;
   dlg.m_pParam = &lfi;
   dlg.DoModal();

   return 0;
}
