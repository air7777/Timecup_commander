//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "MessageFormC.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMessageForm *MessageForm;
extern int Language;
//---------------------------------------------------------------------------
__fastcall TMessageForm::TMessageForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TMessageForm::FormActivate(TObject *Sender)
{
				 if (Language == 1)
				 {
		 MessageForm->Label1->Caption = "������ ��� �������, �� �� �������� � ����������";
		 MessageForm->Button1->Caption = "��������";
		 MessageForm->Button2->Caption = "���������� ��� ����������";
				 }
				 else
				 {
   		 MessageForm->Label1->Caption = "The recipe was changed but not saved to the device";
		 MessageForm->Button1->Caption = "Cancel";
		 MessageForm->Button2->Caption = "Continue without saving";
				 }

}
//---------------------------------------------------------------------------

