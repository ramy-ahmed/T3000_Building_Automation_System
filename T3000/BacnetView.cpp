﻿// DialogCM5_BacNet.cpp : implementation file
// DialogCM5 Bacnet programming by Fance 2013 05 01
/*
2014-04-16
Update by Fance
1.Fix the modbus DLL .If the device disconnected , the  "read_one" and "write_one" function will error until you connect again.
2.Add a new setting interface in the bacnet view.
2014-04-11
Update by Fance
1.Get the monitor data from the bacnet device.
2.Draw the graphic on the screen.

2014-03-28
Update by Fance
1.Update the Graphic view class.Now all the position and parameter can changed more easier. 
2014-03-27
Update by Fance
1.Finished the function ,Open the Testo USB port and read the Testo-435 data.Put the two function into the Dll,named "DllFunction.cpp"
2.Finished the Zigbee RSSI List , all the data will show on the list normally;

2014-03-20
Update by Fance
1.Change the graphic static,from staticex to own draw.
2.Change the ud_str.h ,some of the monitor struct.
2014-03-19
Update by Fance
1.Update the progress bar ,change the position to the button of the view.
2.Add read monitor data function.
2014-03-18
Update by Fance
1.Net cable disconnected ,the scan result also show the last connected device . ->Fix it.
2.If the Co2 external sensor count is a invalid value , the CO2 Window will crash . ->Fix it.
2014-03-17
Update by Fance
1.Support read Zigbee Tstat.(Not finished)
2014-03-14
Update by Fance
1.Program IDE user interface , panel upper 32 can't send program command,
	eg:  IF 36-OUT1 >5 THEN 87-IN2 = 3   (This shows 36-OUT1 is not defind)
	Fix this problem.
2.About program TIME-ON , TIME-OFF INTERVAL command ,now it can use normally.
3.Debug window don't always create when user don't needed.It will create when the user press SHIFT + 'D'
2014-03-07
Update by Fance
1.Allow the user to change the IP address and subnet and gateway.
2.Optimze the building graphic , user can add lable (inputs ,outputs,variable) and monitor this lable on the building view.
3.The device of CO2 show a wrong firmware version. Fix it.
4.Fix part of code which may crash the T3000.
2014-02-21
Update by Fance
1.BacnetTstat background color ,white and gray interval.
2.ISP tool in T3000 can't work , because the port 4321 is already bind by the T3000 .Already fix the problem
3.Modify the debug window hot key to shift + D

2014-02-20
Update by Fance
1.Code the BacnetScreenEdit.cpp , user can add PIC file and label in the pic file.user can monitor the value in the graphic.
2.Fix the BacnetScreenEdit.cpp memory leak.
3.Fix the BacnetScreen.cpp . No longer show  Text mode.
4.Fix the Bacnet object instance , T3000 no longer has the static instance ,use a random value.

2014-02-10
	Update by Fance
	1.TreeView update , support refresh different  com and network device.the Treeview of the device no longer
	  display a error status.

2014-01-13
	Update by Fance
	1.Add BacnetScreenEdit.cpp User can edit screen window,add label delete label.
	2.Modify the ListTree if one device is choose by user , It will change his color to red.
	3.Modify the range ,user can change the range such as  "ON/OFF" "OFF/ON" .In "ON/OFF" range the ON is 1 but in "OFF/ON" the ON it is 0;
	4.Change the Bacnet Main window. Insert all the property dialog into the view window.User can operated by the mouse click or keyboard input;
	5.Add Load config all and write config file .The file will be saved with "*.prg" and also user can loaded.
	6.Fix a bug that the program function "Interval" is not normally used;
2013-12-19
	Update by Fance
	1.Monify the list class,add function which we can select the list item.Also add list backcolor function;
	2.Add bacnet "Monitor" "Graphic" and "Screens".
	3.Use GDI to draw the bacnet monitor data.
	4.Add bacnet IP Scan,now T3000 can scan our own bacnet ip products.
	5.Add bacnet config file ,user can save the bacnet data to the config file,or load the config file;
	6.Fix the tree view can't show the bacnet products normally.
	7.Fix the transmission results,"RX" "TX" "ERROR".Now it can show bacnet ip transfer results normally;
	8.Fix all the bacnet listctrl , add function user can use keyboard operate it.
	9.Improve the function when user choose one of the bacnet list item ,if it is a combobox ,it will auto pull-down,and show all the items for user to choose.
	10.Improve  when user use bacnet function to change one of the value ,if the data change fail ,the data will resume to origin instead of show the error data on the screen.
	11.Fix program code dialog crashed,when the text length longer than the MAX_PATH;
	12.Fix when the label length can longer than the structure defined.
	13.Change the communication protocol from bacnetMSTP to BacnetIp;
	14.Save the bacnet instance id to mdb sw version and hw version,save the ip address to boudrate;
*/

#include "stdafx.h"
#include "T3000.h"
#include "DialogCM5_BacNet.h"
#include "CM5\MyOwnListCtrl.h"
#include "BacnetInput.h"
#include "BacnetOutput.h"
#include "BacnetProgram.h"
#include "BacnetVariable.h"
#include "globle_function.h"

#include "gloab_define.h"
#include "datalink.h"
#include "BacnetWait.h"
#include "Bacnet_Include.h"
#include "CM5\ud_str.h"
#include "BacnetWeeklyRoutine.h"
#include "BacnetAnnualRoutine.h"
#include "AnnualRout_InsertDia.h"
#include "BacnetController.h"
#include "BacnetScreen.h"
#include "BacnetMonitor.h"
#include "BacnetProgramEdit.h"
#include "rs485.h"
#include "BacnetScheduleTime.h"
#include "BacnetAlarmLog.h"
#include "BacnetAlarmWindow.h"
#include "BacnetTstat.h"
#include "BacnetSetting.h"
//bool CM5ProcessPTA(	BACNET_PRIVATE_TRANSFER_DATA * data);
//bool need_to_read_description = true;
int g_gloab_bac_comport = 1;
CString temp_device_id,temp_mac,temp_vendor_id;
HANDLE hwait_thread;
BacnetWait *WaitDlg=NULL;
extern int Station_NUM;
BacnetScreen *Screen_Window = NULL;
CBacnetProgram *Program_Window =NULL;
CBacnetInput *Input_Window = NULL;
CBacnetOutput *Output_Window = NULL;
CBacnetVariable *Variable_Window = NULL;
BacnetWeeklyRoutine *WeeklyRoutine_Window = NULL;
BacnetAnnualRoutine *AnnualRoutine_Window = NULL;
BacnetController *Controller_Window = NULL;
CBacnetMonitor *Monitor_Window = NULL;
CBacnetAlarmLog *AlarmLog_Window = NULL;
CBacnetTstat *Tstat_Window = NULL;
CBacnetSetting * Setting_Window = NULL;
extern CBacnetAlarmWindow * AlarmWindow_Window;
extern bool is_in_scan_mode;
extern char mycode[1024];
int click_resend_time = 0;//当点击的时候，要切换device时 发送whois的次数;
// CDialogCM5_BacNet
CString IP_ADDRESS;
_Refresh_Info Bacnet_Refresh_Info;

//#define WM_SEND_OVER     WM_USER + 1287
// int m_Input_data_length;
extern void  init_info_table( void );
extern void Init_table_bank();

IMPLEMENT_DYNCREATE(CDialogCM5_BacNet, CFormView)

CDialogCM5_BacNet::CDialogCM5_BacNet()
	: CFormView(CDialogCM5_BacNet::IDD)
	, m_bac_obtain_ip_mode(0)
{
	m_MSTP_THREAD = true;
	m_cur_tab_sel = 0;
	//CM5_hThread = NULL;
}

CDialogCM5_BacNet::~CDialogCM5_BacNet()
{
}

void CDialogCM5_BacNet::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_LIST1, m_device_list_info);


	DDX_Control(pDX, IDC_BAC_MAINTAB, m_bac_main_tab);
}

BEGIN_MESSAGE_MAP(CDialogCM5_BacNet, CFormView)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CDialogCM5_BacNet::OnBnClickedButtonTest)
	ON_MESSAGE(WM_FRESH_CM_LIST,Fresh_UI)
	ON_MESSAGE(MY_RESUME_DATA, AllMessageCallBack)
	ON_MESSAGE(WM_DELETE_NEW_MESSAGE_DLG,Delete_New_Dlg)	

	
	ON_WM_TIMER()
	ON_NOTIFY(TCN_SELCHANGE, IDC_BAC_MAINTAB, &CDialogCM5_BacNet::OnTcnSelchangeBacMaintab)
END_MESSAGE_MAP()


// CDialogCM5_BacNet diagnostics

#ifdef _DEBUG
void CDialogCM5_BacNet::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CDialogCM5_BacNet::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

