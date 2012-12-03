#include <iostream>
#include <stdio.h>

using namespace std;

void main(int argc, char *argv[])
{
   cout << "Make resource v1.0" << endl;

   // 2 параметра: имя файла-источника, имя файла-приемника
   if (argc != 3) return;

   char buf[1024];
   FILE *pSrcFile = fopen(argv[1], "rb");
   if (!pSrcFile)
   {
      printf("Source file not found!\n");
      return;
   }
   
   FILE *pDestFile = fopen(argv[2], "wb");
   if (!pDestFile)
   {
      printf("Can not create dest file!\n");
      fclose(pSrcFile);
      return;
   }

   // Оба файла открыты
   // Определяем размер первого из них
   fpos_t pos;

   fseek(pSrcFile, 0, SEEK_END);
   if (fgetpos(pSrcFile, &pos) != 0)
   {
      printf("Can not determine source file size!\n");
      fclose(pSrcFile);
      fclose(pDestFile);
      return;
   }

   // Пишем данные в файл
   fseek(pSrcFile, 0, SEEK_SET);

   fwrite(&pos, 1, sizeof(pos), pDestFile);
   size_t bytes_read;
   while ( (bytes_read=fread(buf, 1, 1024, pSrcFile)) != 0)
      fwrite(buf, 1, bytes_read, pDestFile);

   fclose(pSrcFile);
   fclose(pDestFile);
}