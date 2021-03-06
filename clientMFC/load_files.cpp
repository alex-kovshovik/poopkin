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

int b_load_files_ready;  // ���������� ���������� ��� ������������� �������
__int64 g_total_size;    // ���������� ����������, � ������� ����� ��������� ������ ����� ����������� ������
__int64 g_bytes_read;    // ���� ��� ��������� (��� ����������� �������� �����������)
HWND   g_hWndProgress;   // ���������� ���� � ��������� �����������

// ���������� ������� ����������� 1, ���� ����� ���������� �����������
// (���� ��� �� �����������, � ������� ����� - �� ����� ����� ������ ��� ��������, ���� ��� �� ������)
int BlockReadCallback(DWORD bytes)
{
   if (g_total_size == 0) return 1;
   g_bytes_read += bytes;

   double percent = (double)g_bytes_read*1000.0/(double)g_total_size;

   PostMessage(g_hWndProgress, WM_SET_PROGRESS, (int)percent, 0);

   return (int)SendMessage(g_hWndProgress, WM_CONFIRM_CONTINUE, 0, 0);
}

// ��� ���� ��� ������� �����������
int ReceivingFileCallback(const char *fname)
{
   // ����������� ������� ������� ������ ���� ������ ����� ��� ������
   // ������ ��� � 100 ���������� ��� �������
   static DWORD last_time = 0;

   DWORD time = GetTickCount();
   if (time - last_time > 100)
   {
      last_time = time;
      SendMessage(g_hWndProgress, WM_SET_FILENAME, 0, (LPARAM)fname);
      return (int)SendMessage(g_hWndProgress, WM_CONFIRM_CONTINUE, 0, 0);
   }
   
   // ���������� � ������� ���������� �� ����������� ���� ����� ������ ��� � 100 ����������
   return 1;
}

// ����������, ����� ��������� ������ �������� �����
int ErrReceivingFileCallback(const char *msg, const char *fname)
{
   // �������� ������������, ��� � ������� ������ ������ "���������" �� ��������
   CCopyAskDialog dlg;
   dlg.m_filename = fname;
   dlg.m_caption  = msg;
   dlg.m_dlg_kind = CCopyAskDialog::KND_FAST_COPY;
   if (dlg.DoModal() != IDOK) return 0; // ���������� �������� �����������

   if (dlg.m_button == CCopyAskDialog::BN_SKIP) return 1;  // ��� ���������, ���������� ���� ����
   if (dlg.m_button == CCopyAskDialog::BN_SKIPALL) return 2; // ��� ���������, ���������� ��� ����� � ��������

   return 1;  // � ��������� ������� ���� ��� ��������� - ���������� �����������
}

