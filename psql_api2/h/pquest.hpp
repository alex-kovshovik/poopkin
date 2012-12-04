#ifndef __PQUEST_HPP__
#define __PQUEST_HPP__

// Подключаем описания типов данных
#include "pdate.hpp"
#include "pdatetime.hpp"
#include "pint.hpp"
#include "pstring.hpp"
#include "pdouble.hpp"

// Будем использовать пространства имен для исключения конфликтов
namespace PSQL_API
{
   // Класс, представляющий собой соединение с БД
   class PEnv
   {
      SQLHENV   henv;       // Дескриптор соединения
      SQLHDBC   hdbc;       // Дескриптор источника данных

      void Disconnect();    // Отключиться от БД

      int  err_code;        // Код последней ошибки (справочник кодов ошибок - в errors.txt пока)
      String err_msg;       // Детальное сообщение об ошибке
   public:
   
      // Конструкторы и деструктор
      PEnv();
      ~PEnv() {Disconnect();}

      // Функции для работы с Quest
      SQLHDBC GetConnectionHandle() {return hdbc;}  // Возвращает дескриптор источника данных

      // Функции для вызова пользователем
      int Connect(const char *DSN, const char *user_name="admin", const char *pwd="");          // Подключиться к БД (через DSN)
      int GetErrorCode() {return err_code;}
      const char *GetErrorMessage() {return err_msg;}

      // Функции обработки ошибок
      void FormatErrorMessage(SQLSMALLINT HandleType, SQLHANDLE Handle, SQLRETURN retcode);    // Формирует строку с ошибками

      // Функции создания источников данных
      int CreateAccessDSN(const char *file_path, const char *DSN);
   };

   // Класс для поддержки оператора "запятая"
   class Quest;
   class PGTStream
   {
      Quest *m_quest;
      int     err_count;   // Количество ошибок при связывании полей
      String  errors;      // Строка с текстами каждой ошибки
   public:

      // Конструктор
      PGTStream(Quest *q) { m_quest = q; }

      // Сброс потока (пока касается только ошибок)
      void Reset()
      {
         err_count = 0;
         errors = "";  // Просто очищаем строку, память не освобождаем
      }

      // Получение количества ошибок и строку со списком ошибок
      int GetErrorCount() {return err_count;}
      const char *GetErrors() {return errors;}

      // Перегруженный оператор "запятая"
      PGTStream &operator,(PGenType &gt_in);
   };

   // Для создания запросов к БД и получения результатов
   class Quest
   {
      SQLHSTMT m_hstmt;          // Дескриптор запроса к БД
      PEnv    *m_env_p;          // Указатель на соединение
      int      err_code;         // Код последней ошибки
      String   err_msg;          // Строка с сообщением об ошибке
      String   sql;              // Строка, где хранится последний запрос

      // Состояние работы Quest
      int b_was_sqlexecute;  // Признак того, что был запуск SQLExecute и что нужно освободить курсор
      int n_col;             // Номер текущей связываем колонки

      PGTStream gt_stream;   // Поток результатов запроса
      
      void Reset();          // Сброс предыдущего запроса

      // Обработка ошибок
      void FormatErrorMessage(SQLRETURN retcode);  // Формирование строки с ошибками
   public:

      // Конструкторы и деструктор
      Quest(PEnv &env_in);
      ~Quest() {}  // В деструкторе пока ничего не делаем

      // Функции для работы с запросами
      int Statement(const char *stmt_in);
      int ExecSQL();  // Выполнить запрос
      
      // Получение результатов запроса
      int Fetch();    // Вытянуть одну строку запроса
      int BindField(PGenType &gt);   // Привязка GenType'а к результату запроса
      PGTStream &BindFields(void);   // Связывание сразу нескольких GenType'ов

      // Обработка ошибок связывания переменных с результатом запроса
      int GetBindErrorCount() {return gt_stream.GetErrorCount();}
      const char *GetBindErrors() {return gt_stream.GetErrors();}

      // Прочие сервисные фукнции
      int GetErrorCode() {return err_code;}
      const char *GetErrorMessage() {return err_msg;}
   };
}
#endif
