// ������� ������ ���� ����� ������
#ifndef __PGENTYPE_HPP__
#define __PGENTYPE_HPP__

// ��� ���������� ���������� ���� ����� ������������ ������������ ����
namespace PSQL_API
{
   typedef enum {
                  PGT_UNKNOWN,   // ��������� ��� ������
                  PGT_INT,       // �����
                  PGT_DOUBLE,    // ������� ������������
                  PGT_MONEY,     // ������
                  PGT_STRING,    // ������
                  PGT_DATE,
                  PGT_DATETIME   // ���� � �����
                } PTypeId;

   // ��� �������� ���������� ���������� ���� ����� ������ ����� P ����� ������� ������ �����
   class String;
   class PGenType
   {
   protected:
      PTypeId GT_type;    // ��� ����������
   public:
      int GetSQLType();   // �������� ��� ������ SQL �� ��������� ���� PGenType
      virtual void* GetBufferPtr()=0;       // ��������� ��������� �� ����� ����������
      virtual int GetBufferSize()=0;        // ��������� ������� ������
      virtual int Allocate(int new_len) {return 0;} // ������ ��� String
      virtual int GetErrorCode() {return 0;}  // ���� ������ ��� ���� � ����-�������

      // ������ �������� �������
      virtual void CopyTo(String &str_in, const char *format = "") = 0;
      virtual void CopyTo(char *str_in, const char *format = "") = 0;
      virtual void AddTo(String &str_in, const char *format = "") = 0;
      virtual void AddTo(char *str_in, const char *format = "") = 0;

      PGenType *next_p;   // ��������� ��� ����������� ������ �� PGenType'��
   };
}

#endif
