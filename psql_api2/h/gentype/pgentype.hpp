// Главный предок всех типов данных
#ifndef __PGENTYPE_HPP__
#define __PGENTYPE_HPP__

// Для исключения конфликтов имен будем использовать пространства имен
namespace PSQL_API
{
   typedef enum {
                  PGT_UNKNOWN,   // Ошибочный тип данных
                  PGT_INT,       // Целое
                  PGT_DOUBLE,    // Двойное вещественное
                  PGT_MONEY,     // Деньги
                  PGT_STRING,    // Строка
                  PGT_DATE,
                  PGT_DATETIME   // Дата и время
                } PTypeId;

   // Для двойного исключения конфликтов имен будем писать букву P перед именами важных типов
   class String;
   class PGenType
   {
   protected:
      PTypeId GT_type;    // Тип переменной
   public:
      int GetSQLType();   // Получаем тип данных SQL на основании типа PGenType
      virtual void* GetBufferPtr()=0;       // Получение указателя на буфер переменной
      virtual int GetBufferSize()=0;        // Получение размера буфера
      virtual int Allocate(int new_len) {return 0;} // Только для String
      virtual int GetErrorCode() {return 0;}  // Пока только для даты и даты-времени

      // Прочие полезные функции
      virtual void CopyTo(String &str_in, const char *format = "") = 0;
      virtual void CopyTo(char *str_in, const char *format = "") = 0;
      virtual void AddTo(String &str_in, const char *format = "") = 0;
      virtual void AddTo(char *str_in, const char *format = "") = 0;

      PGenType *next_p;   // Указатель для организации списка из PGenType'ов
   };
}

#endif