//The window which created by the button ,will delete when the wait dialog send this message,to this window.
//It means ,it has done .we don't needed.
LRESULT CDialogCM5_BacNet::Delete_New_Dlg(WPARAM wParam,LPARAM lParam)
{
	int message_type = wParam;
	switch(message_type)
	{
	case 0:
		if(WaitDlg!=NULL)
		{
			Sleep(50);
			if(WaitDlg)
				delete WaitDlg;
			WaitDlg = NULL;

			if(bac_read_which_list == BAC_READ_SVAE_CONFIG)
			{
				if(bac_read_all_results)
					::PostMessageW(MainFram_hwd,MY_BAC_CONFIG_READ_RESULTS,1,NULL);
				return 0;
			}

			if(bac_read_which_list == BAC_READ_INPUT_LIST)
			{
				if(bac_input_read_results)
				{
					if(Input_Window == NULL)
					{
						Input_Window = new CBacnetInput;
						Input_Window->Create(IDD_DIALOG_BACNET_INPUT,this);	
					}
					else
					{
						::PostMessage(m_input_dlg_hwnd,WM_REFRESH_BAC_INPUT_LIST,NULL,NULL);
					}
					Input_Window->ShowWindow(SW_SHOW);
					m_bac_main_tab.SetCurSel(WINDOW_INPUT);
				}
				else
				{
					if(hwait_thread)
					{
						TerminateThread(hwait_thread,0);
						hwait_thread = NULL;
					}
					MessageBox(_T("Inputs list read time out!"));	
				}
				return 0;
			}

			if(bac_read_which_list == BAC_READ_OUTPUT_LIST)
			{
				if(bac_output_read_results)
				{
					if(Output_Window == NULL)
					{
						Output_Window = new CBacnetOutput;
						Output_Window->Create(IDD_DIALOG_BACNET_OUTPUT,this);	
					}
					else
					{
						::PostMessage(m_output_dlg_hwnd,WM_REFRESH_BAC_OUTPUT_LIST,NULL,NULL);
					}
					Output_Window->ShowWindow(SW_SHOW);
					m_bac_main_tab.SetCurSel(WINDOW_OUTPUT);

				}
				else
				{
					if(hwait_thread)
					{
						TerminateThread(hwait_thread,0);
						hwait_thread = NULL;
					}
					MessageBox(_T("Outputs list read time out!"));
				}
				return 0;
			}

			if(bac_read_which_list == BAC_READ_VARIABLE_LIST)
			{
				if(bac_variable_read_results)
				{
					if(Variable_Window == NULL)
					{
						Variable_Window = new CBacnetVariable;
						Variable_Window->Create(IDD_DIALOG_BACNET_VARIABLE,this);	
					}
					else
					{
						::PostMessage(m_variable_dlg_hwnd,WM_REFRESH_BAC_VARIABLE_LIST,NULL,NULL);
					}
					Variable_Window->ShowWindow(SW_SHOW);
					m_bac_main_tab.SetCurSel(WINDOW_VARIABLE);

				}
				else
				{
					if(hwait_thread)
					{
						TerminateThread(hwait_thread,0);
						hwait_thread = NULL;
					}
					MessageBox(_T("Variable list read time out!"));
				}
				return 0;
			}

			if(bac_read_which_list == BAC_READ_PROGRAM_LIST)
			{
				if(bac_program_read_results)
				{
					//CBacnetProgram DLG;
					//DLG.DoModal();

					if(Program_Window == NULL)
					{
						Program_Window = new CBacnetProgram;
						Program_Window->Create(IDD_DIALOG_BACNET_PROGRAM,this);	
					}
					else
					{
						::PostMessage(m_pragram_dlg_hwnd,WM_REFRESH_BAC_PROGRAM_LIST,NULL,NULL);
					}
					Program_Window->ShowWindow(SW_SHOW);
					m_bac_main_tab.SetCurSel(WINDOW_PROGRAM);

				}
				else
				{
					if(hwait_thread)
					{
						TerminateThread(hwait_thread,0);
						hwait_thread = NULL;
					}
					MessageBox(_T("Program list read time out!"));
				}
				return 0;
			}

			if(bac_read_which_list == BAC_READ_WEEKLY_LIST)
			{
				if(bac_weekly_read_results)
				{
					//CBacnetProgram DLG;
					//DLG.DoModal();

					if(WeeklyRoutine_Window == NULL)
					{
						WeeklyRoutine_Window = new BacnetWeeklyRoutine;
						WeeklyRoutine_Window->Create(IDD_DIALOG_BACNET_WEEKLY_ROUTINES,this);	
					}
					else
					{
						::PostMessage(m_weekly_dlg_hwnd,WM_REFRESH_BAC_WEEKLY_LIST,NULL,NULL);
					}
					WeeklyRoutine_Window->ShowWindow(SW_SHOW);
					m_bac_main_tab.SetCurSel(WINDOW_WEEKLY);
				}
				else
				{
					if(hwait_thread)
					{
						TerminateThread(hwait_thread,0);
						hwait_thread = NULL;
					}
					MessageBox(_T("Weekly list read time out!"));
				}
				return 0;
			}
#if 0
			if(bac_read_which_list == BAC_READ_WEEKLY_LIST)
			{
				if(bac_weekly_read_results)
				{
					BacnetWeeklyRoutine Dlg;
					Dlg.DoModal();
				}
				else
					MessageBox(_T("Weekly Routine list read time out!"));
			}
#endif
			if(bac_read_which_list == BAC_READ_ANNUAL_LIST)
			{
				if(bac_annual_read_results)
				{
					if(AnnualRoutine_Window == NULL)
					{
						AnnualRoutine_Window = new BacnetAnnualRoutine;
						AnnualRoutine_Window->Create(IDD_DIALOG_BACNET_ANNUAL_ROUTINES,this);
					}
					else
					{
						::PostMessage(m_annual_dlg_hwnd,WM_REFRESH_BAC_ANNUAL_LIST,NULL,NULL);
					}
					AnnualRoutine_Window->ShowWindow(SW_SHOW);
					m_bac_main_tab.SetCurSel(WINDOW_ANNUAL);
				}
				else
				{
					if(hwait_thread)
					{
						TerminateThread(hwait_thread,0);
						hwait_thread = NULL;
					}
					MessageBox(_T("Annual Routine list read time out!"));
				}
			}

			if(bac_read_which_list == BAC_READ_CONTROLLER_LIST)
			{
				if(bac_controller_read_results)
				{
					if(Controller_Window == NULL)
					{
						Controller_Window = new BacnetController;
						Controller_Window->Create(IDD_DIALOG_BACNET_CONTROLLER,this);
					}
					else
					{
						::PostMessage(m_controller_dlg_hwnd,WM_REFRESH_BAC_CONTROLLER_LIST,NULL,NULL);
					}
					Controller_Window->ShowWindow(SW_SHOW);
					m_bac_main_tab.SetCurSel(WINDOW_CONTROLLER);
				}
				else
				{
					if(hwait_thread)
					{
						TerminateThread(hwait_thread,0);
						hwait_thread = NULL;
					}
					MessageBox(_T("Controller list read time out!"));
				}
			}
			if(bac_read_which_list == BAC_READ_SCREEN_LIST)
			{
				if(bac_screen_read_results)
				{
					if(Screen_Window == NULL)
					{
						Screen_Window = new BacnetScreen;
						Screen_Window->Create(IDD_DIALOG_BACNET_SCREENS,this);	
					}
					else
					{
						::PostMessage(m_screen_dlg_hwnd,WM_REFRESH_BAC_SCREEN_LIST,NULL,NULL);
					}
					Screen_Window->ShowWindow(SW_SHOW);
					m_bac_main_tab.SetCurSel(WINDOW_SCREEN);
				}
				else
				{
					if(hwait_thread)
					{
						TerminateThread(hwait_thread,0);
						hwait_thread = NULL;
					}
					MessageBox(_T("Screens list read time out!"));
				}
			}
			if(bac_read_which_list == BAC_READ_MONITOR_LIST)
			{
				if(bac_monitor_read_results)
				{
					if(Monitor_Window == NULL)
					{
						Monitor_Window = new CBacnetMonitor;
						Monitor_Window->Create(IDD_DIALOG_BACNET_MONITOR,this);
					}
					else
					{
						::PostMessage(m_screen_dlg_hwnd,WM_REFRESH_BAC_SCREEN_LIST,NULL,NULL);
					}
					Monitor_Window->ShowWindow(SW_SHOW);
					m_bac_main_tab.SetCurSel(WINDOW_MONITOR);

				}
				else
				{
					if(hwait_thread)
					{
						TerminateThread(hwait_thread,0);
						hwait_thread = NULL;
					}
					MessageBox(_T("Monitor list read time out!"));
				}
			}

			if(bac_read_which_list == BAC_READ_ALARMLOG_LIST)
			{
				if(bac_alarmlog_read_results)
				{
					if(AlarmLog_Window == NULL)
					{
						AlarmLog_Window = new CBacnetAlarmLog;
						AlarmLog_Window->Create(IDD_DIALOG_BACNET_ALARMLOG,this);
					}
					else
					{
						//Refresh the alarmlog list;
					}
					AlarmLog_Window->ShowWindow(SW_SHOW);
					m_bac_main_tab.SetCurSel((WINDOW_ALARMLOG));
				}
				else
				{
					if(hwait_thread)
					{
						TerminateThread(hwait_thread,0);
						hwait_thread = NULL;
					}
					MessageBox(_T("AlarmLog read time out!"));
				}
			}

			if(bac_read_which_list == BAC_READ_PROGRAMCODE_LIST)
			{
				if(bac_programcode_read_results)
				{
					CBacnetProgramEdit Dlg;
					Dlg.DoModal();
				}
				else
					MessageBox(_T("Program list read time out!"));
			}

			if(bac_read_which_list == BAC_READ_WEEKLTCODE_LIST)
			{
				if(bac_weeklycode_read_results)
				{
					CBacnetScheduleTime Dlg;
					Dlg.DoModal();
				}
				else
					MessageBox(_T("Weekly schedual time read time out!"));
			}
								
			if(bac_read_which_list == BAC_READ_ANNUALCODE_LIST)
			{
				if(bac_annualcode_read_results)
				{
					AnnualRout_InsertDia Dlg;
					Dlg.DoModal();
				}
				else
					MessageBox(_T("Annual day time read time out!"));
			}


			if((bac_read_which_list == BAC_READ_BASIC_SETTING_COMMAND) || (bac_read_which_list == BAC_READ_ALL_LIST))
			{
				if(bac_basic_setting_read_results)
				{
					//PostMessage(WM_FRESH_CM_LIST,READ_SETTING_COMMAND,NULL);
				}
			}
		}
		break;
	case DELETE_WINDOW_MSG:
		m_bac_main_tab.SetFocus();
	//	m_bac_main_tab.SetCurSel(m_cur_tab_sel);
		break;
		//if(Screen_Window != NULL)
		//{
		//	//Sleep(1000);
		//	delete Screen_Window;
		//	Screen_Window = NULL;
		//}

		//if(Program_Window !=NULL)
		//{
		//	delete Program_Window;
		//	Program_Window = NULL;
		//}

		//if(WeeklyRoutine_Window !=NULL)
		//{
		//	delete WeeklyRoutine_Window;
		//	WeeklyRoutine_Window = NULL;
		//}

		//if(Input_Window !=NULL)
		//{
		//	delete Input_Window;
		//	Input_Window = NULL;
		//}

		//if(Output_Window !=NULL)
		//{
		//	delete Output_Window;
		//	Output_Window = NULL;
		//}
		//if(Variable_Window !=NULL)
		//{
		//	delete Variable_Window;
		//	Variable_Window = NULL;
		//}
		//if(Monitor_Window !=NULL)
		//{
		//	delete Monitor_Window;
		//	Monitor_Window = NULL;
		//}
		//if(Controller_Window !=NULL)
		//{
		//	delete Controller_Window;
		//	Controller_Window = NULL;
		//}
		//if(AnnualRoutine_Window !=NULL)
		//{
		//	delete AnnualRoutine_Window;
		//	AnnualRoutine_Window = NULL;
		//}
		//break;
	}
	
	return 0;
}
// CDialogCM5_BacNet message handlers

LRESULT  CDialogCM5_BacNet::AllMessageCallBack(WPARAM wParam, LPARAM lParam)
{
	_MessageInvokeIDInfo *pInvoke =(_MessageInvokeIDInfo *)lParam;
	bool msg_result=WRITE_FAIL;
	msg_result = MKBOOL(wParam);
	CString Show_Results;
	CString temp_cs = pInvoke->task_info;
	if(msg_result)
	{
		for (int i=0;i<BAC_PROGRAM_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Program_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Program_Info[i].task_result = true;
		}

		for (int i=0;i<BAC_PROGRAMCODE_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Programcode_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Programcode_Info[i].task_result = true;
		}

		for (int i=0;i<BAC_OUTPUT_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Output_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Output_Info[i].task_result = true;
		}

		for (int i=0;i<BAC_INPUT_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Input_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Input_Info[i].task_result = true;
		}
		for (int i=0;i<BAC_VARIABLE_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Variable_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Variable_Info[i].task_result = true;
		}

		for (int i=0;i<BAC_WEEKLY_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Weekly_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Weekly_Info[i].task_result = true;
		}

		for (int i=0;i<BAC_WEEKLYCODE_GOUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Weeklycode_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Weeklycode_Info[i].task_result = true;
		}


		for (int i=0;i<BAC_ANNUAL_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Annual_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Annual_Info[i].task_result = true;
		}

		for (int i=0;i<BAC_TIME_COMMAND_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Time_Command[i].invoke_id)
				Bacnet_Refresh_Info.Read_Time_Command[i].task_result = true;
		}

		for (int i=0;i<BAC_BASIC_SETTING_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_BasicSetting_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_BasicSetting_Info[i].task_result = true;
		}

		for (int i=0;i<BAC_CONTROLLER_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Controller_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Controller_Info[i].task_result = true;
		}

		for (int i=0;i<BAC_SCREEN_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Screen_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Screen_Info[i].task_result = true;
		}

		for (int i=0;i<BAC_MONITOR_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Monitor_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Monitor_Info[i].task_result = true;
		}
		for (int i=0;i<BAC_ANNUALCODE_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Annualcode_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Annualcode_Info[i].task_result = true;
		}

		for (int i=0;i<BAC_ALARMLOG_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_AlarmLog_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_AlarmLog_Info[i].task_result = true;
		}

		for (int i=0;i<BAC_TSTAT_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Tstat_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Tstat_Info[i].task_result = true;
		}


		Show_Results = temp_cs + _T("Success!");
		SetPaneString(BAC_SHOW_MISSION_RESULTS,Show_Results);
		TRACE(Show_Results);
		TRACE(_T("\r\n"));
	}
	else
	{
		for (int i=0;i<BAC_PROGRAM_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Program_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Program_Info[i].task_result = false;
		}

		for (int i=0;i<BAC_PROGRAMCODE_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Programcode_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Programcode_Info[i].task_result = false;
		}

		for (int i=0;i<BAC_OUTPUT_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Output_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Output_Info[i].task_result = false;
		}

		for (int i=0;i<BAC_INPUT_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Input_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Input_Info[i].task_result = false;
		}
		for (int i=0;i<BAC_VARIABLE_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Variable_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Variable_Info[i].task_result = false;
		}

		for (int i=0;i<BAC_WEEKLY_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Weekly_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Weekly_Info[i].task_result = false;
		}

		for (int i=0;i<BAC_WEEKLYCODE_GOUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Weeklycode_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Weeklycode_Info[i].task_result = false;
		}

		for (int i=0;i<BAC_ANNUAL_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Annual_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Annual_Info[i].task_result = false;
		}

		for (int i=0;i<BAC_TIME_COMMAND_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Time_Command[i].invoke_id)
				Bacnet_Refresh_Info.Read_Time_Command[i].task_result = false;
		}

		for (int i=0;i<BAC_BASIC_SETTING_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_BasicSetting_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_BasicSetting_Info[i].task_result = false;
		}

		for (int i=0;i<BAC_CONTROLLER_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Controller_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Controller_Info[i].task_result = false;
		}
		for (int i=0;i<BAC_SCREEN_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Screen_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Screen_Info[i].task_result = false;
		}
		for (int i=0;i<BAC_MONITOR_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Monitor_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Monitor_Info[i].task_result = false;
		}
		for (int i=0;i<BAC_ANNUALCODE_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Annualcode_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Annualcode_Info[i].task_result = false;
		}

		for (int i=0;i<BAC_ALARMLOG_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_AlarmLog_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_AlarmLog_Info[i].task_result = false;
		}
		for (int i=0;i<BAC_TSTAT_GROUP;i++)
		{
			if(pInvoke->Invoke_ID==Bacnet_Refresh_Info.Read_Tstat_Info[i].invoke_id)
				Bacnet_Refresh_Info.Read_Tstat_Info[i].task_result = false;
		}

		Show_Results = temp_cs + _T("Fail!");
		SetPaneString(BAC_SHOW_MISSION_RESULTS,Show_Results);
		//AfxMessageBox(Show_Results);
		//MessageBox(_T("Bacnet operation fail!"));
	}
	
	if(pInvoke)
		delete pInvoke;
	return 0;
}

void CDialogCM5_BacNet::OnBnClickedButtonTest()
{
	// TODO: Add your control notification handler code here
	Send_WhoIs_Global(g_bac_instance, g_bac_instance);
}


void CDialogCM5_BacNet::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	//#ifdef Fance_Enable_Test
	//Fresh();//Fance
	Initial_All_Point();



	//#endif
	// TODO: Add your specialized code here and/or call the base class
}

