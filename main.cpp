#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <ctype.h>
#include <stdio.h>
#include <wchar.h>

/**查找程序部分**/
#define MAX_VERTEX_NUM 200
int MAX_LINE_NUM = 8;
int istrans[MAX_VERTEX_NUM] = {0};

/*一堆数据结构*/
typedef struct enode_type
{
	int adjvex;					//邻接点的位置//
	int weight;					//权值（即线路编号)//
	struct enode_type *nextarc; //指向下一个邻接点//
} enode;

typedef struct vnode_type
{
	int id;		   //顶点的编号//
	char name[20]; //顶点的名字，即站的名字//
	enode *fadj;   //指向第一个点//
} vnode;

typedef struct Graph_type
{
	vnode vertices[MAX_VERTEX_NUM]; //顶点集合(vertex)//
	int vexnum;						//图的顶点数目//
	int arcnum;						//图的边数目//

} Graph;

typedef struct pathnode_type
{
	int top;					//路径的数目//
	int path[MAX_VERTEX_NUM];	//路径信息//
	int trans[MAX_VERTEX_NUM];	//换乘点信息,0为非换乘点，1为换乘点//
	int counter;				//换乘的次数//
	struct pathnode_type *next; //指向下一个可抵达终点的路径信息节点//
} pathnode;

typedef struct LINK_type
{
	pathnode *head; //路径接口//
} LINK;

//获取站点的编号，从1-130//
int locatevex(Graph *G, char *a);
//获取当前线路号//
int getline(Graph *G, int i, int j);
//获取换乘点//
void gettrans(Graph *G, pathnode *all);
//建立带无向网络//
void create_graph(Graph *G);
//DFS深度优先搜索出所有可能的路线走法//
void DFS(Graph *G, int stack[], int visited[], int v, int destination, int top, LINK *L);
//输出最少换乘路径信息//
void print_lesstransfer(Graph *G, LINK *L);
//最少换乘的函数接口，参数1：起始站，参数2：终点站//
int lesstransfer(char start[], char destination[]);
//创建一个路径链表//
void create_path(LINK *L);
//插入一个路径信息//
void addpath(LINK *L, pathnode *p);
//计费，（按每个站点之间1公里简易计算，与实际有很大差别）//
int caculate_price(int station);
//乘车时间
float caculate_time(int station, int count);
//乘车公里数
float caculate_length(int station, int count);

//站点查询线路,可实现换乘站点自动判断
int station_search_line(char station[], int parameter);

/**WINDOWS窗体部分**/
/*handle of this program*/
HINSTANCE hInst;
HWND hwnd, hwnd2; /*handle for window*/

/*the main window class name*/
static TCHAR szClassName[] = TEXT("MetroWay");
static TCHAR szClassName2[] = TEXT("MetroWayMap");

/*the main window's title name*/
static TCHAR szTitle[] = _T("广州地铁线路查询系统");
static TCHAR szTitle2[] = _T("广州地铁线路图");

/*检查文档是否存在 存在返回真,否则返回假并弹窗报错*/
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

static TCHAR szTransInfo[999];	 //输出换乘线路查询的缓冲区
static TCHAR szStationInfo[999]; //输出站点查询结果的缓冲区

