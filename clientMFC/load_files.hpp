#pragma once

UINT AsyncLoadFiles( LPVOID pParam );

extern int b_load_files_ready;  // Глобальная переменная для синхронизации потоков

// Структура, указатель на которую мы будем передавать в запускаемый поток
struct SLoadFilesInfo
{
   SLoadFilesInfo(PUSHKIN_CLIENT::CPushkinClient &cl_in) : client(cl_in) {}

   std::vector<SFileInfo> list;  // Список загружаемых файлов
   String src_path;              // Исходный путь на сервере
   String dest_path;             // Путь назначения на клиенте
   PUSHKIN_CLIENT::CPushkinClient client;        // Для сохранения информации о сервере - для асинхронного скачивания
   HWND       hWnd;              // Окно родителя
   bool       b_fast_copy;       // Быстрое копирование файлов - копирование списком
};

// Структура, хранящая информацию для загрузки одного файла
struct SLoadFileInfo
{
   String   src_dir;   // Директорий на сервере
   String   dest_dir;  // Директорий на клиенте
   String   fname;     // Имя файла
   __int64  fsize;     // Размер файла

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