void CDialogCM5_BacNet::Tab_Initial()
{
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	//为Tab Control增加两个页面;
	m_bac_main_tab.InsertItem(WINDOW_INPUT, _T("Input   "));
	m_bac_main_tab.InsertItem(WINDOW_OUTPUT, _T("Output   "));
	m_bac_main_tab.InsertItem(WINDOW_VARIABLE, _T("Variable   "));
	m_bac_main_tab.InsertItem(WINDOW_PROGRAM, _T("Program   "));
	m_bac_main_tab.InsertItem(WINDOW_CONTROLLER, _T("Controller   "));
	m_bac_main_tab.InsertItem(WINDOW_SCREEN, _T("Graphic   "));
	m_bac_main_tab.InsertItem(WINDOW_WEEKLY, _T("Weekly Routine   "));
	m_bac_main_tab.InsertItem(WINDOW_ANNUAL, _T("Annual Routine   "));
	m_bac_main_tab.InsertItem(WINDOW_MONITOR, _T("Monitor   "));
	m_bac_main_tab.InsertItem(WINDOW_ALARMLOG, _T("Alarm Log   "));
	m_bac_main_tab.InsertItem(WINDOW_TSTAT, _T("Tstat   "));
	m_bac_main_tab.InsertItem(WINDOW_SETTING, _T("Setting   "));

	pDialog[WINDOW_INPUT] = Input_Window = new CBacnetInput;
	pDialog[WINDOW_OUTPUT] =Output_Window = new CBacnetOutput;
	pDialog[WINDOW_VARIABLE] = Variable_Window = new CBacnetVariable;
	pDialog[WINDOW_PROGRAM] = Program_Window = new CBacnetProgram;
	pDialog[WINDOW_CONTROLLER] = Controller_Window = new BacnetController;
	pDialog[WINDOW_SCREEN] = Screen_Window = new BacnetScreen;
	pDialog[WINDOW_WEEKLY] = WeeklyRoutine_Window = new BacnetWeeklyRoutine;
	pDialog[WINDOW_ANNUAL] = AnnualRoutine_Window = new BacnetAnnualRoutine;
	pDialog[WINDOW_MONITOR] = Monitor_Window = new CBacnetMonitor;
	pDialog[WINDOW_ALARMLOG] = AlarmLog_Window = new CBacnetAlarmLog;
	pDialog[WINDOW_TSTAT] = Tstat_Window = new CBacnetTstat;
	pDialog[WINDOW_SETTING] = Setting_Window = new CBacnetSetting;
	//创建两个对话框;
	Input_Window->Create(IDD_DIALOG_BACNET_INPUT, &m_bac_main_tab);
	Output_Window->Create(IDD_DIALOG_BACNET_OUTPUT, &m_bac_main_tab);
	Variable_Window->Create(IDD_DIALOG_BACNET_VARIABLE, &m_bac_main_tab);
	Program_Window->Create(IDD_DIALOG_BACNET_PROGRAM, &m_bac_main_tab);
	Controller_Window->Create(IDD_DIALOG_BACNET_CONTROLLER, &m_bac_main_tab);
	Screen_Window->Create(IDD_DIALOG_BACNET_SCREENS, &m_bac_main_tab);
	WeeklyRoutine_Window->Create(IDD_DIALOG_BACNET_WEEKLY_ROUTINES, &m_bac_main_tab);
	AnnualRoutine_Window->Create(IDD_DIALOG_BACNET_ANNUAL_ROUTINES, &m_bac_main_tab);
	Monitor_Window->Create(IDD_DIALOG_BACNET_MONITOR, &m_bac_main_tab);
	AlarmLog_Window->Create(IDD_DIALOG_BACNET_ALARMLOG,&m_bac_main_tab);
	Tstat_Window->Create(IDD_DIALOG_BACNET_TSTAT,&m_bac_main_tab);
	Setting_Window->Create(IDD_DIALOG_BACNET_SETTING,&m_bac_main_tab);
	//设定在Tab内显示的范围;
	CRect rc;
	m_bac_main_tab.GetClientRect(rc);
	rc.top += 20;
	//rc.bottom -= 8;
	//rc.left += 8;
	//rc.right -= 8;

	for (int i=0;i<WINDOW_TAB_COUNT;i++)
	{
		pDialog[i]->MoveWindow(&rc);
	}


	//显示初始页面
	pDialog[WINDOW_INPUT]->ShowWindow(SW_SHOW);
	pDialog[WINDOW_OUTPUT]->ShowWindow(SW_HIDE);
	pDialog[WINDOW_VARIABLE]->ShowWindow(SW_HIDE);
	pDialog[WINDOW_PROGRAM]->ShowWindow(SW_HIDE);
	pDialog[WINDOW_CONTROLLER]->ShowWindow(SW_HIDE);
	pDialog[WINDOW_SCREEN]->ShowWindow(SW_HIDE);
	pDialog[WINDOW_WEEKLY]->ShowWindow(SW_HIDE);
	pDialog[WINDOW_ANNUAL]->ShowWindow(SW_HIDE);
	pDialog[WINDOW_MONITOR]->ShowWindow(SW_HIDE);
	pDialog[WINDOW_ALARMLOG]->ShowWindow(SW_HIDE);
	pDialog[WINDOW_TSTAT]->ShowWindow(SW_HIDE);
	pDialog[WINDOW_SETTING]->ShowWindow(SW_HIDE);
	g_hwnd_now = m_input_dlg_hwnd;
	//保存当前选择
//	m_CurSelTab = WINDOW_INPUT;

}

void CDialogCM5_BacNet::Initial_All_Point()
{
	m_Input_data.clear();
	m_Variable_data.clear();
	m_Output_data.clear();
	m_Program_data.clear();
	m_Weekly_data.clear();
	m_Annual_data.clear();
	m_Schedual_Time_data.clear();
	m_controller_data.clear();
	m_screen_data.clear();
	m_monitor_data.clear();
	m_alarmlog_data.clear();
	m_Tstat_data.clear();
	//vector <Str_TstatInfo_point> m_Tstat_data;
	for(int i=0;i<BAC_INPUT_ITEM_COUNT;i++)
	{
		Str_in_point temp_in;
		memset(temp_in.description,0,21);
		memset(temp_in.label,0,9);
		m_Input_data.push_back(temp_in);

	}
	for(int i=0;i<BAC_OUTPUT_ITEM_COUNT;i++)
	{
		Str_out_point temp_out;
		memset(temp_out.description,0,21);
		memset(temp_out.label,0,9);
		m_Output_data.push_back(temp_out);
	}
	for (int i=0;i<BAC_VARIABLE_ITEM_COUNT;i++)
	{
		Str_variable_point temp_variable;
		memset(temp_variable.description,0,21);
		memset(temp_variable.label,0,9);
		m_Variable_data.push_back(temp_variable);
	}
	for(int i=0;i<BAC_PROGRAM_ITEM_COUNT;i++)
	{
		Str_program_point temp_program;
		memset(temp_program.description,0,21);
		memset(temp_program.label,0,9);
		temp_program.bytes = 400;//初始化时默认为400的长度，避免读不到数据;
		m_Program_data.push_back(temp_program);
	}
	for(int i=0;i<BAC_WEEKLY_ROUTINES_COUNT;i++)
	{
		Str_weekly_routine_point temp_weekly;
		m_Weekly_data.push_back(temp_weekly);
	}
	for (int i=0;i<BAC_ANNUAL_ROUTINES_COUNT;i++)
	{
		Str_annual_routine_point temp_annual;
		m_Annual_data.push_back(temp_annual);
	}

	for (int i=0;i<BAC_WEEKLY_ROUTINES_COUNT;i++)
	{
		Str_schedual_time_point temp_schedual;
		m_Schedual_Time_data.push_back(temp_schedual);
	}

	for (int i=0;i<BAC_CONTROLLER_COUNT;i++)
	{
		Str_controller_point temp_controller;
		m_controller_data.push_back(temp_controller);
	}
	for (int i=0;i<BAC_SCREEN_COUNT;i++)
	{
		Control_group_point temp_screen;
		m_screen_data.push_back(temp_screen);
	}
	for (int i=0;i<BAC_MONITOR_COUNT;i++)
	{
		Str_monitor_point temp_monitor;
		m_monitor_data.push_back(temp_monitor);
	}

	for (int i=0;i<BAC_ALARMLOG_COUNT;i++)
	{
		Alarm_point temp_alarmpoint;
		memset(temp_alarmpoint.alarm_message,0,ALARM_MESSAGE_SIZE);
		m_alarmlog_data.push_back(temp_alarmpoint);
	}
	for(int i=0;i<BAC_TSTAT_COUNT;i++)
	{
		Str_TstatInfo_point temp_tststpoint;
		temp_tststpoint.product_model = 255;  //default 255  means no device;
		m_Tstat_data.push_back(temp_tststpoint);
		
	}
	
}
//__declspec(dllexport) HANDLE	Get_RS485_Handle();
HWND temp_hwnd;
//INPUT int test_function_return_value();
void CDialogCM5_BacNet::Fresh()
{
	PHOSTENT  hostinfo;  
	char  name[255];  
	CString  ip; 

	if(  gethostname  (  name,  sizeof(name))  ==  0)  
	{  
		if((hostinfo  =  gethostbyname(name))  !=  NULL)  
		{  
			ip  =  inet_ntoa  (*(struct  in_addr  *)*hostinfo->h_addr_list);  
		}  
	}  

	CStringArray TEMPCS;
	SplitCStringA(TEMPCS, ip, _T("."));
	int ttttt = TEMPCS.GetSize();
	if((int)TEMPCS.GetSize() ==4)
	{
		if(0)
		{
		my_ip[0] = _wtoi(TEMPCS.GetAt(0));
		my_ip[1] = _wtoi(TEMPCS.GetAt(1));
		my_ip[2] = _wtoi(TEMPCS.GetAt(2));
		my_ip[3] = _wtoi(TEMPCS.GetAt(3));
		}
		my_ip[3] = _wtoi(TEMPCS.GetAt(0));
		my_ip[2] = _wtoi(TEMPCS.GetAt(1));
		my_ip[1] = _wtoi(TEMPCS.GetAt(2));
		my_ip[0] = _wtoi(TEMPCS.GetAt(3));
	}


	Initial_bac(g_gloab_bac_comport);
	//m_cm5_date_picker.EnableWindow(0);
	//m_cm5_time_picker.EnableWindow(0);
	//GetDlgItem(IDC_BAC_SYNC_LOCAL_PC)->EnableWindow(0);
	//GetDlgItem(IDC_BTN_BAC_WRITE_TIME)->EnableWindow(FALSE);
	//m_cm5_time_picker.SetFormat(_T("HH:mm"));

	SetTimer(1,500,NULL);
	SetTimer(2,15000,NULL);//定时器2用于间隔发送 whois;不知道设备什么时候会被移除;
	SetTimer(3,1000,NULL); //Check whether need  show Alarm dialog.
	BacNet_hwd = this->m_hWnd;

	CString temp_cs;
	temp_cs.Format(_T("%d"),g_bac_instance);
	GetDlgItem(IDC_STATIC_CM_DEVICE_ID)->SetWindowTextW(temp_cs);
	temp_cs.Format(_T("%d"),g_mac);
	GetDlgItem(IDC_STATIC_CM5_MAC)->SetWindowTextW(temp_cs);
	Station_NUM = g_mac;

	//GetDlgItem(IDC_STATIC_CM_DEVICE_ID)->SetWindowTextW(bac_cs_device_id);
	//GetDlgItem(IDC_STATIC_CM5_MAC)->SetWindowTextW(bac_cs_mac);
	GetDlgItem(IDC_STATIC_CM5_VENDOR_ID)->SetWindowTextW(bac_cs_vendor_id);
	//Send_WhoIs_Global(-1,-1);
	Send_WhoIs_Global(g_bac_instance, g_bac_instance);
	SetTimer(4,500,NULL);
	click_resend_time = 4;
#if 0
	Sleep(300);
	bool find_exsit = false;

	for (int i=0;i<(int)m_bac_scan_com_data.size();i++)
	{
		if(m_bac_scan_com_data.at(i).device_id == g_bac_instance)
			find_exsit = true;
	}
	
	if(find_exsit)
	{
		//PostMessage(WM_FRESH_CM_LIST,MENU_CLICK,TYPE_SVAE_CONFIG);
 		//Post_Write_Message(g_bac_instance,CONNECTED_WITH_DEVICE,0,0,sizeof(Str_connected_point),BacNet_hwd ,_T("Connect with device"));
		#if 1
		PostMessage(WM_FRESH_CM_LIST,MENU_CLICK,TYPE_ALL);
		CString temp_cstring;
		temp_cstring.Format(_T("Connected"),g_bac_instance);
		SetPaneString(1,temp_cstring);
		m_bac_main_tab.SetFocus();
		#endif
	}
	else
	{
		SetPaneString(1,_T("Device is offline!"));
		MessageBox(_T("Device is offline!"),_T("Notice"),MB_OK | MB_ICONINFORMATION);;
	}
#endif
	//prevent when user doesn't click the bacnet device,the view also initial ,It's a waste of resource;
#if 1
	static bool initial_once = true;
	
	if(initial_once)
	{
		initial_once = false;
		Tab_Initial();
	}
#endif


}









typedef struct BACnet_Object_Property_Value_Own {
	BACNET_OBJECT_TYPE object_type;
	uint32_t object_instance;
	BACNET_PROPERTY_ID object_property;
	uint32_t array_index;
	BACNET_APPLICATION_DATA_VALUE value;
	HTREEITEM t_PropertyChild;
} BACNET_OBJECT_PROPERTY_VALUE_Own;

typedef struct _DEVICE_INFO
{
	uint32_t i_device_id;
	uint32_t i_vendor_id;
	uint32_t i_mac;
	HTREEITEM t_DeviceChild;
	vector	<BACnet_Object_Property_Value_Own> my_Property_value;

}DEVICE_INFO;



//vector <_DEVICE_INFO> g_device_info;

//Str_in_point Private_data[100];
//int Private_data_length;
//HWND      m_input_dlg_hwnd;
//HWND      m_pragram_dlg_hwnd;