//显示放大线路图窗口的进程函数
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
		MessageBox(hwnd, "放大的线路图已关闭。", "提示", MB_OK);
		//PostQuitMessage(0); 此处未调用该函数，应用的消息队列中也就未添加WM_QUIT消息，
			// 在WinMain函数的消息循环中继续等待用户或系统消息，
		 // 消息循环没结束，整个主函数WinMan也就没结束。
			break;
	case WM_SIZE:
		{
			if(wParam==SIZE_MINIMIZED)//窗口被最小化
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

	static HFONT hFont; //逻辑字体

	static HWND hLabLine;		  //输出线路表
	static HWND hLabLineTitle;	  //输出线路表标题
	static HWND hBtnCompleteLine; //输出完整线路表的按钮
	static HWND hLabCompleteLine; //输出完整线路表的文本框

	static HWND hBtnZoom; //按钮--点击放大
	static HWND hLabRote; //静态文本框--路线查询
	static HWND hLabFind; //静态文本框--站点查询

	static HWND hLabStart; //静态文本框--起始
	static HWND hLabEnd;   //静态文本框--终点
	static HWND hLabState; //静态文本框--站点名

	static HWND hEditStart; //单行文本输入框--输入起点
	static HWND hEditEnd;	//单行文本输入框--输入终点
	static HWND hEditState; //单行文本输入框--输入站点

	static HWND hBtnSearchL;	//查询换乘按钮
	static HWND hBtnSearchR;	//查询站点按钮
	static HWND hLabSearchInfo; //输出查询结果的文本框

	int wmId, wmEvent;
	TCHAR szStart[100]; //起点站名的缓冲区
	TCHAR szEnd[100];	//终点站名的缓冲区
	TCHAR szState[100]; //查询线路的站名的缓冲区

	TCHAR szfile[999];		//读入文档的缓冲区
	TCHAR szLine[300];		//输出线路表的缓冲区
	TCHAR szWholeLine[999]; //输出完整线路表的缓冲区

	switch (message)
	{
	case WM_CREATE:
	{
		//创建逻辑字体
		hFont = CreateFont(-14 /*高*/, -7 /*宽*/, 0, 0, 400 /*一般这个值设为400*/,
						   FALSE /*斜体?*/, FALSE /*下划线?*/, FALSE /*删除线?*/, DEFAULT_CHARSET,
						   OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
						   FF_DONTCARE, TEXT("微软雅黑"));

		/*创建地铁路线的位图*/
		HWND hwndStatic = CreateWindow(TEXT("STATIC"),
									   NULL,
									   WS_CHILD | SS_BITMAP | WS_VISIBLE,
									   0, 0, 300, 100, hwnd, (HMENU)27, hInst, NULL);
		HBITMAP hbmp = (HBITMAP)LoadImage(NULL, "metroway.bmp", IMAGE_BITMAP, 479, 392, LR_LOADFROMFILE);
		if (hbmp)
		{
			SendMessage(hwndStatic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmp);
		}

		/*静态文本框——说明文本*/
		//提示点击放大图片
		hBtnZoom = CreateWindow(TEXT("button"), TEXT("点击图片放大"),
								WS_CHILD | WS_VISIBLE | BS_FLAT,
								235, 400, 100, 26,
								hwnd, (HMENU)1, hInst, NULL);

		//提示查询换乘路线
		hLabRote = CreateWindow(TEXT("static"), TEXT("查询换乘线路："),
								WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE /*垂直居中*/ | SS_CENTER /*水平居中*/,
								120 /*x坐标*/, 430 /*y坐标*/, 100 /*宽度*/, 26 /*高度*/,
								hwnd /*父窗口句柄*/, (HMENU)2 /*控件ID*/, hInst /*当前程序实例句柄*/, NULL);
		//提示查询站点信息
		hLabFind = CreateWindow(TEXT("static"), TEXT("查询站点信息："),
								WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE /*垂直居中*/ | SS_RIGHT /*水平居右*/,
								340 /*x坐标*/, 430 /*y坐标*/, 70 /*宽度*/, 26 /*高度*/,
								hwnd /*父窗口句柄*/, (HMENU)3 /*控件ID*/, hInst /*当前程序实例句柄*/, NULL);

		/*静态文本框——给输入框提示的说明文本*/
		//【起始站】
		hLabStart = CreateWindow(TEXT("static"), TEXT("起点："),
								 WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE /*垂直居中*/ | SS_RIGHT /*水平居右*/,
								 20 /*x坐标*/, 460 /*y坐标*/, 70 /*宽度*/, 26 /*高度*/,
								 hwnd /*父窗口句柄*/, (HMENU)4 /*控件ID*/, hInst /*当前程序实例句柄*/, NULL);
		//【终点站】
		hLabEnd = CreateWindow(TEXT("static"), TEXT("终点："),
							   WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE /*垂直居中*/ | SS_RIGHT /*水平居右*/,
							   20, 500, 70, 26,
							   hwnd, (HMENU)5, hInst, NULL);
		//【站点名】
		hLabState = CreateWindow(TEXT("static"), TEXT("站点名："),
								 WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE /*垂直居中*/ | SS_RIGHT /*水平居右*/,
								 270, 460, 70, 26,
								 hwnd, (HMENU)6, hInst, NULL);

		/**单行文本框控件**/
		//输入起点名
		hEditStart = CreateWindow(TEXT("edit"), TEXT(""),
								  WS_CHILD | WS_VISIBLE | WS_BORDER /*边框*/ | ES_AUTOHSCROLL /*水平滚动*/,
								  100, 460, 140, 26,
								  hwnd, (HMENU)7, hInst, NULL);
		//输入终点名
		hEditEnd = CreateWindow(TEXT("edit"), TEXT(""),
								WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL /*水平滚动*/,
								100, 500, 140, 26,
								hwnd, (HMENU)8, hInst, NULL);
		//输入站点名
		hEditState = CreateWindow(TEXT("edit"), TEXT(""),
								  WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL /*水平滚动*/,
								  340, 460, 140, 26,
								  hwnd, (HMENU)9, hInst, NULL);

		//查询按钮
		hBtnSearchL = CreateWindow(TEXT("button"), TEXT("查询"),
								   WS_CHILD | WS_VISIBLE | BS_FLAT /*扁平样式*/,
								   120, 540, 70, 30,
								   hwnd, (HMENU)10, hInst, NULL);
		hBtnSearchR = CreateWindow(TEXT("button"), TEXT("查询"),
								   WS_CHILD | WS_VISIBLE | BS_FLAT /*扁平样式*/,
								   340, 540, 70, 30,
								   hwnd, (HMENU)26, hInst, NULL);
		/*显示页面右侧线路表*/
		//输出线路表
		hLabLine = CreateWindow(TEXT("static"), TEXT("我是一个文本框"), WS_CHILD | WS_VISIBLE | SS_LEFT, 490, 40, 300, 200, hwnd, (HMENU)11, hInst, NULL);

		//线路表的提示文字
		hLabLineTitle = CreateWindow(TEXT("static"), TEXT("地铁线路表"), WS_CHILD | WS_VISIBLE | SS_CENTER, 550, 10, 100, 26, hwnd, (HMENU)12, hInst, NULL);

		//显示完整线路表的按钮
		hBtnCompleteLine = CreateWindow(TEXT("button"), TEXT("显示完整线路表"), WS_CHILD | WS_VISIBLE | BS_FLAT, 550, 250, 120, 30, hwnd, (HMENU)13, hInst, NULL);

		//显示完整线路表的文本框
		//hLabCompleteLine = CreateWindow(TEXT("static"), TEXT("输出完整线路表"), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 490, 300, 300, 200, hwnd, (HMENU)14, hInst, NULL);

		//显示查询结果的文本框
		//hLabSearchInfo = CreateWindow(TEXT("static"), TEXT("输出查询结果"), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 50, 580, 400, 200, hwnd, (HMENU)15, hInst, NULL);

		//依次设置控件的字体

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

		/*检查数据文档是否存在*/
		DWORD RSize;
		HANDLE fLine, fStationinfo, fStation, fStationdata;
		fLine = (HANDLE)CreateFile(TEXT("line.txt"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		fStationinfo = (HANDLE)CreateFile(TEXT("stationinfo.txt"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		fStationdata = (HANDLE)CreateFile(TEXT("stationdata.txt"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		fStation = (HANDLE)CreateFile(TEXT("station.txt"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

		CheckFile(fStationinfo);
		CheckFile(fStationdata);
		CheckFile(fStation);

		//创建窗口时显示简略线路表
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
		case 10: //点击了查询路线的按钮
			//获取输入框的数据
			{
				int i = 0, j = 0;
				char start[20], destination[20];

				i = GetWindowText(hEditStart, szStart, 100);
				j = GetWindowText(hEditEnd, szEnd, 100);

				//查询换乘路线信息
				if (i > 0 && j > 0)
				{
					sscanf(szStart, "%s", start);
					sscanf(szEnd, "%s", destination);
					if (strcmp(start, destination) == 0)
					{
						MessageBox(hwnd, TEXT("输入的起始站和终点站相同！"), TEXT("NOTICE"), MB_ICONINFORMATION);
					}
					else
					{
						if (lesstransfer(start, destination) != 0) //最小换乘接口//
						{
							MessageBox(NULL, szTransInfo, TEXT("路线信息"), NULL);
						}
					}
				}
				else
				{
					MessageBox(hwnd, TEXT("请输入完整的起始站和终点站名！"), TEXT("NOTICE"), MB_ICONINFORMATION);
				}

				break;
			}
		case 26:
		{
			//点击了查询站点信息的按钮
			int k = 0;
			char station[20];
			k = GetWindowText(hEditState, szState, 100);

			if (k > 0)
			{
				sscanf(szState, "%s", station);
				if (station_search_line(station, 1) == -1)
				{
					TCHAR szTempInfo[100];
					sprintf(szTempInfo, "提示：未查询到%s站的信息！", station);
					MessageBox(hwnd, szTempInfo, TEXT("NOTICE"), MB_ICONINFORMATION);
				}
				else
				{
					//SetWindowText(hLabSearchInfo, szStationInfo);
					MessageBox(NULL, szStationInfo, "站点信息", NULL);
				}
			}
			else if (k <= 0)
			{
				MessageBox(hwnd, TEXT("请输入站点名！"), TEXT("NOTICE"), MB_ICONINFORMATION);
			}
			break;
		}
		case 13: //点击显示详细线路
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

				charLen += sprintf(szWholeLine + charLen, "【地铁%d号线】\n", i);
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
				MessageBox(NULL, szWholeLine, "广州地铁线路表", NULL);
			}

			break;
		}
		case 1:
			//点击放大图片

			ShowWindow(hwnd2, SW_SHOW);
			UpdateWindow(hwnd2);

			break;
		}

	case WM_PAINT:

		hdc = BeginPaint(hwnd, &ps);

		//检查文件

		EndPaint(hwnd, &ps);
		break;

		/*设置Static控件的背景颜色和文本背景、文本颜色*/
	case WM_CTLCOLORSTATIC:
		hdcStatic = (HDC)wParam;
		SetTextColor(hdcStatic, RGB(38, 135, 133)); //蓝绿色
		SetBkMode(hdcStatic, TRANSPARENT);			//透明背景
		return (INT_PTR)GetStockObject(NULL_BRUSH); //无颜色画刷
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

/*检查文档是否存在 存在返回真,否则返回假并弹窗报错*/
int CheckFile(HANDLE hfile)
{
	if (hfile == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, "数据文件缺失！", "错误", MB_OK);
		return 0;
	}
	else
	{
		/*every thing is OK*/
		return 1;
	}
}

//最少换乘的函数接口，参数1：起始站，参数2：终点站//
int lesstransfer(char start[], char destination[])
{
	Graph G;
	LINK L;

	int v1, v2;
	int stack[MAX_VERTEX_NUM] = {0}; //初始化
	int visited[MAX_VERTEX_NUM] = {0};
	create_graph(&G); //创建一个无向网络//
	create_path(&L);  //创建保存路径的链表//
	v1 = locatevex(&G, start);
	v2 = locatevex(&G, destination);

	if (v1 == -1 || v2 == -1)
	{
		TCHAR szTempInfo[100];
		sprintf(szTempInfo, "您输入的站名有误！\n");
		MessageBox(hwnd, szTempInfo, TEXT("NOTICE"), MB_ICONINFORMATION);
		return 0;
	}

	DFS(&G, stack, visited, v1, v2, 0, &L);
	print_lesstransfer(&G, &L);
	return 1;
}

//获取站点的编号，从1-130//
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
//创建一个无向网络
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
		//利用头插法插入结点
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
		//无向网络，双向邻接
		p = (enode *)malloc(sizeof(enode));
		p->adjvex = i;
		p->weight = weight;
		p->nextarc = G->vertices[j].fadj;
		G->vertices[j].fadj = p;
	}
	fclose(fp);
}
/*利用深度搜索的递归找出所有的路径
参数一：图  参数2：临时存放路径顺序数组  参数3：是否访问  参数4：当前位置  参数5：到达的位置  参数6：栈顶  参数7：所有路径的链表*/
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

		return; //递归边界条件
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

//输出路径//
void print_lesstransfer(Graph *G, LINK *L)
{
	int countSZ = 0; //输出换乘路径的缓冲区指针
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
			countSZ += sprintf(szTransInfo + countSZ, "\n【时间最短路线】\n");

			for (k = 0; k < (q->top); k++)
			{
				if (q->trans[k])
				{
					countSZ += sprintf(szTransInfo + countSZ, "%s(在此换乘%d号线)---->", G->vertices[q->path[k]].name, G->vertices[q->path[k]].fadj->weight);
					i++;
				}
				else
				{
					line = G->vertices[q->path[k]].fadj->weight;
					countSZ += sprintf(szTransInfo + countSZ, "%s(%d|%d号线)-->", G->vertices[q->path[k]].name, G->vertices[q->path[k]].id, line);
					i++;
				}
			}
			countSZ += sprintf(szTransInfo + countSZ, "%s(终点)\n", G->vertices[q->path[(q->top)]].name);

			countSZ += sprintf(szTransInfo + countSZ, "乘车时间:%.1f分钟\n", caculate_time(i, maximum));
			break; //为了避免重复，直接跳出
		}
	}
	countSZ += sprintf(szTransInfo + countSZ, "\n");
	r = L->head;
	countSZ += sprintf(szTransInfo + countSZ, "\n【最少换乘线路】\n");
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
					countSZ += sprintf(szTransInfo + countSZ, "%s(在此换乘%d号线)---->", G->vertices[r->path[k]].name, G->vertices[r->path[k]].fadj->weight);
					i++;
				}
				else
				{
					line = G->vertices[r->path[k]].fadj->weight;
					countSZ += sprintf(szTransInfo + countSZ, "%s(%d|%d号线)-->", G->vertices[r->path[k]].name, G->vertices[r->path[k]].id, line);
					i++;
				}
			}
			countSZ += sprintf(szTransInfo + countSZ, "%s(终点)\n", G->vertices[r->path[(r->top)]].name);

			if (maximum == 0)
				countSZ += sprintf(szTransInfo + countSZ, "不需要换乘\n");
			else
				countSZ += sprintf(szTransInfo + countSZ, "需要换乘:%d次\n", maximum);
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
			countSZ += sprintf(szTransInfo + countSZ, "\n【路程最短路线】\n");

			for (k = 0; k < (m->top); k++)
			{
				if (m->trans[k])
				{
					countSZ += sprintf(szTransInfo + countSZ, "%s(在此换乘%d号线)---->", G->vertices[m->path[k]].name, G->vertices[m->path[k]].fadj->weight);
					i++;
				}
				else
				{
					line = G->vertices[m->path[k]].fadj->weight;
					countSZ += sprintf(szTransInfo + countSZ, "%s(%d|%d号线)-->", G->vertices[m->path[k]].name, G->vertices[m->path[k]].id, line);
					i++;
				}
			}
			countSZ += sprintf(szTransInfo + countSZ, "%s(终点)\n", G->vertices[m->path[(q->top)]].name);

			countSZ += sprintf(szTransInfo + countSZ, "乘车公里数:%.1f公里\n", caculate_length(i, maximum));

			break; //为了避免重复，直接跳出
		}
	}
	countSZ += sprintf(szTransInfo + countSZ, "\n");
}

