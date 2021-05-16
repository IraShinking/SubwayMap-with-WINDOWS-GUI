#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <ctype.h>
#include <stdio.h>
#include <wchar.h>

/**���ҳ��򲿷�**/
#define MAX_VERTEX_NUM 200
int MAX_LINE_NUM = 8;
int istrans[MAX_VERTEX_NUM] = {0};

/*һ�����ݽṹ*/
typedef struct enode_type
{
	int adjvex;					//�ڽӵ��λ��//
	int weight;					//Ȩֵ������·���)//
	struct enode_type *nextarc; //ָ����һ���ڽӵ�//
} enode;

typedef struct vnode_type
{
	int id;		   //����ı��//
	char name[20]; //��������֣���վ������//
	enode *fadj;   //ָ���һ����//
} vnode;

typedef struct Graph_type
{
	vnode vertices[MAX_VERTEX_NUM]; //���㼯��(vertex)//
	int vexnum;						//ͼ�Ķ�����Ŀ//
	int arcnum;						//ͼ�ı���Ŀ//

} Graph;

typedef struct pathnode_type
{
	int top;					//·������Ŀ//
	int path[MAX_VERTEX_NUM];	//·����Ϣ//
	int trans[MAX_VERTEX_NUM];	//���˵���Ϣ,0Ϊ�ǻ��˵㣬1Ϊ���˵�//
	int counter;				//���˵Ĵ���//
	struct pathnode_type *next; //ָ����һ���ɵִ��յ��·����Ϣ�ڵ�//
} pathnode;

typedef struct LINK_type
{
	pathnode *head; //·���ӿ�//
} LINK;

//��ȡվ��ı�ţ���1-130//
int locatevex(Graph *G, char *a);
//��ȡ��ǰ��·��//
int getline(Graph *G, int i, int j);
//��ȡ���˵�//
void gettrans(Graph *G, pathnode *all);
//��������������//
void create_graph(Graph *G);
//DFS����������������п��ܵ�·���߷�//
void DFS(Graph *G, int stack[], int visited[], int v, int destination, int top, LINK *L);
//������ٻ���·����Ϣ//
void print_lesstransfer(Graph *G, LINK *L);
//���ٻ��˵ĺ����ӿڣ�����1����ʼվ������2���յ�վ//
int lesstransfer(char start[], char destination[]);
//����һ��·������//
void create_path(LINK *L);
//����һ��·����Ϣ//
void addpath(LINK *L, pathnode *p);
//�Ʒѣ�����ÿ��վ��֮��1������׼��㣬��ʵ���кܴ���//
int caculate_price(int station);
//�˳�ʱ��
float caculate_time(int station, int count);
//�˳�������
float caculate_length(int station, int count);

//վ���ѯ��·,��ʵ�ֻ���վ���Զ��ж�
int station_search_line(char station[], int parameter);

/**WINDOWS���岿��**/
/*handle of this program*/
HINSTANCE hInst;
HWND hwnd, hwnd2; /*handle for window*/

/*the main window class name*/
static TCHAR szClassName[] = TEXT("MetroWay");
static TCHAR szClassName2[] = TEXT("MetroWayMap");

/*the main window's title name*/
static TCHAR szTitle[] = _T("���ݵ�����·��ѯϵͳ");
static TCHAR szTitle2[] = _T("���ݵ�����·ͼ");

/*����ĵ��Ƿ���� ���ڷ�����,���򷵻ؼٲ���������*/
int CheckFile(HANDLE hfile);

/*forward declarations of function implements message*/
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProc2(HWND, UINT, WPARAM, LPARAM);