//extern  MSTP_Port;
volatile struct mstp_port_struct_t MSTP_Port;
static void Read_Properties(
    void)
{
    uint32_t device_id = 0;
    bool status = false;
    unsigned max_apdu = 0;
    BACNET_ADDRESS src;
    bool next_device = false;
    static unsigned index = 0;
    static unsigned property = 0;
    /* list of required (and some optional) properties in the
       Device Object
       note: you could just loop through
       all the properties in all the objects. */
	 const int object_props[] = {
	PROP_OBJECT_LIST//PROP_MODEL_NAME//PROP_OBJECT_LIST
	 };
    //const int object_props[] = {
    //    PROP_OBJECT_IDENTIFIER,
    //    PROP_OBJECT_NAME,
    //    PROP_OBJECT_TYPE,
    //    PROP_SYSTEM_STATUS,
    //    PROP_VENDOR_NAME,
    //    PROP_VENDOR_IDENTIFIER,
    //    PROP_MODEL_NAME,
    //    PROP_FIRMWARE_REVISION,
    //    PROP_APPLICATION_SOFTWARE_VERSION,
    //    PROP_PROTOCOL_VERSION,
    //    PROP_PROTOCOL_SERVICES_SUPPORTED,
    //    PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED,
    //    PROP_MAX_APDU_LENGTH_ACCEPTED,
    //    PROP_SEGMENTATION_SUPPORTED,
    //    PROP_LOCAL_TIME,
    //    PROP_LOCAL_DATE,
    //    PROP_UTC_OFFSET,
    //    PROP_DAYLIGHT_SAVINGS_STATUS,
    //    PROP_APDU_SEGMENT_TIMEOUT,
    //    PROP_APDU_TIMEOUT,
    //    PROP_NUMBER_OF_APDU_RETRIES,
    //    PROP_TIME_SYNCHRONIZATION_RECIPIENTS,
    //    PROP_MAX_MASTER,
    //    PROP_MAX_INFO_FRAMES,
    //    PROP_DEVICE_ADDRESS_BINDING,
    //    /* note: PROP_OBJECT_LIST is missing cause
    //       we need to get it with an index method since
    //       the list could be very large */
    //    /* some proprietary properties */
    //    514, 515,
    //    /* end of list */
    //    -1
    //};

    if (address_count()) {
        if (address_get_by_index(index, &device_id, &max_apdu, &src)) {
            if (object_props[property] < 0)
                next_device = true;
            else {
                /*status*/ g_invoke_id= Send_Read_Property_Request(device_id,  /* destination device */
                    OBJECT_DEVICE, device_id, (BACNET_PROPERTY_ID)object_props[property],
                    BACNET_ARRAY_ALL);
              //  if (status)	Fance
              //      property++;
            }
        } else
            next_device = true;
        if (next_device) {
            next_device = false;
           // index++;Fance
            if (index >= MAX_ADDRESS_CACHE)
                index = 0;
            property = 0;
        }
    }

    return;
}
//uint32_t g_bac_instance;
//uint32_t g_vendor_id;
//uint32_t g_mac;
//HTREEITEM FirstChild ;
//HTREEITEM SecondChild ;
//HTREEITEM DeviceChild;





/*
 * This is called when we receive a private transfer packet.
 * We parse the data, send the private part for processing and then send the
 * response which the application generates.
 * If there are any BACnet level errors we send an error response from here.
 * If there are any application level errors they will be packeged up in the
 * response block which we send back to the originator of the request.
 *
 */



 






#if 0
void Localhandler_read_property_ack(
	uint8_t * service_request,
	uint16_t service_len,
	BACNET_ADDRESS * src,
	BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data)
{
	int len = 0;
	BACNET_READ_PROPERTY_DATA data;

	(void) src;
	(void) service_data;        /* we could use these... */
	len = rp_ack_decode_service_request(service_request, service_len, &data);
#if 0
	fprintf(stderr, "Received Read-Property Ack!\n");
#endif
	if (len > 0)
	{
		local_rp_ack_print_data(&data);
		::PostMessage(BacNet_hwd,WM_FRESH_CM_LIST,WM_COMMAND_WHO_IS,NULL);
	}
}
#endif

#if 0
static void LocalIAmHandler(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src)
{
	
    int len = 0;
    uint32_t device_id = 0;
    unsigned max_apdu = 0;
    int segmentation = 0;
    uint16_t vendor_id = 0;

    (void) src;
    (void) service_len;
    len =  iam_decode_service_request(service_request, &device_id, &max_apdu,
        &segmentation, &vendor_id);




    fprintf(stderr, "Received I-Am Request");
    if (len != -1) 
	{
        fprintf(stderr, " from %u!\n", device_id);
        address_add(device_id, max_apdu, src);
    } else
        fprintf(stderr, "!\n");

	if(src->mac_len==1)
		temp_mac.Format(_T("%d"),src->mac[0]);

	temp_device_id.Format(_T("%d"),device_id);
	temp_vendor_id.Format(_T("%d"),vendor_id);
	g_bac_instance =device_id;
	g_mac = _wtoi(temp_mac);

	TRACE(_T("Find ") + temp_device_id +_T("  ") + temp_mac + _T("\r\n"));

	::PostMessage(BacNet_hwd,WM_FRESH_CM_LIST,WM_COMMAND_WHO_IS,NULL);
	return;

}
#endif


LRESULT CDialogCM5_BacNet::Fresh_UI(WPARAM wParam,LPARAM lParam)
{
	int command_type = wParam;
	int button_click = 0;
	CString temp_cs;
	CTime	TimeTemp;
	switch(command_type)
	{
	case WM_COMMAND_WHO_IS:
		temp_cs.Format(_T("%d"),g_bac_instance);
		GetDlgItem(IDC_STATIC_CM_DEVICE_ID)->SetWindowTextW(temp_cs);
		temp_cs.Format(_T("%d"),g_mac);
		GetDlgItem(IDC_STATIC_CM5_MAC)->SetWindowTextW(temp_cs);
		Station_NUM = g_mac;
#if 0
		/*((CStatic *)*/GetDlgItem(IDC_STATIC_CM_DEVICE_ID)->SetWindowTextW(bac_cs_device_id);
		g_bac_instance = _wtoi(bac_cs_device_id);
		/*((CStatic *)*/GetDlgItem(IDC_STATIC_CM5_MAC)->SetWindowTextW(bac_cs_mac);
		bac_gloab_panel = _wtoi(bac_cs_mac);
		Station_NUM = bac_gloab_panel;;
		/*((CStatic *)*/GetDlgItem(IDC_STATIC_CM5_VENDOR_ID)->SetWindowTextW(bac_cs_vendor_id);
#endif
		break;
	//case TIME_COMMAND:
	//	if(Device_time.year<2000)
	//		temp_year = Device_time.year + 2000;
	//	TimeTemp = CTime(temp_year,Device_time.month,Device_time.dayofmonth,Device_time.ti_hour,Device_time.ti_min,Device_time.ti_sec);
	//	

	//	m_cm5_time_picker.SetFormat(_T("HH:mm"));
	//	m_cm5_time_picker.SetTime(&TimeTemp);

	//	//m_cm5_date_picker.SetFormat(_T("YY/MM/DD"));
	//	m_cm5_date_picker.SetTime(&TimeTemp);
	//	break;
	//case READ_SETTING_COMMAND:
	//	((CIPAddressCtrl *)GetDlgItem(IDC_IPADDRESS_BAC_IP))->SetAddress(Device_Basic_Setting.reg.ip_addr[0],
	//		Device_Basic_Setting.reg.ip_addr[1],Device_Basic_Setting.reg.ip_addr[2],Device_Basic_Setting.reg.ip_addr[3]);
	//	((CIPAddressCtrl *)GetDlgItem(IDC_IPADDRESS_BAC_SUBNET))->SetAddress(Device_Basic_Setting.reg.subnet[0],
	//		Device_Basic_Setting.reg.subnet[1],Device_Basic_Setting.reg.subnet[2],Device_Basic_Setting.reg.subnet[3]);
	//	((CIPAddressCtrl *)GetDlgItem(IDC_IPADDRESS_BAC_GATEWAY))->SetAddress(Device_Basic_Setting.reg.gate_addr[0],
	//		Device_Basic_Setting.reg.gate_addr[1],Device_Basic_Setting.reg.gate_addr[2],Device_Basic_Setting.reg.gate_addr[3]);

	//	if(Device_Basic_Setting.reg.tcp_type == 0)
	//	{
	//		((CButton *)GetDlgItem(IDC_RADIO_BAC_IP_AUTO))->SetCheck(true);
	//		((CButton *)GetDlgItem(IDC_RADIO_BAC_IP_STATIC))->SetCheck(false);
	//		((CIPAddressCtrl *)GetDlgItem(IDC_IPADDRESS_BAC_IP))->EnableWindow(FALSE);
	//		((CIPAddressCtrl *)GetDlgItem(IDC_IPADDRESS_BAC_SUBNET))->EnableWindow(FALSE);
	//		((CIPAddressCtrl *)GetDlgItem(IDC_IPADDRESS_BAC_GATEWAY))->EnableWindow(FALSE);
	//	}
	//	else if(Device_Basic_Setting.reg.tcp_type == 1)
	//	{
	//		((CButton *)GetDlgItem(IDC_RADIO_BAC_IP_AUTO))->SetCheck(false);
	//		((CButton *)GetDlgItem(IDC_RADIO_BAC_IP_STATIC))->SetCheck(true);
	//		((CIPAddressCtrl *)GetDlgItem(IDC_IPADDRESS_BAC_IP))->EnableWindow(true);
	//		((CIPAddressCtrl *)GetDlgItem(IDC_IPADDRESS_BAC_SUBNET))->EnableWindow(true);
	//		((CIPAddressCtrl *)GetDlgItem(IDC_IPADDRESS_BAC_GATEWAY))->EnableWindow(true);
	//	}
	//	break;
	case MENU_CLICK:

		button_click = lParam;
		if(button_click == WRITEPRGFLASH_COMMAND)
		{
			WriteFlash();
		}
		else
		{
			tsm_free_all_invoke_id();//每次点击的时候都将所有INVOKE ID 清零;
			Show_Wait_Dialog_And_SendMessage(button_click);
		}
		break;
	case TYPE_SVAE_CONFIG:
		tsm_free_all_invoke_id();//每次点击的时候都将所有INVOKE ID 清零;
		Show_Wait_Dialog_And_SendMessage(TYPE_SVAE_CONFIG);
		break;
	default: 
		break;
	}

	
	return 0;
}




void CDialogCM5_BacNet::Show_Wait_Dialog_And_SendMessage(int read_list_type)
{
	bac_read_which_list = read_list_type;
	if(WaitDlg==NULL)
	{
		if(bac_read_which_list == BAC_READ_SVAE_CONFIG)
		{
			WaitDlg = new BacnetWait((int)BAC_WAIT_READ_DATA_WRITE_CONFIG);//如果是保存为ini文件 就要读取全部的data;
		}
		else
		{
			WaitDlg = new BacnetWait((int)BAC_WAIT_NORMAL_READ);
		}
		WaitDlg->Create(IDD_DIALOG_BACNET_WAIT,this);
		WaitDlg->ShowWindow(SW_SHOW);


		RECT RECT_SET1;
		GetClientRect(&RECT_SET1);
		ClientToScreen(&RECT_SET1);
		WaitDlg->MoveWindow(RECT_SET1.left + 50,RECT_SET1.bottom - 19,800,20);
		//RECT RECT_SET1;
		//GetWindowRect(&RECT_SET1);
		//WaitDlg->MoveWindow(RECT_SET1.left+100,RECT_SET1.top+400,560/*RECT_SET1.left+270*//*RECT_SET1.right/2+20*/,100);
	}

	//::PostMessage(BacNet_hwd,WM_SEND_OVER,0,0);
	if(hwait_thread==NULL)
	{
		hwait_thread =CreateThread(NULL,NULL,Send_read_Command_Thread,this,NULL, NULL);

	}
}



