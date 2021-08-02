//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ProcessFormC.h"
#include "TC_com_main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TProcessForm *ProcessForm;
//---------------------------------------------------------------------------
__fastcall TProcessForm::TProcessForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TProcessForm::FormCreate(TObject *Sender)
{
Form1->ConnectButton->Click();    // Пытаемся установить связь

}
//---------------------------------------------------------------------------
