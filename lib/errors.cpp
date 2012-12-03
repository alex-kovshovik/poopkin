#include <stdio.h>
#include "commands.h"

const SErrorDescriptor g_errors[] = 
   { ERR_WRONG_LOGIN_OF_PWD,  "Ошибочный логин или пароль",
     ERR_SERVER_IS_DOWN,      "Сервер мертв",
     ERR_WRONG_LOGIN_DATA,    "Ошибочные данные для авторизации",
     ERR_NOT_AUTHORIZED,      "Команда доступна только после авторизации",
     ERR_FILE_NOT_FOUND,      "Файл не найден",
     ERR_WRITING_FILE,        "Ошибка записи в файл",
     ERR_BLOCK_SIZE_TOOBIG,   "Размер блока слишком большой",
     ERR_BLOCK_SIZE,          "Ошибочный размер полученных данных",
     ERR_RECEIVED_FILE_SIZE,  "Неправильный размер полученного файла",
     ERR_UNKNOWN_COMMAND,     "Неизвестная команда",
     ERR_ZERO_PATH_LEN,       "Нулевая длина пути",
     ERR_SEARCHING_FILES,     "Ошибка при поиске файлов",
     ERR_OUT_OF_MEMORY,       "Не достаточно памяти для выполнения операции",
     ERR_STRING_IS_TOO_LONG,  "Слишком длинная строка сообщения",
     ERR_GET_PROCESS_LIST,    "Ошибка получения списка процессов",
     ERR_MAKING_SCREENSHOT,   "Ошибка получения изображения на экране",
     ERR_STARTING_PROCESS,    "Ошибка создания процесса",
     ERR_WINAMP_NOT_FOUND,    "Не найдено окно Winamp'a",
     ERR_WRONG_SCR_FORMAT,    "Запрошен ошибочный формат изображения",
     ERR_WRONG_JPEG_QUALITY,  "Запрошено ошибочное качество сжатия JPEG (должно быть от 0 до 100)",
     ERR_CONVERTING_TO_JPEG,  "Ошибка конвертирования изображения в JPEG",
     ERR_OPENING_FILE,        "Ошибка открытия файла",
     ERR_CREATING_PROCESS,    "Ошибка создания процесса",
     ERR_FPATH_PARAM_NULL,    "Нулевой параметр fpath в функции отсылки файла на клиента",
     ERR_GETDISTFREESPACE,    "Ошибка выполнения функции GetDiskFreeSpace",
     ERR_DELETING_FILE,       "Ошибка удаления файла",
     0, 0
   };

const char *err_getting_data = "Ошибка получения данных";

// Получение текста ошибки по ее коду
const char *GetErrorText(int code)
{
   for (int i=0; g_errors[i].code != 0; i++)
      if (g_errors[i].code == code) return g_errors[i].text;

   static char buf[128];
   sprintf(buf, "Неизвестная ошибка номер %i", code);

   return buf;
}