static int resend_count=0;
DWORD WINAPI  CDialogCM5_BacNet::Send_read_Command_Thread(LPVOID lpVoid)
{
	CDialogCM5_BacNet *pParent = (CDialogCM5_BacNet *)lpVoid;
	bac_read_all_results = false;
	for (int i=0;i<BAC_TSTAT_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_TSTAT_LIST) || (bac_read_which_list == BAC_READ_ALL_LIST))
		{
			Bacnet_Refresh_Info.Read_Tstat_Info[i].command = 0;
			Bacnet_Refresh_Info.Read_Tstat_Info[i].device_id = 0;
			Bacnet_Refresh_Info.Read_Tstat_Info[i].start_instance =0;
			Bacnet_Refresh_Info.Read_Tstat_Info[i].end_instance = 0;
			Bacnet_Refresh_Info.Read_Tstat_Info[i].has_resend_yes_or_no = 0;
			Bacnet_Refresh_Info.Read_Tstat_Info[i].resend_count =0;
			Bacnet_Refresh_Info.Read_Tstat_Info[i].task_result = BAC_RESULTS_UNKONW;
			Bacnet_Refresh_Info.Read_Tstat_Info[i].invoke_id = -1;
			bac_tstat_read_results = 0;
			Bacnet_Refresh_Info.Read_Tstat_Info[i].timeout_count = 0;
		}
	}
	for (int i=0; i<BAC_MONITOR_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_MONITOR_LIST) || (bac_read_which_list == TYPE_SVAE_CONFIG)|| (bac_read_which_list ==BAC_READ_ALL_LIST))
		//if(bac_read_which_list == BAC_READ_MONITOR_LIST)
		{
			Bacnet_Refresh_Info.Read_Monitor_Info[i].command = 0;
			Bacnet_Refresh_Info.Read_Monitor_Info[i].device_id = 0;
			Bacnet_Refresh_Info.Read_Monitor_Info[i].start_instance =0;
			Bacnet_Refresh_Info.Read_Monitor_Info[i].end_instance = 0;
			Bacnet_Refresh_Info.Read_Monitor_Info[i].has_resend_yes_or_no = 0;
			Bacnet_Refresh_Info.Read_Monitor_Info[i].resend_count =0;
			Bacnet_Refresh_Info.Read_Monitor_Info[i].task_result = BAC_RESULTS_UNKONW;
			Bacnet_Refresh_Info.Read_Monitor_Info[i].invoke_id = -1;
			bac_monitor_read_results = 0;
			Bacnet_Refresh_Info.Read_Monitor_Info[i].timeout_count = 0;
		}
	}

	for (int i=0;i<BAC_SCREEN_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_SCREEN_LIST) || (bac_read_which_list == TYPE_SVAE_CONFIG)|| (bac_read_which_list ==BAC_READ_ALL_LIST))
		{
			Bacnet_Refresh_Info.Read_Screen_Info[i].command = 0;
			Bacnet_Refresh_Info.Read_Screen_Info[i].device_id = 0;
			Bacnet_Refresh_Info.Read_Screen_Info[i].start_instance =0;
			Bacnet_Refresh_Info.Read_Screen_Info[i].end_instance = 0;
			Bacnet_Refresh_Info.Read_Screen_Info[i].has_resend_yes_or_no = 0;
			Bacnet_Refresh_Info.Read_Screen_Info[i].resend_count =0;
			Bacnet_Refresh_Info.Read_Screen_Info[i].task_result = BAC_RESULTS_UNKONW;
			Bacnet_Refresh_Info.Read_Screen_Info[i].invoke_id = -1;
			bac_screen_read_results = 0;
			Bacnet_Refresh_Info.Read_Screen_Info[i].timeout_count = 0;
		}
	}

	for (int i=0;i<BAC_TIME_COMMAND_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_TIME_COMMAND) || (bac_read_which_list ==BAC_READ_ALL_LIST) || (bac_read_which_list == TYPE_SVAE_CONFIG))
		{
			Bacnet_Refresh_Info.Read_Time_Command[i].command = 0;
			Bacnet_Refresh_Info.Read_Time_Command[i].device_id = 0;
			Bacnet_Refresh_Info.Read_Time_Command[i].start_instance =0;
			Bacnet_Refresh_Info.Read_Time_Command[i].end_instance = 0;
			Bacnet_Refresh_Info.Read_Time_Command[i].has_resend_yes_or_no = 0;
			Bacnet_Refresh_Info.Read_Time_Command[i].resend_count =0;
			Bacnet_Refresh_Info.Read_Time_Command[i].task_result = BAC_RESULTS_UNKONW;
			Bacnet_Refresh_Info.Read_Time_Command[i].invoke_id = -1;
			bac_time_command_read_results = 0;
			Bacnet_Refresh_Info.Read_Time_Command[i].timeout_count = 0;
		}
	}

	for (int i=0;i<BAC_BASIC_SETTING_GROUP;i++)//prg文件暂时不保存这些基本设置的东西，像IP地址和mac地址;
	{
		if((bac_read_which_list == BAC_READ_BASIC_SETTING_COMMAND) || (bac_read_which_list ==BAC_READ_ALL_LIST))
		{
			Bacnet_Refresh_Info.Read_BasicSetting_Info[i].command = 0;
			Bacnet_Refresh_Info.Read_BasicSetting_Info[i].device_id = 0;
			Bacnet_Refresh_Info.Read_BasicSetting_Info[i].start_instance =0;
			Bacnet_Refresh_Info.Read_BasicSetting_Info[i].end_instance = 0;
			Bacnet_Refresh_Info.Read_BasicSetting_Info[i].has_resend_yes_or_no = 0;
			Bacnet_Refresh_Info.Read_BasicSetting_Info[i].resend_count =0;
			Bacnet_Refresh_Info.Read_BasicSetting_Info[i].task_result = BAC_RESULTS_UNKONW;
			Bacnet_Refresh_Info.Read_BasicSetting_Info[i].invoke_id = -1;
			bac_basic_setting_read_results = 0;
			Bacnet_Refresh_Info.Read_BasicSetting_Info[i].timeout_count = 0;
		}
	}

	

	for(int i=0;i<BAC_WEEKLY_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_WEEKLY_LIST) || (bac_read_which_list ==BAC_READ_ALL_LIST) || (bac_read_which_list ==TYPE_SVAE_CONFIG))
		{
			Bacnet_Refresh_Info.Read_Weekly_Info[i].command = 0;
			Bacnet_Refresh_Info.Read_Weekly_Info[i].device_id = 0;
			Bacnet_Refresh_Info.Read_Weekly_Info[i].start_instance =0;
			Bacnet_Refresh_Info.Read_Weekly_Info[i].end_instance =0;
			Bacnet_Refresh_Info.Read_Weekly_Info[i].has_resend_yes_or_no = 0;
			Bacnet_Refresh_Info.Read_Weekly_Info[i].resend_count = 0;
			Bacnet_Refresh_Info.Read_Weekly_Info[i].task_result = BAC_RESULTS_UNKONW;
			Bacnet_Refresh_Info.Read_Weekly_Info[i].invoke_id = -1;
			bac_weekly_read_results = 0;
			Bacnet_Refresh_Info.Read_Weekly_Info[i].timeout_count = 0;
		}
	}

	for(int i=0;i<BAC_ANNUAL_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_ANNUAL_LIST) ||(bac_read_which_list ==BAC_READ_ALL_LIST) || (bac_read_which_list ==TYPE_SVAE_CONFIG))
		{
			Bacnet_Refresh_Info.Read_Annual_Info[i].command = 0;
			Bacnet_Refresh_Info.Read_Annual_Info[i].device_id = 0;
			Bacnet_Refresh_Info.Read_Annual_Info[i].start_instance =0;
			Bacnet_Refresh_Info.Read_Annual_Info[i].end_instance =0;
			Bacnet_Refresh_Info.Read_Annual_Info[i].has_resend_yes_or_no = 0;
			Bacnet_Refresh_Info.Read_Annual_Info[i].resend_count = 0;

			Bacnet_Refresh_Info.Read_Annual_Info[i].task_result = BAC_RESULTS_UNKONW;
			Bacnet_Refresh_Info.Read_Annual_Info[i].invoke_id = -1;

			bac_annual_read_results = 0;
			Bacnet_Refresh_Info.Read_Annual_Info[i].timeout_count = 0;
		}
	}
	
	for (int i=0;i<BAC_PROGRAMCODE_GROUP;i++)
	{

		if((bac_read_which_list == BAC_READ_PROGRAMCODE_LIST) || (bac_read_which_list ==TYPE_SVAE_CONFIG))
		{
			Bacnet_Refresh_Info.Read_Programcode_Info[i].command = 0;
			Bacnet_Refresh_Info.Read_Programcode_Info[i].device_id = 0;
			Bacnet_Refresh_Info.Read_Programcode_Info[i].start_instance =0;
			Bacnet_Refresh_Info.Read_Programcode_Info[i].end_instance =0;
			Bacnet_Refresh_Info.Read_Programcode_Info[i].has_resend_yes_or_no = 0;
			Bacnet_Refresh_Info.Read_Programcode_Info[i].resend_count = 0;

			Bacnet_Refresh_Info.Read_Programcode_Info[i].task_result = BAC_RESULTS_UNKONW;
			Bacnet_Refresh_Info.Read_Programcode_Info[i].invoke_id = -1;
			bac_programcode_read_results = 0;
			//bac_program_read_results = 0;
			Bacnet_Refresh_Info.Read_Programcode_Info[i].timeout_count = 0;
		}
	}

	for (int i=0;i<BAC_WEEKLYCODE_GOUP;i++)
	{

		if((bac_read_which_list == BAC_READ_WEEKLTCODE_LIST) || (bac_read_which_list ==TYPE_SVAE_CONFIG))
		{
			Bacnet_Refresh_Info.Read_Weeklycode_Info[i].command = 0;
			Bacnet_Refresh_Info.Read_Weeklycode_Info[i].device_id = 0;
			Bacnet_Refresh_Info.Read_Weeklycode_Info[i].start_instance =0;
			Bacnet_Refresh_Info.Read_Weeklycode_Info[i].end_instance =0;
			Bacnet_Refresh_Info.Read_Weeklycode_Info[i].has_resend_yes_or_no = 0;
			Bacnet_Refresh_Info.Read_Weeklycode_Info[i].resend_count = 0;

			Bacnet_Refresh_Info.Read_Weeklycode_Info[i].task_result = BAC_RESULTS_UNKONW;
			Bacnet_Refresh_Info.Read_Weeklycode_Info[i].invoke_id = -1;
			bac_weeklycode_read_results = 0;
			Bacnet_Refresh_Info.Read_Weeklycode_Info[i].timeout_count = 0;
		}
	}

	for (int i=0;i<BAC_PROGRAM_GROUP;i++)
	{

		if((bac_read_which_list == BAC_READ_PROGRAM_LIST) || (bac_read_which_list ==BAC_READ_ALL_LIST) ||(bac_read_which_list ==TYPE_SVAE_CONFIG))
		{
			Bacnet_Refresh_Info.Read_Program_Info[i].command = 0;
			Bacnet_Refresh_Info.Read_Program_Info[i].device_id = 0;
			Bacnet_Refresh_Info.Read_Program_Info[i].start_instance =0;
			Bacnet_Refresh_Info.Read_Program_Info[i].end_instance =0;
			Bacnet_Refresh_Info.Read_Program_Info[i].has_resend_yes_or_no = 0;
			Bacnet_Refresh_Info.Read_Program_Info[i].resend_count = 0;

			Bacnet_Refresh_Info.Read_Program_Info[i].task_result = BAC_RESULTS_UNKONW;
			Bacnet_Refresh_Info.Read_Program_Info[i].invoke_id = -1;

			bac_program_read_results = 0;
			Bacnet_Refresh_Info.Read_Program_Info[i].timeout_count = 0;
		}
	}

	for (int i=0;i<BAC_INPUT_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_INPUT_LIST) || (bac_read_which_list ==BAC_READ_ALL_LIST) ||(bac_read_which_list ==TYPE_SVAE_CONFIG))
		{
			Bacnet_Refresh_Info.Read_Input_Info[i].command = 0;
			Bacnet_Refresh_Info.Read_Input_Info[i].device_id = 0;
			Bacnet_Refresh_Info.Read_Input_Info[i].start_instance =0;
			Bacnet_Refresh_Info.Read_Input_Info[i].end_instance =0;
			Bacnet_Refresh_Info.Read_Input_Info[i].has_resend_yes_or_no = 0;
			Bacnet_Refresh_Info.Read_Input_Info[i].resend_count = 0;
			Bacnet_Refresh_Info.Read_Input_Info[i].task_result = BAC_RESULTS_UNKONW;
			Bacnet_Refresh_Info.Read_Input_Info[i].invoke_id = -1;
			bac_input_read_results = 0;
			Bacnet_Refresh_Info.Read_Input_Info[i].timeout_count = 0;

		}
	}
	for (int i=0;i<BAC_ANNUALCODE_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_ANNUALCODE_LIST) || (bac_read_which_list ==BAC_READ_ALL_LIST) ||(bac_read_which_list ==TYPE_SVAE_CONFIG))
		{
			Bacnet_Refresh_Info.Read_Annualcode_Info[i].command = 0;
			Bacnet_Refresh_Info.Read_Annualcode_Info[i].device_id = 0;
			Bacnet_Refresh_Info.Read_Annualcode_Info[i].start_instance =0;
			Bacnet_Refresh_Info.Read_Annualcode_Info[i].end_instance =0;
			Bacnet_Refresh_Info.Read_Annualcode_Info[i].has_resend_yes_or_no = 0;
			Bacnet_Refresh_Info.Read_Annualcode_Info[i].resend_count = 0;
			Bacnet_Refresh_Info.Read_Annualcode_Info[i].task_result = BAC_RESULTS_UNKONW;
			Bacnet_Refresh_Info.Read_Annualcode_Info[i].invoke_id = -1;
			bac_annualcode_read_results = 0;
			Bacnet_Refresh_Info.Read_Annualcode_Info[i].timeout_count = 0;

		}
	}

	for (int i=0;i<BAC_OUTPUT_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_OUTPUT_LIST) || (bac_read_which_list ==BAC_READ_ALL_LIST) ||(bac_read_which_list ==TYPE_SVAE_CONFIG))
		{
			Bacnet_Refresh_Info.Read_Output_Info[i].command = 0;
			Bacnet_Refresh_Info.Read_Output_Info[i].device_id = 0;
			Bacnet_Refresh_Info.Read_Output_Info[i].start_instance =0;
			Bacnet_Refresh_Info.Read_Output_Info[i].end_instance =0;
			Bacnet_Refresh_Info.Read_Output_Info[i].has_resend_yes_or_no = 0;
			Bacnet_Refresh_Info.Read_Output_Info[i].resend_count = 0;
			Bacnet_Refresh_Info.Read_Output_Info[i].task_result = BAC_RESULTS_UNKONW;
			Bacnet_Refresh_Info.Read_Output_Info[i].invoke_id = -1;
			bac_output_read_results = 0;
			Bacnet_Refresh_Info.Read_Output_Info[i].timeout_count = 0;

		}
	}

	for (int i=0;i<BAC_VARIABLE_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_VARIABLE_LIST) || (bac_read_which_list ==BAC_READ_ALL_LIST) ||(bac_read_which_list ==TYPE_SVAE_CONFIG))
		{
			Bacnet_Refresh_Info.Read_Variable_Info[i].command = 0;
			Bacnet_Refresh_Info.Read_Variable_Info[i].device_id = 0;
			Bacnet_Refresh_Info.Read_Variable_Info[i].start_instance =0;
			Bacnet_Refresh_Info.Read_Variable_Info[i].end_instance =0;
			Bacnet_Refresh_Info.Read_Variable_Info[i].has_resend_yes_or_no = 0;
			Bacnet_Refresh_Info.Read_Variable_Info[i].task_result = BAC_RESULTS_UNKONW;
			Bacnet_Refresh_Info.Read_Variable_Info[i].invoke_id = -1;
			Bacnet_Refresh_Info.Read_Variable_Info[i].resend_count = 0;
			bac_variable_read_results = 0;
			Bacnet_Refresh_Info.Read_Variable_Info[i].timeout_count = 0;
		}
	}

	for (int i=0;i<BAC_CONTROLLER_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_CONTROLLER_LIST) || (bac_read_which_list == TYPE_SVAE_CONFIG)|| (bac_read_which_list ==BAC_READ_ALL_LIST))
		{
			Bacnet_Refresh_Info.Read_Controller_Info[i].command = 0;
			Bacnet_Refresh_Info.Read_Controller_Info[i].device_id = 0;
			Bacnet_Refresh_Info.Read_Controller_Info[i].start_instance =0;
			Bacnet_Refresh_Info.Read_Controller_Info[i].end_instance =0;
			Bacnet_Refresh_Info.Read_Controller_Info[i].has_resend_yes_or_no = 0;
			Bacnet_Refresh_Info.Read_Controller_Info[i].task_result = BAC_RESULTS_UNKONW;
			Bacnet_Refresh_Info.Read_Controller_Info[i].invoke_id = -1;
			Bacnet_Refresh_Info.Read_Controller_Info[i].resend_count = 0;
			bac_controller_read_results = 0;
				Bacnet_Refresh_Info.Read_Controller_Info[i].timeout_count = 0;
		}
	}


	for (int i=0;i<BAC_ALARMLOG_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_ALARMLOG_LIST)/* || (bac_read_which_list ==BAC_READ_ALL_LIST)*/)
		{
			Bacnet_Refresh_Info.Read_AlarmLog_Info[i].command = 0;
			Bacnet_Refresh_Info.Read_AlarmLog_Info[i].device_id = 0;
			Bacnet_Refresh_Info.Read_AlarmLog_Info[i].start_instance =0;
			Bacnet_Refresh_Info.Read_AlarmLog_Info[i].end_instance =0;
			Bacnet_Refresh_Info.Read_AlarmLog_Info[i].has_resend_yes_or_no = 0;
			Bacnet_Refresh_Info.Read_AlarmLog_Info[i].task_result = BAC_RESULTS_UNKONW;
			Bacnet_Refresh_Info.Read_AlarmLog_Info[i].invoke_id = -1;
			Bacnet_Refresh_Info.Read_AlarmLog_Info[i].resend_count = 0;
			bac_alarmlog_read_results = 0;
			Bacnet_Refresh_Info.Read_AlarmLog_Info[i].timeout_count = 0;
		}
	}

	for (int i=0;i<BAC_TSTAT_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_TSTAT_LIST) || (bac_read_which_list ==BAC_READ_ALL_LIST))
		{
			resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				g_invoke_id = GetPrivateData(g_bac_instance,READTSTAT_T3000,(BAC_READ_GROUP_NUMBER)*i,
					3+(BAC_READ_GROUP_NUMBER)*i,sizeof(Str_TstatInfo_point));
				Sleep(SEND_COMMAND_DELAY_TIME);	
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_Tstat_Info[i].command = READTSTAT_T3000;
			Bacnet_Refresh_Info.Read_Tstat_Info[i].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_Tstat_Info[i].start_instance = (BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Tstat_Info[i].end_instance =3+(BAC_READ_GROUP_NUMBER)*i;
			//Bacnet_Refresh_Info.Read_Monitor_Info[i].end_instance =2+(BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Tstat_Info[i].invoke_id = g_invoke_id;
			CString temp_cs;
			temp_cs.Format(_T("Task ID = %d. Read Tstat From %d to %d "),g_invoke_id,
				Bacnet_Refresh_Info.Read_Tstat_Info[i].start_instance,
				Bacnet_Refresh_Info.Read_Tstat_Info[i].end_instance);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
		}
	}

	for (int i=0;i<BAC_MONITOR_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_MONITOR_LIST) || (bac_read_which_list == TYPE_SVAE_CONFIG) || (bac_read_which_list ==BAC_READ_ALL_LIST))
		//if(bac_read_which_list == BAC_READ_MONITOR_LIST)
		{
			resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				g_invoke_id = GetPrivateData(g_bac_instance,READMONITOR_T3000,(BAC_READ_GROUP_NUMBER)*i,
					3+(BAC_READ_GROUP_NUMBER)*i,
				//	2+(BAC_READ_GROUP_NUMBER)*i,
					sizeof(Str_monitor_point));
				Sleep(SEND_COMMAND_DELAY_TIME);	
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_Monitor_Info[i].command = READMONITOR_T3000;
			Bacnet_Refresh_Info.Read_Monitor_Info[i].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_Monitor_Info[i].start_instance = (BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Monitor_Info[i].end_instance =3+(BAC_READ_GROUP_NUMBER)*i;
			//Bacnet_Refresh_Info.Read_Monitor_Info[i].end_instance =2+(BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Monitor_Info[i].invoke_id = g_invoke_id;
			CString temp_cs;
			temp_cs.Format(_T("Task ID = %d. Read Monitor Item From %d to %d "),g_invoke_id,
				Bacnet_Refresh_Info.Read_Monitor_Info[i].start_instance,
				Bacnet_Refresh_Info.Read_Monitor_Info[i].end_instance);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
		}
	}

	for (int i=0;i<BAC_SCREEN_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_SCREEN_LIST) || (bac_read_which_list == TYPE_SVAE_CONFIG)|| (bac_read_which_list ==BAC_READ_ALL_LIST))
		//if(bac_read_which_list == BAC_READ_SCREEN_LIST)
		{
			resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				g_invoke_id = GetPrivateData(g_bac_instance,READSCREEN_T3000,(BAC_READ_GROUP_NUMBER)*i,
					3+(BAC_READ_GROUP_NUMBER)*i,
					sizeof(Control_group_point));
				Sleep(SEND_COMMAND_DELAY_TIME);	
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_Screen_Info[i].command = READSCREEN_T3000;
			Bacnet_Refresh_Info.Read_Screen_Info[i].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_Screen_Info[i].start_instance = (BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Screen_Info[i].end_instance =3+(BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Screen_Info[i].invoke_id = g_invoke_id;
			CString temp_cs;
			temp_cs.Format(_T("Task ID = %d. Read Screens Item From %d to %d "),g_invoke_id,
				Bacnet_Refresh_Info.Read_Screen_Info[i].start_instance,
				Bacnet_Refresh_Info.Read_Screen_Info[i].end_instance);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
		}
	}

	for (int i=0;i<BAC_TIME_COMMAND_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_TIME_COMMAND) || (bac_read_which_list ==BAC_READ_ALL_LIST) || (bac_read_which_list == TYPE_SVAE_CONFIG))
		{
			resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				g_invoke_id = GetPrivateData(g_bac_instance,
					TIME_COMMAND,0,
					0,
					sizeof(Time_block_mini));
				Sleep(SEND_COMMAND_DELAY_TIME);
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_Time_Command[i].command = TIME_COMMAND;
			Bacnet_Refresh_Info.Read_Time_Command[i].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_Time_Command[i].start_instance = 0;
			Bacnet_Refresh_Info.Read_Time_Command[i].end_instance =0;
			Bacnet_Refresh_Info.Read_Time_Command[i].invoke_id = g_invoke_id;

			CString temp_cs_show;
			temp_cs_show.Format(_T("Task ID = %d. Read device time "),g_invoke_id);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs_show);

			//Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,_T("1111111111111"));
			//Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd);
		}
	}



	for (int i=0;i<BAC_BASIC_SETTING_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_BASIC_SETTING_COMMAND) || (bac_read_which_list ==BAC_READ_ALL_LIST))
		{
			resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				g_invoke_id = GetPrivateData(g_bac_instance,
					READ_SETTING_COMMAND,0,
					0,
					sizeof(Str_Setting_Info));
				Sleep(SEND_COMMAND_DELAY_TIME);
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_BasicSetting_Info[i].command = READ_SETTING_COMMAND;
			Bacnet_Refresh_Info.Read_BasicSetting_Info[i].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_BasicSetting_Info[i].start_instance = 0;
			Bacnet_Refresh_Info.Read_BasicSetting_Info[i].end_instance =0;
			Bacnet_Refresh_Info.Read_BasicSetting_Info[i].invoke_id = g_invoke_id;

			CString temp_cs_show;
			temp_cs_show.Format(_T("Task ID = %d. Read Basic Setting "),g_invoke_id);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs_show);

		}
	}

	// TODO: Add your control notification handler code here
	for (int i=0;i<BAC_WEEKLY_GROUP;i++)
	{

		if((bac_read_which_list == BAC_READ_WEEKLY_LIST) || (bac_read_which_list ==BAC_READ_ALL_LIST) || (bac_read_which_list ==TYPE_SVAE_CONFIG))
		//if(bac_read_which_list == BAC_READ_WEEKLY_LIST)
		{
			resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				g_invoke_id = GetPrivateData(g_bac_instance,
					READWEEKLYROUTINE_T3000,(BAC_READ_GROUP_NUMBER)*i,
					3+(BAC_READ_GROUP_NUMBER)*i,
					sizeof(Str_weekly_routine_point));
				Sleep(SEND_COMMAND_DELAY_TIME);
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_Weekly_Info[i].command = READWEEKLYROUTINE_T3000;
			Bacnet_Refresh_Info.Read_Weekly_Info[i].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_Weekly_Info[i].start_instance = (BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Weekly_Info[i].end_instance =3+(BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Weekly_Info[i].invoke_id = g_invoke_id;

			CString temp_cs;
			temp_cs.Format(_T("Task ID = %d. Read Weekly Routine Item From %d to %d "),g_invoke_id,
				Bacnet_Refresh_Info.Read_Weekly_Info[i].start_instance,
				Bacnet_Refresh_Info.Read_Weekly_Info[i].end_instance);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
		}
	}

	for (int i=0;i<BAC_ANNUAL_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_ANNUAL_LIST) ||(bac_read_which_list ==BAC_READ_ALL_LIST) || (bac_read_which_list ==TYPE_SVAE_CONFIG))
		//if(bac_read_which_list == BAC_READ_ANNUAL_LIST)
		{
			resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				g_invoke_id = GetPrivateData(g_bac_instance,
					READANNUALROUTINE_T3000,(BAC_READ_GROUP_NUMBER)*i,
					3+(BAC_READ_GROUP_NUMBER)*i,
					sizeof(Str_annual_routine_point));
				Sleep(SEND_COMMAND_DELAY_TIME);
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_Annual_Info[i].command = READANNUALROUTINE_T3000;
			Bacnet_Refresh_Info.Read_Annual_Info[i].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_Annual_Info[i].start_instance = (BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Annual_Info[i].end_instance =3+(BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Annual_Info[i].invoke_id = g_invoke_id;


			CString temp_cs;
			temp_cs.Format(_T("Task ID = %d. Read Annual Routine Item From %d to %d "),g_invoke_id,
				Bacnet_Refresh_Info.Read_Annual_Info[i].start_instance,
				Bacnet_Refresh_Info.Read_Annual_Info[i].end_instance);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
		}
	}


	for (int i=0;i<BAC_OUTPUT_GROUP;i++)
	{
		
		if((bac_read_which_list == BAC_READ_OUTPUT_LIST) || (bac_read_which_list ==BAC_READ_ALL_LIST) || (bac_read_which_list ==TYPE_SVAE_CONFIG))
		{
			resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				g_invoke_id = GetPrivateData(g_bac_instance,
											 READOUTPUT_T3000,(BAC_READ_GROUP_NUMBER)*i,
											 3+(BAC_READ_GROUP_NUMBER)*i,
											 sizeof(Str_out_point));
				Sleep(SEND_COMMAND_DELAY_TIME);
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_Output_Info[i].command = READOUTPUT_T3000;
			Bacnet_Refresh_Info.Read_Output_Info[i].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_Output_Info[i].start_instance = (BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Output_Info[i].end_instance =3+(BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Output_Info[i].invoke_id = g_invoke_id;

			CString temp_cs;
			temp_cs.Format(_T("Task ID = %d. Read Output List Item From %d to %d "),g_invoke_id,
				Bacnet_Refresh_Info.Read_Output_Info[i].start_instance,
				Bacnet_Refresh_Info.Read_Output_Info[i].end_instance);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
		}
	}

	for (int i=0;i<BAC_ALARMLOG_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_ALARMLOG_LIST) /*|| (bac_read_which_list ==BAC_READ_ALL_LIST)*/)
		{
			resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				g_invoke_id = GetPrivateData(g_bac_instance,READALARM_T3000,i,i,sizeof(Alarm_point));
				Sleep(SEND_COMMAND_DELAY_TIME);
				//if(g_invoke_id<0)
				//	TRACE(_T("Input g_invoke_id = %d ,resend count = %d\r\n"),g_invoke_id,resend_count);
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_AlarmLog_Info[i].command = READALARM_T3000;
			Bacnet_Refresh_Info.Read_AlarmLog_Info[i].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_AlarmLog_Info[i].start_instance = i;
			Bacnet_Refresh_Info.Read_AlarmLog_Info[i].end_instance = i;
			Bacnet_Refresh_Info.Read_AlarmLog_Info[i].invoke_id = g_invoke_id;


			CString temp_cs;
			temp_cs.Format(_T("Task ID = %d. Read Alarm Log List %d"),g_invoke_id,
				Bacnet_Refresh_Info.Read_AlarmLog_Info[i].start_instance);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
		}
	}



	for (int i=0;i<BAC_INPUT_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_INPUT_LIST) || (bac_read_which_list ==BAC_READ_ALL_LIST) || (bac_read_which_list ==TYPE_SVAE_CONFIG))
		{
			resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				g_invoke_id = GetPrivateData(g_bac_instance,READINPUT_T3000,(BAC_READ_GROUP_NUMBER)*i,3+(BAC_READ_GROUP_NUMBER)*i,sizeof(Str_in_point));
				Sleep(SEND_COMMAND_DELAY_TIME);
				//if(g_invoke_id<0)
				//	TRACE(_T("Input g_invoke_id = %d ,resend count = %d\r\n"),g_invoke_id,resend_count);
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_Input_Info[i].command = READINPUT_T3000;
			Bacnet_Refresh_Info.Read_Input_Info[i].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_Input_Info[i].start_instance = (BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Input_Info[i].end_instance =3+(BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Input_Info[i].invoke_id = g_invoke_id;


			CString temp_cs;
			temp_cs.Format(_T("Task ID = %d. Read Input List Item From %d to %d "),g_invoke_id,
				Bacnet_Refresh_Info.Read_Input_Info[i].start_instance,
				Bacnet_Refresh_Info.Read_Input_Info[i].end_instance);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
		}
	}

	for (int i=0;i<BAC_VARIABLE_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_VARIABLE_LIST) || (bac_read_which_list ==BAC_READ_ALL_LIST) || (bac_read_which_list ==TYPE_SVAE_CONFIG))
		{
			int resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				g_invoke_id = GetPrivateData(g_bac_instance,READVARIABLE_T3000,(BAC_READ_GROUP_NUMBER)*i,3+(BAC_READ_GROUP_NUMBER)*i,sizeof(Str_variable_point));
				//TRACE(_T("g_invoke_id = %d\r\n",g_invoke_id));
				Sleep(SEND_COMMAND_DELAY_TIME);
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_Variable_Info[i].command = READVARIABLE_T3000;
			Bacnet_Refresh_Info.Read_Variable_Info[i].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_Variable_Info[i].start_instance = (BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Variable_Info[i].end_instance = 3+(BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Variable_Info[i].invoke_id = g_invoke_id;
			CString temp_cs;
			temp_cs.Format(_T("Task ID = %d. Read Variable List Item From %d to %d "),g_invoke_id,
				Bacnet_Refresh_Info.Read_Variable_Info[i].start_instance,
				Bacnet_Refresh_Info.Read_Variable_Info[i].end_instance);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
		}
	}

	for (int i=0;i<BAC_PROGRAM_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_PROGRAM_LIST) || (bac_read_which_list ==BAC_READ_ALL_LIST) || (bac_read_which_list ==TYPE_SVAE_CONFIG))
		{
			int resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				g_invoke_id = GetPrivateData(g_bac_instance,READPROGRAM_T3000,(BAC_READ_GROUP_NUMBER)*i,3+(BAC_READ_GROUP_NUMBER)*i,sizeof(Str_program_point));
				Sleep(SEND_COMMAND_DELAY_TIME);
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_Program_Info[i].command = READPROGRAM_T3000;
			Bacnet_Refresh_Info.Read_Program_Info[i].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_Program_Info[i].start_instance = (BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Program_Info[i].end_instance = 3+(BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Program_Info[i].invoke_id = g_invoke_id;
			CString temp_cs;
			temp_cs.Format(_T("Task ID = %d. Read Program List Item From %d to %d "),g_invoke_id,
				Bacnet_Refresh_Info.Read_Program_Info[i].start_instance,
				Bacnet_Refresh_Info.Read_Program_Info[i].end_instance);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
		}
	}
	if(bac_read_which_list == BAC_READ_PROGRAMCODE_LIST)
	{
			int resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				memset(mycode,0,1024);
				g_invoke_id = GetPrivateData(g_bac_instance,READPROGRAMCODE_T3000,program_list_line,program_list_line,m_Program_data.at(program_list_line).bytes + 10);
				Sleep(SEND_COMMAND_DELAY_TIME);
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_Programcode_Info[program_list_line].command = READPROGRAMCODE_T3000;
			Bacnet_Refresh_Info.Read_Programcode_Info[program_list_line].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_Programcode_Info[program_list_line].start_instance = program_list_line;
			Bacnet_Refresh_Info.Read_Programcode_Info[program_list_line].end_instance = program_list_line;
			Bacnet_Refresh_Info.Read_Programcode_Info[program_list_line].invoke_id = g_invoke_id;
			CString temp_cs;
			temp_cs.Format(_T("Task ID = %d. Read Programcode List Item From %d to %d "),g_invoke_id,
				Bacnet_Refresh_Info.Read_Programcode_Info[program_list_line].start_instance,
				Bacnet_Refresh_Info.Read_Programcode_Info[program_list_line].end_instance);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
	}
	else if(bac_read_which_list ==TYPE_SVAE_CONFIG)
	{
		for (int i=0;i<BAC_PROGRAMCODE_GROUP;i++)
		{
				int resend_count = 0;
				do 
				{
					resend_count ++;
					if(resend_count>RESEND_COUNT)
						goto myend;
					g_invoke_id = GetPrivateData(g_bac_instance,READPROGRAMCODE_T3000,i,i,m_Program_data.at(i).bytes + 10);
					Sleep(SEND_COMMAND_DELAY_TIME);
				} while (g_invoke_id<0);

				Bacnet_Refresh_Info.Read_Programcode_Info[i].command = READPROGRAMCODE_T3000;
				Bacnet_Refresh_Info.Read_Programcode_Info[i].device_id = g_bac_instance;
				Bacnet_Refresh_Info.Read_Programcode_Info[i].start_instance = i;
				Bacnet_Refresh_Info.Read_Programcode_Info[i].end_instance = i;
				Bacnet_Refresh_Info.Read_Programcode_Info[i].invoke_id = g_invoke_id;
				CString temp_cs;
				temp_cs.Format(_T("Task ID = %d. Read Programcode List Item From %d to %d "),g_invoke_id,
					Bacnet_Refresh_Info.Read_Programcode_Info[i].start_instance,
					Bacnet_Refresh_Info.Read_Programcode_Info[i].end_instance);
				Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
		}
	}
#if 0
	for (int i=0;i<BAC_PROGRAMCODE_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_PROGRAMCODE_LIST) || (bac_read_which_list ==TYPE_SVAE_CONFIG))
		{
			int resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				g_invoke_id = GetPrivateData(g_bac_instance,READPROGRAMCODE_T3000,i,i,m_Program_data.at(i).bytes + 10);
				Sleep(SEND_COMMAND_DELAY_TIME);
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_Programcode_Info[i].command = READPROGRAMCODE_T3000;
			Bacnet_Refresh_Info.Read_Programcode_Info[i].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_Programcode_Info[i].start_instance = i;
			Bacnet_Refresh_Info.Read_Programcode_Info[i].end_instance = i;
			Bacnet_Refresh_Info.Read_Programcode_Info[i].invoke_id = g_invoke_id;
			CString temp_cs;
			temp_cs.Format(_T("Task ID = %d. Read Programcode List Item From %d to %d "),g_invoke_id,
				Bacnet_Refresh_Info.Read_Programcode_Info[i].start_instance,
				Bacnet_Refresh_Info.Read_Programcode_Info[i].end_instance);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
		}
	}
