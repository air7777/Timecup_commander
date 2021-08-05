//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <string.h>
#include "TC_com_main.h"
#include "ProcessFormC.h"
#include "MessageFormC.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "wclBluetooth"
#pragma link "AdvGrid"
#pragma link "AdvObj"
#pragma link "BaseGrid"
#pragma link "CalcEdit"
#pragma link "AdvCalculatorDropdown"
#pragma link "AdvDropDown"
#pragma link "AdvTrackBar"
#pragma link "AdvTrackBarDropDown"
#pragma link "DBAdvTrackBar"

#pragma link "AdvShapeButton"
#pragma link "AdvGlassButton"
#pragma link "PictureList"
#pragma link "AdvUtil"
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
void __fastcall StateMachine(void);
unsigned short __fastcall crc16(volatile unsigned char *sbuf,unsigned short len);
void __fastcall GetCRC(void);
void __fastcall PutCRC(void);
void __fastcall SendCommand(char Command);
void __fastcall SendData(void);
void __fastcall CustomRecipeSettingsToGrid(void);
void __fastcall Create_Tradition_Recipe_Series(void);
void __fastcall Tradition_Recipe_Set_Default_Values(void);
int __fastcall Calculate_Time(int power,int Tbeg,int Tend);
int __fastcall Add_Recipe_Step_Points(TLineSeries *Power_Series,TLineSeries *Temperature_Series, int temperature, int power, int time, char need_label);
int __fastcall Calculate_Temperature(int power,int Tbeg,int time);
int __fastcall ReceiveData(void);
int __fastcall PutCharBLE(char InChar);
char __fastcall GetCharBLE(void);
int __fastcall OutBuffFree(void);
int __fastcall InBuffUsed(void);
void __fastcall FlushInBuffer(void);
void __fastcall FlushOutBuffer(void);
int __fastcall SendBufferBLE(void);
void __fastcall	 TraditionRecipeSettingsToGrid(void);
void __fastcall	 TraditionRecipeGridToSettings(void);
void __fastcall  TraditionRecipeTrackBarToSettings(void);
void __fastcall	 TraditionRecipeSettingsToTrackBar(void);
void __fastcall	  Create_Custom_Recipe_Series(void);
void __fastcall	  CustomRecipeGridToSettings(void);
void __fastcall	  CustomRecipeSettingsToGridRow(int CurrentRow);
int  __fastcall	  Command(void);
void  __fastcall NextCommand(void);
void  __fastcall AddCommand(int command);
void  __fastcall ClearCommandStack(void);
void __fastcall UpdateDeviceControlSettings(char refresh_mode);
void load_default_recipes(void);


TForm1 *Form1;
char InBuffer [BUFFER_LENGTH];
char OutBuffer [BUFFER_LENGTH];
int InBufferIndexBeg, InBufferIndexEnd;
int OutBufferIndex;
bool ReadingCharacteristicInProgress;
bool SettingsToTrackBarInProgress = false;
bool CustomRecipeSettingsToBoxInProgress = false;
bool FoamSensorOn = false;
int CurrentStep, CurrentRecipe;
int TimeAxisMaximum = 5*MIN;
int InitialScale = 5*MIN;
bool IsConnected = false;
int CommandStack[COMMAND_STACK_LENGTH];
int CommandIndex = 0;
int StackBottomIndex = -1;
 TChart *CurrentChart;
command_packet_charint UART_packet;
charint_recipe_settings recipe_settings;
char VM_state,_VM_state,entry,pause_return_state;
unsigned char attempt;
long int Global_timer,Pause_timer;
int waiting_bytes_number;
int  current_temperature,processor_temperature,heater_temperature;
int volume=200;
int time_beg,time_end, temp_beg,temp_beg_global,temp_end;
int first_point;
int BadData;
char liquid;
crc16_charint CRC;
bool TraditionalRecipeChanged;
bool CustomRecipeChanged;
bool NoConnectionProcessing;
bool JustProgramStarted = true;
int Language = 1;
TStringList *OperationList= new TStringList;

__custom_recipe_charint  Recipes;

