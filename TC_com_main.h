//---------------------------------------------------------------------------

#ifndef TC_com_mainH
#define TC_com_mainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include "wclBluetooth.hpp"
#include <Vcl.ComCtrls.hpp>
#include <AnsiStrings.hpp>
#include <Clipbrd.hpp>
#include <Data.DB.hpp>
#include <Datasnap.DBClient.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Mask.hpp>
#include <VCLTee.Chart.hpp>
#include <VCLTee.Series.hpp>
#include <VclTee.TeeGDIPlus.hpp>
#include <VCLTee.TeEngine.hpp>
#include <VCLTee.TeeProcs.hpp>
#include <Vcl.Grids.hpp>
#include "AdvGrid.hpp"
#include "AdvObj.hpp"
#include "BaseGrid.hpp"
#include "CalcEdit.hpp"
#include "AdvCalculatorDropdown.hpp"
#include "AdvDropDown.hpp"
#include "AdvTrackBar.hpp"
#include "AdvTrackBarDropDown.hpp"
#include "DBAdvTrackBar.hpp"
#include "AdvShapeButton.hpp"
#include "AdvGlassButton.hpp"
#include <System.ImageList.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.Graphics.hpp>
#include <Vcl.Samples.Spin.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.ButtonGroup.hpp>
#include <Vcl.BaseImageCollection.hpp>
#include <Vcl.ImageCollection.hpp>
#include <Vcl.VirtualImageList.hpp>
#include "PictureList.hpp"
#include <Vcl.Imaging.pngimage.hpp>
#include "AdvUtil.hpp"