#endif

	for (int i=0;i<BAC_CONTROLLER_GROUP;i++)
	{
		if((bac_read_which_list == BAC_READ_CONTROLLER_LIST) || (bac_read_which_list ==TYPE_SVAE_CONFIG)|| (bac_read_which_list ==BAC_READ_ALL_LIST))
		{
			int resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				g_invoke_id = GetPrivateData(g_bac_instance,READCONTROLLER_T3000,(BAC_READ_GROUP_NUMBER)*i,3+(BAC_READ_GROUP_NUMBER)*i,sizeof(Str_controller_point));
				Sleep(SEND_COMMAND_DELAY_TIME);
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_Controller_Info[i].command = READCONTROLLER_T3000;
			Bacnet_Refresh_Info.Read_Controller_Info[i].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_Controller_Info[i].start_instance = (BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Controller_Info[i].end_instance = 3+(BAC_READ_GROUP_NUMBER)*i;
			Bacnet_Refresh_Info.Read_Controller_Info[i].invoke_id = g_invoke_id;
			CString temp_cs;
			temp_cs.Format(_T("Task ID = %d. Read Controller List Item From %d to %d "),g_invoke_id,
				Bacnet_Refresh_Info.Read_Controller_Info[i].start_instance,
				Bacnet_Refresh_Info.Read_Controller_Info[i].end_instance);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
		}
	}


	if(bac_read_which_list == BAC_READ_WEEKLTCODE_LIST)
	{
		int resend_count = 0;
		do 
		{
			resend_count ++;
			if(resend_count>RESEND_COUNT)
				goto myend;
			g_invoke_id = GetPrivateData(g_bac_instance,READTIMESCHEDULE_T3000,weekly_list_line,weekly_list_line,WEEKLY_SCHEDULE_SIZE);
			Sleep(SEND_COMMAND_DELAY_TIME);
		} while (g_invoke_id<0);

		Bacnet_Refresh_Info.Read_Weeklycode_Info[weekly_list_line].command = READTIMESCHEDULE_T3000;
		Bacnet_Refresh_Info.Read_Weeklycode_Info[weekly_list_line].device_id = g_bac_instance;
		Bacnet_Refresh_Info.Read_Weeklycode_Info[weekly_list_line].start_instance = weekly_list_line;
		Bacnet_Refresh_Info.Read_Weeklycode_Info[weekly_list_line].end_instance = weekly_list_line;
		Bacnet_Refresh_Info.Read_Weeklycode_Info[weekly_list_line].invoke_id = g_invoke_id;
		CString temp_cs;
		temp_cs.Format(_T("Task ID = %d. Read annuale Schedule time List Item From %d to %d "),g_invoke_id,
			Bacnet_Refresh_Info.Read_Weeklycode_Info[weekly_list_line].start_instance,
			Bacnet_Refresh_Info.Read_Weeklycode_Info[weekly_list_line].end_instance);
		Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
	}
	else if(bac_read_which_list ==TYPE_SVAE_CONFIG)
	{
		for (int i=0;i<BAC_WEEKLYCODE_GOUP;i++)
		{
			int resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				g_invoke_id = GetPrivateData(g_bac_instance,READTIMESCHEDULE_T3000,i,i,WEEKLY_SCHEDULE_SIZE);
				Sleep(SEND_COMMAND_DELAY_TIME);
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_Weeklycode_Info[i].command = READTIMESCHEDULE_T3000;
			Bacnet_Refresh_Info.Read_Weeklycode_Info[i].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_Weeklycode_Info[i].start_instance = i;
			Bacnet_Refresh_Info.Read_Weeklycode_Info[i].end_instance = i;
			Bacnet_Refresh_Info.Read_Weeklycode_Info[i].invoke_id = g_invoke_id;
			CString temp_cs;
			temp_cs.Format(_T("Task ID = %d. Read Weekly Schedule time List Item From %d to %d "),g_invoke_id,
				Bacnet_Refresh_Info.Read_Weeklycode_Info[i].start_instance,
				Bacnet_Refresh_Info.Read_Weeklycode_Info[i].end_instance);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
		}
	}


	if(bac_read_which_list == BAC_READ_ANNUALCODE_LIST)
	{
		int resend_count = 0;
		do 
		{
			resend_count ++;
			if(resend_count>RESEND_COUNT)
				goto myend;
			g_invoke_id = GetPrivateData(g_bac_instance,READANNUALSCHEDULE_T3000,annual_list_line,annual_list_line,ANNUAL_CODE_SIZE);
			Sleep(SEND_COMMAND_DELAY_TIME);
		} while (g_invoke_id<0);

		Bacnet_Refresh_Info.Read_Annualcode_Info[annual_list_line].command = READANNUALSCHEDULE_T3000;
		Bacnet_Refresh_Info.Read_Annualcode_Info[annual_list_line].device_id = g_bac_instance;
		Bacnet_Refresh_Info.Read_Annualcode_Info[annual_list_line].start_instance = annual_list_line;
		Bacnet_Refresh_Info.Read_Annualcode_Info[annual_list_line].end_instance = annual_list_line;
		Bacnet_Refresh_Info.Read_Annualcode_Info[annual_list_line].invoke_id = g_invoke_id;
		CString temp_cs;
		temp_cs.Format(_T("Task ID = %d. Read Annual day List Item From %d to %d "),g_invoke_id,
			Bacnet_Refresh_Info.Read_Annualcode_Info[annual_list_line].start_instance,
			Bacnet_Refresh_Info.Read_Annualcode_Info[annual_list_line].end_instance);
		Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
	}
	else if(bac_read_which_list ==TYPE_SVAE_CONFIG)
	{
		for (int i=0;i<BAC_ANNUALCODE_GROUP;i++)
		{
			int resend_count = 0;
			do 
			{
				resend_count ++;
				if(resend_count>RESEND_COUNT)
					goto myend;
				g_invoke_id = GetPrivateData(g_bac_instance,READANNUALSCHEDULE_T3000,i,i,ANNUAL_CODE_SIZE);
				Sleep(SEND_COMMAND_DELAY_TIME);
			} while (g_invoke_id<0);

			Bacnet_Refresh_Info.Read_Annualcode_Info[i].command = READTIMESCHEDULE_T3000;
			Bacnet_Refresh_Info.Read_Annualcode_Info[i].device_id = g_bac_instance;
			Bacnet_Refresh_Info.Read_Annualcode_Info[i].start_instance = i;
			Bacnet_Refresh_Info.Read_Annualcode_Info[i].end_instance = i;
			Bacnet_Refresh_Info.Read_Annualcode_Info[i].invoke_id = g_invoke_id;
			CString temp_cs;
			temp_cs.Format(_T("Task ID = %d. Read Annual day time List Item From %d to %d "),g_invoke_id,
				Bacnet_Refresh_Info.Read_Annualcode_Info[i].start_instance,
				Bacnet_Refresh_Info.Read_Annualcode_Info[i].end_instance);
			Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs);
		}
	}

	if( bac_read_which_list == BAC_READ_ALL_LIST)
	{
		g_CommunicationType = 1;
		SetCommunicationType(1);
		if(Open_Socket2(IP_ADDRESS,6001))	//在线程里面去连接设备 用于modbus的 协议通讯;
		{

		}
	}

		hwait_thread = NULL;

	return 0;
		//::PostMessage(BacNet_hwd,WM_SEND_OVER,0,0);

