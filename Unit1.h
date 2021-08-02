//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------
class TFrame1 : public TFrame
{
__published:	// IDE-managed Components
	TEdit *Edit1;
private:	// User declarations
public:		// User declarations
	__fastcall TFrame1(TComponent* Owner);
};
//------------------------------------------------------------------------
extern PACKAGE TFrame1 *Frame1;
//---------------------------------------------------------------------
#endif