//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TwclBluetoothManager *wclBluetoothManager1;
	TwclGattClient *wclGattClient1;
	TListView *lvDevices;
	TListView *lvCharacteristics;
	TListView *lvServices;
	TListView *lvDescriptors;
	TEdit *edCharVal;
	TButton *btSetValue;
	TMemo *Memo1;
	TButton *copyButton1;
	TButton *clearButton2;
	TLabel *Label1;
	TButton *ReadStatusButton;
	TButton *BadDataButton;
	TPageControl *MainPageControl;
	TTabSheet *TraditionTabSheet;
	TTabSheet *CustomTabSheet0;
	TTabSheet *CustomTabSheet1;
	TTabSheet *CustomTabSheet2;
	TTimer *StateMachineTimer;
	TTabSheet *CustomTabSheet3;
	TOpenDialog *OpenDialog1;
	TSaveDialog *SaveDialog1;
	TPanel *TraditionalPanel;
	TChart *TraditionChart;
	TLineSeries *Tradition_Recipe_Power_Series;
	TLineSeries *Tradition_Recipe_Temperature_Series;
	TAdvStringGrid *TraditionRecipeGrid;
	TAdvTrackBar *AdvTrackBar2;
	TAdvTrackBar *AdvTrackBar4;
	TAdvTrackBar *AdvTrackBar6;
	TAdvTrackBar *AdvTrackBar8;
	TAdvTrackBar *AdvTrackBar10;
	TAdvTrackBar *AdvTrackBar3;
	TPanel *CustomPanel;
	TChart *CustomChart;
	TLineSeries *Recipe_1_Power_Series;
	TLineSeries *Recipe_1_Temperature_Series;
	TAdvStringGrid *CustomRecipe0Grid;
	TGroupBox *heatGroupBox;
	TLabel *Label4;
	TLabel *Label5;
	TLabel *Label6;
	TSpinEdit *PowerSpinEdit;
	TSpinEdit *TemperatureSpinEdit;
	TAdvCalculatorDropdown *TimeDropdown;
	TComboBox *OperationComboBox;
	TGroupBox *pauseGroupBox;
	TLabel *Label9;
	TAdvCalculatorDropdown *PauseTimeDropdown;
	TGroupBox *signalGroupBox;
	TLabel *Label7;
	TLabel *Label8;
	TSpinEdit *SignalSeriesSpinEdit;
	TSpinEdit *SignalTicksSpinEdit;
	TTabSheet *HandControlSheet;
	TGroupBox *GroupBox2;
	TLabel *Label2;
	TLabel *Label12;
	TLabel *Label13;
	TLabel *Label14;
	TLabel *Label15;
	TLabel *Label16;
	TSpeedButton *SaveToFileCustomRecipeButton;
	TSpeedButton *ReadFromFileCustomRecipeButton;
	TSpeedButton *SaveTraditionalRecipeButton;
	TSpeedButton *ResetTraditionalRecipeButton;
	TSpeedButton *ReadTraditionalRecipeButton;
	TSpeedButton *SpeedButton19;
	TSpeedButton *SpeedButton18;
	TSpeedButton *MinusScaleButton;
	TSpeedButton *PlusScaleButton;
	TSpeedButton *AllScaleButton;
	TSpeedButton *SpeedButton20;
	TSpeedButton *SpeedButton24;
	TSpeedButton *SpeedButton25;
	TButton *ConnectButton;
	TImageList *ImageList1;
	TTimer *CheckStatusTimer;
	TImage *Image1;
	TImage *Image3;
	TImage *Image0;
	TImage *Image2;
	TImage *Image4;
	TImage *Image5;
	TImage *Image6;
	TImage *Image7;
	TImage *Image8;
	TImage *Image9;
	TImage *Image10;
	TImage *Image11;
	TImage *Image12;
	TLabel *RecipeLabel;
	TPanel *Panel2;
	TSpeedButton *LiquidButton;
	TSpeedButton *TimeButton;
	TSpeedButton *StartButton;
	TSpeedButton *PlusButton;
	TSpeedButton *MinusButton;
	TImage *StartLEDImage;
	TImage *ImageE;
	TImage *ImageBlank;
	TImage *Image13;
	TImage *GrayImage;
	TImage *RedImage;
	TLabel *Label18;
	TLabel *Label10;
	TLabel *TargetTemperaturaLabel;
	TLabel *Label17;
	TLabel *SoftwareVersionLabel;
	TImage *ImageP;
	TButton *RUN_TRADITIONAL_RECIPE;
	TSpeedButton *RunRecipeButton;
	TSpeedButton *StopRecipeButton;
	TSpeedButton *SpeedButton1;
	TSpeedButton *SpeedButton2;
	TSpeedButton *SaveCustomRecipeButton;
	TSpeedButton *ReadCustomRecipeButton;
	TAdvTrackBar *CustomRecipe0TrackBar1;
	TAdvTrackBar *CustomRecipe0TrackBar2;
	TSpeedButton *ResetCustomRecipeButton;
	TImage *Image14;
	TPanel *Panel1;
	TImage *LeftDigitImage;
	TImage *RightDigitImage;
	TImage *DigitalPointImage;
	TImage *TemperatureLEDImage;
	TImage *TimeLEDImage;
	TImage *MilkLEDImage;
	TImage *CoffeeLEDImage;
	TImage *WaterLEDImage;
	TImage *ConnectImage;
	TLabel *Label3;
	TLabel *StepLabel;
	TLabel *Label19;
	TLabel *CupsNumberLabel;
	TGroupBox *FoamSensorGroupBox;
	TLabel *Label20;
	TSpinEdit *FoamSensorSpinEdit;
	TLabel *Label21;
	TLabel *Label22;
	TSpinEdit *ThresholdSpinEdit;
	TLabel *RecipeNameLabel;
	TLabel *Label11;
	TBevel *Bevel1;
	TBevel *Bevel2;
	TBevel *Bevel3;
	TButton *RESET_CONTROLLERButton1;
	TButton *Save_ini_file;
	TButton *LanguageButton;
	void __fastcall wclBluetoothManager1DeviceFound(TObject *Sender, TwclBluetoothRadio * const Radio,
		  const __int64 Address);
	void __fastcall wclBluetoothManager1DiscoveringStarted(TObject *Sender, TwclBluetoothRadio * const Radio);
	void __fastcall wclBluetoothManager1DiscoveringCompleted(TObject *Sender, TwclBluetoothRadio * const Radio,
          const int Error);
	void __fastcall wclGattClient1Connect(TObject *Sender, const int Error);
	void __fastcall btSetValueClick(TObject *Sender);
	void __fastcall wclGattClient1CharacteristicChanged(TObject *Sender, const WORD Handle,
          const TwclGattCharacteristicValue Value);
	void __fastcall copyButton1Click(TObject *Sender);
	void __fastcall clearButton2Click(TObject *Sender);

     void __fastcall StartButtonClick(TObject *Sender);
		void __fastcall StateMachineTimerTimer(TObject *Sender);

        void __fastcall ReadTraditionalRecipeButtonClick(TObject *Sender);
        void __fastcall SaveTraditionalRecipeButtonClick(TObject *Sender);
        void __fastcall ResetTraditionalRecipeButtonClick(TObject *Sender);
        void __fastcall ReadCustomRecipeButtonClick(TObject *Sender);


        void __fastcall ReadStatusButtonClick(TObject *Sender);
        void __fastcall BadDataButtonClick(TObject *Sender);
        void __fastcall LiquidButtonClick(TObject *Sender);
        void __fastcall InsertStepButtonClick(TObject *Sender);
        void __fastcall PlusButtonClick(TObject *Sender);
        void __fastcall MinusButtonClick(TObject *Sender);
        void __fastcall Button14Click(TObject *Sender);
	void __fastcall SaveCustomRecipeButtonClick(TObject *Sender);
	void __fastcall TraditionRecipeGridGetEditorType(TObject *Sender, int ACol, int ARow,
          TEditorType &AEditor);
	void __fastcall AdvTrackBar0Change(TObject *Sender);
	void __fastcall TraditionRecipeGridCanEditCell(TObject *Sender, int ARow, int ACol,
          bool &CanEdit);
	void __fastcall TraditionRecipeGridSelectCell(TObject *Sender, int ACol, int ARow,
          bool &CanSelect);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall TraditionRecipeGridCellsChanged(TObject *Sender, TRect &R);
	void __fastcall CustomRecipe0GridClickCell(TObject *Sender, int ARow, int ACol);
	void __fastcall CustomTabSheet0Show(TObject *Sender);
	void __fastcall TimeButtonClick(TObject *Sender);
	void __fastcall DeleteStepButtonClick(TObject *Sender);
	void __fastcall ApplyInputValues(TObject *Sender);
	void __fastcall CustomRecipe0GridSelectCell(TObject *Sender, int ACol, int ARow,
          bool &CanSelect);
	void __fastcall SaveToFileCustomRecipeButtonClick(TObject *Sender);
	void __fastcall ReadFromFileCustomRecipeButtonClick(TObject *Sender);
	void __fastcall PlusScaleButtonClick(TObject *Sender);
	void __fastcall MinusScaleButtonClick(TObject *Sender);
	void __fastcall CustomChartClickSeries(TCustomChart *Sender, TChartSeries *Series, int ValueIndex,
          TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall CustomRecipe0TrackBar1Change(TObject *Sender);
	void __fastcall MainPageControlChange(TObject *Sender);
	void __fastcall AllScaleButtonClick(TObject *Sender);
	void __fastcall ConnectButtonClick(TObject *Sender);
	void __fastcall CheckStatusTimerTimer(TObject *Sender);
	void __fastcall CheckInputValues(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall HandControlSheetShow(TObject *Sender);
	void __fastcall HandControlSheetHide(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall RunRecipeButtonClick(TObject *Sender);
	void __fastcall StopRecipeButtonClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall ResetCustomRecipeButtonClick(TObject *Sender);
	void __fastcall TraditionRecipeGridDragScroll(TObject *Sender, int TopRow, int LeftCol,
          TDragScrollDirection &DragScrollDir, bool &CanScroll);
	void __fastcall RESET_CONTROLLERButton1Click(TObject *Sender);
	void __fastcall Save_ini_fileClick(TObject *Sender);
	void __fastcall LanguageButtonClick(TObject *Sender);



private:	// User declarations
public:		// User declarations
 void __fastcall CustomRecipeSettingsToBox(void);
 void __fastcall CustomRecipeBoxToSettings(void);
void __fastcall	  SetConnectedStatus(bool connected);
void __fastcall SetLanguage(int Language);

TwclBluetoothRadio* __fastcall GetRadio();
	 TwclGattCharacteristics FCharacteristics;
	 TwclGattDescriptors FDescriptors;
	 TwclGattServices FServices;
	  TwclBluetoothRadio *  mRadio;
	  TwclGattService mService;
	  TwclGattCharacteristic mCharacteristic;
	  String TestString;
	  String StrLabel2;
	  int mCharacteristic_found;
	  TImage *Images[16];
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------

 #define BIT0                (0x0001u)
#define BIT1                (0x0002u)
#define BIT2                (0x0004u)
#define BIT3                (0x0008u)
#define BIT4                (0x0010u)
#define BIT5                (0x0020u)
#define BIT6                (0x0040u)
#define BIT7                (0x0080u)
#define BIT8                (0x0100u)
#define BIT9                (0x0200u)
#define BITA                (0x0400u)
#define BITB                (0x0800u)
#define BITC                (0x1000u)
#define BITD                (0x2000u)
#define BITE                (0x4000u)
#define BITF                (0x8000u)


#define BUFFER_LENGTH  2000
#define BLE_PACKET_LENGTH  20
 // ********************** UART_command master-slave*************

#define NO_COMMAND     0
#define LIQUID_BUTTON_PRESS  2
#define TIME_BUTTON_PRESS    3
#define PLUS_BUTTON_PRESS    4
#define MINUS_BUTTON_PRESS   5
#define START_BUTTON_PRESS   6
#define SET_LIQUID_TYPE      8
#define SET_EDIT_MODE      10
#define SET_HEATER_MODE      12
#define READ_CUSTOM_RECIPES_DATA      17
#define WRITE_CUSTOM_RECIPES_DATA      18
#define READ_TRADITIONAL_RECIPE_DATA      19
#define WRITE_TRADITIONAL_RECIPE_DATA      20
#define RESET_TRADITIONAL_RECIPE_DATA      21
#define READ_STATUS      23
#define RESET_CONTROLLER      24
#define RUN_RECIPE      25
#define STOP_RECIPE      26
#define BAD_DATA_RESULT      27
#define RESET_CUSTOM_RECIPE_DATA      28


#define COMMAND_STACK_LENGTH 100

#define BOILING_THRESHOLD 45


typedef struct _recipe_settings
{
unsigned short int Coffee_Target_Temperature;
unsigned short int Coffee_Low_Power_level;
unsigned short int Coffee_Middle_Power_level;
unsigned short int Coffee_Impulses_Number;
unsigned short int Coffee_Boiling_Threshold;
unsigned short int Coffee_Initial_Power_Level;
char Coffee_Temperature_Correction;
char Coffee_BT_enable;
unsigned short int Coffee_Cups_Number;
unsigned long int Coffee_Impulses_Pause;
unsigned long int Coffee_Impulses_Duration;
unsigned long int Coffee_Holding_Time;
} __recipe_settings;


#define RECIPE_SETTINGS_LENGTH 28

typedef union _recipe_charint
{
	__recipe_settings current;
	unsigned char char_recipe_settings[RECIPE_SETTINGS_LENGTH];
} charint_recipe_settings;

/*
typedef struct _command_packet
{
unsigned char command;
unsigned char current_temperature;
unsigned char processor_temperature;
unsigned char heater_temperature;
unsigned char software_version;
unsigned char LEDs;
unsigned char water_level;
unsigned char no_water;
unsigned char Recipe_number;
unsigned char target_temperature;
unsigned char left_digit;
unsigned char right_digit;
unsigned char reserved_0;
unsigned char reserved_1;
unsigned char reserved_2;
unsigned char reserved_3;
unsigned short int water_sensor_raw_data;
} __command_packet;
 */
typedef struct _command_packet
{
unsigned char command;
unsigned char current_temperature;
unsigned char processor_temperature;
unsigned char heater_temperature;
unsigned char software_version;
unsigned char LEDs;
unsigned char water_level;
unsigned char Recipe_number;
unsigned char target_temperature;
unsigned char step_number;
unsigned char reserved_0;
unsigned char reserved_1;
unsigned char reserved_2;
char Coffee_Temperature_Correction;
unsigned short int Coffee_Cups_Number;
unsigned short int water_sensor_raw_data;
} __command_packet;


#define COMMAND_PACKET_LENGTH 18
#define CRC_LENGTH 2

typedef union _command_packet_charint
{
    __command_packet command_packet;
	unsigned char command_packet_char[COMMAND_PACKET_LENGTH];
} command_packet_charint;

typedef union _crc16_charint
{
    unsigned short crc16_current;
    unsigned char crc16_current_char[CRC_LENGTH];
} crc16_charint;


#define CLEAR 0
#define DATA 1

// Liquids and Recipes
#define RECIPE_0     0
#define RECIPE_1     1
#define RECIPE_2     2
#define RECIPE_3     3
#define COFFEE     4
#define WATER      5
#define MILK       6


// ********************** VM_state *************
#define  VM_IDLE  0
#define  VM_EXEC_COMMAND  1
#define  VM_RECEIVE_DATA  2
#define  VM_CHECK_DATA  3
#define  VM_NO_CONNECTION  4
#define  VM_BAD_DATA   5
#define  VM_CORRECT_DATA 6
#define  VM_STOP  7
#define  VM_SEND_DATA 8
#define  VM_PAUSE 9

//   Время
#define M20_mSEC 1
#define M100_mSEC 6
#define SEC 64L
#define MIN 60L*SEC
#define HOUR 60L*MIN
#define LOW_POWER_TIME 3*MIN
#define TEMP_CO 4.187
// Параметры меандра при поддержании температуры
#define MICRO_POWER 10
#define TIME_ADD  3*SEC

/*********** custom_recipe *******************************/

// Все значения дб кратны 4 - выравнивание данных - передача по uart
#define  NUMBER_OF_STEPS_IN_RECIPE 28
#define  NUMBER_OF_RECIPES 4
#define  RECIPE_NAME_LENGTH 16
#define  RECIPE_ELEMENT_LENGTH 8
#define  RECIPES_DATA_LENGTH ((RECIPE_NAME_LENGTH+RECIPE_ELEMENT_LENGTH*NUMBER_OF_STEPS_IN_RECIPE)*NUMBER_OF_RECIPES)

// ********************** Recipe[][].Operation *************
#define  END_R  110
#define  HEATING_R  111
#define  BUZZER_R  112
#define  FOAM_SENSOR_ON_R  113
#define  FOAM_SENSOR_OFF_R  114
#define  GOTO_RECIPE_R  115
#define SET_BOILING_THRESHOLD 116
#define  PAUSE_R  117

// ********************** Grid col number *************
#define  TEMPERATURE_COL_NUMBER  2
#define  POWER_COL_NUMBER  3
#define  TIME_COL_NUMBER  4

// ********************** Combo item index *************
#define  HEATING_ITEM_INDEX  0
#define  PAUSE_ITEM_INDEX  1
#define  BUZZER_ITEM_INDEX  2
#define  FOAM_SENSOR_ON_ITEM_INDEX  3
#define  FOAM_SENSOR_OFF_ITEM_INDEX  4
#define  SET_BOILING_THRESHOLD_ITEM_INDEX  5

// ********************** BT Status *************
#define  DISCONNECTED  0
#define  CONNECTED  1
// ********************** Attempts number *************
#define  ATTEMPTS_NUMBER  5

typedef struct Recipe_Element
{
  unsigned short int Temperature;
  unsigned short int Power;
  unsigned long int Time;
} _Recipe_Element;

typedef struct _Recipe
{
  unsigned char name[RECIPE_NAME_LENGTH];
  _Recipe_Element Step[NUMBER_OF_STEPS_IN_RECIPE];
} __Recipe;

typedef union _custom_recipe_charint
{
    __Recipe current[NUMBER_OF_RECIPES];
	unsigned char char_custom_recipe[RECIPES_DATA_LENGTH];
} __custom_recipe_charint;







#endif