myend:	hwait_thread = NULL;
		//AfxMessageBox(_T("Send Command Timeout!!!!!!!!!!"));
		::PostMessage(BacNet_hwd,WM_DELETE_NEW_MESSAGE_DLG,0,0);
	return 0;
}

void CDialogCM5_BacNet::SetConnected_IP(LPCTSTR myip)
{
	IP_ADDRESS.Empty();
	IP_ADDRESS.Format(_T("%s"),myip);
	//IP_ADDRESS = myip;
}

void CDialogCM5_BacNet::WriteFlash()
{
	// TODO: Add your control notification handler code here
	//WRITEPRGFLASH_COMMAND
	int resend_count = 0;
	do 
	{
		resend_count ++;
		if(resend_count>10)
			break;
		g_invoke_id = GetPrivateData(g_bac_instance,WRITEPRGFLASH_COMMAND,0,0,0);
		Sleep(100);
	} while (g_invoke_id<0);

	if(g_invoke_id>0)
	{
		CString temp_cs_show;
		temp_cs_show.Format(_T("Task ID = %d. Write into flash "),g_invoke_id);
		Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd,temp_cs_show);
	}


	//Post_Invoke_ID_Monitor_Thread(MY_INVOKE_ID,g_invoke_id,BacNet_hwd);
}










