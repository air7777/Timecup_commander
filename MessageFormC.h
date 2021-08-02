//---------------------------------------------------------------------------

#ifndef MessageFormCH
#define MessageFormCH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TMessageForm : public TForm
{
__published:	// IDE-managed Components
	TLabel *Label1;
	TButton *Button1;
	TButton *Button2;
	void __fastcall FormActivate(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TMessageForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMessageForm *MessageForm;
//---------------------------------------------------------------------------
#endif
