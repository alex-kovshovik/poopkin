#pragma once

UINT AsyncLoadFiles( LPVOID pParam );

extern int b_load_files_ready;  // ���������� ���������� ��� ������������� �������

// ���������, ��������� �� ������� �� ����� ���������� � ����������� �����
struct SLoadFilesInfo
{
   SLoadFilesInfo(PUSHKIN_CLIENT::CPushkinClient &cl_in) : client(cl_in) {}

   std::vector<SFileInfo> list;  // ������ ����������� ������
   String src_path;              // �������� ���� �� �������
   String dest_path;             // ���� ���������� �� �������
   PUSHKIN_CLIENT::CPushkinClient client;        // ��� ���������� ���������� � ������� - ��� ������������ ����������
   HWND       hWnd;              // ���� ��������
   bool       b_fast_copy;       // ������� ����������� ������ - ����������� �������
};

// ���������, �������� ���������� ��� �������� ������ �����
struct SLoadFileInfo
{
   String   src_dir;   // ���������� �� �������
   String   dest_dir;  // ���������� �� �������
   String   fname;     // ��� �����
   __int64  fsize;     // ������ �����

   SLoadFileInfo()
   {
      fsize = 0;
   }

   SLoadFileInfo(const SLoadFileInfo &lfi_in)
   {
      src_dir  = lfi_in.src_dir;
      dest_dir = lfi_in.dest_dir;
      fname    = lfi_in.fname;
      fsize    = lfi_in.fsize;
   }

   SLoadFileInfo &operator=(const SLoadFileInfo lfi_in)
   {
      src_dir  = lfi_in.src_dir;
      dest_dir = lfi_in.dest_dir;
      fname    = lfi_in.fname;
      fsize    = lfi_in.fsize;

      return *this;
   }
};
