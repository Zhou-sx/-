#include <graphics.h>
#include <time.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <memory>
using namespace std;
//画布大小
const int X_length = 500;
const int Y_length = 1000;
const int X_mid = X_length / 2;
const int Y_mid = Y_length / 2;
//方格大小
const int unit = 50;
const int X_unit = X_length / unit;
const int Y_unit = Y_length / unit;
//棋盘矩阵
int board[X_unit][Y_unit];
//19个形状
/*
方形 0x660 长方形 0x4444,0xf00
右T型 0x3220,0x710,0x2260,0x8e0
左T型 0xc440,0x2e00,0x4460,0x740
左Z型 0x4620,0x360 右Z型 0x2640,0xc60
三角型 0x4e00,0x4640,0xe40,0x4c40
*/
char c = 0;//控制游戏的字符 27表示退出
int score = 0;
vector<vector<unsigned int>> RockArray = {
	{0x660},
	{0x4444,0xf00},
	{0x3220,0x710,0x2260,0x8e0},
	{0xc440,0x2e00,0x4460,0x740},
	{0x4620,0x360},
	{0x2640,0xc60},
	{0x4e00,0x4640,0xe40,0x4c40}
};
//15种颜色
vector <int> COLORS = { 0xAA0000,0x00AA00,0xAAAA00,0x0000AA,0xAA00AA,0x0055AA,0xAAAAAA,0x555555,0xFF5555,
						0x55FF55,0xFFFF55,0x5555FF,0xFF55FF,0x55FFFF,0xFFFFFF };
