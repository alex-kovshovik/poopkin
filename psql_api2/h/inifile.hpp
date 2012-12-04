// Модуль для чтения и записи параметров из .ini файла
#ifndef __INIFILE_HPP__
#define __INIFILE_HPP__

#include <stdio.h>
#include "pstring.hpp"
#define YES 1
#define NO 0

class CIniFile
{
   char file_name[128]; // Имя файла параметров
   int GetString(char *&str_in,PSQL_API::String &str_out); // Считывает одну строку из файла и приращивает указатель

public:
   char LastError[500]; // Строка с сообщение о последней ошибке
   // Инициализация и открытие файла
   int Init(const char *file_name_in); // Открывает файл

   // Чтение данных из файла
   int GetString(char *path_in,char *str_out); // Получает значение параметра, записывает его в строку
   int IsSet(char *path_in); // Проверяет, есть ли параметр по данномй пути
   
   // Запись данных в файл
   int WriteString(char *path_in,char *str_in); // Записывает строку-параметр в файл
};

#endif