void create_path(LINK *L)
{
	//新建链表操作//
	L->head = (pathnode *)malloc(sizeof(pathnode));
	L->head->next = NULL;
}

void addpath(LINK *L, pathnode *p)
{
	//链表的插入操作//
	p->next = L->head->next;
	L->head->next = p;
}
//求出同一条边上的两点间的线路号//
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

//求出每条路线上的转乘次数//
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
//简易价格计算//
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
//乘车公里数计算//
float caculate_length(int station, int count)
{
	static float f = 0;
	f += 0.2;
	if (f == 1)
		f = 0;
	return (2 * station + count * 1 + f);
}
//乘车时间计算//
float caculate_time(int station, int count)
{
	static float f = 0;
	f += 0.5;
	if (f == 1)
		f = 0;
	return (3 * station + count * 6 + f);
}

//站点名查找线路信息  自动判断换乘点 //
int station_search_line(char station[], int parameter)
{
	Graph G;
	enode *p;
	FILE *fp;

	int line[10] = {0}; //两条换乘线路 数组长度要4 三条要6
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

			sprintf(szStationInfo, "【%s】站位于地铁%d号线\n", station, line[0]);
		}
		else if (line[1] != 0)
		{

			sprintf(szStationInfo, "【%s】站是换乘站，位于地铁%d号线和地铁%d线\n", station, line[0], line[1]);
		}
		else
		{
			sprintf(szStationInfo, "【%s】站是换乘站，位于地铁%d号线和地铁%d线\n", station, line[0], line[2]);
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

		MessageBox(hwnd, TEXT("查询站点函数：参数传递错误！"), TEXT("NOTICE"), MB_ICONINFORMATION);
	}
}