void CDialogCM5_BacNet::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	bool connect_results = Get_MSTP_Connect_Status();
	static bool Old_connect_results = false;
	
	switch(nIDEvent)
	{
	case 1:
		if(Old_connect_results != connect_results)//Prevent repeatedly to redraw the Static.
		{
			if(connect_results)
			{
				SetPaneString(BAC_SHOW_CONNECT_RESULTS,_T("Connected"));
			}
			else
			{
				SetPaneString(BAC_SHOW_CONNECT_RESULTS,_T("Disconnected"));
			}
		}

		break;
	case 2:
		{
		//if(!is_in_scan_mode)
		//	m_bac_scan_com_data.clear();
		if(g_bac_instance>0)
			Send_WhoIs_Global(-1, -1);
		}
		break;
	case 3:
		{
			if(AlarmWindow_Window!=NULL)
			{
				if(bac_show_alarm_window)//显示;
				{
					if(!(CBacnetAlarmWindow *)AlarmWindow_Window->IsWindowVisible())
					{
						AlarmWindow_Window->ShowWindow(SW_SHOW);
					}
				}
				else//Not show
				{
					if((CBacnetAlarmWindow *)AlarmWindow_Window->IsWindowVisible())
					{
						AlarmWindow_Window->ShowWindow(SW_HIDE);
					}
				}
			}
		}
		break;
	case 4:
		{
			click_resend_time --;
			Send_WhoIs_Global(-1, -1);
			bool find_exsit = false;

			for (int i=0;i<(int)m_bac_scan_com_data.size();i++)
			{
				if(m_bac_scan_com_data.at(i).device_id == g_bac_instance)
					find_exsit = true;
			}


			if(find_exsit)
			{
				KillTimer(4);
				//PostMessage(WM_FRESH_CM_LIST,MENU_CLICK,TYPE_SVAE_CONFIG);
				//Post_Write_Message(g_bac_instance,CONNECTED_WITH_DEVICE,0,0,sizeof(Str_connected_point),BacNet_hwd ,_T("Connect with device"));
#if 1
				PostMessage(WM_FRESH_CM_LIST,MENU_CLICK,TYPE_ALL);
				CString temp_cstring;
				temp_cstring.Format(_T("Connected"),g_bac_instance);
				SetPaneString(1,temp_cstring);
				m_bac_main_tab.SetFocus();
				bac_select_device_online = true;
#endif
			}
			else
			{
				if(click_resend_time == 0)
				{
					bac_select_device_online = false;
					KillTimer(4);
					SetPaneString(1,_T("Device is offline!"));
					MessageBox(_T("No response! Please Check the connection!"),_T("Notice"),MB_OK | MB_ICONINFORMATION);;
				}
				else
				{
					TRACE(_T("Resend Who is count = %d\r\n"),4 - click_resend_time);
				}
			}
			
		}
		break;
	default:
		break;
	}
	Old_connect_results = connect_results;

	CFormView::OnTimer(nIDEvent);
}


void CDialogCM5_BacNet::OnTcnSelchangeBacMaintab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	int selected = m_bac_main_tab.GetCurSel();
	static int old_selected_item = WINDOW_PROGRAM;
	switch(old_selected_item)
	{
	case WINDOW_PROGRAM:
		((CBacnetProgram*)pDialog[WINDOW_PROGRAM])->Unreg_Hotkey();
		break;
	case WINDOW_SCREEN:
		((BacnetScreen*)pDialog[WINDOW_SCREEN])->Unreg_Hotkey();
		break;
	case WINDOW_WEEKLY:
		((BacnetWeeklyRoutine*)pDialog[WINDOW_WEEKLY])->Unreg_Hotkey();
		break;
	case WINDOW_ANNUAL:
		((BacnetAnnualRoutine*)pDialog[WINDOW_ANNUAL])->Unreg_Hotkey();
		break;
	case WINDOW_MONITOR:
		((CBacnetMonitor*)pDialog[WINDOW_MONITOR])->Unreg_Hotkey();
		break;
	default:
		break;
	}

	for (int i=0;i<WINDOW_TAB_COUNT;i++)
	{
		if(i==selected)
		{
			pDialog[i]->ShowWindow(SW_SHOW);
			switch(i)
			{
			case WINDOW_INPUT:
				//PostMessage(WM_FRESH_CM_LIST,MENU_CLICK,TYPE_INPUT);
				((CBacnetInput *)pDialog[i])->Fresh_Input_List(NULL,NULL);
				g_hwnd_now = m_input_dlg_hwnd;
				break;
			case WINDOW_OUTPUT:
				//PostMessage(WM_FRESH_CM_LIST,MENU_CLICK,TYPE_OUTPUT);
				((CBacnetOutput *)pDialog[i])->Fresh_Output_List(NULL,NULL);
				g_hwnd_now = m_output_dlg_hwnd;
					break;
			case WINDOW_VARIABLE:
				((CBacnetVariable *)pDialog[i])->Fresh_Variable_List(NULL,NULL);
				g_hwnd_now = m_variable_dlg_hwnd;
				break;
			case WINDOW_PROGRAM:
				((CBacnetProgram *)pDialog[i])->Fresh_Program_List(NULL,NULL);
				((CBacnetProgram*)pDialog[i])->Reg_Hotkey();
				g_hwnd_now = m_pragram_dlg_hwnd;
				break;
			case WINDOW_CONTROLLER:
				//PostMessage(WM_FRESH_CM_LIST,MENU_CLICK,TYPE_CONTROLLER);
				((BacnetController *)pDialog[i])->Fresh_Controller_List(NULL,NULL);
				g_hwnd_now = m_controller_dlg_hwnd;
				break;
			case WINDOW_SCREEN:
				((BacnetScreen *)pDialog[i])->Fresh_Screen_List(NULL,NULL);
				((BacnetScreen*)pDialog[i])->Reg_Hotkey();
				g_hwnd_now = m_screen_dlg_hwnd;
				break;
			case WINDOW_WEEKLY:
				((BacnetWeeklyRoutine *)pDialog[i])->Fresh_Weekly_List(NULL,NULL);
				((BacnetWeeklyRoutine*)pDialog[i])->Reg_Hotkey();
				g_hwnd_now = m_weekly_dlg_hwnd;
				break;
			case WINDOW_ANNUAL:
				((BacnetAnnualRoutine *)pDialog[i])->Fresh_Annual_Routine_List(NULL,NULL);
				((BacnetAnnualRoutine*)pDialog[i])->Reg_Hotkey();
				g_hwnd_now = m_annual_dlg_hwnd;
				break;
			case WINDOW_MONITOR:
				((CBacnetMonitor *)pDialog[i])->Fresh_Monitor_List(NULL,NULL);
				((CBacnetMonitor *)pDialog[i])->Fresh_Monitor_Input_List(NULL,NULL);
				((CBacnetMonitor*)pDialog[i])->Reg_Hotkey();
				g_hwnd_now = m_monitor_dlg_hwnd;
				break;
			case WINDOW_ALARMLOG:
				//增加刷新list和改变当前window 的hwnd;
				PostMessage(WM_FRESH_CM_LIST,MENU_CLICK,TYPE_ALARMLOG);
				((CBacnetAlarmLog *)pDialog[i])->Fresh_Alarmlog_List(NULL,NULL);
				g_hwnd_now = m_alarmlog_dlg_hwnd;
				break;
			case WINDOW_TSTAT:
				//PostMessage(WM_FRESH_CM_LIST,MENU_CLICK,TYPE_TSTAT);
			//	((CBacnetTstat *)pDialog[i])->Fresh_Alarmlog_List(NULL,NULL);
				((CBacnetTstat *)pDialog[i])->Fresh_Tstat_List(NULL,NULL);
				
				g_hwnd_now = m_tstat_dlg_hwnd;
				break;
			case WINDOW_SETTING:
				((CBacnetSetting *)pDialog[i])->Fresh_Setting_UI(READ_SETTING_COMMAND,NULL);
				g_hwnd_now = m_setting_dlg_hwnd;
				break;
			}
			
//			pDialog[i]->OnInitDialog();
		}
		else
		{
#if 0
			switch(i)
			{
			case WINDOW_PROGRAM:
				((CBacnetProgram*)pDialog[i])->Unreg_Hotkey();
				break;
			case WINDOW_SCREEN:
				((BacnetScreen*)pDialog[i])->Unreg_Hotkey();
				break;
			case WINDOW_WEEKLY:
				((BacnetWeeklyRoutine*)pDialog[i])->Unreg_Hotkey();
				break;
			case WINDOW_ANNUAL:
				((BacnetAnnualRoutine*)pDialog[i])->Unreg_Hotkey();
				break;
			case WINDOW_MONITOR:
				((CBacnetMonitor*)pDialog[i])->Unreg_Hotkey();
				break;
			}
#endif
			pDialog[i]->ShowWindow(SW_HIDE);
		}
	}
	if(old_selected_item!= selected)
		old_selected_item = selected;
	*pResult = 0;
}


BOOL CDialogCM5_BacNet::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_TAB) 
	{
		m_cur_tab_sel = m_bac_main_tab.GetCurSel();
		m_cur_tab_sel = (++m_cur_tab_sel)%WINDOW_TAB_COUNT;
		m_bac_main_tab.SetCurSel(m_cur_tab_sel);

		for (int i=0;i<WINDOW_TAB_COUNT;i++)
		{
			if(i==m_cur_tab_sel)
			{
				pDialog[i]->ShowWindow(SW_SHOW);
				switch(i)
				{
				case WINDOW_INPUT:
					((CBacnetInput *)pDialog[i])->Fresh_Input_List(NULL,NULL);
					g_hwnd_now = m_input_dlg_hwnd;
					break;
				case WINDOW_OUTPUT:
					((CBacnetOutput *)pDialog[i])->Fresh_Output_List(NULL,NULL);
					g_hwnd_now = m_output_dlg_hwnd;
					break;
				case WINDOW_VARIABLE:
					((CBacnetVariable *)pDialog[i])->Fresh_Variable_List(NULL,NULL);
					g_hwnd_now = m_variable_dlg_hwnd;
					break;
				case WINDOW_PROGRAM:
					((CBacnetProgram *)pDialog[i])->Fresh_Program_List(NULL,NULL);
					((CBacnetProgram*)pDialog[i])->Reg_Hotkey();
					g_hwnd_now = m_pragram_dlg_hwnd;
					break;
				case WINDOW_CONTROLLER:
					((BacnetController *)pDialog[i])->Fresh_Controller_List(NULL,NULL);
					g_hwnd_now = m_controller_dlg_hwnd;
					break;
				case WINDOW_SCREEN:
					((BacnetScreen *)pDialog[i])->Fresh_Screen_List(NULL,NULL);
					((BacnetScreen*)pDialog[i])->Reg_Hotkey();
					g_hwnd_now = m_screen_dlg_hwnd;
					break;
				case WINDOW_WEEKLY:
					((BacnetWeeklyRoutine *)pDialog[i])->Fresh_Weekly_List(NULL,NULL);
					((BacnetWeeklyRoutine*)pDialog[i])->Reg_Hotkey();
					g_hwnd_now = m_weekly_dlg_hwnd;
					break;
				case WINDOW_ANNUAL:
					((BacnetAnnualRoutine *)pDialog[i])->Fresh_Annual_Routine_List(NULL,NULL);
					((BacnetAnnualRoutine*)pDialog[i])->Reg_Hotkey();
					g_hwnd_now = m_annual_dlg_hwnd;
					break;
				case WINDOW_MONITOR:
					((CBacnetMonitor *)pDialog[i])->Fresh_Monitor_List(NULL,NULL);
					((CBacnetMonitor *)pDialog[i])->Fresh_Monitor_Input_List(NULL,NULL);
					((CBacnetMonitor*)pDialog[i])->Reg_Hotkey();
					g_hwnd_now = m_monitor_dlg_hwnd;
					break;
				case WINDOW_TSTAT:
					PostMessage(WM_FRESH_CM_LIST,MENU_CLICK,TYPE_TSTAT);
					//	((CBacnetTstat *)pDialog[i])->Fresh_Alarmlog_List(NULL,NULL);
					g_hwnd_now = m_tstat_dlg_hwnd;
					break;
				case WINDOW_SETTING:
					((CBacnetSetting *)pDialog[i])->Fresh_Setting_UI(NULL,NULL);
					g_hwnd_now = m_setting_dlg_hwnd;
					break;
				}

				//			pDialog[i]->OnInitDialog();
			}
			else
			{
				switch(i)
				{
				case WINDOW_PROGRAM:
					((CBacnetProgram*)pDialog[i])->Unreg_Hotkey();
					break;
				case WINDOW_SCREEN:
					((BacnetScreen*)pDialog[i])->Unreg_Hotkey();
					break;
				case WINDOW_WEEKLY:
					((BacnetWeeklyRoutine*)pDialog[i])->Unreg_Hotkey();
					break;
				case WINDOW_ANNUAL:
					((BacnetAnnualRoutine*)pDialog[i])->Unreg_Hotkey();
					break;
				case WINDOW_MONITOR:
					((CBacnetMonitor*)pDialog[i])->Unreg_Hotkey();
					break;
				}
				pDialog[i]->ShowWindow(SW_HIDE);
			}
		}

		return TRUE;
	}
	else if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_DOWN)
	{
		m_cur_tab_sel = m_bac_main_tab.GetCurSel();
		pDialog[m_cur_tab_sel]->SetFocus();
	}

	return CFormView::PreTranslateMessage(pMsg);
}