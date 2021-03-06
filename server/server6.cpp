#include <iostream>
#include "server.h"
#include "commands.h"
#include "common.h"
#include "strlist.h"
#include "version.h"
#include "jpeg.h"
#include "ScreenShoter.hpp"

using namespace PUSHKIN_SERVER;
using namespace std;

#pragma warning (disable: 4996)

// ��������� ��������� � ������ ������
int CPushkinServClient::GetScreenShot()
{
   // ����� �������� � ������� ����������� ������ 
   // � �������� ������ ��� Jpeg, ���� ������ Jpeg
   UINT format;
   UINT quality;

   if (ReceiveUINT(m_socket, format) != P_YES) return P_ERROR;

   if (format!=1 && format!=2) return SendReply(m_socket, CC_ERROR, ERR_WRONG_SCR_FORMAT);

   // format=1 - BMP
   // format=2 - JPG

   if (format == 2)
      if (ReceiveUINT(m_socket, quality) != P_YES) return P_ERROR;

   if (quality > 100) return SendReply(m_socket, CC_ERROR, ERR_WRONG_JPEG_QUALITY);

   // ��������� ����������� �������
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   CBinaryBuffer bin;  // ����� ���������� �������� ������������ ������

   // ������ ����� � �������� ������
   CScreenShoter shoter;

   if (format == 1) // BMP
   {
      if (!shoter.MakeScreenshot()) return SendReply(m_socket, CC_ERROR, ERR_MAKING_SCREENSHOT);

      bin.AddUINT(shoter.GetBitmapLength());
      bin.AddSTR((const char*)shoter.GetBitmapBuffer(), shoter.GetBitmapLength());
   }
   else
   if (format == 2) // JPG
   {
      if (!shoter.MakeJpeg(quality)) return SendReply(m_socket, CC_ERROR, ERR_CONVERTING_TO_JPEG);

      bin.AddUINT(shoter.GetJpegLength());
      bin.AddSTR((const char*)shoter.GetJpegBuffer(), shoter.GetJpegLength());
   }

   // �������� ������������� ����������
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // �������� ������
   if (bin.Send(m_socket) != P_YES) return P_ERROR;
   
   return P_YES;
}
 