UINT AsyncLoadFilesImpl(LPVOID pParam)
{
   // ����� �������� ��� ����������
   SLoadFilesInfo *lfi_p = (SLoadFilesInfo*)pParam;
   SLoadFilesInfo lfi(lfi_p->client);

   lfi.list        = lfi_p->list;
   lfi.dest_path   = lfi_p->dest_path;
   lfi.src_path    = lfi_p->src_path;
   lfi.hWnd        = lfi_p->hWnd;
   lfi.b_fast_copy = lfi_p->b_fast_copy;

   // �������� ��������� ������ ���������� � ��������
   if (lfi.client.Reconnect() != P_YES)
   {
      MessageBox(lfi.hWnd, "������ �������� ��������������� ���������� � ��������", "Error", MB_OK|MB_ICONSTOP);
      return 0;
   }

   // �������� ��������������
   if (lfi.client.Relogin() != P_YES)
   {
      MessageBox(lfi.hWnd, "������ ��������� ����������� �� �������", "Error", MB_OK|MB_ICONSTOP);
      return 0;
   }

   const char *capt1 = "���������� ������ ���������� ������...";
   const char *capt2 = "�������� ��������� ���������...";
   const char *capt3 = NULL;
   
   if (lfi.b_fast_copy) capt3 = "������� ����������� ������...";
   else capt3 = "����������� ������...";

   //===================================================================================
   // �������������� ������ ���������� ������ (������ ������� �� ��������� ������)
   //===================================================================================
   SendMessage(lfi.hWnd, WM_SET_CAPTION, 0, (LPARAM)capt1);  // ������������� ����� �������� �������

   g_total_size = 0;
   vector<SFileInfo> file_list;

   // ���� �� ���� ���������� ������ � ���������
   for (int i=0; i<(int)lfi.list.size(); i++)
   {
      // ���� ����� ���� �������, �� ����� ������� ������ ��������� ������ ��� ������������ � ������ � ���,
      // ����� ����� ���� ��������� ����� � ������
      if (lfi.list[i].attrs & FILE_ATTRIBUTE_DIRECTORY)
      {
         if (lfi.list[i].name == "." ||lfi.list[i].name == "..") continue;  // ����� ����� ����� ����������

         // ��������� � ������ ������ ��-������
         SFileInfo fi;
         fi = lfi.list[i];
         fi.name = (String)"\\" + fi.name;
         file_list.push_back(fi);

         // ������� ������ ������ � ��������� �� �������
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
         // ����� ���� ��������� ���� � ������ ����������
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

   // ���������� �������������� ������ � ��������� ����
   FILE *pFile = fopen("shit.txt", "wt");
   if (pFile)
   {
      for (int i=0; i<(int)file_list.size(); i++)
         fprintf(pFile, "%s\n", file_list[i].name.c_str());

      // � ����� ���������� �������� ������
      fprintf(pFile, "\n\n������ �����: %0.0f\n", (double)g_total_size);
      fclose(pFile);
   }

   int cur_progress = 0;

   //===================================================================================
   // �������������� ��������� ��������� ��� �����������
   //===================================================================================
   SendMessage(lfi.hWnd, WM_SET_CAPTION, 0, (LPARAM)capt2);  // ������������� ����� �������� �������
   SendMessage(lfi.hWnd, WM_SET_RANGE, 0, file_list.size());

   for (int i=0; i<(int)file_list.size(); i++)
   {
      // ���������� �������
      PostMessage(lfi.hWnd, WM_SET_PROGRESS, i, 0);

      // ����������, ���������� ��� ���
      if (!SendMessage(lfi.hWnd, WM_CONFIRM_CONTINUE, 0, 0))
      {
         SendMessage(lfi.hWnd, WM_CLOSE_PROGRESS, 0, 0);
         return 0;
      }

      // ������� �������, ���� �� �� ����������
      if (file_list[i].attrs & FILE_ATTRIBUTE_DIRECTORY)
         if (!PathFileExists(lfi.dest_path+file_list[i].name))
            CreateDirectory(lfi.dest_path+file_list[i].name, NULL);
   }
   
   //===================================================================================
   // �������� �����
   //===================================================================================
   SendMessage(lfi.hWnd, WM_SET_CAPTION, 0, (LPARAM)capt3);  // ������������� ����� �������� �������
   SendMessage(lfi.hWnd, WM_SET_RANGE, 0, 1000);  // ����� ������������ 1000%-��� ������

   g_bytes_read = 0;       // ���� ��� ���������
   lfi.client.BlockReadCallback = BlockReadCallback;
   if (lfi.b_fast_copy)  // ������ � ������ �������� ����������� ��� ����� ������������
   {
      lfi.client.ReceivingFileCallback = ReceivingFileCallback;
      lfi.client.ErrReceivingFileCallback = ErrReceivingFileCallback;
   }
   g_hWndProgress = lfi.hWnd;

   bool b_skip_err_files = false;  // ������� ����, ��� ���� ���������� ��� ����� � �������� ������
   int n_errors = 0;               // ���������� ������ �����������

   if (lfi.b_fast_copy)
   {
      int ret = lfi.client.GetFilesList(lfi.src_path, file_list, lfi.dest_path);
      if (ret == P_NO) goto Exit;     // ������� ����������� �������
      if (ret != P_YES)
         MessageBox(lfi.hWnd, lfi.client.GetErrorMessage(), "Error", MB_OK|MB_ICONSTOP);
   }
   else  // ����� ����������� ����������� �� ������ �����
      // ��� ���� �������, ���� ����� ������� (������ 5 ���)
   for (int i=0; i<(int)file_list.size(); i++)
   {
      // ����������, ���������� ��� ���
      if (!SendMessage(lfi.hWnd, WM_CONFIRM_CONTINUE, 0, 0))
         break;

      // �������� ����
      String src_path;
      String dest_path;

      src_path = lfi.src_path;
      src_path += file_list[i].name;

      // ���������� ��� ����������� �����
      char buf[128];
      sprintf(buf, "%i", i);
      SendMessage(lfi.hWnd, WM_SET_FILENAME, 0, (LPARAM)src_path.c_str());

      dest_path = lfi.dest_path;
      dest_path += file_list[i].name;

      // ���� ��� ����������, �� �������� �� �������, ���� ��� �� ����������
      if (file_list[i].attrs & FILE_ATTRIBUTE_DIRECTORY)
      {
         // ����� ���� ���������� - �������� �� �������
         if (!PathFileExists(dest_path))
            CreateDirectory(dest_path, NULL);
      }
      else
      {
         // ����� ���� ���� - �������� ���
         // ���� ���� ��� ����������, �� ���������� �������� ���
         int ret;
         if (PathFileExists(dest_path))
         {
            if (MessageBox(lfi.hWnd, "���� ����������, ��������?", "������", MB_YESNO|MB_ICONQUESTION) == IDYES)
               ret = lfi.client.GetFileEx(src_path, dest_path);
            else
               goto Exit; // ��� ������ ���� ������ �������
         }
         else
            ret = lfi.client.GetFile(src_path, dest_path);

         if (ret == P_NO) goto Exit;
         while (ret == P_ERROR)
         {
            // ���������� ��� ����� � �������� ������         
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
               goto Exit;  // ����� ��������� ������� �����������

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

// ��� ������ ���� ����������� ������
void CLoadFilesDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
   CDialog::OnShowWindow(bShow, nStatus);

   // ��� ������������� ������� �� �������� ������� ����������� ������
   // ��������� ����� ������, �. �. ��� ���������� � ������ ����� ���������
   SLoadFilesInfo *lfi = (SLoadFilesInfo*)m_pParam;
   lfi->hWnd = m_hWnd;
   CWinThread *t = AfxBeginThread(AsyncLoadFilesImpl, m_pParam);
}

void CLoadFilesDialog::OnBnClickedCancel()
{
   bWantStop = true;
//   OnCancel();
}

// ����������� �������� ������ � �������
UINT AsyncLoadFiles( LPVOID pParam )
{
   // �������� ���� ��� ���������� ������ � ����������� �������� ����� ����������
   SLoadFilesInfo *lfi_p = (SLoadFilesInfo*)pParam;
   SLoadFilesInfo lfi(lfi_p->client);

   lfi.list        = lfi_p->list;
   lfi.dest_path   = lfi_p->dest_path;
   lfi.src_path    = lfi_p->src_path;
   lfi.b_fast_copy = lfi_p->b_fast_copy;

   b_load_files_ready = 1;  // ����������� �������� ����� ����������

   // ����� ��� ������ �������, �������� ��������� �����
   // ������ ���� �������� ����������� � ��������� ������
   CLoadFilesDialog dlg;
   dlg.m_pParam = &lfi;
   dlg.DoModal();

   return 0;
}
