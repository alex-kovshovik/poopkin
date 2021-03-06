#ifndef __PQUEST_HPP__
#define __PQUEST_HPP__

// ���������� �������� ����� ������
#include "pdate.hpp"
#include "pdatetime.hpp"
#include "pint.hpp"
#include "pstring.hpp"
#include "pdouble.hpp"

// ����� ������������ ������������ ���� ��� ���������� ����������
namespace PSQL_API
{
   // �����, �������������� ����� ���������� � ��
   class PEnv
   {
      SQLHENV   henv;       // ���������� ����������
      SQLHDBC   hdbc;       // ���������� ��������� ������

      void Disconnect();    // ����������� �� ��

      int  err_code;        // ��� ��������� ������ (���������� ����� ������ - � errors.txt ����)
      String err_msg;       // ��������� ��������� �� ������
   public:
   
      // ������������ � ����������
      PEnv();
      ~PEnv() {Disconnect();}

      // ������� ��� ������ � Quest
      SQLHDBC GetConnectionHandle() {return hdbc;}  // ���������� ���������� ��������� ������

      // ������� ��� ������ �������������
      int Connect(const char *DSN, const char *user_name="admin", const char *pwd="");          // ������������ � �� (����� DSN)
      int GetErrorCode() {return err_code;}
      const char *GetErrorMessage() {return err_msg;}

      // ������� ��������� ������
      void FormatErrorMessage(SQLSMALLINT HandleType, SQLHANDLE Handle, SQLRETURN retcode);    // ��������� ������ � ��������

      // ������� �������� ���������� ������
      int CreateAccessDSN(const char *file_path, const char *DSN);
   };

   // ����� ��� ��������� ��������� "�������"
   class Quest;
   class PGTStream
   {
      Quest *m_quest;
      int     err_count;   // ���������� ������ ��� ���������� �����
      String  errors;      // ������ � �������� ������ ������
   public:

      // �����������
      PGTStream(Quest *q) { m_quest = q; }

      // ����� ������ (���� �������� ������ ������)
      void Reset()
      {
         err_count = 0;
         errors = "";  // ������ ������� ������, ������ �� �����������
      }

      // ��������� ���������� ������ � ������ �� ������� ������
      int GetErrorCount() {return err_count;}
      const char *GetErrors() {return errors;}

      // ������������� �������� "�������"
      PGTStream &operator,(PGenType &gt_in);
   };

   // ��� �������� �������� � �� � ��������� �����������
   class Quest
   {
      SQLHSTMT m_hstmt;          // ���������� ������� � ��
      PEnv    *m_env_p;          // ��������� �� ����������
      int      err_code;         // ��� ��������� ������
      String   err_msg;          // ������ � ���������� �� ������
      String   sql;              // ������, ��� �������� ��������� ������

      // ��������� ������ Quest
      int b_was_sqlexecute;  // ������� ����, ��� ��� ������ SQLExecute � ��� ����� ���������� ������
      int n_col;             // ����� ������� ��������� �������

      PGTStream gt_stream;   // ����� ����������� �������
      
      void Reset();          // ����� ����������� �������

      // ��������� ������
      void FormatErrorMessage(SQLRETURN retcode);  // ������������ ������ � ��������
   public:

      // ������������ � ����������
      Quest(PEnv &env_in);
      ~Quest() {}  // � ����������� ���� ������ �� ������

      // ������� ��� ������ � ���������
      int Statement(const char *stmt_in);
      int ExecSQL();  // ��������� ������
      
      // ��������� ����������� �������
      int Fetch();    // �������� ���� ������ �������
      int BindField(PGenType &gt);   // �������� GenType'� � ���������� �������
      PGTStream &BindFields(void);   // ���������� ����� ���������� GenType'��

      // ��������� ������ ���������� ���������� � ����������� �������
      int GetBindErrorCount() {return gt_stream.GetErrorCount();}
      const char *GetBindErrors() {return gt_stream.GetErrors();}

      // ������ ��������� �������
      int GetErrorCode() {return err_code;}
      const char *GetErrorMessage() {return err_msg;}
   };
}
#endif
