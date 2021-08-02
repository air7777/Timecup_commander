//---------------------------------------------------------------------------

#ifndef ProcessFormCH
#define ProcessFormCH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TProcessForm : public TForm
{
__published:	// IDE-managed Components
	TLabel *Label1;
	void __fastcall FormCreate(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TProcessForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TProcessForm *ProcessForm;
//---------------------------------------------------------------------------
#endif