class Block {
public:
	Block() {};
	Block(int BlockType, int ColorType) {
		_BlockType = BlockType;
		//选择初始的形状 默认处于0号状态
		_RotateType = 0;//初始化物体中心位置，i,j表示相对于中心偏移了几个单位 
		_Center_X = X_mid;
		_Center_Y = 2 * unit;
		_Color = COLORS[ColorType];
		//初始化时要进行GameOver判断
		if (GAME_IS_OVER()) c = 27;//27为esc
	}
	//平移 0：默认下落-1：左移1：右移  只判断一次移动
	int move(int speed = unit, int flag = 0) {
		show_erase(1);//在判断Is_mutable之前先将原来的擦去
		int x = _Center_X + flag * unit, y = _Center_Y;
		if (flag == 0) y += speed;
		int res = Is_mutable(x, y, _RotateType);
		if (res == 1) {
			_Center_X = x;
			_Center_Y = y;
			show_erase(0);
		}
		else if (res == 2) {
			_Center_Y = y;
			show_erase(0);
		}
		else {
			show_erase(0);
		}
		return res;
	}
	//旋转
	int rotate() {
		int RotateType = (_RotateType == RockArray[_BlockType].size() - 1) ? 0 : _RotateType + 1;
		show_erase(1);
		int res = Is_mutable(_Center_X, _Center_Y + unit, RotateType);
		if (res == 1) {
			_RotateType = RotateType;
			_Center_Y += unit;//旋转并下落
			show_erase(0);
		}
		else if (res == 2) {
			_Center_Y += unit;//不旋转只下落
			show_erase(0);
		}
		else {
			show_erase(0);
		}
		return res;
	}
	//判断是否可以移动
	int Is_mutable(int Center_X, int Center_Y, int RotateType) {
		unsigned int rock = RockArray[_BlockType][RotateType];
		unsigned int mask = (unsigned int)1 << 15;//1左移15位 掩码
		int x, y, res = 1;
		for (int i = -2;i < 2;i++) {
			for (int j = -2;j < 2;j++) {
				x = (Center_X + j * unit) / unit;
				y = (Center_Y + i * unit) / unit;
				if (rock & mask) {
					//注意x,y不能越界
					if (x < 0 || x >= X_unit) {
						if (y < Y_unit) res = 2;//2表示不能左右动，可能还可以向下移动 2不能覆盖3
						else return 3;
					}
					else if (y >= Y_unit || board[x][y]) return 3;//3表示彻底不能动 有BUG当左右移动至已被占的格时会被误入3，解决办法是分开右移和下降
				}
				mask >>= 1;//右移一位
			}
		}
		return res;
	}
	//GameOver判定
	bool GAME_IS_OVER() {
		if (Is_mutable(_Center_X, _Center_Y, _RotateType) == 3) return true;
		return false;
	}
	//显示Block
	void show_erase(int flag = 0) {
		unsigned int rock = RockArray[_BlockType][_RotateType];
		unsigned int mask = (unsigned int)1 << 15;//1左移15位 掩码
		int x, y;
		for (int i = -2;i < 2;i++) {
			for (int j = -2;j < 2;j++) {
				x = _Center_X + j * unit;
				y = _Center_Y + i * unit;
				if (rock & mask) {
					if (flag == 0) {
						setlinecolor(WHITE);
						setfillcolor(_Color);
						board[x / unit][y / unit] = _Color;//标记该格 用颜色标记
						fillrectangle(x, y, x + unit, y + unit);
					}
					else {
						setlinecolor(BLACK);
						setfillcolor(BLACK);
						board[x / unit][y / unit] = 0;//删除该格标记
						fillrectangle(x, y, x + unit, y + unit);
					}
				}
				mask >>= 1;//右移一位
			}
		}
	}
private:
	//物体中心位置
	int _Center_X;
	int _Center_Y;
	int _BlockType;//0-6
	int _RotateType;//旋转型
	int _Color;//填充颜色
};
class Blockmanager {
public:
	Blockmanager(int BlockType, int ColorType) : piml(make_shared<class Block>(BlockType, ColorType)) {} //seed随机形状0-6
	int move(int speed = unit, int flag = 0) {
		return (*piml).move(speed, flag);
	}
	int rotate() {
		return (*piml).rotate();
	}
	void show_erase(int flag = 0) {
		(*piml).show_erase(flag);
	}
private:
	shared_ptr<class Block> piml;
};
void check_erase() {
	vector<int> erase_level;
	for (int i = Y_unit - 1;i >= 0;i--) {
		for (int j = 0;j < X_unit;j++) {
			if (board[j][i] == 0) break;
			if (j == X_unit - 1) erase_level.push_back(i);
		}
	}
	if (!erase_level.empty()) {
		unsigned int loc = 0;
		//loc表示该行要下落的层数
		for (int i = Y_unit - 1;i >= 0;i--) {
			int temp = i;
			while (loc < erase_level.size() && temp == erase_level[loc]) {
				loc++;
				temp--;
			}
			for (int j = 0;j < X_unit;j++) {
				if (i - loc < 0)
					board[j][i] = 0;
				else
					board[j][i] = board[j][i - loc];
				int x = j * unit, y = i * unit;
				if (board[j][i]) {
					setlinecolor(WHITE);
					setfillcolor(board[j][i]);//选择颜色
				}
				else
				{
					setlinecolor(BLACK);
					setfillcolor(BLACK);
				}
				fillrectangle(x, y, x + unit, y + unit);
			}
		}
		score += loc * 100;
	}
}
void show_score() {
	LOGFONT f;
	gettextstyle(&f);						// 获取当前字体设置
	f.lfHeight = 30;						// 设置字体高度为 48
	_tcscpy_s(f.lfFaceName, _T("黑体"));		// 设置字体为“黑体”(高版本 VC 推荐使用 _tcscpy_s 函数)
	f.lfQuality = ANTIALIASED_QUALITY;		// 设置输出效果为抗锯齿  
	settextstyle(&f);						// 设置字体样式
	//settextstyle(50, 0, _T("Consolas"));
	outtextxy(0, 0, _T("得分:"));
	TCHAR s[5];
	_stprintf_s(s, _T("%d"), score);//输出值1024
	outtextxy(80, 0, s);
	//outtextxy(0, 50, _T("Designed By Zhousx"));
}
void run() {
	int flag = 0;
	Blockmanager b(rand() % 7, rand() % 15);
	while (c != 27) {
		if (_kbhit()) {
			c = _getch();
			switch (c)
			{
			case 's':
				flag = b.move(unit);
				break;
			case 'a':
				flag = b.move(unit, -1);
				flag = b.move(unit);
				break;
			case 'd':
				flag = b.move(unit, 1);
				flag = b.move(unit);
				break;
			case ' ':
				flag = b.rotate();
				break;
			default:
				break;
			}
		}
		else {
			c = '.';
			flag = b.move(unit);
		}
		if (flag == 3) {
			check_erase();//检查是否可以消去某一层
			show_score();//更新得分
			break;
		}
		Sleep(200);
	}
}
void init_graph() {
	LOGFONT f;
	gettextstyle(&f);						// 获取当前字体设置
	f.lfHeight = 30;						// 设置字体高度为 30
	_tcscpy_s(f.lfFaceName, _T("黑体"));		// 设置字体为“黑体”(高版本 VC 推荐使用 _tcscpy_s 函数)
	f.lfQuality = ANTIALIASED_QUALITY;		// 设置输出效果为抗锯齿  
	settextstyle(&f);						// 设置字体样式
	outtextxy(175, 450, _T("俄罗斯方块"));
	f.lfHeight = 20;
	settextstyle(&f);
	outtextxy(190, 500, _T("按任意键开始"));
	outtextxy(0, 0, _T("Designed By Zhousx"));
}
void over_graph() {
	LOGFONT f;
	gettextstyle(&f);						// 获取当前字体设置
	f.lfHeight = 30;						// 设置字体高度为 30
	_tcscpy_s(f.lfFaceName, _T("黑体"));		// 设置字体为“黑体”(高版本 VC 推荐使用 _tcscpy_s 函数)
	f.lfQuality = ANTIALIASED_QUALITY;		// 设置输出效果为抗锯齿  
	settextstyle(&f);						// 设置字体样式
	outtextxy(175, 450, _T("Game Over!"));
	_getch();
}
/****************************************************/
int main() {
	srand((unsigned int)time(NULL));
	initgraph(X_length, Y_length);
	memset(board, 0, sizeof(board));//初始化棋盘
	init_graph();
	//
	c = _getch();
	cleardevice();
	while (1) {
		if (_kbhit()) {
			c = _getch();
		}
		run();
		if (c == 27) {
			over_graph();
			break;
		}
	}
	_getch();
	closegraph();
}