//---------------------------------------------------------------------------
int __fastcall OutBuffFree(void)
{
	if (OutBufferIndex > 0)
		return(-1);
	else
		return (0);

}
//---------------------------------------------------------------------------
int __fastcall InBuffUsed(void)
{
   return (InBufferIndexEnd - InBufferIndexBeg);
}
//---------------------------------------------------------------------------
void __fastcall FlushInBuffer(void)
{
  InBufferIndexEnd = InBufferIndexBeg = 0;
}
//---------------------------------------------------------------------------
void __fastcall FlushOutBuffer(void)
{
  OutBufferIndex = 0;
}
//---------------------------------------------------------------------------
int __fastcall PutCharBLE(char InChar)
{

 int Res,i;

 if (!IsConnected) return 1;


 Res  =  WCL_E_SUCCESS;

	OutBuffer[OutBufferIndex] = InChar;
	OutBufferIndex++;
	if (OutBufferIndex >= BUFFER_LENGTH)
	{
	   Res  =  SendBufferBLE();
	}

	return (Res);
}
//---------------------------------------------------------------------------
int __fastcall SendBufferBLE(void)
{
 TwclGattCharacteristicValue Val;
 int Res,i,j;
 if (!IsConnected) return 1;

Form1->StateMachineTimer->Enabled = false;


	for (i = 0,j = 0; i < BUFFER_LENGTH && IsConnected; i++,j++)
	{
		Val.Length =  j+1;
		Val[j] = OutBuffer[i];
		if (j >= BLE_PACKET_LENGTH-1 || i >= OutBufferIndex-1)    // Отправляем по BLE_PACKET_LENGTH символов или последнюю часть
		{

		   j = -1;    // перед входом в следующую итерацию станет 0
			Res = Form1->wclGattClient1->WriteCharacteristicValue(Form1->mCharacteristic, Val,plNone);   // Val - array
			Sleep(20);        // Задержка - при скорости 9600 при отправке пропадают символы    было 10, для модуля BT05 пришлось увеличить
			 if (Res != WCL_E_SUCCESS)
			 {
				OutBufferIndex = 0;    // Очистка
				Form1->SetConnectedStatus(DISCONNECTED);
				 if (Language == 1)
				 {
					MessageDlgPos("Нет связи, ошибка: 0x" + IntToHex(Res, 8), mtError,  TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				 else
				 {
					MessageDlgPos("No communication, error: 0x" + IntToHex(Res, 8), mtError,  TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				Form1->StateMachineTimer->Enabled = true;
				return(Res);
			 }

		}
		if (i == OutBufferIndex-1)     // Дошли до конца буфера
		{
			OutBufferIndex = 0;    // Очистка
            break;
		}
	}
	Form1->StateMachineTimer->Enabled = true;
    return (WCL_E_SUCCESS);
}
//---------------------------------------------------------------------------
char __fastcall GetCharBLE(void)
{    char result;

 if (!IsConnected) return 1;

	while (ReadingCharacteristicInProgress);
	result = InBuffer [InBufferIndexBeg];
	InBufferIndexBeg++;
	if (InBufferIndexBeg >= InBufferIndexEnd)
	{
	  InBufferIndexBeg = InBufferIndexEnd = 0;
	}
	 return result;
}
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TwclBluetoothRadio* __fastcall TForm1::GetRadio()
{
  // Look for first available radio
  for (int i = 0; i < wclBluetoothManager1->Count; i++)
	if (wclBluetoothManager1->Radios[i]->Available)
	  // Return first non MS.
	  return wclBluetoothManager1->Radios[i];

//  MessageDlg("No one Bluetooth Radio found.", mtError,
  SetConnectedStatus(DISCONNECTED);
  MessageDlgPos("Не найден адаптер Bluetooth.", mtError,TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 if (Language == 1)
				 {
					MessageDlgPos("Не найден адаптер Bluetooth.", mtError,TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				 else
				 {
					MessageDlgPos("Bluetooth adapter not found.", mtError,TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }

  return NULL;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::wclBluetoothManager1DeviceFound(TObject *Sender, TwclBluetoothRadio * const Radio,
          const __int64 Address)
{
 TwclBluetoothDeviceType DevType = dtMixed;
  int Res = ((TwclBluetoothRadio*)Radio)->GetRemoteDeviceType(Address, DevType);

  TListItem* Item = lvDevices->Items->Add();
  Item->Caption = IntToHex(Address, 12);
  Item->SubItems->Add(""); // We can not read a device's name here.
  Item->Data = (void*)Radio; // To use it later.
  if (Res != WCL_E_SUCCESS)
	Item->SubItems->Add("Error: 0x" + IntToHex(Res, 8));
  else
	switch (DevType)
	{
	  case dtClassic:
		Item->SubItems->Add("Classic");
		break;
	  case dtBle:
		Item->SubItems->Add("BLE");
		break;
	  case dtMixed:
		Item->SubItems->Add("Mixed");
		break;
	  default:
		Item->SubItems->Add("Unknown");
		break;
	}

 // TraceEvent(Address, "Device found", "", "");
}
//---------------------------------------------------------------------------
void __fastcall TForm1::wclBluetoothManager1DiscoveringStarted(TObject *Sender, TwclBluetoothRadio * const Radio)
{
	lvDevices->Items->Clear();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::wclBluetoothManager1DiscoveringCompleted(TObject *Sender,
          TwclBluetoothRadio * const Radio, const int Error)
{
int found = 0;
__int64 Address = 0;
 int Res = 0;
 int i = 0;

 if (lvDevices->Items->Count == 0)
 {
 //	MessageDlg("No BLE devices were found.", mtInformation,	  TMsgDlgButtons() << mbOK, 0);
 	SetConnectedStatus(DISCONNECTED);
				 if (Language == 1)
				 {
					MessageDlgPos("Не найдены устройства BLE.", mtInformation,	  TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				 else
				 {
					MessageDlgPos("No BLE devices.", mtInformation,	  TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
 }
  else
  {
	// Here we can update found devices names.
	for ( i = 0; i < lvDevices->Items->Count; i++)
	{
	  TListItem* Item = lvDevices->Items->Item[i];

	   Address = StrToInt64("$" + Item->Caption);
	   mRadio =  (TwclBluetoothRadio * )Item->Data;
	  String DevName = "";
	   Res = ((TwclBluetoothRadio*)mRadio)->GetRemoteName(Address, DevName);
	  if (Res != WCL_E_SUCCESS)
		Item->SubItems->Strings[0] = "Error: 0x" + IntToHex(Res, 8);
	  else
	  {
		Item->SubItems->Strings[0] = DevName;
		if (AnsiContainsText(Item->SubItems->Strings[0],"Timecup"))    // Ищем девайс Timecup
		{
		  found = 1;
		  wclGattClient1->Address = Address;
		  break;
		}
	  }
	}
	if (!found)    // если не найдено устройство с именем timecup, ищем с именем  HMsoft
	{
		for ( i = 0; i < lvDevices->Items->Count; i++)
		{
		  TListItem* Item = lvDevices->Items->Item[i];

		   Address = StrToInt64("$" + Item->Caption);
		   mRadio =  (TwclBluetoothRadio * )Item->Data;
		  String DevName = "";
		   Res = ((TwclBluetoothRadio*)mRadio)->GetRemoteName(Address, DevName);
		  if (Res != WCL_E_SUCCESS)
			Item->SubItems->Strings[0] = "Error: 0x" + IntToHex(Res, 8);
		  else
		  {
			Item->SubItems->Strings[0] = DevName;
			if (AnsiContainsText(Item->SubItems->Strings[0],"HMsoft"))    // Ищем девайс HMsoft
			{
			  found = 1;
			  wclGattClient1->Address = Address;
			  break;
			}
		  }
		}
    }
  }


   if (!found)
   {
	SetConnectedStatus(DISCONNECTED);
				 if (Language == 1)
				 {
					MessageDlgPos("Устройство не найдено", mtWarning, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				 else
				 {
					MessageDlgPos("Device not found", mtWarning, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
	return;
   }
        // Connect
	// On Windows 10 1607 and above we can face issue with GATT devices that
			// has been paired. There are few different bugs in Windows BLE driver
			// on different OS builds. The solution is to unpair device before connect.
	unsigned short Build;
	if (wclGetWinVer(Build) == verWin10 && Build >= 1607)
	{
	  // Ignore any result code cause it does not matter for us.
	  mRadio->RemoteUnpair(wclGattClient1->Address);
	}
	// And now we can connect.
	 Res = wclGattClient1->Connect(mRadio);
	if (Res != WCL_E_SUCCESS)
	{
	  SetConnectedStatus(DISCONNECTED);
				 if (Language == 1)
				 {
					MessageDlgPos("Ошибка соединения: 0x" + IntToHex(Res, 8), mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				 else
				 {
					MessageDlgPos("Connection error: 0x" + IntToHex(Res, 8), mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
	}

}
//---------------------------------------------------------------------------
void __fastcall TForm1::wclGattClient1Connect(TObject *Sender, const int Error)
{
   int i,Res;
int found = 0;

	lvServices->Items->Clear();
	FServices.Length = 0;

		 Res = wclGattClient1->ReadServices(goReadFromDevice, FServices);
	  if (Res != WCL_E_SUCCESS)
	  {
		SetConnectedStatus(DISCONNECTED);
				 if (Language == 1)
				 {
					MessageDlgPos("Ошибка чтения сервисов: 0x" + IntToHex(Res, 8), mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				 else
				 {
					MessageDlgPos("Error reading services: 0x" + IntToHex(Res, 8), mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
		return;
	  }

	  if (FServices.Length == 0)
	  {
		SetConnectedStatus(DISCONNECTED);
				 if (Language == 1)
				 {
					MessageDlgPos("Количество сервисов равно нулю: 0x" + IntToHex(Res, 8), mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				 else
				 {
					MessageDlgPos("The number of services is zero: 0x" + IntToHex(Res, 8), mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
		return;
	  }

	  for ( i = 0; i < FServices.Length; i++)
	  {
		 mService = FServices[i];

		TListItem* Item = lvServices->Items->Add();
		if (mService.Uuid.IsShortUuid)
		 {
		 	Item->Caption = IntToHex(mService.Uuid.ShortUuid, 4);
			 if (mService.Uuid.ShortUuid == 0xFFE0)
			 {
			  found = 1;
			  break;
			 }
		 }
		else
		  Item->Caption = Sysutils::GUIDToString(mService.Uuid.LongUuid);
		Item->SubItems->Add(BoolToStr(mService.Uuid.IsShortUuid, true));
		Item->SubItems->Add(IntToHex(mService.Handle, 4));
	  }


   FCharacteristics.Length = 0;
  lvCharacteristics->Items->Clear();

  if (!found)
  {
//	MessageDlg("Not found 0xFFE0 service", mtWarning, TMsgDlgButtons() << mbOK, 0);
	SetConnectedStatus(DISCONNECTED);
	MessageDlgPos("Не найден 0xFFE0 сервис", mtWarning, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 if (Language == 1)
				 {
					MessageDlgPos("Не найден 0xFFE0 сервис", mtWarning, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				 else
				 {
					MessageDlgPos("0xFFE0 service not found", mtWarning, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
	return;
  }
  found = 0;

   Res = wclGattClient1->ReadCharacteristics(mService, goNone,	FCharacteristics);
  if (Res != WCL_E_SUCCESS)
  {
//	MessageDlg("Error: 0x" + IntToHex(Res, 8), mtError, TMsgDlgButtons() << mbOK, 0);
	SetConnectedStatus(DISCONNECTED);
				 if (Language == 1)
				 {
					MessageDlgPos("Ошибка чтения характеристики: 0x" + IntToHex(Res, 8), mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				 else
				 {
					MessageDlgPos("Error reading characteristic: 0x" + IntToHex(Res, 8), mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
	return;
  }

  if (FCharacteristics.Length == 0)
  {
	SetConnectedStatus(DISCONNECTED);
				 if (Language == 1)
				 {
					MessageDlgPos("Количество характеристик равно нулю", mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				 else
				 {
					MessageDlgPos("Количество характеристик равно нулю", mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
	return;
  }

  for (int i = 0; i < FCharacteristics.Length; i++)
  {
	mCharacteristic = FCharacteristics[i];

	TListItem* Item = lvCharacteristics->Items->Add();
	if (mCharacteristic.Uuid.IsShortUuid)
	{
	  Item->Caption = IntToHex(mCharacteristic.Uuid.ShortUuid, 4);
		 if (mCharacteristic.Uuid.ShortUuid == 0xFFE1)
		 {
		  mCharacteristic_found = 1;
		  break;
		 }
	}
	 else
	  Item->Caption = Sysutils::GUIDToString(mCharacteristic.Uuid.LongUuid);

	Item->SubItems->Add(BoolToStr(mCharacteristic.Uuid.IsShortUuid, true));
	Item->SubItems->Add(IntToHex(mCharacteristic.ServiceHandle, 4));
	Item->SubItems->Add(IntToHex(mCharacteristic.Handle, 4));
	Item->SubItems->Add(IntToHex(mCharacteristic.ValueHandle, 4));
	Item->SubItems->Add(BoolToStr(mCharacteristic.IsBroadcastable, true));
	Item->SubItems->Add(BoolToStr(mCharacteristic.IsReadable, true));
	Item->SubItems->Add(BoolToStr(mCharacteristic.IsWritable, true));
	Item->SubItems->Add(BoolToStr(mCharacteristic.IsWritableWithoutResponse, true));
	Item->SubItems->Add(BoolToStr(mCharacteristic.IsSignedWritable, true));
	Item->SubItems->Add(BoolToStr(mCharacteristic.IsNotifiable, true));
	Item->SubItems->Add(BoolToStr(mCharacteristic.IsIndicatable, true));
	Item->SubItems->Add(BoolToStr(mCharacteristic.HasExtendedProperties, true));
  }

 // Дескрипторы

  FDescriptors.Length = 0;
  lvDescriptors->Items->Clear();

  if (!mCharacteristic_found)
  {
//	MessageDlg("Not found 0xFFE1 characteristic", mtWarning, TMsgDlgButtons() << mbOK, 0);
	SetConnectedStatus(DISCONNECTED);
				 if (Language == 1)
				 {
	MessageDlgPos("Не найдена характеристика 0xFFE1 ", mtWarning, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				 else
				 {
	MessageDlgPos("Not found characteristic 0xFFE1 ", mtWarning, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
	return;
  }

 // TwclGattCharacteristic Characteristic = 	FCharacteristics[lvCharacteristics->Selected->Index];
   Res = wclGattClient1->ReadDescriptors(mCharacteristic, goNone,	FDescriptors);
  if (Res != WCL_E_SUCCESS)
  {
//	MessageDlg("Error: 0x" + IntToHex(Res, 8), mtError,	  TMsgDlgButtons() << mbOK, 0);
	SetConnectedStatus(DISCONNECTED);
				 if (Language == 1)
				 {
	MessageDlgPos("Ошибка чтения дескрипторов: 0x" + IntToHex(Res, 8), mtError,	  TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				 else
				 {
	MessageDlgPos("Error reading descriptors: 0x" + IntToHex(Res, 8), mtError,	  TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
	return;
  }

  if (FDescriptors.Length == 0)
  {
	SetConnectedStatus(DISCONNECTED);
				 if (Language == 1)
				 {
	MessageDlgPos("Количество дескрипторов равно нулю", mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				 else
				 {
	MessageDlgPos("The number of descriptors is zero.", mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
	return;
  }

  for (int i = 0; i < FDescriptors.Length; i++)
  {
	TwclGattDescriptor Descriptor = FDescriptors[i];

	TListItem* Item = lvDescriptors->Items->Add();
	if (Descriptor.Uuid.IsShortUuid)
	  Item->Caption = IntToHex(Descriptor.Uuid.ShortUuid, 4);
	else
	  Item->Caption = Sysutils::GUIDToString(Descriptor.Uuid.LongUuid);
	Item->SubItems->Add(BoolToStr(Descriptor.Uuid.IsShortUuid, true));
	Item->SubItems->Add(IntToHex(Descriptor.ServiceHandle, 4));
	Item->SubItems->Add(IntToHex(Descriptor.CharacteristicHandle, 4));
	Item->SubItems->Add(IntToHex(Descriptor.Handle, 4));
	switch (Descriptor.DescriptorType)
	{
	  case dtCharacteristicExtendedProperties:
		Item->SubItems->Add("dtCharacteristicExtendedProperties");
		break;
	  case dtCharacteristicUserDescription:
		Item->SubItems->Add("dtCharacteristicUserDescription");
        break;
	  case dtClientCharacteristicConfiguration:
        Item->SubItems->Add("dtClientCharacteristicConfiguration");
        break;
	  case dtServerCharacteristicConfiguration:
        Item->SubItems->Add("dtServerCharacteristicConfiguration");
        break;
      case dtCharacteristicFormat:
		Item->SubItems->Add("dtCharacteristicFormat");
        break;
      case dtCharacteristicAggregateFormat:
        Item->SubItems->Add("dtCharacteristicAggregateFormat");
		break;
      default:
        Item->SubItems->Add("dtCustomDescriptor");
        break;
	}
  }


  // Подписка
 //************** Фрагмент кода из кнопки Write CCCD subscribe    BLEScannerCpp
  	Res = wclGattClient1->WriteClientConfiguration(mCharacteristic, true,	goNone, plNone);
  if (Res != WCL_E_SUCCESS)
	MessageDlgPos("Subscription CCCD Error: 0x" + IntToHex(Res, 8), mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
	//***********************


	 Res = wclGattClient1->Subscribe(mCharacteristic);
		if (Res != WCL_E_SUCCESS)
		{
		//	MessageDlg("Error: 0x" + IntToHex(Res, 8), mtError, TMsgDlgButtons() << mbOK, 0);
			SetConnectedStatus(DISCONNECTED);
			MessageDlgPos("Ошибка подписки: 0x" + IntToHex(Res, 8), mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 if (Language == 1)
				 {
			MessageDlgPos("Ошибка подписки: 0x" + IntToHex(Res, 8), mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				 else
				 {
			MessageDlgPos("Subscription Error: 0x" + IntToHex(Res, 8), mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
		}
		else
		{
		 SetConnectedStatus(CONNECTED);
			 ReadCustomRecipeButton->Click();
			 ReadTraditionalRecipeButton->Click();
			 JustProgramStarted = false;
		}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::btSetValueClick(TObject *Sender)
{
char Res = 57;
bool last = false;
 String Str = edCharVal->Text;

  int i = 0;

  while (i < Str.Length())
  {
	  PutCharBLE(Str[i+1]);
	  i ++;
  }

	SendBufferBLE();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::wclGattClient1CharacteristicChanged(TObject *Sender, const WORD Handle,
		  const TwclGattCharacteristicValue Value)
{
	String Str = "";

	ReadingCharacteristicInProgress = true;

	for (int i = 0; i < Value.Length; i++)
	{
		InBuffer [InBufferIndexEnd] = Value[i];
		InBufferIndexEnd++;
		if (InBufferIndexEnd >= BUFFER_LENGTH)
		{
           InBufferIndexBeg = InBufferIndexEnd = 0;
		}
//		Str =  " 0x" + IntToHex(Value[i],2) + Str;
//		Str =  "." + IntToStr(Value[i]) + Str;
		Str = Str + (char) Value[i];
	}
	if (Memo1->GetTextLen() > 2000)
		 Memo1->Clear();
	Memo1->Lines->Text += Str;
	ReadingCharacteristicInProgress = false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::copyButton1Click(TObject *Sender)
{
   Memo1->SelectAll();
  Memo1->CopyToClipboard();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::clearButton2Click(TObject *Sender)
{
 Memo1->Lines->Clear();
}
//---------------------------------------------------------------------------



void __fastcall TForm1::StartButtonClick(TObject *Sender)
{
// command = START_BUTTON_PRESS;
 AddCommand(START_BUTTON_PRESS);
 attempt = ATTEMPTS_NUMBER;
 Form1->Label1->Caption = "Send...";
}
//---------------------------------------------------------------------------


int __fastcall CheckBuffer(void)
{    unsigned short int ret,i;
     unsigned char j;
ret = 0;
  switch (Command())
  {
		  case READ_CUSTOM_RECIPES_DATA:
                    ret = 1;
               if (CRC.crc16_current != crc16(Recipes.char_custom_recipe,RECIPES_DATA_LENGTH))
                  ret = 0;
               break;
          case READ_TRADITIONAL_RECIPE_DATA:
                    ret = 1;
               if (CRC.crc16_current != crc16(recipe_settings.char_recipe_settings,RECIPE_SETTINGS_LENGTH))
                  ret = 0;
			   break;
		  case RUN_RECIPE:
		  case STOP_RECIPE:
		  case RESET_TRADITIONAL_RECIPE_DATA:
		  case RESET_CUSTOM_RECIPE_DATA:
		  case WRITE_TRADITIONAL_RECIPE_DATA:
		  case WRITE_CUSTOM_RECIPES_DATA:
		  case LIQUID_BUTTON_PRESS:
          case TIME_BUTTON_PRESS:
          case PLUS_BUTTON_PRESS:
          case MINUS_BUTTON_PRESS:
          case START_BUTTON_PRESS:
		  case READ_STATUS:
               ret = 1;
			   if (CRC.crc16_current != crc16(UART_packet.command_packet_char,COMMAND_PACKET_LENGTH))
                ret = 0;
             break;
  }
return(ret);
}
//---------------------------------------------------------------------------

int __fastcall ReceiveData(void)
{    int ret,i;
ret = 0;
  switch (Command())
  {
          case READ_CUSTOM_RECIPES_DATA:
				 if (InBuffUsed() == RECIPES_DATA_LENGTH+CRC_LENGTH)
                  {
				   for (i = 0;i < RECIPES_DATA_LENGTH && IsConnected; i++)
					   Recipes.char_custom_recipe[i] = GetCharBLE();
                   GetCRC();
                   ret = 1;
                  }
                  else
                    ret = 0;
               break;
          case READ_TRADITIONAL_RECIPE_DATA:
				 if (InBuffUsed() == RECIPE_SETTINGS_LENGTH+CRC_LENGTH)
                  {
                   for (i = 0;i < RECIPE_SETTINGS_LENGTH && IsConnected; i++)
					   recipe_settings.char_recipe_settings[i] = GetCharBLE();
                   GetCRC();
                   ret = 1;
                  }
                  else
                    ret = 0;
			   break;
		  case RUN_RECIPE:         //номер рецепта уже в переменной liquid
		  case STOP_RECIPE:         //номер рецепта уже в переменной liquid
		  case RESET_TRADITIONAL_RECIPE_DATA:
		  case RESET_CUSTOM_RECIPE_DATA:
		  case WRITE_TRADITIONAL_RECIPE_DATA:
		  case WRITE_CUSTOM_RECIPES_DATA:
          case LIQUID_BUTTON_PRESS:
          case TIME_BUTTON_PRESS:
          case PLUS_BUTTON_PRESS:
          case MINUS_BUTTON_PRESS:
          case START_BUTTON_PRESS:
          case READ_STATUS:
				 if (InBuffUsed() == COMMAND_PACKET_LENGTH+CRC_LENGTH)
				  {
				   for (i = 0;i < COMMAND_PACKET_LENGTH && IsConnected; i++)
					   UART_packet.command_packet_char[i] = GetCharBLE();
				   GetCRC();
                   ret = 1;
                  }
                  else
                    ret = 0;
             break;
  }
return(ret);
}
//---------------------------------------------------------------------------
void __fastcall SendCommand(char Command)
{ unsigned int i;

  FlushInBuffer();
  FlushOutBuffer();

			UART_packet.command_packet.command = Command;
            UART_packet.command_packet.current_temperature = 0;
            UART_packet.command_packet.processor_temperature = 0;
            UART_packet.command_packet.heater_temperature = 0;
			UART_packet.command_packet.software_version = 0;
			UART_packet.command_packet.LEDs = 0;
			UART_packet.command_packet.water_sensor_raw_data = 0;
			UART_packet.command_packet.Recipe_number = liquid;

  for (i=0; i<COMMAND_PACKET_LENGTH && IsConnected; i++)
  {
	   //	while (OutBuffFree() < 1);
		PutCharBLE(UART_packet.command_packet_char[i]);                               //!!!!
  }
  CRC.crc16_current = crc16(UART_packet.command_packet_char,COMMAND_PACKET_LENGTH);
  PutCRC();
  SendBufferBLE();

}
//---------------------------------------------------------------------------
void __fastcall SendData(void)
{ unsigned int i;
  unsigned char j;

  FlushInBuffer();
  FlushOutBuffer();

	switch (Command())
    {
	  case WRITE_TRADITIONAL_RECIPE_DATA:
		  CRC.crc16_current = crc16(recipe_settings.char_recipe_settings,RECIPE_SETTINGS_LENGTH);
		for (i=0,j=0; i<RECIPE_SETTINGS_LENGTH && IsConnected; i++,j++)
        {

			  PutCharBLE(recipe_settings.char_recipe_settings[i]);               //!!!
        }
        PutCRC();
        break;
      case WRITE_CUSTOM_RECIPES_DATA:
		  CRC.crc16_current = crc16(Recipes.char_custom_recipe,RECIPES_DATA_LENGTH);
        for (i=0,j=0; i<RECIPES_DATA_LENGTH && IsConnected; i++,j++)
        {

			  PutCharBLE(Recipes.char_custom_recipe[i]);                         //!!!
        }
        PutCRC();
        break;
	}
	SendBufferBLE();
}
//---------------------------------------------------------------------------
void __fastcall StateMachine(void)
{  int i;

Global_timer++;

    if (_VM_state != VM_state) entry = 1; else entry = 0;
    _VM_state = VM_state;

    switch (VM_state)
    {
      case VM_IDLE:
					  if (Command() != NO_COMMAND)
					  {
						VM_state = VM_EXEC_COMMAND;
						  if (!IsConnected)
						  {
								VM_state =  VM_STOP;
								ClearCommandStack();
								 if (Language == 1)
								 {
								MessageDlgPos("Нет связи с устройством", mtError,  TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
								 }
								 else
								 {
								MessageDlgPos("No connection to device", mtError,  TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
								 }
						  }
					  }
				break;
	  case VM_EXEC_COMMAND:
						SendCommand(Command());        // Сначала передаем команду
						ProcessForm->Left = Form1->Left + (Form1->ClientWidth - ProcessForm->Width)/2;        // ProcessForm - по центру
						ProcessForm->Top = Form1->Top + (Form1->ClientHeight - ProcessForm->Height - (ProcessForm->Height/3))/2;
						switch (Command())
						{
								case WRITE_TRADITIONAL_RECIPE_DATA:
								case WRITE_CUSTOM_RECIPES_DATA:
										VM_state = VM_SEND_DATA;
										Pause_timer = Global_timer +  5*M20_mSEC;
										if (!ProcessForm->Visible)     // Выводим окно с сообщением о процессе чтения
										{
											 if (Language == 1)
											 {
												ProcessForm->Label1->Caption = "Идет запись...";
											 }
											 else
											 {
												ProcessForm->Label1->Caption = "Recording in progress...";
											 }
											ProcessForm->Show();
										}
								  break;
								case RUN_RECIPE:
								case STOP_RECIPE:
								case RESET_TRADITIONAL_RECIPE_DATA:
								case RESET_CUSTOM_RECIPE_DATA:
								case LIQUID_BUTTON_PRESS:
								case TIME_BUTTON_PRESS:
								case PLUS_BUTTON_PRESS:
								case MINUS_BUTTON_PRESS:
								case START_BUTTON_PRESS:
                                case RESET_CONTROLLER:        // Будет разрыв связи
								case READ_STATUS:
                                        VM_state = VM_RECEIVE_DATA;
                                        waiting_bytes_number = COMMAND_PACKET_LENGTH+CRC_LENGTH;
										Pause_timer = Global_timer +  20*M20_mSEC;
                                  break;
								case READ_CUSTOM_RECIPES_DATA:
										if (!ProcessForm->Visible)     // Выводим окно с сообщением о процессе чтения
										{
											 if (Language == 1)
											 {
												ProcessForm->Label1->Caption = "Читаем рецепты пользователя...";
											 }
											 else
											 {
												ProcessForm->Label1->Caption = "Reading user recipes...";
											 }
											ProcessForm->Show();
										}
										VM_state = VM_RECEIVE_DATA;
                                        waiting_bytes_number = RECIPES_DATA_LENGTH+CRC_LENGTH;
										Pause_timer = Global_timer +  100*M20_mSEC;
                                  break;
								case READ_TRADITIONAL_RECIPE_DATA:
										if (!ProcessForm->Visible)     // Выводим окно с сообщением о процессе чтения
										{
											 if (Language == 1)
											 {
												ProcessForm->Label1->Caption = "Читаем настройки...";
											 }
											 else
											 {
												ProcessForm->Label1->Caption = "Reading settings ...";
											 }
											ProcessForm->Show();
										}
										VM_state = VM_RECEIVE_DATA;
                                        waiting_bytes_number = RECIPE_SETTINGS_LENGTH+CRC_LENGTH;
										Pause_timer = Global_timer +  100*M20_mSEC;
                                  break;
                        }
                break;
      case VM_SEND_DATA:
                 if (Global_timer > Pause_timer)
                   {
                      SendData();
                      VM_state = VM_RECEIVE_DATA;
                      waiting_bytes_number = COMMAND_PACKET_LENGTH+CRC_LENGTH;
					  Pause_timer = Global_timer +  200*M20_mSEC;
                   }
                break;
      case VM_RECEIVE_DATA:
				 if (Global_timer > Pause_timer || InBuffUsed() == waiting_bytes_number)
                   {
					  if (ReceiveData())
                      {
						VM_state = VM_CHECK_DATA;
					  }
                      else
                      {
						VM_state = VM_NO_CONNECTION;
						NoConnectionProcessing = true;
						if (attempt > 0) attempt--;
                      }
				   }
                break;
      case VM_CHECK_DATA:

					  if (CheckBuffer())
                      {
						VM_state = VM_CORRECT_DATA;
						NoConnectionProcessing = false;
                      }
                      else
                      {
						VM_state = VM_BAD_DATA;
						NoConnectionProcessing = true;
						if (attempt > 0) attempt--;
					  }
                break;
      case VM_BAD_DATA:
	  case VM_NO_CONNECTION:
						Form1->ConnectImage->Picture = Form1->GrayImage->Picture;
						Form1->Label1->Caption = "NO_CONNECTION attempt=" + IntToStr(attempt);
						Form1->Memo1->Lines->Text +=  "/**NO_CONNECTION**/";     // отделяем посылки в Memo1
						if (ProcessForm->Visible) ProcessForm->Close();
						if (attempt)
						  { VM_state = VM_IDLE;  }
						else
						  {
							ClearCommandStack();
							Form1->SetConnectedStatus(DISCONNECTED);
							Form1->wclBluetoothManager1->Close();
							AddCommand(READ_STATUS);
							AddCommand(READ_STATUS);
//							MessageDlgPos("Нет связи с устройством", mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
							VM_state = VM_STOP;
						  }
				break;
	  case VM_CORRECT_DATA:
						Form1->ConnectImage->Picture = Form1->RedImage->Picture;
				  if (UART_packet.command_packet.command != BAD_DATA_RESULT)
				  {
						switch (Command())
						{
								case WRITE_TRADITIONAL_RECIPE_DATA:
										TraditionalRecipeChanged = false;
									break;
								case WRITE_CUSTOM_RECIPES_DATA:
                                        CustomRecipeChanged = false;
									break;
								case READ_TRADITIONAL_RECIPE_DATA:
									 TraditionRecipeSettingsToGrid();
									 TraditionRecipeSettingsToTrackBar();
									 Create_Tradition_Recipe_Series();
									 if (Form1->MainPageControl->TabIndex == 0) Form1->AllScaleButton->Click();

									 TraditionalRecipeChanged = false;
									break;
								case RESET_TRADITIONAL_RECIPE_DATA:
										TraditionalRecipeChanged = false;
										Form1->ReadTraditionalRecipeButton->Click();

									break;
								case RESET_CUSTOM_RECIPE_DATA:
										CustomRecipeChanged = false;
										Form1->ReadCustomRecipeButton->Click();

									break;
								case READ_CUSTOM_RECIPES_DATA:
									 CustomRecipeSettingsToGrid();
									 Create_Custom_Recipe_Series();
									 if (Form1->MainPageControl->TabIndex != 0) Form1->AllScaleButton->Click();
									  CustomRecipeChanged = false;
									break;
								case READ_STATUS:
									UpdateDeviceControlSettings(DATA);
									break;
						}
						VM_state = VM_STOP;
						Form1->Memo1->Lines->Text +=  "/**CORRECT_DATA**/";     // отделяем посылки в Memo1
						if (ProcessForm->Visible) ProcessForm->Close();
						Form1->Label1->Caption = "CORRECT_DATA";
				  }
				  else
				  {
					MessageDlgPos("BAD_DATA_RESULT: 0x" , mtError,  TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);						VM_state = VM_BAD_DATA;
						if (attempt > 0) attempt--;
				  }
				break;
	  case VM_STOP:
                        NextCommand();
						VM_state = VM_IDLE;
                break;
      case VM_PAUSE:
                      if (Pause_timer < Global_timer)   VM_state = pause_return_state;
                      break;
    }

}
//---------------------------------------------------------------------------
void __fastcall TForm1::StateMachineTimerTimer(TObject *Sender)
{
  StateMachine();
}
//---------------------------------------------------------------------------





void __fastcall TForm1::ReadTraditionalRecipeButtonClick(TObject *Sender)
{
 int Result = mrOk;
	 if (TraditionalRecipeChanged)
	{
		MessageForm->Left = Form1->Left + (Form1->ClientWidth - MessageForm->Width)/2;        // MessageForm - по центру
		MessageForm->Top = Form1->Top + (Form1->ClientHeight - MessageForm->Height - (MessageForm->Height/3))/2;
	   Result = MessageForm->ShowModal();
	}
	if (Result == mrOk)
	{
	  AddCommand(READ_TRADITIONAL_RECIPE_DATA);
	 attempt = ATTEMPTS_NUMBER;
	 Form1->Label1->Caption = "Send...";
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SaveTraditionalRecipeButtonClick(TObject *Sender)
{
	TraditionRecipeGridToSettings();

	AddCommand(READ_STATUS);
  AddCommand(WRITE_TRADITIONAL_RECIPE_DATA);
 attempt = ATTEMPTS_NUMBER;
 Form1->Label1->Caption = "Send...";

}
//---------------------------------------------------------------------------




void __fastcall TForm1::ResetTraditionalRecipeButtonClick(TObject *Sender)
{
 int Result = mrOk;
	 if (TraditionalRecipeChanged)
	{
		MessageForm->Left = Form1->Left + (Form1->ClientWidth - MessageForm->Width)/2;        // MessageForm - по центру
		MessageForm->Top = Form1->Top + (Form1->ClientHeight - MessageForm->Height - (MessageForm->Height/3))/2;
	   Result = MessageForm->ShowModal();
	}
	if (Result == mrOk)
	{
		AddCommand(RESET_TRADITIONAL_RECIPE_DATA);
		 attempt = ATTEMPTS_NUMBER;
		 Form1->Label1->Caption = "Send...";
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ReadCustomRecipeButtonClick(TObject *Sender)
{
 int Result = mrOk;
	 if (CustomRecipeChanged)
	{
		MessageForm->Left = Form1->Left + (Form1->ClientWidth - MessageForm->Width)/2;        // MessageForm - по центру
		MessageForm->Top = Form1->Top + (Form1->ClientHeight - MessageForm->Height - (MessageForm->Height/3))/2;
	   Result = MessageForm->ShowModal();
	}
	if (Result == mrOk)
	{
		 AddCommand(READ_CUSTOM_RECIPES_DATA);
		 attempt = ATTEMPTS_NUMBER;
		 Form1->Label1->Caption = "Send...";
	}
}
//---------------------------------------------------------------------------

void __fastcall Create_Tradition_Recipe_Series(void)
{  int i;
   Form1->Tradition_Recipe_Power_Series->Clear();
   Form1->Tradition_Recipe_Temperature_Series->Clear();
   time_beg = 0;
   temp_beg = temp_beg_global;
   first_point = 1;

   Add_Recipe_Step_Points(Form1->Tradition_Recipe_Power_Series,Form1->Tradition_Recipe_Temperature_Series, recipe_settings.current.Coffee_Target_Temperature-10,recipe_settings.current.Coffee_Initial_Power_Level, 0,1);
   Add_Recipe_Step_Points(Form1->Tradition_Recipe_Power_Series,Form1->Tradition_Recipe_Temperature_Series,recipe_settings.current.Coffee_Target_Temperature,recipe_settings.current.Coffee_Low_Power_level, 0,1);
   Add_Recipe_Step_Points(Form1->Tradition_Recipe_Power_Series,Form1->Tradition_Recipe_Temperature_Series,recipe_settings.current.Coffee_Target_Temperature,10,recipe_settings.current.Coffee_Holding_Time,1);
   Add_Recipe_Step_Points(Form1->Tradition_Recipe_Power_Series,Form1->Tradition_Recipe_Temperature_Series,0,recipe_settings.current.Coffee_Middle_Power_level,4*SEC,1);
   Add_Recipe_Step_Points(Form1->Tradition_Recipe_Power_Series,Form1->Tradition_Recipe_Temperature_Series,0,0,recipe_settings.current.Coffee_Impulses_Pause,1);
   for (i=0; i<recipe_settings.current.Coffee_Impulses_Number-1; i++)
   {
	   Add_Recipe_Step_Points(Form1->Tradition_Recipe_Power_Series,Form1->Tradition_Recipe_Temperature_Series,0,100,recipe_settings.current.Coffee_Impulses_Duration,0);
	   Add_Recipe_Step_Points(Form1->Tradition_Recipe_Power_Series,Form1->Tradition_Recipe_Temperature_Series,PAUSE_R,0,recipe_settings.current.Coffee_Impulses_Pause,0);
   }
	   Add_Recipe_Step_Points(Form1->Tradition_Recipe_Power_Series,Form1->Tradition_Recipe_Temperature_Series,0,100,recipe_settings.current.Coffee_Impulses_Duration,1);
       Add_Recipe_Step_Points(Form1->Tradition_Recipe_Power_Series,Form1->Tradition_Recipe_Temperature_Series,END_R,0,0,1);

 }

//----------возвращает количество созданных точек-----------------------------------------------------------------
int __fastcall Add_Recipe_Step_Points(TLineSeries *Power_Series,TLineSeries *Temperature_Series, int temperature, int power, int time, char need_label)
{    int time_result,min,sec,time_add,micro_power, number_of_points;
   AnsiString label;

   number_of_points = 0;

 if (temperature == END_R && power ==0 && time == 0)         // конец
 {
    time_end = time_beg;
	temp_end = temp_beg;
    if (need_label) {min = (time_end/SEC)/60; sec = (time_end/SEC)%60; label = min;  label +="m ";  label +=sec;   label +="sec"; }
    Power_Series->AddXY(time_beg,0,label);
	if (first_point) { Temperature_Series->AddXY(time_beg,temp_beg); number_of_points++;} // Чтобы не создавать двойные точки
	Temperature_Series->AddXY(time_end,temp_end); number_of_points++;
	time_beg = time_end;
    temp_beg = temp_end;
	first_point = 0;
	TimeAxisMaximum = time_end;
	return (number_of_points);
 }

 if (temperature < temp_beg && temperature != 0 && power ==0)         // охлаждение до температуры
 {
	time_end = time_beg + SEC*Calculate_Time(5,temp_beg,temperature);  // Охлаждение - тот же нагрев, только 5% мощностью и в обратную сторону
    temp_end = temperature;
    if (need_label) {min = (time_end/SEC)/60; sec = (time_end/SEC)%60; label = min;  label +="m ";  label +=sec;   label +="sec"; }
    Power_Series->AddXY(time_beg,0);
    Power_Series->AddXY(time_end,0,label);
	if (first_point) {Temperature_Series->AddXY(time_beg,temp_beg); number_of_points++;} // Чтобы не создавать двойные точки
	Temperature_Series->AddXY(time_end,temp_end); number_of_points++;
    time_beg = time_end;
	temp_beg = temp_end;
	first_point = 0;
	TimeAxisMaximum = time_end;
	return (number_of_points);
 }
 if (temperature == PAUSE_R && power ==0 && time != 0)         // пауза
 {
    time_end = time_beg + time;
	temp_end = Calculate_Temperature(power,temp_beg,time);   // Охлаждение - тот же нагрев, только 5% мощностью и в обратную сторону
	if (need_label) {min = (time_end/SEC)/60; sec = (time_end/SEC)%60; label = min;  label +="m ";  label +=sec;   label +="sec"; }
	Power_Series->AddXY(time_beg,power);
    Power_Series->AddXY(time_end,power,label);
	if (first_point) { Temperature_Series->AddXY(time_beg,temp_beg); number_of_points++;} // Чтобы не создавать двойные точки
	Temperature_Series->AddXY(time_end,temp_end); number_of_points++;
    time_beg = time_end;
    temp_beg = temp_end;
	first_point = 0;
	TimeAxisMaximum = time_end;
	return (number_of_points);
 }

 if ((temperature >= 0 && temperature <= 100) && power !=0 && time == 0)         // нагрев/охлаждение до температуры мощностью без времени
 {
	if (temperature <= temp_beg) power = 0;         // если охлаждаем
	if (temperature <= temp_beg_global) temperature = temp_beg_global;     // нельзя охладить ниже окружающей температуры
	time_end = time_beg + SEC*Calculate_Time(power,temp_beg,temperature);
    temp_end = temperature;
	if (need_label) {min = (time_end/SEC)/60; sec = (time_end/SEC)%60; label = min;  label +="m ";  label +=sec;   label +="sec"; }
    Power_Series->AddXY(time_beg,power);
    Power_Series->AddXY(time_end,power,label);
	if (first_point) { Temperature_Series->AddXY(time_beg,temp_beg); number_of_points++;} // Чтобы не создавать двойные точки
	Temperature_Series->AddXY(time_end,temp_end);  number_of_points++;
    time_beg = time_end;
    temp_beg = temp_end;
	first_point = 0;
	TimeAxisMaximum = time_end;
	return (number_of_points);
 }

 if ((temperature >= 0 && temperature <= 100) && power !=0 && time != 0)        // нагрев/охл до температуры мощностью определенное время
 {
	if (temperature == 0) temperature = 100; // если нагрев по времени без указания температуры, то макс температура = 100град
	if (temperature <= temp_beg_global) temperature = temp_beg_global; // нельзя охладить ниже окружающей температуры
	if (temperature < temp_beg) power = 0;   // если охлаждение
	if ((power && Calculate_Temperature(power,temp_beg,time) > temperature) ||    // две точки - сначала нагреваем до темп, затем поддерживаем ее
		(!power && Calculate_Temperature(power,temp_beg,time) < temperature))     // две точки - сначала охлаждаем до темп, затем поддерживаем ее
	{
       time_end = time_beg + SEC*Calculate_Time(power,temp_beg,temperature);
       temp_end = temperature;
    if (need_label) {min = (time_end/SEC)/60; sec = (time_end/SEC)%60; label = min;  label +="m ";  label +=sec;   label +="sec"; }
       time_result = time_beg + time;
       // 1я точка
	   if (temperature != temp_beg)   // создаем только если нужно изменять температуру
       {
          Power_Series->AddXY(time_beg,power);
		  Power_Series->AddXY(time_end,power,label);
		  if (first_point) { Temperature_Series->AddXY(time_beg,temp_beg); number_of_points++;}  // Чтобы не создавать двойные точки
		  Temperature_Series->AddXY(time_end,temp_end);  number_of_points++;
          time_beg = time_end;
          temp_beg = temp_end;
          first_point = 0;
       }
	   // 2я точка - поддерживаем температуру  если  не 100град и датчик пенки не включен
	   if (FoamSensorOn && temperature == 100) { FoamSensorOn = false; TimeAxisMaximum = time_end; return (number_of_points);  }         // сработал датчик пенки, выключаем его и заканчиваем шаг
       time_end = time_result;
	   temp_end = temperature;
       if (need_label) {min = (time_end/SEC)/60; sec = (time_end/SEC)%60; label = min;  label +="m ";  label +=sec;   label +="sec"; }
	   time_add = TIME_ADD;
       if (time_beg + time_add < time_result) // Если нужно генерить меандр
       {
          Power_Series->AddXY(time_beg,0);
          for (time_add = TIME_ADD,micro_power = 0;time_beg + time_add < time_result;time_beg+=time_add)
          {
           Power_Series->AddXY(time_beg + time_add,micro_power);
           if (micro_power == MICRO_POWER) micro_power = 0; else micro_power = MICRO_POWER;
           Power_Series->AddXY(time_beg + time_add,micro_power);
          }
          Power_Series->AddXY(time_end,micro_power,label);
       } else
       {   //не хватает места для меандра
         Power_Series->AddXY(time_beg,MICRO_POWER);
         Power_Series->AddXY(time_end,MICRO_POWER,label);
       }
	   if (first_point) { Temperature_Series->AddXY(time_beg,temp_beg);  number_of_points++;} // Чтобы не создавать двойные точки
	   Temperature_Series->AddXY(time_end,temp_end);  number_of_points++;
       time_beg = time_end;
       temp_beg = temp_end;
       first_point = 0;
    }
	else   // точная или недостаточная мощность, чтобы достичь треб температуры, раньше истечет время - одна точка
    {
       time_end = time_beg + time;
       temp_end = Calculate_Temperature(power,temp_beg,time);
       if (need_label) {min = (time_end/SEC)/60; sec = (time_end/SEC)%60; label = min;  label +="m ";  label +=sec;   label +="sec"; }
       Power_Series->AddXY(time_beg,power);
       Power_Series->AddXY(time_end,power,label);
	   if (first_point) { Temperature_Series->AddXY(time_beg,temp_beg); number_of_points++;} // Чтобы не создавать двойные точки
	   Temperature_Series->AddXY(time_end,temp_end);  number_of_points++;
       time_beg = time_end;
	   temp_beg = temp_end;
       first_point = 0;
    }

 }
 TimeAxisMaximum = time_end;
 return (number_of_points);
}
//---------------------------------------------------------------------------

void __fastcall Tradition_Recipe_Set_Default_Values(void)
{
    recipe_settings.current.Coffee_Target_Temperature = 94;
	recipe_settings.current.Coffee_Holding_Time = 90*SEC;
    recipe_settings.current.Coffee_Low_Power_level = 30;
    recipe_settings.current.Coffee_Middle_Power_level = 70;
	recipe_settings.current.Coffee_Impulses_Duration = 69*M20_mSEC;
    recipe_settings.current.Coffee_Impulses_Number = 4;
    recipe_settings.current.Coffee_Boiling_Threshold = BOILING_THRESHOLD;
	recipe_settings.current.Coffee_Impulses_Pause = 2*SEC;
	recipe_settings.current.Coffee_Initial_Power_Level = 100;
	recipe_settings.current.Coffee_Temperature_Correction = 0;
    volume = 200;
 temp_beg_global = 22;


}
//---------------------------------------------------------------------------
// Возвращает время(сек) нагрева воды мощностью power(% от 320вт) до температуры Tend с температуры Tbeg
// Вместо мощности 500вт ставим меньшую, тк нагревается не только вода, но и корпус кофеварки
int __fastcall Calculate_Time(int power,int Tbeg,int Tend)
{
  if (Tend < Tbeg || power == 0) power = 5; // остываем


 return(abs((TEMP_CO*volume*(Tend-Tbeg))/(320*power/100)));
}
//---------------------------------------------------------------------------
// Возвращает температуру воды при нагреве/охлаждении мощностью power(% от 320вт) от температуры Tbeg , время нагрева - time(сек)*SEC
// Вместо мощности 500вт ставим меньшую, тк нагревается не только вода, но и корпус кофеварки
int __fastcall Calculate_Temperature(int power,int Tbeg,int time)
{   int temp;

	temp = ((time*(320*power/100))/SEC+TEMP_CO*volume*Tbeg)/(TEMP_CO*volume);

	if (power == 0)    // охлаждение
	{
		power = 5;
		temp = ((time*(320*power/100))/SEC+TEMP_CO*volume*Tbeg)/(TEMP_CO*volume);
		temp = 2*Tbeg - temp;
	}
 return(temp);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

// Вычисляет CRC16
unsigned short __fastcall crc16(volatile unsigned char *sbuf,unsigned short len)
{
    unsigned short crc=0xFFFF;

    while(len){
        crc=(unsigned char)(crc >> 8) | (crc << 8);
        crc^=(unsigned char) *sbuf;
        crc^=(unsigned char)(crc & 0xff) >> 4;
        crc^=(crc << 8) << 4;
        crc^=((crc & 0xff) << 4) << 1;
        len--;
        sbuf++;
    }
    return crc;
}//crc16()
//---------------------------------------------------------------------------
void __fastcall GetCRC(void)
{
 if (!IsConnected) return;

 CRC.crc16_current_char[1] = GetCharBLE(); // Старший байт
 CRC.crc16_current_char[0] = GetCharBLE(); // Младший байт
}

//---------------------------------------------------------------------------
void __fastcall PutCRC(void)
{
 if (!IsConnected) return;
 PutCharBLE(CRC.crc16_current_char[1]);
 PutCharBLE(CRC.crc16_current_char[0]);
}

//---------------------------------------------------------------------------
void __fastcall TForm1::ReadStatusButtonClick(TObject *Sender)
{
	AddCommand(READ_STATUS);
	 attempt = ATTEMPTS_NUMBER;
	 Form1->Label1->Caption = "Send...";
}
//---------------------------------------------------------------------------

void __fastcall TForm1::BadDataButtonClick(TObject *Sender)
{
 if (!IsConnected) return;
 PutCharBLE(1);
   PutCharBLE(1);
   SendBufferBLE();
}
//---------------------------------------------------------------------------


void __fastcall TForm1::LiquidButtonClick(TObject *Sender)
{
 AddCommand(LIQUID_BUTTON_PRESS);
 attempt = ATTEMPTS_NUMBER;
 Form1->Label1->Caption = "Send...";
}

//---------------------------------------------------------------------------

void __fastcall TForm1::PlusButtonClick(TObject *Sender)
{
 AddCommand(PLUS_BUTTON_PRESS);
 attempt = 3;
 Form1->Label1->Caption = "Send...";
}
//---------------------------------------------------------------------------

void __fastcall TForm1::MinusButtonClick(TObject *Sender)
{
 AddCommand(MINUS_BUTTON_PRESS);
 attempt = ATTEMPTS_NUMBER;
 Form1->Label1->Caption = "Send...";
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button14Click(TObject *Sender)
{
Create_Tradition_Recipe_Series();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SaveCustomRecipeButtonClick(TObject *Sender)
{
  AddCommand(WRITE_CUSTOM_RECIPES_DATA);
 attempt = ATTEMPTS_NUMBER;
 Form1->Label1->Caption = "Send...";
}

//---------------------------------------------------------------------------

void __fastcall TForm1::TraditionRecipeGridGetEditorType(TObject *Sender, int ACol,
		  int ARow, TEditorType &AEditor)
{
	switch (ARow)
	{
				case 5:
				case 3:
				case 7:
						AEditor = edCalculatorDropDown;
					break;
	}
}
//---------------------------------------------------------------------------
void __fastcall	 TraditionRecipeSettingsToGrid(void)
{

// Form1->TraditionRecipeGrid->Ints[1][0] = volume;
// Form1->TraditionRecipeGrid->Ints[1][1] = temp_beg_global;
 Form1->TraditionRecipeGrid->Ints[1][0] = recipe_settings.current.Coffee_Initial_Power_Level;
 Form1->TraditionRecipeGrid->Ints[1][1] = recipe_settings.current.Coffee_Low_Power_level;
 Form1->TraditionRecipeGrid->Ints[1][2] = recipe_settings.current.Coffee_Target_Temperature;
 Form1->TraditionRecipeGrid->Floats[1][3] = recipe_settings.current.Coffee_Holding_Time/SEC;
 Form1->TraditionRecipeGrid->Ints[1][4] = recipe_settings.current.Coffee_Middle_Power_level;
 Form1->TraditionRecipeGrid->Floats[1][5] = (float)recipe_settings.current.Coffee_Impulses_Duration/SEC;
 Form1->TraditionRecipeGrid->Ints[1][6] = recipe_settings.current.Coffee_Impulses_Number;
 Form1->TraditionRecipeGrid->Floats[1][7] = (float)recipe_settings.current.Coffee_Impulses_Pause/SEC;
 Form1->TraditionRecipeGrid->Ints[1][8] = recipe_settings.current.Coffee_Boiling_Threshold;
}
//---------------------------------------------------------------------------
void __fastcall	 TraditionRecipeGridToSettings(void)
{

//   volume =  Form1->TraditionRecipeGrid->Ints[1][0];
//   temp_beg_global =  Form1->TraditionRecipeGrid->Ints[1][1];
   recipe_settings.current.Coffee_Initial_Power_Level = Form1->TraditionRecipeGrid->Ints[1][0];
   recipe_settings.current.Coffee_Low_Power_level =  Form1->TraditionRecipeGrid->Ints[1][1];
   recipe_settings.current.Coffee_Target_Temperature = Form1->TraditionRecipeGrid->Ints[1][2] ;
   recipe_settings.current.Coffee_Holding_Time =   Form1->TraditionRecipeGrid->Floats[1][3]*SEC ;
   recipe_settings.current.Coffee_Middle_Power_level =   Form1->TraditionRecipeGrid->Ints[1][4] ;
   recipe_settings.current.Coffee_Impulses_Duration =  Form1->TraditionRecipeGrid->Floats[1][5]*SEC ;
   recipe_settings.current.Coffee_Impulses_Number = Form1->TraditionRecipeGrid->Ints[1][6] ;
   recipe_settings.current.Coffee_Impulses_Pause =   Form1->TraditionRecipeGrid->Floats[1][7]*SEC;
   recipe_settings.current.Coffee_Boiling_Threshold = Form1->TraditionRecipeGrid->Ints[1][8];
}


void __fastcall TForm1::AdvTrackBar0Change(TObject *Sender)
{
 if (!SettingsToTrackBarInProgress)
 {
	TraditionRecipeTrackBarToSettings();
	 TraditionRecipeSettingsToGrid();
	Create_Tradition_Recipe_Series();
	TraditionalRecipeChanged = true;
  }
}
//---------------------------------------------------------------------------


void __fastcall TForm1::TraditionRecipeGridCanEditCell(TObject *Sender, int ARow, int ACol,
		  bool &CanEdit)
{
		if (ARow != 5 && ARow != 3 && ARow != 7)
	{
	  CanEdit = false;
	}

}
//---------------------------------------------------------------------------

void __fastcall TForm1::TraditionRecipeGridSelectCell(TObject *Sender, int ACol, int ARow,
		  bool &CanSelect)
{
	if (ARow != 5 && ARow != 3 && ARow != 7)
	{
	  CanSelect = false;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormCreate(TObject *Sender)
{
	/* Open an instance. */
	TCustomIniFile* SettingsFile = new TIniFile(ChangeFileExt(Application->ExeName, ".ini"));


	try
	{
		/*
		Read all saved values from the last session. The section name
		is the name of the form. Also, use the form's properties as defaults.
		*/
		Language     = SettingsFile->ReadInteger("ApplicationLanguage", "Language", Language );
	}
	catch(Exception* e)
	{
	}

	delete SettingsFile;
  // сначала custom рецепты по умолчанию
 load_default_recipes();
	 // имена рецептов переносим
	 Form1->CustomTabSheet0->Caption = "(4) " + AnsiString( (char *)Recipes.current[0].name,16);
	 Form1->CustomTabSheet1->Caption = "(5) " + AnsiString( (char *)Recipes.current[1].name,16);
	 Form1->CustomTabSheet2->Caption = "(6) " + AnsiString( (char *)Recipes.current[2].name,16);
	 Form1->CustomTabSheet3->Caption = "(7) " + AnsiString( (char *)Recipes.current[3].name,16);

	SetLanguage(Language);

// Инициализация картинок режима Ручное управление
	Images[0]  = Image0;
	Images[1]  = Image1;
	Images[2]  = Image2;
	Images[3]  = Image3;
	Images[4]  = Image4;
	Images[5]  = Image5;
	Images[6]  = Image6;
	Images[7]  = Image7;
	Images[8]  = Image8;
	Images[9]  = Image9;
	Images[10]  = Image10;
	Images[11]  = Image11;
	Images[12]  = Image12;
		Images[13]  = ImageE;
		Images[14]  = ImageP;
		Images[15]  = ImageBlank;

	SetConnectedStatus(DISCONNECTED);

		 Form1->CustomRecipe0Grid->ColWidths[0] =25;
		 Form1->CustomRecipe0Grid->ColWidths[1] =540;
		  Form1->CustomRecipe0Grid->HideColumn(2);
		  Form1->CustomRecipe0Grid->HideColumn(3);
		  Form1->CustomRecipe0Grid->HideColumn(4);
		  Form1->CustomRecipe0Grid->HideColumn(5);
		heatGroupBox->Top = 454;
		heatGroupBox->Left = 16;
		FoamSensorGroupBox->Top = 454;
		FoamSensorGroupBox->Left = 16;
		FoamSensorSpinEdit->MaxValue =  NUMBER_OF_STEPS_IN_RECIPE+1;
		signalGroupBox->Top = 454;
		signalGroupBox->Left = 16;
		pauseGroupBox->Top = 454;
		pauseGroupBox->Left = 16;
		heatGroupBox->Visible = false;
		signalGroupBox->Visible = false;
		pauseGroupBox->Visible = false;
 /*
		 TraditionRecipeGrid->Cells[0][0] = "Этап 1. Мощность нагревателя, %.";
		 TraditionRecipeGrid->Cells[0][1] = "Этап 2. Мощность нагревателя, %.";
		 TraditionRecipeGrid->Cells[0][2] = "Этап 3. Температура приготовления, град С.";
		 TraditionRecipeGrid->Cells[0][3] = "Этап 3. Время приготовления, сек.";
		 TraditionRecipeGrid->Cells[0][4] = "Этап 4. Мощность на этапе подъема пенки, %.";
		 TraditionRecipeGrid->Cells[0][5] = "Этап 5. Длительность импульса нагревателя, сек.";
		 TraditionRecipeGrid->Cells[0][6] = "Этап 5. Количество импульсов нагревателя.";
		 TraditionRecipeGrid->Cells[0][7] = "Этап 5. Пауза между импульсами нагревателя, сек.";
		 TraditionRecipeGrid->Cells[0][8] = "Чувствительность датчика подъема пенки.";
 */
 // настройки по умолчанию

	Tradition_Recipe_Set_Default_Values();
	MainPageControl->TabIndex = 0;

	 CurrentChart = TraditionChart;
	 TraditionRecipeSettingsToTrackBar();
	 TraditionRecipeSettingsToGrid();
	 Create_Tradition_Recipe_Series();
	 AllScaleButton->Click();

//	ConnectButton->Click();    // Пытаемся установить связь
	UpdateDeviceControlSettings(CLEAR);      // пока стираем все в режиме Ручное управление

}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void __fastcall	 TraditionRecipeTrackBarToSettings(void)
{
	recipe_settings.current.Coffee_Initial_Power_Level = (Form1->AdvTrackBar2->Position/10)*10;
	recipe_settings.current.Coffee_Low_Power_level = (Form1->AdvTrackBar3->Position/10)*10;
   recipe_settings.current.Coffee_Target_Temperature = Form1->AdvTrackBar4->Position;
   recipe_settings.current.Coffee_Middle_Power_level =   (Form1->AdvTrackBar6->Position/10)*10;
   recipe_settings.current.Coffee_Impulses_Number = Form1->AdvTrackBar8->Position;
   recipe_settings.current.Coffee_Boiling_Threshold = Form1->AdvTrackBar10->Position;
}
//---------------------------------------------------------------------------
void __fastcall	 TraditionRecipeSettingsToTrackBar(void)
{
 SettingsToTrackBarInProgress = true;
 Form1->AdvTrackBar2->Position  = recipe_settings.current.Coffee_Initial_Power_Level;
 Form1->AdvTrackBar3->Position  = recipe_settings.current.Coffee_Low_Power_level;
 Form1->AdvTrackBar4->Position  = recipe_settings.current.Coffee_Target_Temperature;
 Form1->AdvTrackBar6->Position  = recipe_settings.current.Coffee_Middle_Power_level;
 Form1->AdvTrackBar8->Position  = recipe_settings.current.Coffee_Impulses_Number;
 Form1->AdvTrackBar10->Position  = recipe_settings.current.Coffee_Boiling_Threshold;
  SettingsToTrackBarInProgress = false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TraditionRecipeGridCellsChanged(TObject *Sender, TRect &R)
{
if (!SettingsToTrackBarInProgress)
 {
	TraditionRecipeGridToSettings();
	Create_Tradition_Recipe_Series();
  }
}

//---------------------------------------------------------------------------
 void __fastcall	  Create_Custom_Recipe_Series(void)
 {
	  int  step;
	  int chart_point_number = -1;      // чтобы считать с 1 - 0я точка - исходное состояние - создается две точки
	  int points_quantity;

   Form1->Recipe_1_Power_Series->Clear();
   Form1->Recipe_1_Temperature_Series->Clear();
   time_beg = 0;
   temp_beg = temp_beg_global;
   first_point = 1;
   FoamSensorOn = false;

	 for (step=0;step<NUMBER_OF_STEPS_IN_RECIPE && Recipes.current[CurrentRecipe].Step[step].Temperature != END_R; step++)
	 {
		if (Recipes.current[CurrentRecipe].Step[step].Temperature == FOAM_SENSOR_ON_R)  FoamSensorOn = true;
		if (Recipes.current[CurrentRecipe].Step[step].Temperature == FOAM_SENSOR_OFF_R)  FoamSensorOn = false;
		if ((Recipes.current[CurrentRecipe].Step[step].Power && Recipes.current[CurrentRecipe].Step[step].Temperature != BUZZER_R) ||
		Recipes.current[CurrentRecipe].Step[step].Temperature == PAUSE_R)   // Точки только если нагрев и пауза
		{
			points_quantity = Add_Recipe_Step_Points(Form1->Recipe_1_Power_Series,Form1->Recipe_1_Temperature_Series,
			Recipes.current[CurrentRecipe].Step[step].Temperature,Recipes.current[CurrentRecipe].Step[step].Power,Recipes.current[CurrentRecipe].Step[step].Time,1);
		  	chart_point_number += points_quantity;
			Form1->CustomRecipe0Grid->Ints[5][step] = chart_point_number;   // номер точки графика для функции указания текущей точки при клике на строку
		}
		else
		{
			Form1->CustomRecipe0Grid->Ints[5][step] = chart_point_number;   // номер точки графика для функции указания текущей точки при клике на строку
        }
	 }
 }
//---------------------------------------------------------------------------

 void __fastcall CustomRecipeGridToSettings(void)
 {
	  int  step;


	 for (step=0;step<Form1->CustomRecipe0Grid->RowCount && step < NUMBER_OF_STEPS_IN_RECIPE-1; step++)
	 {
		   Recipes.current[CurrentRecipe].Step[step].Temperature = Form1->CustomRecipe0Grid->Ints[2][step];
		   Recipes.current[CurrentRecipe].Step[step].Power = Form1->CustomRecipe0Grid->Ints[3][step];
		   Recipes.current[CurrentRecipe].Step[step].Time = Form1->CustomRecipe0Grid->Ints[4][step];
	 }
		   Recipes.current[CurrentRecipe].Step[step].Temperature = END_R;
		   Recipes.current[CurrentRecipe].Step[step].Power = 0;
		   Recipes.current[CurrentRecipe].Step[step].Time = 0;
 }
//---------------------------------------------------------------------------

 void __fastcall CustomRecipeSettingsToGrid(void)
 {
	  int  step;

	 for (step=0;step<NUMBER_OF_STEPS_IN_RECIPE && Recipes.current[CurrentRecipe].Step[step].Temperature != END_R; step++)
	 {
		  Form1->CustomRecipe0Grid->Cells[0][step] = IntToStr ((int)(step+1));
		  Form1->CustomRecipe0Grid->ReadOnly[1][step] = true;
		  CustomRecipeSettingsToGridRow(step);
		  Form1->CustomRecipe0Grid->RowCount = step+1;
	 }
	 Form1->CustomRecipe0Grid->RowCount = step;
	 // имена рецептов переносим
	 Form1->CustomTabSheet0->Caption = "(4) " + AnsiString( (char *)Recipes.current[0].name,16);
	 Form1->CustomTabSheet1->Caption = "(5) " + AnsiString( (char *)Recipes.current[1].name,16);
	 Form1->CustomTabSheet2->Caption = "(6) " + AnsiString( (char *)Recipes.current[2].name,16);
	 Form1->CustomTabSheet3->Caption = "(7) " + AnsiString( (char *)Recipes.current[3].name,16);

 }

//---------------------------------------------------------------------------

void __fastcall TForm1::CustomRecipe0GridClickCell(TObject *Sender, int ARow, int ACol)

{   int x,y, xVal, max_min_difference;
	   CurrentChart->Canvas->Pen->Color = clBlue;
	   CurrentChart->Canvas->Pen->Width = 2;
	   CurrentChart->Canvas->Pen->Style = psSolid;
	   CurrentChart->Canvas->Brush->Style = bsClear;
	   // если не видно точки, перемещаем график  по Х
	   max_min_difference =  CurrentChart->BottomAxis->Maximum - CurrentChart->BottomAxis->Minimum;
	   xVal = Recipe_1_Temperature_Series->XValue[CustomRecipe0Grid->Ints[5][ARow]];
	   if (xVal > CurrentChart->BottomAxis->Maximum)
	   {
			CurrentChart->BottomAxis->Maximum += xVal - CurrentChart->BottomAxis->Maximum + 20*SEC;
			CurrentChart->BottomAxis->Minimum = CurrentChart->BottomAxis->Maximum - max_min_difference;
	   }
	   if (xVal < CurrentChart->BottomAxis->Minimum)
	   {
			CurrentChart->BottomAxis->Minimum -=   CurrentChart->BottomAxis->Minimum - xVal + 20*SEC;
			CurrentChart->BottomAxis->Maximum = CurrentChart->BottomAxis->Minimum + max_min_difference;
	   }
	   // если не видно точки, перемещаем график  по Y
	   max_min_difference =  CurrentChart->LeftAxis->Maximum - CurrentChart->LeftAxis->Minimum;
	   xVal = Recipe_1_Temperature_Series->YValue[CustomRecipe0Grid->Ints[5][ARow]];
	   if (xVal > CurrentChart->LeftAxis->Maximum)
	   {
			CurrentChart->LeftAxis->Maximum += xVal - CurrentChart->LeftAxis->Maximum + 10;
			CurrentChart->LeftAxis->Minimum = CurrentChart->LeftAxis->Maximum - max_min_difference;
	   }
	   if (xVal < CurrentChart->LeftAxis->Minimum)
	   {
			CurrentChart->LeftAxis->Minimum -=   CurrentChart->LeftAxis->Minimum - xVal + 10;
			CurrentChart->LeftAxis->Maximum = CurrentChart->LeftAxis->Minimum + max_min_difference;
	   }
	   CurrentChart->Repaint();   // затираем предыдущие метки
       // рисуем метку на текущей точке
	   x = CurrentChart->Axes->Bottom->CalcXPosValue(Recipe_1_Temperature_Series->XValue[CustomRecipe0Grid->Ints[5][ARow]]);
	   y = CurrentChart->Axes->Left->CalcYPosValue(Recipe_1_Temperature_Series->YValue[CustomRecipe0Grid->Ints[5][ARow]]);
		CurrentChart->Canvas->Ellipse(x-5,y-5,x+5,y+5);




}
//---------------------------------------------------------------------------

void __fastcall TForm1::CustomTabSheet0Show(TObject *Sender)
{
 CustomPanel->Parent = CustomTabSheet0;
 CurrentRecipe = 0;
 CurrentChart = CustomChart;
 CustomRecipe0TrackBar1->Position = volume;
 CustomRecipe0TrackBar2->Position = temp_beg_global;
 Create_Custom_Recipe_Series();

}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

 void __fastcall TForm1:: CustomRecipeSettingsToBox(void)
 {
 CustomRecipeSettingsToBoxInProgress = true;

	 if (Recipes.current[CurrentRecipe].Step[CurrentStep].Power && Recipes.current[CurrentRecipe].Step[CurrentStep].Temperature <= 100)
	{
	   OperationComboBox->ItemIndex = HEATING_ITEM_INDEX;
		PowerSpinEdit->Value =  Recipes.current[CurrentRecipe].Step[CurrentStep].Power;
		TemperatureSpinEdit->Value =  Recipes.current[CurrentRecipe].Step[CurrentStep].Temperature;
		TimeDropdown->Value =  (float)(Recipes.current[CurrentRecipe].Step[CurrentStep].Time)/SEC;
		heatGroupBox->Visible = true;
		signalGroupBox->Visible = false;
		pauseGroupBox->Visible = false;
		FoamSensorGroupBox->Visible = false;
	}
	  if (Recipes.current[CurrentRecipe].Step[CurrentStep].Temperature == BUZZER_R)
	  {
		 OperationComboBox->ItemIndex = BUZZER_ITEM_INDEX;
		SignalSeriesSpinEdit->Value =  Recipes.current[CurrentRecipe].Step[CurrentStep].Power;
		SignalTicksSpinEdit->Value =  Recipes.current[CurrentRecipe].Step[CurrentStep].Time;
		heatGroupBox->Visible = false;
		signalGroupBox->Visible = true;
		pauseGroupBox->Visible = false;
		FoamSensorGroupBox->Visible = false;
	  }
	  if (Recipes.current[CurrentRecipe].Step[CurrentStep].Temperature == PAUSE_R)
	  {
		 OperationComboBox->ItemIndex = PAUSE_ITEM_INDEX;
		PauseTimeDropdown->Value =  (float)(Recipes.current[CurrentRecipe].Step[CurrentStep].Time/SEC);
		heatGroupBox->Visible = false;
		signalGroupBox->Visible = false;
		pauseGroupBox->Visible = true;
		FoamSensorGroupBox->Visible = false;
	  }
	  if (Recipes.current[CurrentRecipe].Step[CurrentStep].Temperature == FOAM_SENSOR_ON_R)
	  {
		 OperationComboBox->ItemIndex = FOAM_SENSOR_ON_ITEM_INDEX;
		 FoamSensorGroupBox->Visible = true;
		 FoamSensorSpinEdit->Value =  Recipes.current[CurrentRecipe].Step[CurrentStep].Time;
		 ThresholdSpinEdit->Value =  Recipes.current[CurrentRecipe].Step[CurrentStep].Power;
		heatGroupBox->Visible = false;
		signalGroupBox->Visible = false;
		pauseGroupBox->Visible = false;
	  }
	  if (Recipes.current[CurrentRecipe].Step[CurrentStep].Temperature == FOAM_SENSOR_OFF_R)
	  {
		 OperationComboBox->ItemIndex = FOAM_SENSOR_OFF_ITEM_INDEX;
  		heatGroupBox->Visible = false;
		signalGroupBox->Visible = false;
		pauseGroupBox->Visible = false;
		FoamSensorGroupBox->Visible = false;
	  }

  CustomRecipeSettingsToBoxInProgress = false;
 }
//---------------------------------------------------------------------------

void __fastcall TForm1::TimeButtonClick(TObject *Sender)
{
 // command = TIME_BUTTON_PRESS;
  AddCommand(TIME_BUTTON_PRESS);
 attempt = ATTEMPTS_NUMBER;
 Form1->Label1->Caption = "Send...";

}
//---------------------------------------------------------------------------

void __fastcall TForm1::InsertStepButtonClick(TObject *Sender)
{
 int step = CurrentStep;

 if (CustomRecipe0Grid->RowCount >= NUMBER_OF_STEPS_IN_RECIPE-1)
 {
	 if (Language == 1)
	 {
	  MessageDlgPos("Количество шагов рецепта может быть не более " +IntToStr(NUMBER_OF_STEPS_IN_RECIPE-1), mtError,  TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
	 }
	 else
	 {
	 MessageDlgPos("The number of steps in the recipe can be no more than " +IntToStr(NUMBER_OF_STEPS_IN_RECIPE-1), mtError,  TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
	 }
  return;
 }

 	CustomRecipe0Grid->InsertRows(CurrentStep, 1);
	  CustomRecipe0Grid->Ints[2][CurrentStep] = 80;
	  CustomRecipe0Grid->Ints[3][CurrentStep] = 50;
	  CustomRecipe0Grid->Ints[4][CurrentStep]  =  SEC*10;
	CustomRecipeGridToSettings();
	CustomRecipeSettingsToGridRow(CurrentStep);
	CustomRecipe0Grid->AutoNumberCol(0);
	Create_Custom_Recipe_Series();
	CurrentStep = step;
	CustomRecipeSettingsToBox();
	CustomRecipeChanged = true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::DeleteStepButtonClick(TObject *Sender)
{
int step = CurrentStep;
 CustomRecipe0Grid->RemoveRows(CurrentStep, 1);
 	CustomRecipeGridToSettings();

	CustomRecipe0Grid->AutoNumberCol(0);
	Create_Custom_Recipe_Series();
	CurrentStep = step;
	CustomRecipeSettingsToBox();
    CustomRecipeChanged = true;

}
//---------------------------------------------------------------------------
 void __fastcall TForm1::CustomRecipeBoxToSettings(void)
 {
  if (CustomRecipeSettingsToBoxInProgress) return;

	 switch (OperationComboBox->ItemIndex)
	 {
	 case HEATING_ITEM_INDEX:
		  Recipes.current[CurrentRecipe].Step[CurrentStep].Temperature =  TemperatureSpinEdit->Value;
		  Recipes.current[CurrentRecipe].Step[CurrentStep].Power =   PowerSpinEdit->Value;
		  Recipes.current[CurrentRecipe].Step[CurrentStep].Time =   SEC*TimeDropdown->Value;
		break;
	 case PAUSE_ITEM_INDEX:
		  Recipes.current[CurrentRecipe].Step[CurrentStep].Temperature =  PAUSE_R;
		  Recipes.current[CurrentRecipe].Step[CurrentStep].Power =   0;
		  Recipes.current[CurrentRecipe].Step[CurrentStep].Time =   SEC*PauseTimeDropdown->Value;
		break;
	 case BUZZER_ITEM_INDEX:
		  Recipes.current[CurrentRecipe].Step[CurrentStep].Temperature =  BUZZER_R;
		  Recipes.current[CurrentRecipe].Step[CurrentStep].Power =   SignalSeriesSpinEdit->Value;
		  Recipes.current[CurrentRecipe].Step[CurrentStep].Time =   SignalTicksSpinEdit->Value;
		break;
	 case FOAM_SENSOR_ON_ITEM_INDEX:
		  Recipes.current[CurrentRecipe].Step[CurrentStep].Temperature =  FOAM_SENSOR_ON_R;
		  Recipes.current[CurrentRecipe].Step[CurrentStep].Power =   ThresholdSpinEdit->Value;
		  Recipes.current[CurrentRecipe].Step[CurrentStep].Time =   FoamSensorSpinEdit->Value;
		break;
	 case FOAM_SENSOR_OFF_ITEM_INDEX:
		  Recipes.current[CurrentRecipe].Step[CurrentStep].Temperature =  FOAM_SENSOR_OFF_R;
		  Recipes.current[CurrentRecipe].Step[CurrentStep].Power =   0;
		  Recipes.current[CurrentRecipe].Step[CurrentStep].Time =   0;
		break;
	 }
 }

void __fastcall TForm1::ApplyInputValues(TObject *Sender)
{
int step = CurrentStep;

	CheckInputValues(Sender);

	if (!CustomRecipeSettingsToBoxInProgress)
	{
			CustomRecipeBoxToSettings();
			CustomRecipeSettingsToGridRow(CurrentStep);
			Create_Custom_Recipe_Series();
	}

	CustomRecipe0Grid->SelectRows(step,1);         // Чтобы не уходить с текущей строки
	CustomRecipeSettingsToBox();
    CustomRecipeChanged = true;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::CustomRecipe0GridSelectCell(TObject *Sender, int ACol, int ARow,
          bool &CanSelect)
{
int x,y;
	CurrentStep = ARow;
	CustomRecipeSettingsToBox();

}
//---------------------------------------------------------------------------
 void __fastcall CustomRecipeSettingsToGridRow(int CurrentRow)
 {
	 if (Language == 1)
	 {
	  if (Recipes.current[CurrentRecipe].Step[CurrentRow].Power)
	  {
		Form1->CustomRecipe0Grid->Cells[1][CurrentRow] = "Нагрев, мощность " + IntToStr ((int)Recipes.current[CurrentRecipe].Step[CurrentRow].Power)+ "%" ;
		if (Recipes.current[CurrentRecipe].Step[CurrentRow].Temperature)
			Form1->CustomRecipe0Grid->Cells[1][CurrentRow] += ", до температуры " + IntToStr ((int)Recipes.current[CurrentRecipe].Step[CurrentRow].Temperature) + " град С " ;
		if (Recipes.current[CurrentRecipe].Step[CurrentRow].Time)
			Form1->CustomRecipe0Grid->Cells[1][CurrentRow] += ", продолжительность " + FloatToStrF((float)(Recipes.current[CurrentRecipe].Step[CurrentRow].Time)/SEC, ffFixed, 3, 2) + " сек";
		Form1->CustomRecipe0Grid->Cells[1][CurrentRow] += ".";
	  }


	  if (Recipes.current[CurrentRecipe].Step[CurrentRow].Temperature == FOAM_SENSOR_ON_R)
	  {
		Form1->CustomRecipe0Grid->Cells[1][CurrentRow] = "Датчик пенки включить, после срабатывания перейти на шаг "
			 + IntToStr ((int)Recipes.current[CurrentRecipe].Step[CurrentRow].Time);
			 if (Recipes.current[CurrentRecipe].Step[CurrentRow].Power)
			 {
				 Form1->CustomRecipe0Grid->Cells[1][CurrentRow] += ", чувствительность "
				 + IntToStr ((int)Recipes.current[CurrentRecipe].Step[CurrentRow].Power);
			 }
	  }
	  if (Recipes.current[CurrentRecipe].Step[CurrentRow].Temperature == FOAM_SENSOR_OFF_R)
			Form1->CustomRecipe0Grid->Cells[1][CurrentRow] = "Датчик пенки выключить";
	  if (Recipes.current[CurrentRecipe].Step[CurrentRow].Temperature == BUZZER_R)
			Form1->CustomRecipe0Grid->Cells[1][CurrentRow] = "Включить сигнал - " + IntToStr ((int)Recipes.current[CurrentRecipe].Step[CurrentRow].Power)
			+ " раз по " + IntToStr ((int)Recipes.current[CurrentRecipe].Step[CurrentRow].Time);
	  if (Recipes.current[CurrentRecipe].Step[CurrentRow].Temperature == PAUSE_R)
		 Form1->CustomRecipe0Grid->Cells[1][CurrentRow] = "Пауза " + FloatToStr ((float)(Recipes.current[CurrentRecipe].Step[CurrentRow].Time/SEC)) + " сек.";

	  Form1->CustomRecipe0Grid->Ints[2][CurrentRow] = Recipes.current[CurrentRecipe].Step[CurrentRow].Temperature;
	  Form1->CustomRecipe0Grid->Ints[3][CurrentRow] = Recipes.current[CurrentRecipe].Step[CurrentRow].Power;
	  Form1->CustomRecipe0Grid->Ints[4][CurrentRow]  =  Recipes.current[CurrentRecipe].Step[CurrentRow].Time;
	 }
	 else
	 {
	  if (Recipes.current[CurrentRecipe].Step[CurrentRow].Power)
	  {
		Form1->CustomRecipe0Grid->Cells[1][CurrentRow] = "Heating, power " + IntToStr ((int)Recipes.current[CurrentRecipe].Step[CurrentRow].Power)+ "%" ;
		if (Recipes.current[CurrentRecipe].Step[CurrentRow].Temperature)
			Form1->CustomRecipe0Grid->Cells[1][CurrentRow] += ", to temperature " + IntToStr ((int)Recipes.current[CurrentRecipe].Step[CurrentRow].Temperature) + " deg С " ;
		if (Recipes.current[CurrentRecipe].Step[CurrentRow].Time)
			Form1->CustomRecipe0Grid->Cells[1][CurrentRow] += ", duration " + FloatToStrF((float)(Recipes.current[CurrentRecipe].Step[CurrentRow].Time)/SEC, ffFixed, 3, 2) + " sec";
		Form1->CustomRecipe0Grid->Cells[1][CurrentRow] += ".";
	  }


	  if (Recipes.current[CurrentRecipe].Step[CurrentRow].Temperature == FOAM_SENSOR_ON_R)
	  {
		Form1->CustomRecipe0Grid->Cells[1][CurrentRow] = "Switch on the crema sensor, after triggering go to step "
			 + IntToStr ((int)Recipes.current[CurrentRecipe].Step[CurrentRow].Time);
			 if (Recipes.current[CurrentRecipe].Step[CurrentRow].Power)
			 {
				 Form1->CustomRecipe0Grid->Cells[1][CurrentRow] += ", sensitivity "
				 + IntToStr ((int)Recipes.current[CurrentRecipe].Step[CurrentRow].Power);
			 }
	  }
	  if (Recipes.current[CurrentRecipe].Step[CurrentRow].Temperature == FOAM_SENSOR_OFF_R)
			Form1->CustomRecipe0Grid->Cells[1][CurrentRow] = "Switch off the crema sensor";
	  if (Recipes.current[CurrentRecipe].Step[CurrentRow].Temperature == BUZZER_R)
			Form1->CustomRecipe0Grid->Cells[1][CurrentRow] = "Enable signal - " + IntToStr ((int)Recipes.current[CurrentRecipe].Step[CurrentRow].Power)
			+ " times in " + IntToStr ((int)Recipes.current[CurrentRecipe].Step[CurrentRow].Time);
	  if (Recipes.current[CurrentRecipe].Step[CurrentRow].Temperature == PAUSE_R)
		 Form1->CustomRecipe0Grid->Cells[1][CurrentRow] = "Pause " + FloatToStr ((float)(Recipes.current[CurrentRecipe].Step[CurrentRow].Time/SEC)) + " sec.";

	  Form1->CustomRecipe0Grid->Ints[2][CurrentRow] = Recipes.current[CurrentRecipe].Step[CurrentRow].Temperature;
	  Form1->CustomRecipe0Grid->Ints[3][CurrentRow] = Recipes.current[CurrentRecipe].Step[CurrentRow].Power;
	  Form1->CustomRecipe0Grid->Ints[4][CurrentRow]  =  Recipes.current[CurrentRecipe].Step[CurrentRow].Time;
	 }
 }

void __fastcall TForm1::SaveToFileCustomRecipeButtonClick(TObject *Sender)
{
SaveDialog1->Filter = "CSV files (*.csv)|*.csv";

SaveDialog1->FileName = Form1->MainPageControl->ActivePage->Caption.SubString(4,20) + ".csv";

  if (SaveDialog1->Execute())
  {
//	if (FileExists(SaveDialog1->FileName))
//		throw(Exception("File already exists. Cannot overwrite."));
//	else
		  Form1->CustomRecipe0Grid->UnHideColumn(2);
		  Form1->CustomRecipe0Grid->UnHideColumn(3);
		  Form1->CustomRecipe0Grid->UnHideColumn(4);
		  Form1->CustomRecipe0Grid->UnHideColumn(5);

	  CustomRecipe0Grid->SaveToCSV(SaveDialog1->FileName);
	  AnsiString RecipeName = 	ExtractFileName(SaveDialog1->FileName);
	  AnsiString RecipeNum = Form1->MainPageControl->ActivePage->Caption.SubString(0,4);   // префикс названия вкладки - номер рецепта(0)
	  RecipeName = RecipeName.SubString(0,RecipeName.Pos(".") - 1);       // выделяем  имя без расширения
	  Form1->MainPageControl->ActivePage->Caption = RecipeNum + RecipeName;
	  strncpy((char *)Recipes.current[CurrentRecipe].name, RecipeName.c_str(),RECIPE_NAME_LENGTH-1);       // Копируем в поле текущего рецепта
		  Form1->CustomRecipe0Grid->HideColumn(2);
		  Form1->CustomRecipe0Grid->HideColumn(3);
		  Form1->CustomRecipe0Grid->HideColumn(4);
		  Form1->CustomRecipe0Grid->HideColumn(5);

  }

}
//---------------------------------------------------------------------------
void __fastcall TForm1::ReadFromFileCustomRecipeButtonClick(TObject *Sender)
{
 int Result = mrOk;
	OpenDialog1->Filter = "CSV files (*.csv)|*.csv";
	OpenDialog1->FileName = Form1->MainPageControl->ActivePage->Caption + ".csv";

	 if (CustomRecipeChanged)
	{
		MessageForm->Left = Form1->Left + (Form1->ClientWidth - MessageForm->Width)/2;        // MessageForm - по центру
		MessageForm->Top = Form1->Top + (Form1->ClientHeight - MessageForm->Height - (MessageForm->Height/3))/2;
	   Result = MessageForm->ShowModal();
	}
	if (Result == mrOk)
	{
	  if (OpenDialog1->Execute())
	  {
		if (FileExists(OpenDialog1->FileName))
		{
		  Form1->CustomRecipe0Grid->UnHideColumn(2);
		  Form1->CustomRecipe0Grid->UnHideColumn(3);
		  Form1->CustomRecipe0Grid->UnHideColumn(4);
		  Form1->CustomRecipe0Grid->UnHideColumn(5);

			CustomRecipe0Grid->LoadFromCSV(OpenDialog1->FileName);
			CustomRecipeChanged = true;
		  Form1->CustomRecipe0Grid->HideColumn(2);
		  Form1->CustomRecipe0Grid->HideColumn(3);
		  Form1->CustomRecipe0Grid->HideColumn(4);
		  Form1->CustomRecipe0Grid->HideColumn(5);

		}
		else
			throw(Exception("File does not exist."));
	  }

	  AnsiString RecipeName = 	ExtractFileName(OpenDialog1->FileName);
	  AnsiString RecipeNum = Form1->MainPageControl->ActivePage->Caption.SubString(0,4);   // префикс названия вкладки - номер рецепта(0)
	  RecipeName = RecipeName.SubString(0,RecipeName.Pos(".") - 1);       // выделяем  имя без расширения
	  Form1->MainPageControl->ActivePage->Caption = RecipeNum + RecipeName;
	  strncpy((char *)Recipes.current[CurrentRecipe].name, RecipeName.c_str(),RECIPE_NAME_LENGTH-1);       // Копируем в поле текущего рецепта

	  CustomRecipeGridToSettings();
	  Create_Custom_Recipe_Series();
	  CustomRecipeSettingsToGrid();
	  AllScaleButton->Click();

    }
}
//---------------------------------------------------------------------------




void __fastcall TForm1::PlusScaleButtonClick(TObject *Sender)
{
	CurrentChart->ZoomPercent(110);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::MinusScaleButtonClick(TObject *Sender)
{
    CurrentChart->ZoomPercent(90);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CustomChartClickSeries(TCustomChart *Sender, TChartSeries *Series,
		  int ValueIndex, TMouseButton Button, TShiftState Shift, int X,
          int Y)
{
int ARow;
		if (Series != Recipe_1_Temperature_Series) return;
 // фокус на шаг в гриде

	for (ARow = 0; ARow < NUMBER_OF_STEPS_IN_RECIPE && ARow < CustomRecipe0Grid->RowCount; ARow++) {
	   if (CustomRecipe0Grid->Ints[5][ARow] == ValueIndex+1)
	   {
			CustomRecipe0Grid->Row = ARow;
			break;
	   }
	}

}
//---------------------------------------------------------------------------


void __fastcall TForm1::CustomRecipe0TrackBar1Change(TObject *Sender)
{
			volume  = CustomRecipe0TrackBar1->Position;
			temp_beg_global =  CustomRecipe0TrackBar2->Position;
  switch (MainPageControl->TabIndex)
  {
	case 0:
			 if (!SettingsToTrackBarInProgress)
			 {
				TraditionRecipeTrackBarToSettings();
				 TraditionRecipeSettingsToGrid();
				Create_Tradition_Recipe_Series();
			  }
			 break;
	case 1:
	case 2:
	case 3:
	case 4:
			Create_Custom_Recipe_Series();
			 break;
  }

}
//---------------------------------------------------------------------------


void __fastcall TForm1::MainPageControlChange(TObject *Sender)
{
  switch (MainPageControl->TabIndex)
  {
	case 0:
			 liquid = COFFEE;
			 break;
	case 1:
			 CurrentRecipe = 0;
			 liquid = RECIPE_0;      // для передачи в устройство через команду RUN_RECIPE
			 break;
	case 2:
			 CurrentRecipe = 1;
			 liquid = RECIPE_1;
			 break;
	case 3:
			 CurrentRecipe = 2;
			 liquid = RECIPE_2;
			 break;
	case 4:
			 CurrentRecipe = 3;
			 liquid = RECIPE_3;
			 break;
  }

  if (MainPageControl->TabIndex == 0)
  {
	 CurrentChart = TraditionChart;
	 TraditionRecipeSettingsToTrackBar();
	 TraditionRecipeSettingsToGrid();
	 Create_Tradition_Recipe_Series();
	 AllScaleButton->Click();
  }
  else
  {
	 CurrentChart = CustomChart;
	 Create_Custom_Recipe_Series();
	 CustomRecipeSettingsToGrid();

	 AllScaleButton->Click();
  }
			 CustomRecipe0TrackBar1->Position = volume;
			 CustomRecipe0TrackBar2->Position = temp_beg_global;
  switch (MainPageControl->TabIndex)
  {
	case 1:
			 CustomPanel->Parent = CustomTabSheet0;
			 break;
	case 2:
			 CustomPanel->Parent = CustomTabSheet1;
			 break;
	case 3:
			 CustomPanel->Parent = CustomTabSheet2;
			 break;
	case 4:
			 CustomPanel->Parent = CustomTabSheet3;
			 break;
  }

}
//---------------------------------------------------------------------------



void __fastcall TForm1::AllScaleButtonClick(TObject *Sender)
{
		if (CurrentChart->BottomAxis->Minimum < TimeAxisMaximum)       // max всегда дб > min
	{
		CurrentChart->BottomAxis->Maximum = TimeAxisMaximum + 10*SEC;
		CurrentChart->BottomAxis->Minimum =  0;
	}
	else
	{
		CurrentChart->BottomAxis->Minimum =  0;
		CurrentChart->BottomAxis->Maximum = TimeAxisMaximum + 10*SEC;
	}

	if (CurrentChart->LeftAxis->Minimum < 110)       // max всегда дб > min
	{
		CurrentChart->LeftAxis->Maximum = 110;
		CurrentChart->LeftAxis->Minimum =  0;
	}
	else
	{
		CurrentChart->LeftAxis->Minimum =  0;
		CurrentChart->LeftAxis->Maximum = 110;
	}

}
//---------------------------------------------------------------------------


void __fastcall TForm1::ConnectButtonClick(TObject *Sender)
{
   if (!IsConnected)       // Пытаемся установить связь
   {
			mCharacteristic_found = 0;
			Memo1->Clear();
			BadData = 0;
			ConnectButton->Enabled = false;
//			ConnectButton->Caption =  "Ищем устройство...";
			ProcessForm->Left = Form1->Left + (Form1->ClientWidth - ProcessForm->Width)/2;        // ProcessForm - по центру
			ProcessForm->Top = Form1->Top + (Form1->ClientHeight - ProcessForm->Height - (ProcessForm->Height/3))/2;
			if (!ProcessForm->Visible)     // Выводим окно с сообщением о процессе чтения
			{
				 if (Language == 1)
				 {
					ProcessForm->Label1->Caption = "Ищем устройство...";
				 }
				 else
				 {
					ProcessForm->Label1->Caption = "Looking for a device ...";
				 }
				ProcessForm->Show();
			}

		  wclBluetoothManager1->Open();

		 TwclBluetoothRadio* Radio = GetRadio();
		  if (Radio != NULL)
		  {
			int Res = Radio->Discover(3, dkBle);
			if (Res != WCL_E_SUCCESS)
			{
				SetConnectedStatus(DISCONNECTED);
				 if (Language == 1)
				 {
					  MessageDlgPos("Ошибка начала поиска: 0x" + IntToHex(Res, 8),mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				 else
				 {
					  MessageDlgPos("Error starting discovering: 0x" + IntToHex(Res, 8),mtError, TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
			}
		  }
   }
	else           // Разрываем связь
   {
		SetConnectedStatus(DISCONNECTED);
		wclBluetoothManager1->Close();
   }

}
//---------------------------------------------------------------------------
void __fastcall	  TForm1::SetConnectedStatus(bool connected)
{
   if (connected == DISCONNECTED)
   {
	IsConnected = false;
				 if (Language == 1)
				 {
					ConnectButton->Hint = "Нет связи с устройством - установить связь";
				 }
				 else
				 {
					ConnectButton->Hint = "No connection to device - establish connection";
				 }
	ConnectButton->ImageIndex = DISCONNECTED;
	Form1->ConnectImage->Picture = Form1->GrayImage->Picture;
	ClearCommandStack();         // обнуляем очередь команд
	CheckStatusTimer->Enabled = false;
	ConnectButton->Enabled = true;

	StateMachineTimer->Interval = 20;
	StateMachineTimer->Enabled = true;

   }
	else
   {
	IsConnected = true;
				 if (Language == 1)
				 {
					ConnectButton->Hint = "Связь установлена - разорвать связь";
				 }
				 else
				 {
					ConnectButton->Hint = "Connection established - break connection";
				 }
	ConnectButton->ImageIndex = CONNECTED;
    Form1->ConnectImage->Picture = Form1->RedImage->Picture;
//     VM_state = VM_STOP;
	StateMachineTimer->Interval = 20;
	StateMachineTimer->Enabled = true;
	CheckStatusTimer->Interval = 5000;
	CheckStatusTimer->Enabled = true;
	ConnectButton->Enabled = true;
	if (MainPageControl->TabIndex == 5) CheckStatusTimer->Interval = 1000;    // вкладка ручное управление, чаще пингуем

   	ClearCommandStack();         // обнуляем очередь команд

   }

   UpdateDeviceControlSettings(CLEAR);
   if (ProcessForm != NULL)	if (ProcessForm->Visible) ProcessForm->Close();

   NoConnectionProcessing = false;
}
//---------------------------------------------------------------------------
// Возвращает текущую команду на выполнение StateMashine
int  __fastcall	  Command(void)
{
	if (CommandIndex <= StackBottomIndex)
		return CommandStack[CommandIndex];
	else
		return NO_COMMAND;
}
//---------------------------------------------------------------------------
void  __fastcall NextCommand(void)
{
	int current_command;

	 if (CommandIndex < COMMAND_STACK_LENGTH) CommandIndex++;
	if (CommandIndex > StackBottomIndex)     // то есть до вызова NextCommand() это  уже была последняя команда - очищаем стек
	{
		ClearCommandStack();
	}
}
//---------------------------------------------------------------------------
void  __fastcall ClearCommandStack(void)
{
	CommandIndex = 0;
	StackBottomIndex = -1;
}
//---------------------------------------------------------------------------
void  __fastcall AddCommand(int command)
{
	if (StackBottomIndex < COMMAND_STACK_LENGTH-1)
	{
		StackBottomIndex++;
		CommandStack[StackBottomIndex] = command;
	}

}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckStatusTimerTimer(TObject *Sender)
{
// периодическая проверка связи с усройством
	if (!NoConnectionProcessing)
	{
		AddCommand(READ_STATUS);
		 attempt = ATTEMPTS_NUMBER;
		 Form1->Label1->Caption = "Send...";
	}
	 // анимация кнопки для показа активности

}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckInputValues(TObject *Sender)
{
//SignalTicksSpinEdit// не может быть 0
 if (PowerSpinEdit->Value < 10 ) PowerSpinEdit->Value = 10;
 if (PowerSpinEdit->Value > 100 ) PowerSpinEdit->Value = 100;
 //  может быть 0
 if (TemperatureSpinEdit->Value < 0 ) TemperatureSpinEdit->Value = 0;
 if (TemperatureSpinEdit->Value < 40 && TemperatureSpinEdit->Value > 0) TemperatureSpinEdit->Value = 40;
 if (TemperatureSpinEdit->Value > 100 ) TemperatureSpinEdit->Value = 100;
 //  может быть 0
 if (TimeDropdown->Value < 0 ) TimeDropdown->Value = 0;
 if (TimeDropdown->Value > 360000 ) TimeDropdown->Value = 360000;     // 100 часов
 //  может быть 0
 if (PauseTimeDropdown->Value < 0 ) PauseTimeDropdown->Value = 0;
 if (PauseTimeDropdown->Value > 360000 ) PauseTimeDropdown->Value = 360000;     // 100 часов
 //  не может быть 0
 if (ThresholdSpinEdit->Value < 0 ) ThresholdSpinEdit->Value = 1;
 if (ThresholdSpinEdit->Value > 100 ) ThresholdSpinEdit->Value = 100;
 //  не может быть 0
 if (SignalSeriesSpinEdit->Value < 0 ) SignalSeriesSpinEdit->Value = 1;
 if (SignalSeriesSpinEdit->Value > 100 ) SignalSeriesSpinEdit->Value = 100;
 //  не может быть 0
 if (SignalTicksSpinEdit->Value < 0 ) SignalTicksSpinEdit->Value = 1;
 if (SignalTicksSpinEdit->Value > 100 ) SignalTicksSpinEdit->Value = 100;

  if (TemperatureSpinEdit->Value == 0 && TimeDropdown->Value == 0)
  {
	   MessageDlgPos("Температура и время не могут одновременно равняться нулю", mtError,  TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 if (Language == 1)
				 {
	   MessageDlgPos("Температура и время не могут одновременно равняться нулю", mtError,  TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
				 else
				 {
	   MessageDlgPos("Temperature and time cannot be zero at the same time", mtError,  TMsgDlgButtons() << mbOK, 0,Form1->Left + 100,Form1->Top + 100);
				 }
	  TimeDropdown->Value = 10;
	  TemperatureSpinEdit->Value = 40;
  }

}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button1Click(TObject *Sender)
{
	AddCommand(RESET_CONTROLLER);
	 attempt = ATTEMPTS_NUMBER;
	 Form1->Label1->Caption = "Send...";

}
//---------------------------------------------------------------------------


void __fastcall UpdateDeviceControlSettings(char refresh_mode)
{
 int liquid_friendly_number;

	 if (refresh_mode == CLEAR)     // гасим дисплей
	 {
//	  UART_packet.command_packet.left_digit = 15;    // пустая картинка цифры
//	  UART_packet.command_packet.right_digit = 15;
      UART_packet.command_packet.LEDs = 0;
	 }
/*
	if (UART_packet.command_packet.left_digit >= 100)       // Проверяем наличие десятичной точки
	{
	   UART_packet.command_packet.left_digit -= 100;
	   Form1->DigitalPointImage->Visible = true;
	}
	else
	{
	   Form1->DigitalPointImage->Visible = false;
	}
*/
	// Цифры дисплея
//	Form1->RightDigitImage->Picture = Form1->Images[UART_packet.command_packet.right_digit]->Picture;
//	Form1->LeftDigitImage->Picture = Form1->Images[UART_packet.command_packet.left_digit]->Picture;
	// Светодиоды
	if (UART_packet.command_packet.LEDs & BIT0)  Form1->CoffeeLEDImage->Picture = Form1->RedImage->Picture; else  Form1->CoffeeLEDImage->Picture = Form1->GrayImage->Picture;
	if (UART_packet.command_packet.LEDs & BIT1)  Form1->WaterLEDImage->Picture = Form1->RedImage->Picture; else  Form1->WaterLEDImage->Picture = Form1->GrayImage->Picture;
	if (UART_packet.command_packet.LEDs & BIT2)  Form1->MilkLEDImage->Picture = Form1->RedImage->Picture; else  Form1->MilkLEDImage->Picture = Form1->GrayImage->Picture;
	if (UART_packet.command_packet.LEDs & BIT3)  Form1->TimeLEDImage->Picture = Form1->RedImage->Picture; else  Form1->TimeLEDImage->Picture = Form1->GrayImage->Picture;
	if (UART_packet.command_packet.LEDs & BIT4)  Form1->TemperatureLEDImage->Picture = Form1->RedImage->Picture; else  Form1->TemperatureLEDImage->Picture = Form1->GrayImage->Picture;
	if (UART_packet.command_packet.LEDs & BIT5)  Form1->StartLEDImage->Picture = Form1->RedImage->Picture; else  Form1->StartLEDImage->Picture = Form1->GrayImage->Picture;




	 processor_temperature = UART_packet.command_packet.processor_temperature;
	 Form1->Label15->Caption = IntToStr(processor_temperature);
	 current_temperature = UART_packet.command_packet.current_temperature;
	 Form1->Label2->Caption  = IntToStr(current_temperature);
	 Form1->Label11->Caption = IntToStr(current_temperature) ;
	 heater_temperature = UART_packet.command_packet.heater_temperature;
	 Form1->Label16->Caption = IntToStr(heater_temperature);
  //	 Form1->Label10->Caption = IntToStr(UART_packet.command_packet.water_sensor_raw_data);
  // Меняем номер рецепта на внешний, для пользователя
	  switch (UART_packet.command_packet.Recipe_number)
	{
	  case 0: liquid_friendly_number = 4;
	   Form1->RecipeNameLabel->Caption = AnsiString( (char *)Recipes.current[UART_packet.command_packet.Recipe_number].name,16);
	  break;
	  case 1: liquid_friendly_number = 5;
	   Form1->RecipeNameLabel->Caption = AnsiString( (char *)Recipes.current[UART_packet.command_packet.Recipe_number].name,16);
	  break;
	  case 2: liquid_friendly_number = 6;
	   Form1->RecipeNameLabel->Caption = AnsiString( (char *)Recipes.current[UART_packet.command_packet.Recipe_number].name,16);
	  break;
	  case 3: liquid_friendly_number = 7;
	   Form1->RecipeNameLabel->Caption = AnsiString( (char *)Recipes.current[UART_packet.command_packet.Recipe_number].name,16);
	  break;
	  case 4: liquid_friendly_number = 1;
				 if (Language == 1)
				 {
					Form1->RecipeNameLabel->Caption = "КОФЕ ТРАДИЦИОННЫЙ";
				 }
				 else
				 {
					Form1->RecipeNameLabel->Caption = "COFFEE TRADITIONAL";
				 }
	  break;
	  case 5: liquid_friendly_number = 2;
				 if (Language == 1)
				 {
					Form1->RecipeNameLabel->Caption = "ВОДА/КОФЕ БЫСТРЫЙ";
				 }
				 else
				 {
					Form1->RecipeNameLabel->Caption = "WATER/COFFEE FAST";
				 }
	  break;
	  case 6: liquid_friendly_number = 3;
				 if (Language == 1)
				 {
					Form1->RecipeNameLabel->Caption = "МОЛОКО";
				 }
				 else
				 {
					Form1->RecipeNameLabel->Caption = "MILK";
				 }
	  break;
	}

	 Form1->RecipeLabel->Caption = IntToStr(liquid_friendly_number);

	 Form1->TargetTemperaturaLabel->Caption = IntToStr(UART_packet.command_packet.target_temperature);
	 Form1->SoftwareVersionLabel->Caption = IntToStr(UART_packet.command_packet.software_version);
	 Form1->StepLabel->Caption = IntToStr(UART_packet.command_packet.step_number);
	 Form1->CupsNumberLabel->Caption = IntToStr(UART_packet.command_packet.Coffee_Cups_Number);
      // Чтобы поддерживать в актуальном состоянии
	  recipe_settings.current.Coffee_Cups_Number = UART_packet.command_packet.Coffee_Cups_Number;
	  recipe_settings.current.Coffee_Temperature_Correction = UART_packet.command_packet.Coffee_Temperature_Correction;;
	  recipe_settings.current.Coffee_BT_enable = 1;

}
//---------------------------------------------------------------------------


void __fastcall TForm1::HandControlSheetShow(TObject *Sender)
{
	if (IsConnected)
	{
		CheckStatusTimer->Interval = 1000;
		UpdateDeviceControlSettings(DATA);
	}
	else
	{
		UpdateDeviceControlSettings(CLEAR);
    }


}
//---------------------------------------------------------------------------

void __fastcall TForm1::HandControlSheetHide(TObject *Sender)
{
	if (IsConnected)
	{
		CheckStatusTimer->Interval = 5000;
	}

}
//---------------------------------------------------------------------------


void __fastcall TForm1::FormShow(TObject *Sender)
{
	ConnectButton->Click();    // Пытаемся установить связь
}
//---------------------------------------------------------------------------



void __fastcall TForm1::RunRecipeButtonClick(TObject *Sender)
{
 int Result = mrOk;
	 if (TraditionalRecipeChanged || CustomRecipeChanged)
	{
		MessageForm->Left = Form1->Left + (Form1->ClientWidth - MessageForm->Width)/2;        // MessageForm - по центру
		MessageForm->Top = Form1->Top + (Form1->ClientHeight - MessageForm->Height - (MessageForm->Height/3))/2;
	   Result = MessageForm->ShowModal();
	}
	if (Result == mrOk)
	{
		AddCommand(RUN_RECIPE);
		 attempt = ATTEMPTS_NUMBER;
		 Form1->Label1->Caption = "Send...";
	}

}
//---------------------------------------------------------------------------

void __fastcall TForm1::StopRecipeButtonClick(TObject *Sender)
{
 AddCommand(STOP_RECIPE);
 attempt = ATTEMPTS_NUMBER;
 Form1->Label1->Caption = "Send...";

}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
 int Result = mrOk;
	 if (TraditionalRecipeChanged || CustomRecipeChanged)
	{
		MessageForm->Left = Form1->Left + (Form1->ClientWidth - MessageForm->Width)/2;        // MessageForm - по центру
		MessageForm->Top = Form1->Top + (Form1->ClientHeight - MessageForm->Height - (MessageForm->Height/3))/2;
	   Result = MessageForm->ShowModal();
	}
	if (Result == mrOk)
	{
			TCustomIniFile* SettingsFile = new TIniFile(ChangeFileExt(Application->ExeName, ".ini"));
		// Store current form properties to be used in later sessions.
		try
		{
			SettingsFile->WriteInteger("ApplicationLanguage", "Language", Language );
		}
		catch(Exception* e)
		{
		}

		delete SettingsFile;

		Action = caFree;
	}
	else
	{
		Action = caNone;
	}


}
//---------------------------------------------------------------------------


void __fastcall TForm1::ResetCustomRecipeButtonClick(TObject *Sender)
{

  int Result = mrOk;
	 if (CustomRecipeChanged)
	{
		MessageForm->Left = Form1->Left + (Form1->ClientWidth - MessageForm->Width)/2;        // MessageForm - по центру
		MessageForm->Top = Form1->Top + (Form1->ClientHeight - MessageForm->Height - (MessageForm->Height/3))/2;
	   Result = MessageForm->ShowModal();
	}
	if (Result == mrOk)
	{
		AddCommand(RESET_CUSTOM_RECIPE_DATA);
		 attempt = ATTEMPTS_NUMBER;
		 Form1->Label1->Caption = "Send...";
	}


}
//---------------------------------------------------------------------------


void __fastcall TForm1::TraditionRecipeGridDragScroll(TObject *Sender, int TopRow,
		  int LeftCol, TDragScrollDirection &DragScrollDir, bool &CanScroll)
{
	CanScroll = false;
}
//---------------------------------------------------------------------------
/****************************set_recipe_name***********************/
void set_recipe_name(__Recipe  *recipe, char *name)
{char i;

    for (i=0;i < strlen(name);i++)  recipe->name[i] = name[i];

    recipe->name[i] = 0;
}
/****************************set_recipe_step***********************/
void set_recipe_step(_Recipe_Element  *step,unsigned short int power,unsigned short int temperature,unsigned long int time)
{
  step->Temperature = temperature;
  step->Power = power;
  step->Time = time;
}

/****************************load_default_recipes***********************/
void load_default_recipes(void)
{
  char recipe, step;

  // 0-й рецепт
  recipe = 0;
  step = 0;
  set_recipe_name(&Recipes.current[recipe], "MORNING");
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 50, 50, 0               );   step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 40,  60, 0             );   step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 30,  90, 0             );   step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 20,  94, 2*MIN           );  step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 0,  FOAM_SENSOR_ON_R, 7       );  step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 70,  0, 6*SEC            );  step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,0,PAUSE_R,2*SEC         );      step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,100,0,50*M20_mSEC       );      step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,0,PAUSE_R,2*SEC         );      step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,100,0,50*M20_mSEC       );     step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,0,PAUSE_R,2*SEC         );     step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,100,0,50*M20_mSEC       );     step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]  ,4,BUZZER_R,4            );      step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]  ,0,END_R,0               );

  // 1-й рецепт
  recipe = 1;
  step = 0;
  set_recipe_name(&Recipes.current[recipe], "FAST");
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 100, 60, 0               );   step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 70,  85, 0             );   step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 30,  94, 0             );   step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 0,  FOAM_SENSOR_ON_R, 6       );  step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 70,  0, 6*SEC            );  step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,0,PAUSE_R,2*SEC         );      step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,100,0,50*M20_mSEC       );      step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,0,PAUSE_R,2*SEC         );      step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,100,0,50*M20_mSEC       );     step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,0,PAUSE_R,2*SEC         );     step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,100,0,50*M20_mSEC       );     step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]  ,4,BUZZER_R,4            );      step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]  ,0,END_R,0               );

  // 2-й рецепт
  recipe = 2;
  step = 0;
  set_recipe_name(&Recipes.current[recipe], "GREEK");
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 40,  88, 0             );   step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 30,  94, 0             );   step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,20,94,15*MIN        );      step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 0,  FOAM_SENSOR_ON_R, 6       );  step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 70,  0, 6*SEC            );  step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,0,PAUSE_R,2*SEC         );      step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,100,0,50*M20_mSEC       );      step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,0,PAUSE_R,2*SEC         );      step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,100,0,50*M20_mSEC       );     step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,0,PAUSE_R,2*SEC         );     step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   ,100,0,50*M20_mSEC       );     step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]  ,4,BUZZER_R,4            );      step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]  ,0,END_R,0               );

  // 3-й рецепт
  recipe = 3;
  step = 0;
  set_recipe_name(&Recipes.current[recipe], "COCOA");
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 100, 70, 0               );   step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 30,  93, 0             );   step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 20,  96, 60*SEC             );   step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]   , 70,  0, 6*SEC            );  step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]  ,4,BUZZER_R,4            );      step++;
  set_recipe_step(&Recipes.current[recipe].Step[step]  ,0,END_R,0               );


}








void __fastcall TForm1::RESET_CONTROLLERButton1Click(TObject *Sender)
{
	AddCommand(RESET_CONTROLLER);
	 attempt = ATTEMPTS_NUMBER;
	 Form1->Label1->Caption = "Send...";

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Save_ini_fileClick(TObject *Sender)
{
	TCustomIniFile* SettingsFile = new TIniFile(ChangeFileExt(Application->ExeName, ".ini"));
	// Store current form properties to be used in later sessions.
	try
	{
		SettingsFile->WriteInteger("ApplicationLanguage", "Language", Language );
	}
	catch(Exception* e)
	{
	}

	delete SettingsFile;

}
//---------------------------------------------------------------------------



void __fastcall TForm1::SetLanguage(int Language)
{
  if (Language == 1)
  {


		 TraditionRecipeGrid->Cells[0][0] = "Этап 1. Мощность нагревателя, %.";
		 TraditionRecipeGrid->Cells[0][1] = "Этап 2. Мощность нагревателя, %.";
		 TraditionRecipeGrid->Cells[0][2] = "Этап 3. Температура приготовления, град С.";
		 TraditionRecipeGrid->Cells[0][3] = "Этап 3. Время приготовления, сек.";
		 TraditionRecipeGrid->Cells[0][4] = "Этап 4. Мощность на этапе подъема пенки, %.";
		 TraditionRecipeGrid->Cells[0][5] = "Этап 5. Длительность импульса нагревателя, сек.";
		 TraditionRecipeGrid->Cells[0][6] = "Этап 5. Количество импульсов нагревателя.";
		 TraditionRecipeGrid->Cells[0][7] = "Этап 5. Пауза между импульсами нагревателя, сек.";
		 TraditionRecipeGrid->Cells[0][8] = "Чувствительность датчика подъема пенки.";

	 TraditionTabSheet->Caption = "(1) Традиционный рецепт";
	 HandControlSheet->Caption = "Ручное управление";
	 CustomRecipe0TrackBar2->TrackLabel->Format =  "Начальная температура: %d град С";
	 CustomRecipe0TrackBar2->Repaint();
	 CustomRecipe0TrackBar1->TrackLabel->Format =  "Объем: %d мл";
	 CustomRecipe0TrackBar1->Repaint();
		SaveTraditionalRecipeButton->Hint = "Записать настройки в устройство";
		ReadTraditionalRecipeButton->Hint = "Читать настройки устройства";
		ResetTraditionalRecipeButton->Hint = "Установить стандартные настройки";
		SpeedButton2->Hint = "Запустить рецепт на выполнение";
		SpeedButton1->Hint = "Остановить";
		SaveCustomRecipeButton->Hint = "Записать рецепты в устройство";
		ReadCustomRecipeButton->Hint = "Читать рецепты из устройства";
		ResetCustomRecipeButton->Hint = "Установить стандартные рецепты";
		SaveToFileCustomRecipeButton->Hint = "Записать рецепт в файл";
		ReadFromFileCustomRecipeButton->Hint = "Читать рецепт из файла";
		RunRecipeButton->Hint = "Запустить рецепт на выполнение";
		StopRecipeButton->Hint = "Остановить";

		PlusScaleButton->Hint = "Увеличить  масштаб";
		MinusScaleButton->Hint = "Уменьшить  масштаб";
		AllScaleButton->Hint = "Показать все";
		SpeedButton25->Hint = "Увеличить  масштаб";
		SpeedButton24->Hint = "Уменьшить  масштаб";
		SpeedButton20->Hint = "Показать все";

		Label9->Caption = "Продолжительность";
		Label7->Caption = "Включений в серии";
		Label8->Caption = "Серий";
		Label20->Caption = "После срабатывания переход на  шаг:";
		Label22->Caption = "Чувствительность";
		Label21->Caption = "Для окончания рецепта после срабатывания введите номер  шага: ";
		Label5->Caption = "Мощность";
		Label4->Caption = "Температура";
		Label6->Caption = "Продолжительность";
		GroupBox2->Caption = "Текущие параметры";
		Label12->Caption = "Температура воды";
		Label13->Caption = "Температура процессора";
		Label14->Caption = "Температура нагревателя";
		Label18->Caption = "Текущий рецепт";
		Label3->Caption = "Текущий шаг";
		Label10->Caption = "Целевая температура";
		Label19->Caption = "Количество чашек";
		Label17->Caption = "Версия ПО";

		RESET_CONTROLLERButton1->Caption = "Сброс контроллера";

		OperationList->Clear();
		OperationList->Add("Нагрев");
		OperationList->Add("Пауза");
		OperationList->Add("Cигнал");
		OperationList->Add("Датчик пенки включить");
		OperationList->Add("Датчик пенки выключить");
		OperationComboBox->Items = OperationList;

		TraditionChart->Hint =  "Используйте кнопки мыши для навигации по графику";
		TraditionChart->LeftAxis->Title->Caption =  "Температура, град С, / Мощность, %";
		CustomChart->LeftAxis->Title->Caption =  "Температура, град С, / Мощность, %";
		CustomChart->Hint =  "Используйте кнопки мыши для навигации по графику";
  }
  else
  {
		 TraditionRecipeGrid->Cells[0][0] = "Step 1. Power, %.";
		 TraditionRecipeGrid->Cells[0][1] = "Step 2. Power, %.";
		 TraditionRecipeGrid->Cells[0][2] = "Step 3. Cooking temperature, deg С.";
		 TraditionRecipeGrid->Cells[0][3] = "Step 3. Time for preparing, sec.";
		 TraditionRecipeGrid->Cells[0][4] = "Step 4. Power, %.";
		 TraditionRecipeGrid->Cells[0][5] = "Step 5. Heater pulse duration, sec.";
		 TraditionRecipeGrid->Cells[0][6] = "Step 5. Number of heater pulses.";
		 TraditionRecipeGrid->Cells[0][7] = "Step 5. Pause between heater pulses, sec.";
		 TraditionRecipeGrid->Cells[0][8] = "Crema lifting sensor sensistivity.";

	 TraditionTabSheet->Caption = "(1) Traditional recipe";
	 HandControlSheet->Caption = "Manual control";
	 CustomRecipe0TrackBar2->TrackLabel->Format =  "Initial temperature: %d deg С";
	 CustomRecipe0TrackBar2->Repaint();
	 CustomRecipe0TrackBar1->TrackLabel->Format =  "Volume: %d ml";
	 CustomRecipe0TrackBar1->Repaint();
		SaveTraditionalRecipeButton->Hint = "Write settings to device";
		ReadTraditionalRecipeButton->Hint = "Read device settings";
		ResetTraditionalRecipeButton->Hint = "Set default settings";
		SpeedButton2->Hint = "Run recipe";
		SpeedButton1->Hint = "Stop recipe";
		SaveCustomRecipeButton->Hint = "Write recipes to the device";
		ReadCustomRecipeButton->Hint = "Read recipes from the device";
		ResetCustomRecipeButton->Hint = "Set standard recipes";
		SaveToFileCustomRecipeButton->Hint = "Write recipe to file";
		ReadFromFileCustomRecipeButton->Hint = "Read recipe from file";
		RunRecipeButton->Hint = "Run recipe";
		StopRecipeButton->Hint = "Stop recipe";

		PlusScaleButton->Hint = "Zoom in";
		MinusScaleButton->Hint = "Zoom out";
		AllScaleButton->Hint = "Show all";
		SpeedButton25->Hint = "Zoom in";
		SpeedButton24->Hint = "Zoom out";
		SpeedButton20->Hint = "Show all";

		Label9->Caption = "Duration";
		Label7->Caption = "Signals in a series";
		Label8->Caption = "Series";
		Label20->Caption = "After triggering, go to step:";
		Label22->Caption = "Sensitivity";
		Label21->Caption = "To end the recipe after operation, enter the step number: ";
		Label5->Caption = "Power";
		Label4->Caption = "Temperature";
		Label6->Caption = "Duration";
		GroupBox2->Caption = "Current parameters";
		Label12->Caption = "Water temperature";
		Label13->Caption = "CPU temperature";
		Label14->Caption = "Heater temperature";
		Label18->Caption = "Current recipe";
		Label3->Caption = "Current step";
		Label10->Caption = "Target temperature";
		Label19->Caption = "Number of cups";
		Label17->Caption = "Version";

		RESET_CONTROLLERButton1->Caption = "Controller reset";

		OperationList->Clear();
		OperationList->Add("Heating");
		OperationList->Add("Pause");
		OperationList->Add("Signal");
		OperationList->Add("Crema sensor enable");
		OperationList->Add("Crema sensor disable");
		OperationComboBox->Items = OperationList;

		TraditionChart->Hint =  "Use mouse buttons to navigate the graph.";
		CustomChart->Hint =  "Use mouse buttons to navigate the graph.";
		TraditionChart->LeftAxis->Title->Caption =  "Temperature, deg. C, / Power, %";
		CustomChart->LeftAxis->Title->Caption =  "Temperature, deg. C, / Power, %";

  }

		Label21->Caption =  Label21->Caption + IntToStr((int)(NUMBER_OF_STEPS_IN_RECIPE));

  if (MainPageControl->TabIndex >= 1 && MainPageControl->TabIndex <= 4 )
  {
	 Create_Custom_Recipe_Series();
	 CustomRecipeSettingsToGrid();
  }

}

void __fastcall TForm1::LanguageButtonClick(TObject *Sender)
{
	 if (Language == 1)
	 {
	   Language = 0;
	   LanguageButton->Caption = "RUS";
	 }
	 else
	 {
	  Language = 1;
	   LanguageButton->Caption = "ENG";
	 }

	 SetLanguage(Language);
}
//---------------------------------------------------------------------------