/*the main function */
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PSTR szCmdLine,
	int iCmdShow)
{

	MSG msg;
	WNDCLASS wndclass, wndclassbmp;

	/*the first stage:register a window class*/
	/*set value for each field in the window class*/
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance; /*current window's handle*/
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szClassName;

	wndclassbmp.style = CS_HREDRAW | CS_VREDRAW;
	wndclassbmp.lpfnWndProc = WndProc2;
	wndclassbmp.cbClsExtra = 0;
	wndclassbmp.cbWndExtra = 0;
	wndclassbmp.hInstance = hInstance; /*current window's handle*/
	wndclassbmp.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclassbmp.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclassbmp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclassbmp.lpszMenuName = NULL;
	wndclassbmp.lpszClassName = szClassName2;

	/*register a window*/
	RegisterClass(&wndclass);
	RegisterClass(&wndclassbmp);
	/*the second stage:create and show window*/
	hwnd = CreateWindow(
		szClassName,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		800,
		650,
		NULL,
		NULL,
		hInstance,
		NULL);
	hwnd2 = CreateWindow(
		szClassName2,
		szTitle2,
		WS_OVERLAPPEDWINDOW | WS_MAXIMIZE | WS_MAXIMIZEBOX|WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		3000,
		2000,
		hwnd,
		NULL,
		hInstance,
		NULL);

	/*show window*/
	ShowWindow(hwnd, iCmdShow);
	/*update(paint) window*/
	UpdateWindow(hwnd);
	ShowWindow(hwnd2, SW_HIDE);
	/*update(paint) window*/
	UpdateWindow(hwnd2);

	/*the third stage:the loop of message*/
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

static TCHAR szTransInfo[999];	 //���������·��ѯ�Ļ�����
static TCHAR szStationInfo[999]; //���վ���ѯ����Ļ�����

//��ʾ�Ŵ���·ͼ���ڵĽ��̺���
LRESULT CALLBACK WndProc2(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	HDC hdcBit;
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	BITMAP bm;


	switch (message)
	{
	case WM_PAINT:
	{
		hdc=GetDC(hwnd);
		HBITMAP hbmp1 = (HBITMAP)LoadImage(NULL, "metroway2.bmp", IMAGE_BITMAP,1150 ,869, LR_LOADFROMFILE);//width=657*1.75,height=579*1.75
		if (hbmp1)
		{
			hdcBit=CreateCompatibleDC(hdc);
			GetObject(hbmp1,sizeof(BITMAP),&bm);
			SelectObject(hdcBit,hbmp1);
			BitBlt(hdc,20,10,bm.bmWidth,bm.bmHeight,hdcBit,0,0,SRCCOPY);
			DeleteDC(hdcBit);
			DeleteObject(hbmp1);
			
		}
		break;
	}
	case WM_CLOSE:
ShowWindow(hwnd, SW_HIDE);
		MessageBox(hwnd, "�Ŵ����·ͼ�ѹرա�", "��ʾ", MB_OK);
		//PostQuitMessage(0); �˴�δ���øú�����Ӧ�õ���Ϣ������Ҳ��δ���WM_QUIT��Ϣ��
			// ��WinMain��������Ϣѭ���м����ȴ��û���ϵͳ��Ϣ��
		 // ��Ϣѭ��û����������������WinManҲ��û������
			break;
	case WM_SIZE:
		{
			if(wParam==SIZE_MINIMIZED)//���ڱ���С��
			{
				ShowWindow(hwnd,SW_HIDE);
			}
		}
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}
/*the fourth stage:the window progress*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;	   /*handle of the device environment*/
	HDC hdcStatic; /*handle of the device environment of static*/
	PAINTSTRUCT ps;
	RECT rect;

	static HFONT hFont; //�߼�����

	static HWND hLabLine;		  //�����·��
	static HWND hLabLineTitle;	  //�����·�����
	static HWND hBtnCompleteLine; //���������·��İ�ť
	static HWND hLabCompleteLine; //���������·����ı���

	static HWND hBtnZoom; //��ť--����Ŵ�
	static HWND hLabRote; //��̬�ı���--·�߲�ѯ
	static HWND hLabFind; //��̬�ı���--վ���ѯ

	static HWND hLabStart; //��̬�ı���--��ʼ
	static HWND hLabEnd;   //��̬�ı���--�յ�
	static HWND hLabState; //��̬�ı���--վ����

	static HWND hEditStart; //�����ı������--�������
	static HWND hEditEnd;	//�����ı������--�����յ�
	static HWND hEditState; //�����ı������--����վ��

	static HWND hBtnSearchL;	//��ѯ���˰�ť
	static HWND hBtnSearchR;	//��ѯվ�㰴ť
	static HWND hLabSearchInfo; //�����ѯ������ı���

	int wmId, wmEvent;
	TCHAR szStart[100]; //���վ���Ļ�����
	TCHAR szEnd[100];	//�յ�վ���Ļ�����
	TCHAR szState[100]; //��ѯ��·��վ���Ļ�����

	TCHAR szfile[999];		//�����ĵ��Ļ�����
	TCHAR szLine[300];		//�����·��Ļ�����
	TCHAR szWholeLine[999]; //���������·��Ļ�����

	switch (message)
	{
	case WM_CREATE:
	{
		//�����߼�����
		hFont = CreateFont(-14 /*��*/, -7 /*��*/, 0, 0, 400 /*һ�����ֵ��Ϊ400*/,
						   FALSE /*б��?*/, FALSE /*�»���?*/, FALSE /*ɾ����?*/, DEFAULT_CHARSET,
						   OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
						   FF_DONTCARE, TEXT("΢���ź�"));

		/*��������·�ߵ�λͼ*/
		HWND hwndStatic = CreateWindow(TEXT("STATIC"),
									   NULL,
									   WS_CHILD | SS_BITMAP | WS_VISIBLE,
									   0, 0, 300, 100, hwnd, (HMENU)27, hInst, NULL);
		HBITMAP hbmp = (HBITMAP)LoadImage(NULL, "metroway.bmp", IMAGE_BITMAP, 479, 392, LR_LOADFROMFILE);
		if (hbmp)
		{
			SendMessage(hwndStatic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmp);
		}

		/*��̬�ı��򡪡�˵���ı�*/
		//��ʾ����Ŵ�ͼƬ
		hBtnZoom = CreateWindow(TEXT("button"), TEXT("���ͼƬ�Ŵ�"),
								WS_CHILD | WS_VISIBLE | BS_FLAT,
								235, 400, 100, 26,
								hwnd, (HMENU)1, hInst, NULL);

		//��ʾ��ѯ����·��
		hLabRote = CreateWindow(TEXT("static"), TEXT("��ѯ������·��"),
								WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE /*��ֱ����*/ | SS_CENTER /*ˮƽ����*/,
								120 /*x����*/, 430 /*y����*/, 100 /*���*/, 26 /*�߶�*/,
								hwnd /*�����ھ��*/, (HMENU)2 /*�ؼ�ID*/, hInst /*��ǰ����ʵ�����*/, NULL);
		//��ʾ��ѯվ����Ϣ
		hLabFind = CreateWindow(TEXT("static"), TEXT("��ѯվ����Ϣ��"),
								WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE /*��ֱ����*/ | SS_RIGHT /*ˮƽ����*/,
								340 /*x����*/, 430 /*y����*/, 70 /*���*/, 26 /*�߶�*/,
								hwnd /*�����ھ��*/, (HMENU)3 /*�ؼ�ID*/, hInst /*��ǰ����ʵ�����*/, NULL);

		/*��̬�ı��򡪡����������ʾ��˵���ı�*/
		//����ʼվ��
		hLabStart = CreateWindow(TEXT("static"), TEXT("��㣺"),
								 WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE /*��ֱ����*/ | SS_RIGHT /*ˮƽ����*/,
								 20 /*x����*/, 460 /*y����*/, 70 /*���*/, 26 /*�߶�*/,
								 hwnd /*�����ھ��*/, (HMENU)4 /*�ؼ�ID*/, hInst /*��ǰ����ʵ�����*/, NULL);
		//���յ�վ��
		hLabEnd = CreateWindow(TEXT("static"), TEXT("�յ㣺"),
							   WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE /*��ֱ����*/ | SS_RIGHT /*ˮƽ����*/,
							   20, 500, 70, 26,
							   hwnd, (HMENU)5, hInst, NULL);
		//��վ������
		hLabState = CreateWindow(TEXT("static"), TEXT("վ������"),
								 WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE /*��ֱ����*/ | SS_RIGHT /*ˮƽ����*/,
								 270, 460, 70, 26,
								 hwnd, (HMENU)6, hInst, NULL);

		/**�����ı���ؼ�**/
		//���������
		hEditStart = CreateWindow(TEXT("edit"), TEXT(""),
								  WS_CHILD | WS_VISIBLE | WS_BORDER /*�߿�*/ | ES_AUTOHSCROLL /*ˮƽ����*/,
								  100, 460, 140, 26,
								  hwnd, (HMENU)7, hInst, NULL);
		//�����յ���
		hEditEnd = CreateWindow(TEXT("edit"), TEXT(""),
								WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL /*ˮƽ����*/,
								100, 500, 140, 26,
								hwnd, (HMENU)8, hInst, NULL);
		//����վ����
		hEditState = CreateWindow(TEXT("edit"), TEXT(""),
								  WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL /*ˮƽ����*/,
								  340, 460, 140, 26,
								  hwnd, (HMENU)9, hInst, NULL);

		//��ѯ��ť
		hBtnSearchL = CreateWindow(TEXT("button"), TEXT("��ѯ"),
								   WS_CHILD | WS_VISIBLE | BS_FLAT /*��ƽ��ʽ*/,
								   120, 540, 70, 30,
								   hwnd, (HMENU)10, hInst, NULL);
		hBtnSearchR = CreateWindow(TEXT("button"), TEXT("��ѯ"),
								   WS_CHILD | WS_VISIBLE | BS_FLAT /*��ƽ��ʽ*/,
								   340, 540, 70, 30,
								   hwnd, (HMENU)26, hInst, NULL);
		/*��ʾҳ���Ҳ���·��*/
		//�����·��
		hLabLine = CreateWindow(TEXT("static"), TEXT("����һ���ı���"), WS_CHILD | WS_VISIBLE | SS_LEFT, 490, 40, 300, 200, hwnd, (HMENU)11, hInst, NULL);

		//��·�����ʾ����
		hLabLineTitle = CreateWindow(TEXT("static"), TEXT("������·��"), WS_CHILD | WS_VISIBLE | SS_CENTER, 550, 10, 100, 26, hwnd, (HMENU)12, hInst, NULL);

		//��ʾ������·��İ�ť
		hBtnCompleteLine = CreateWindow(TEXT("button"), TEXT("��ʾ������·��"), WS_CHILD | WS_VISIBLE | BS_FLAT, 550, 250, 120, 30, hwnd, (HMENU)13, hInst, NULL);

		//��ʾ������·����ı���
		//hLabCompleteLine = CreateWindow(TEXT("static"), TEXT("���������·��"), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 490, 300, 300, 200, hwnd, (HMENU)14, hInst, NULL);

		//��ʾ��ѯ������ı���
		//hLabSearchInfo = CreateWindow(TEXT("static"), TEXT("�����ѯ���"), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 50, 580, 400, 200, hwnd, (HMENU)15, hInst, NULL);

		//�������ÿؼ�������

		SendMessage(hLabFind, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hLabRote, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hBtnZoom, WM_SETFONT, (WPARAM)hFont, NULL);

		SendMessage(hLabStart, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hLabEnd, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hLabState, WM_SETFONT, (WPARAM)hFont, NULL);

		SendMessage(hEditStart, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hEditEnd, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hEditState, WM_SETFONT, (WPARAM)hFont, NULL);

		SendMessage(hBtnSearchL, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hBtnSearchR, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hLabLine, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hLabLineTitle, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hBtnCompleteLine, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hLabCompleteLine, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(hLabSearchInfo, WM_SETFONT, (WPARAM)hFont, NULL);

		/*��������ĵ��Ƿ����*/
		DWORD RSize;
		HANDLE fLine, fStationinfo, fStation, fStationdata;
		fLine = (HANDLE)CreateFile(TEXT("line.txt"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		fStationinfo = (HANDLE)CreateFile(TEXT("stationinfo.txt"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		fStationdata = (HANDLE)CreateFile(TEXT("stationdata.txt"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		fStation = (HANDLE)CreateFile(TEXT("station.txt"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

		CheckFile(fStationinfo);
		CheckFile(fStationdata);
		CheckFile(fStation);

		//��������ʱ��ʾ������·��
		if (CheckFile(fLine) == 1)
		{
			ReadFile(fLine, szLine, GetFileSize(fLine, NULL), &RSize, NULL);

			if (szLine != NULL)
			{
				SetWindowText(hLabLine, szLine);
			}
		}
		CloseHandle(fLine);
		CloseHandle(fStation);
		CloseHandle(fStationdata);
		CloseHandle(fStationinfo);
	}
	break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case 10: //����˲�ѯ·�ߵİ�ť
			//��ȡ����������
			{
				int i = 0, j = 0;
				char start[20], destination[20];

				i = GetWindowText(hEditStart, szStart, 100);
				j = GetWindowText(hEditEnd, szEnd, 100);

				//��ѯ����·����Ϣ
				if (i > 0 && j > 0)
				{
					sscanf(szStart, "%s", start);
					sscanf(szEnd, "%s", destination);
					if (strcmp(start, destination) == 0)
					{
						MessageBox(hwnd, TEXT("�������ʼվ���յ�վ��ͬ��"), TEXT("NOTICE"), MB_ICONINFORMATION);
					}
					else
					{
						if (lesstransfer(start, destination) != 0) //��С���˽ӿ�//
						{
							MessageBox(NULL, szTransInfo, TEXT("·����Ϣ"), NULL);
						}
					}
				}
				else
				{
					MessageBox(hwnd, TEXT("��������������ʼվ���յ�վ����"), TEXT("NOTICE"), MB_ICONINFORMATION);
				}

				break;
			}
		case 26:
		{
			//����˲�ѯվ����Ϣ�İ�ť
			int k = 0;
			char station[20];
			k = GetWindowText(hEditState, szState, 100);

			if (k > 0)
			{
				sscanf(szState, "%s", station);
				if (station_search_line(station, 1) == -1)
				{
					TCHAR szTempInfo[100];
					sprintf(szTempInfo, "��ʾ��δ��ѯ��%sվ����Ϣ��", station);
					MessageBox(hwnd, szTempInfo, TEXT("NOTICE"), MB_ICONINFORMATION);
				}
				else
				{
					//SetWindowText(hLabSearchInfo, szStationInfo);
					MessageBox(NULL, szStationInfo, "վ����Ϣ", NULL);
				}
			}
			else if (k <= 0)
			{
				MessageBox(hwnd, TEXT("������վ������"), TEXT("NOTICE"), MB_ICONINFORMATION);
			}
			break;
		}
		case 13: //�����ʾ��ϸ��·
		{
			int i, j, temp, charLen = 0;
			int a[MAX_VERTEX_NUM] = {0};
			int b[MAX_VERTEX_NUM] = {0};
			int c[MAX_VERTEX_NUM] = {0};

			Graph G;
			FILE *fp;

			create_graph(&G);

			fp = fopen("stationdata.txt", "r");
			for (i = 1; i <= MAX_VERTEX_NUM; i++)
			{
				fscanf(fp, "%d %d %d", &a[i], &b[i], &c[i]);
			}
			temp = 1;
			for (i = 1; i <= MAX_LINE_NUM; i++)
			{

				charLen += sprintf(szWholeLine + charLen, "������%d���ߡ�\n", i);
				for (j = temp; c[j] == i; j++)
				{
					charLen += sprintf(szWholeLine + charLen, "%s->", G.vertices[a[j]].name);
				}
				charLen += sprintf(szWholeLine + charLen, "%s\n", G.vertices[b[--j]].name);
				temp = ++j;
			}

			fclose(fp);
			if (szWholeLine != NULL)
			{
				MessageBox(NULL, szWholeLine, "���ݵ�����·��", NULL);
			}

			break;
		}
		case 1:
			//����Ŵ�ͼƬ

			ShowWindow(hwnd2, SW_SHOW);
			UpdateWindow(hwnd2);

			break;
		}

	case WM_PAINT:

		hdc = BeginPaint(hwnd, &ps);

		//����ļ�

		EndPaint(hwnd, &ps);
		break;

		/*����Static�ؼ��ı�����ɫ���ı��������ı���ɫ*/
	case WM_CTLCOLORSTATIC:
		hdcStatic = (HDC)wParam;
		SetTextColor(hdcStatic, RGB(38, 135, 133)); //����ɫ
		SetBkMode(hdcStatic, TRANSPARENT);			//͸������
		return (INT_PTR)GetStockObject(NULL_BRUSH); //����ɫ��ˢ
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

/*����ĵ��Ƿ���� ���ڷ�����,���򷵻ؼٲ���������*/
int CheckFile(HANDLE hfile)
{
	if (hfile == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, "�����ļ�ȱʧ��", "����", MB_OK);
		return 0;
	}
	else
	{
		/*every thing is OK*/
		return 1;
	}
}

//���ٻ��˵ĺ����ӿڣ�����1����ʼվ������2���յ�վ//
int lesstransfer(char start[], char destination[])
{
	Graph G;
	LINK L;

	int v1, v2;
	int stack[MAX_VERTEX_NUM] = {0}; //��ʼ��
	int visited[MAX_VERTEX_NUM] = {0};
	create_graph(&G); //����һ����������//
	create_path(&L);  //��������·��������//
	v1 = locatevex(&G, start);
	v2 = locatevex(&G, destination);

	if (v1 == -1 || v2 == -1)
	{
		TCHAR szTempInfo[100];
		sprintf(szTempInfo, "�������վ������\n");
		MessageBox(hwnd, szTempInfo, TEXT("NOTICE"), MB_ICONINFORMATION);
		return 0;
	}

	DFS(&G, stack, visited, v1, v2, 0, &L);
	print_lesstransfer(&G, &L);
	return 1;
}

//��ȡվ��ı�ţ���1-130//
int locatevex(Graph *G, char a[])
{
	int i = 1;
	while (strcmp(G->vertices[i].name, a) && i <= G->vexnum)
		i++;
	if (i <= G->vexnum)
		return i;
	else
		return -1;
}
//����һ����������
void create_graph(Graph *G)
{
	enode *p;
	int i, j, k, weight;
	FILE *fp;
	fp = fopen("stationinfo.txt", "r");
	fscanf(fp, "%d%d", &G->vexnum, &G->arcnum);
	fclose(fp);
	fp = fopen("station.txt", "r");
	for (i = 1; i <= G->vexnum; i++)
	{
		fscanf(fp, "%s", G->vertices[i].name);
		G->vertices[i].id = i;
		G->vertices[i].fadj = NULL;
		//����ͷ�巨������
	}
	fclose(fp);
	fp = fopen("stationdata.txt", "r");
	for (k = 1; k <= G->arcnum; k++)
	{
		fscanf(fp, "%d %d %d", &i, &j, &weight);
		p = (enode *)malloc(sizeof(enode));
		p->adjvex = j;
		p->weight = weight;
		p->nextarc = G->vertices[i].fadj;
		G->vertices[i].fadj = p;
		//�������磬˫���ڽ�
		p = (enode *)malloc(sizeof(enode));
		p->adjvex = i;
		p->weight = weight;
		p->nextarc = G->vertices[j].fadj;
		G->vertices[j].fadj = p;
	}
	fclose(fp);
}
/*������������ĵݹ��ҳ����е�·��
����һ��ͼ  ����2����ʱ���·��˳������  ����3���Ƿ����  ����4����ǰλ��  ����5�������λ��  ����6��ջ��  ����7������·��������*/
void DFS(Graph *G, int stack[], int visited[], int v, int destination, int top, LINK *L)
{
	int j, i;
	enode *p;
	pathnode *all;
	stack[top] = v;
	if (v == destination)
	{
		all = (pathnode *)malloc(sizeof(pathnode));
		all->counter = 0;
		all->top = top;
		int line;
		for (i = 0; i <= top; i++)
		{
			all->trans[i] = 0;
			all->path[i] = stack[i];
		}
		gettrans(G, all);
		addpath(L, all);

		return; //�ݹ�߽�����
	}
	else
		visited[v] = 1;
	p = G->vertices[v].fadj;
	while (p)
	{
		if (!visited[p->adjvex])
			DFS(G, stack, visited, p->adjvex, destination, top + 1, L);
		p = p->nextarc;
	}
	stack[top] = 0;
	visited[v] = 0;
}

//���·��//
void print_lesstransfer(Graph *G, LINK *L)
{
	int countSZ = 0; //�������·���Ļ�����ָ��
	int maximum = 999;
	int num = 999;
	int line, k;
	int count = 1;
	int i = 0, j;
	pathnode *p, *r, *q, *m;
	p = L->head;
	while (p->next != NULL)
	{
		p = p->next;
		if (p->counter <= maximum)
			maximum = p->counter;
		if (p->top <= num)
			num = p->top;
	}
	q = L->head;
	while (q->next != NULL)
	{
		q = q->next;
		if (q->top == num)
		{
			i = 0;
			countSZ += sprintf(szTransInfo + countSZ, "\n��ʱ�����·�ߡ�\n");

			for (k = 0; k < (q->top); k++)
			{
				if (q->trans[k])
				{
					countSZ += sprintf(szTransInfo + countSZ, "%s(�ڴ˻���%d����)---->", G->vertices[q->path[k]].name, G->vertices[q->path[k]].fadj->weight);
					i++;
				}
				else
				{
					line = G->vertices[q->path[k]].fadj->weight;
					countSZ += sprintf(szTransInfo + countSZ, "%s(%d|%d����)-->", G->vertices[q->path[k]].name, G->vertices[q->path[k]].id, line);
					i++;
				}
			}
			countSZ += sprintf(szTransInfo + countSZ, "%s(�յ�)\n", G->vertices[q->path[(q->top)]].name);

			countSZ += sprintf(szTransInfo + countSZ, "�˳�ʱ��:%.1f����\n", caculate_time(i, maximum));
			break; //Ϊ�˱����ظ���ֱ������
		}
	}
	countSZ += sprintf(szTransInfo + countSZ, "\n");
	r = L->head;
	countSZ += sprintf(szTransInfo + countSZ, "\n�����ٻ�����·��\n");
	while (r->next != NULL)
	{
		r = r->next;
		if (r->counter == maximum)
		{
			i = 0;

			for (k = 0; k < (r->top); k++)
			{
				if (r->trans[k])
				{
					countSZ += sprintf(szTransInfo + countSZ, "%s(�ڴ˻���%d����)---->", G->vertices[r->path[k]].name, G->vertices[r->path[k]].fadj->weight);
					i++;
				}
				else
				{
					line = G->vertices[r->path[k]].fadj->weight;
					countSZ += sprintf(szTransInfo + countSZ, "%s(%d|%d����)-->", G->vertices[r->path[k]].name, G->vertices[r->path[k]].id, line);
					i++;
				}
			}
			countSZ += sprintf(szTransInfo + countSZ, "%s(�յ�)\n", G->vertices[r->path[(r->top)]].name);

			if (maximum == 0)
				countSZ += sprintf(szTransInfo + countSZ, "����Ҫ����\n");
			else
				countSZ += sprintf(szTransInfo + countSZ, "��Ҫ����:%d��\n", maximum);
		}
	}
	countSZ += sprintf(szTransInfo + countSZ, "\n");
	m = L->head;
	while (m->next != NULL)
	{
		m = m->next;
		if (m->top == num)
		{
			i = 0;
			countSZ += sprintf(szTransInfo + countSZ, "\n��·�����·�ߡ�\n");

			for (k = 0; k < (m->top); k++)
			{
				if (m->trans[k])
				{
					countSZ += sprintf(szTransInfo + countSZ, "%s(�ڴ˻���%d����)---->", G->vertices[m->path[k]].name, G->vertices[m->path[k]].fadj->weight);
					i++;
				}
				else
				{
					line = G->vertices[m->path[k]].fadj->weight;
					countSZ += sprintf(szTransInfo + countSZ, "%s(%d|%d����)-->", G->vertices[m->path[k]].name, G->vertices[m->path[k]].id, line);
					i++;
				}
			}
			countSZ += sprintf(szTransInfo + countSZ, "%s(�յ�)\n", G->vertices[m->path[(q->top)]].name);

			countSZ += sprintf(szTransInfo + countSZ, "�˳�������:%.1f����\n", caculate_length(i, maximum));

			break; //Ϊ�˱����ظ���ֱ������
		}
	}
	countSZ += sprintf(szTransInfo + countSZ, "\n");
}

void create_path(LINK *L)
{
	//�½��������//
	L->head = (pathnode *)malloc(sizeof(pathnode));
	L->head->next = NULL;
}

void addpath(LINK *L, pathnode *p)
{
	//����Ĳ������//
	p->next = L->head->next;
	L->head->next = p;
}
//���ͬһ�����ϵ���������·��//
int getline(Graph *G, int i, int j)
{
	enode *p = G->vertices[i].fadj;
	while (p)
	{
		if (p->adjvex == j)
			return p->weight;
		p = p->nextarc;
	}
	return 0;
}

//���ÿ��·���ϵ�ת�˴���//
void gettrans(Graph *G, pathnode *all)
{
	int i;
	for (i = 0; i < all->top - 1; i++)
		if (getline(G, all->path[i], all->path[i + 1]) != getline(G, all->path[i + 1], all->path[i + 2]))
		{
			all->trans[i + 1] = 1;
			all->counter++;
		}
}
//���׼۸����//
int caculate_price(int station)
{
	if (station <= 4)
		return 2;
	else if (station > 4 && station <= 12)
		return (2 + (station % 4 - 1));
	else if (station > 12 && station <= 24)
		return (4 + (station % 4 - 1));
	else
		return 7;
}
//�˳�����������//
float caculate_length(int station, int count)
{
	static float f = 0;
	f += 0.2;
	if (f == 1)
		f = 0;
	return (2 * station + count * 1 + f);
}
//�˳�ʱ�����//
float caculate_time(int station, int count)
{
	static float f = 0;
	f += 0.5;
	if (f == 1)
		f = 0;
	return (3 * station + count * 6 + f);
}

//վ����������·��Ϣ  �Զ��жϻ��˵� //
int station_search_line(char station[], int parameter)
{
	Graph G;
	enode *p;
	FILE *fp;

	int line[10] = {0}; //����������· ���鳤��Ҫ4 ����Ҫ6
	int num, i, count, clear, l = 0;
	int a[MAX_VERTEX_NUM] = {0};
	int b[MAX_VERTEX_NUM] = {0};
	int c[MAX_VERTEX_NUM] = {0};

	create_graph(&G);
	fp = fopen("stationinfo.txt", "r");
	fscanf(fp, "%d%d", &clear, &count);
	fclose(fp);

	num = locatevex(&G, station);
	fp = fopen("stationdata.txt", "r");

	for (i = 1; i <= count; i++)
		fscanf(fp, "%d %d %d", &a[i], &b[i], &c[i]);
	fclose(fp);
	for (i = 1; i <= count; i++)
	{
		if (num == a[i] || num == b[i])
		{
			line[l++] = c[i];
		}
	}
	if (line[0] == line[1])
		line[1] = 0;

	//	p=G.vertices[num].fadj;
	if (line[1] == 0 && line[0] == 0)
		return -1;
	if (parameter == 1)
	{
		if (line[2] == 0 && line[1] == 0)
		{

			sprintf(szStationInfo, "��%s��վλ�ڵ���%d����\n", station, line[0]);
		}
		else if (line[1] != 0)
		{

			sprintf(szStationInfo, "��%s��վ�ǻ���վ��λ�ڵ���%d���ߺ͵���%d��\n", station, line[0], line[1]);
		}
		else
		{
			sprintf(szStationInfo, "��%s��վ�ǻ���վ��λ�ڵ���%d���ߺ͵���%d��\n", station, line[0], line[2]);
		}
	}
	else if (parameter == 0)
	{
		if (line[2] == 0)
			return 0;
		else
			return 1;
	}
	else
	{

		MessageBox(hwnd, TEXT("��ѯվ�㺯�����������ݴ���"), TEXT("NOTICE"), MB_ICONINFORMATION);
	}
}
