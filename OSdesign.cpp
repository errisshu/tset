// OSdesign.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include <cstring>
#include <cmath>
#include <map>
#include <fstream>
#include "OSdesign.h"
#define n 32
#define N 900
#define M 124
using namespace std;
struct S_block
{
	bool bit[n];
	bool flag;
};
S_block common_data[N];//普通数据
S_block swap_data[M];//对换区
bool bit_map[30][30];//位示图
struct Swap_Map//对换区中的数据与普通数据区的映射
{
	bool addr[10];
	char data[4];
};
Swap_Map swap_map[M];
void initial_block(S_block &block);//初始化一个数据块
void initial_common_data();//初始化普通数据区
void initial_swap_data();//初始化对换区
void initial_bit_map();//初始化位示图
void initial_swap_map();//初始化swap映射
bool apply_common(int m,int* a);//查询是否有m块空闲块剩余，同时将空闲块编号输出
int block_sum(int m);//m个存储块所要占用的数据块数（还要存放索引）
void char_translate_bool(char c, bool b[n],int i);//将一个字符转化为二进制数并保存,从b[n]的i号位置开始
void int_translate_bool(int a, bool b[n], int i);//将一个int型数转为二进制地址存入b[n]中
int bool_translate_int(bool addr[10]);//将10位地址改为十进制数
int bool32_translate_int(bool b[n], int t);//将一个块中的二进制数转化为十进制数
void get_2_index(bool addr2[10], bool data[n], int t);//将一级索引块中的二级索引提出
void print_common();//输出普通发数据区中被占用块的内容
void get_bit_map(); //得到位示图
void Write_Txt();//将数据保存进一个TXT文件中
void Read_Txt();//将TXT文件中的数据读入
int main()
{
	Read_Txt();
	print_common();
	return 0;
}
void initial_block(S_block &block)
{
	for (int i = 0; i < n; i++)
	{
		block.bit[i] = 0;	
	}
	block.flag = 0;
}
void initial_common_data()
{
	for (int i = 0; i < N; i++)
	{
		initial_block(common_data[i]);
	}
}
void initial_swap_data()
{
	for (int i = 0; i < M; i++)
	{
		initial_block(swap_data[i]);
	}
}
void initial_bit_map()
{
	for (int i = 0; i < 30; i++)
	{
		for (int j = 0; j < 30; j++)
		{
			bit_map[i][j] = 0;
		}
	}
}
void initial_swap_map()
{
	for (int i = 0; i < M; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			swap_map[i].addr[j] = 0;
		}
		for (int t = 0; t < 4; t++)
		{
			swap_map[i].data[t] = '\0';
		}
	}
}
void initial()
{
	initial_common_data();
	initial_swap_data();
	initial_bit_map();
	initial_swap_map();
}

int Disk_Write(char data[36],bool addr[10])//存储功能的实现，将索引地址保存，容易出错
{
	int num;//字符数
	int count;//存储块数
	int sum;//包括索引块的存储块数
	num = strlen(data);
	bool f = 1;
	int a[13];//存储空闲块编号
	if (num % 4 != 0)
	{
		count = num / 4 + 1;
	}
	else
	{
		count = num / 4;
	}
	f = apply_common(count, a);
	sum = block_sum(count);
	if (f == 0)
	{
		for (int i = 0; i < sum; i++)
		{
			int j = a[i];
			common_data[j].flag = 1;
		}
		int t = 1;
		int b;
		for (int i = 0; i < num; i++)
		{
			b = a[i / 4];
			char_translate_bool(data[i], common_data[b].bit, t * 8 - 1);//将字符存入前count个内存块中,+1原因是数据的ASCII码转化为二进制只有7位，第一位为0
			t++;
			if (t == 5)
			{
				t = 1;
			}
		}
		int c = a[count];
		int_translate_bool(a[count + 1], common_data[c].bit, 9);//在一级索引模块存入二级模块的地址
		if (count > 3)
		{
			int_translate_bool(a[count + 2], common_data[c].bit, 19);
		}
		if (count > 6)
		{
			int_translate_bool(a[count + 3], common_data[c].bit, 29);
		}
		t = 1;
		for (int i = 0; i < count; i++)
		{
			b = i / 3;
			int_translate_bool(a[i], common_data[a[count + 1 + b]].bit, t * 10 - 1);//将文件块地址存入二级索引中
			t++;
			if (t == 4)
			{
				t = 1;
			}
		}
		int_translate_bool(a[count], addr, 9);//将一级索引地址存入addr中，之后传递给目录
		return 0;
	}
	else
	{
		cout << "空间不足!" << endl;
		return	 -1;
	}
}

int Free_Disk(bool addr[10])
{
	int m = bool_translate_int(addr);//一级索引的地址的十进制数表示
	bool addr1_1[10];//一级索引存储的三个二级索引地址
	bool addr1_2[10];
	bool addr1_3[10];
	for (int i = 0; i < 10; i++)//对三个索引地址初始化
	{
		addr1_1[i] = 0;
		addr1_2[i] = 0;
		addr1_3[i] = 0;
	}
	get_2_index(addr1_1, common_data[m].bit, 0);//得到第一个二级索引地址
	get_2_index(addr1_2, common_data[m].bit, 10);
	get_2_index(addr1_3, common_data[m].bit, 20);
	int m1_1 = bool_translate_int(addr1_1);//一级索引块中二级索引地址的二进制表示
	int m1_2 = bool_translate_int(addr1_2);
	int m1_3 = bool_translate_int(addr1_3);
	bool addr2_1[10];//第一个二级索引中的三个存储块地址
	bool addr2_2[10];
	bool addr2_3[10];
	for (int i = 0; i < 10; i++)//对三个索引地址初始化
	{
		addr2_1[i] = 0;
		addr2_2[i] = 0;
		addr2_3[i] = 0;
	}
	get_2_index(addr2_1, common_data[m1_1].bit, 0);//第一个二级索引块中的三个块地址
	get_2_index(addr2_2, common_data[m1_1].bit, 10);
	get_2_index(addr2_3, common_data[m1_1].bit, 20);
	int m2_1 = bool_translate_int(addr2_1);//二级索引块中三个存储块地址的十进制表示
	int m2_2 = bool_translate_int(addr2_2);
	int m2_3 = bool_translate_int(addr2_3);
	if (common_data[m2_1].flag == 1)//当此块被占用后，将其初始化
	{
		initial_block(common_data[m2_1]);
	}
	if (common_data[m2_2].flag == 1 || m2_2 != 0)//如果数据块地址不为空
	{
		initial_block(common_data[m2_2]);
	}
	if (common_data[m2_3].flag == 1 || m2_3 != 0)
	{
		initial_block(common_data[m2_3]);
	}
	initial_block(common_data[m1_1]);//将二级索引块初始化
	if (m1_2 != 0)
	{
		bool addr2_4[10];//第二个二级索引中的三个存储块地址
		bool addr2_5[10];
		bool addr2_6[10];
		for (int i = 0; i < 10; i++)//对三个索引地址初始化
		{
			addr2_4[i] = 0;
			addr2_5[i] = 0;
			addr2_6[i] = 0;
		}
		get_2_index(addr2_4, common_data[m1_2].bit, 0);//第二个二级索引块中的三个块地址
		get_2_index(addr2_5, common_data[m1_2].bit, 10);
		get_2_index(addr2_6, common_data[m1_2].bit, 20);
		int m2_4 = bool_translate_int(addr2_4);//第二个二级索引块中三个存储块地址的十进制表示
		int m2_5 = bool_translate_int(addr2_5);
		int m2_6 = bool_translate_int(addr2_6);
		if (common_data[m2_4].flag == 1)//当此块被占用后，将其初始化
		{
			initial_block(common_data[m2_4]);
		}
		if (common_data[m2_5].flag == 1 || m2_5 != 0)//如果数据块地址不为空
		{
			initial_block(common_data[m2_5]);
		}
		if (common_data[m2_6].flag == 1 || m2_6 != 0)
		{
			initial_block(common_data[m2_6]);
		}
		initial_block(common_data[m1_2]);//将二级索引块初始化
		if (m1_3 != 0)
		{
			bool addr2_7[10];//第三个二级索引中的三个存储块地址
			bool addr2_8[10];
			bool addr2_9[10];
			for (int i = 0; i < 10; i++)//对三个索引地址初始化
			{
				addr2_7[i] = 0;
				addr2_8[i] = 0;
				addr2_9[i] = 0;
			}
			get_2_index(addr2_7, common_data[m1_3].bit, 0);//第三个二级索引块中的三个块地址
			get_2_index(addr2_8, common_data[m1_3].bit, 10);
			get_2_index(addr2_9, common_data[m1_3].bit, 20);
			int m2_7 = bool_translate_int(addr2_7);//第三个二级索引块中三个存储块地址的十进制表示
			int m2_8 = bool_translate_int(addr2_8);
			int m2_9 = bool_translate_int(addr2_9);
			if (common_data[m2_7].flag == 1)//当此块被占用后，将其初始化
			{
				initial_block(common_data[m2_7]);
			}
			if (common_data[m2_8].flag == 1 || m2_8 != 0)//如果数据块地址不为空
			{
				initial_block(common_data[m2_8]);
			}
			if (common_data[m2_9].flag == 1 || m2_9 != 0)
			{
				initial_block(common_data[m2_9]);
			}
			initial_block(common_data[m1_3]);//将二级索引块初始化

		}
	}
	initial_block(common_data[m]);//将二级索引块初始化
	return 0;
}

bool apply_common(int m, int* a)
{
	bool f=1;
	int sum;
	sum = block_sum(m);
	int count=0;
	int t = 0;
	for (int i = 0; i < N; i++)
	{
		if (count == sum)
		{
			f = 0;
			break;
		}
		if (common_data[i].flag == 0)
		{
			count++;
			a[t++] = i;
		}
	}
	return f;
}

int block_sum(int m)
{
	int sum;
	if (m <= 3)
	{
		sum = m + 2;
	}
	else if (m <= 6)
	{
		sum = m + 3;
	}
	else
	{
		sum = m + 4;
	}
	return sum;
}

void char_translate_bool(char c, bool b[n],int i)
{
	int t = (int)c;
	int count = 0;
	
	while (t >= 1)
	{
		count = t % 2;
		b[i--] = count;
		t = t / 2;
	}
}

void int_translate_bool(int a, bool b[n], int i)
{
	int count = 0;
	while (a >= 1)
	{
		count = a % 2;
		b[i--] = count;
		a = a / 2;
	}
}

int bool_translate_int(bool addr[10])
{
	int sum = 0;
	int t = 0;
	int u = 0;
	for (int i = 9; i >= 0; i--)
	{
		u = pow(2, t);
		sum += addr[i] * u;
		t++;
	}
	return sum;
}

int bool32_translate_int(bool b[n], int t)
{
	int sum = 0;
	int r = 0;
	int u = 0;
	for (int i = 7; i >= 0; i--)
	{
		u = pow(2, r);
		sum += b[t+i] * u;
		r++;
	}
	return sum;
}

void get_2_index(bool addr2[10], bool data[n], int t)
{
	for (int i = 0; i < 10; i++)
	{
		addr2[i] = data[t + i];
	}
}

int GetPageAddr(bool addr[10], bool block_addr[9][10])
{
	int m = bool_translate_int(addr);//一级索引的地址的十进制数表示
	bool addr1_1[10];//一级索引存储的三个二级索引地址
	bool addr1_2[10];
	bool addr1_3[10];
	for (int i = 0; i < 10; i++)//对三个索引地址初始化
	{
		addr1_1[i] = 0;
		addr1_2[i] = 0;
		addr1_3[i] = 0;
	}
	get_2_index(addr1_1, common_data[m].bit, 0);//得到第一个二级索引地址
	get_2_index(addr1_2, common_data[m].bit, 10);
	get_2_index(addr1_3, common_data[m].bit, 20);
	int m1_1 = bool_translate_int(addr1_1);//一级索引块中二级索引地址的二进制表示
	int m1_2 = bool_translate_int(addr1_2);
	int m1_3 = bool_translate_int(addr1_3);
	get_2_index(block_addr[0], common_data[m1_1].bit, 0);//第一个二级索引块中的三个块地址
	get_2_index(block_addr[1], common_data[m1_1].bit, 10);
	get_2_index(block_addr[2], common_data[m1_1].bit, 20);
	if (m1_2 != 0)
	{
		get_2_index(block_addr[3], common_data[m1_2].bit, 0);//第二个二级索引块中的三个块地址
		get_2_index(block_addr[4], common_data[m1_2].bit, 10);
		get_2_index(block_addr[5], common_data[m1_2].bit, 20);
		if (m1_3 != 0)
		{
			get_2_index(block_addr[6], common_data[m1_3].bit, 0);//第三个二级索引块中的三个块地址
			get_2_index(block_addr[7], common_data[m1_3].bit, 10);
			get_2_index(block_addr[8], common_data[m1_3].bit, 20);
		}
		
	}
	
	return 0;
}

int GetPageData(bool addr[10], char data[4])
{

	int m = bool_translate_int(addr);
	int s;//对换表存储的地址
	for (int i = 0; i < M; i++)
	{
		s = bool_translate_int(swap_map[i].addr);
		if (s == m)
		{
			if (swap_map[i].data[0] != '\n')
			{
				for (int j = 0; j < 4; j++)
				{
					data[j] = swap_map[i].data[j];
				}
				return 0;//先查对换区，如果没有，再查普通数据区
			}
			else
			{
				break;
			}
		}
	}
	char c;
	int t = bool32_translate_int(common_data[m].bit, 0);
	if (t!= 0)
	{
		c = (char)t;
		data[0] = c;
	}
	t = bool32_translate_int(common_data[m].bit, 8);
	if (t != 0)
	{
		c = (char)t;
		data[1] = c;
	}
	t = bool32_translate_int(common_data[m].bit, 16);
	if (t != 0)
	{
		c = (char)t;
		data[2] = c;
	}
	t = bool32_translate_int(common_data[m].bit, 24);
	if (t != 0)
	{
		c = (char)t;
		data[3] = c;
	}
	return 0;
}

int SwapToDisk(bool addr[10], char data[4])
{
	int m;
	bool flag = 0;//判断是否有空闲块
	for (int i = 0; i < M; i++)
	{
		if (swap_data[i].flag == 0)
		{
			m = i;
			flag = 1;
			break;
		}
	}
	if (flag == 1)
	{
		int num;//字符数
		num = strlen(data);
		int t = 1;
		for (int i = 0; i < num; i++)
		{
			char_translate_bool(data[i], swap_data[m].bit, t * 8 - 1);
			t++;
			if (t == 5)
			{
				t = 1;
			}
		}
		swap_data[m].flag = 1;
		for (int i = 0; i < 10; i++)//建立映射
		{
			swap_map[m].addr[i] = addr[i];
		}
		for (int i = 0; i < 4; i++)
		{
			swap_map[m].data[i] = data[i];
		}
	}
	else//没有空闲块则将交换区和交换表都清零
	{
		initial_swap_data();
		initial_swap_map();
		SwapToDisk(addr, data);
	}
	return 0;
}


void print_common()
{
	for (int i = 0; i < N; i++)
	{
		if ( common_data[i].flag)
		{
			cout << "已占用块："<<i << endl;
			for (int j = 0; j < n; j++)
			{
				cout << common_data[i].bit[j];
			}
			cout << endl;
		}
	}
}

void get_bit_map()
{
	int t = 0, u = 0;
	for (int i = 0; i < 900; i++)
	{
		if (common_data[i].flag == 1)
		{
			bit_map[u][t] = 1;
		}
		else
		{
			bit_map[u][t] = 0;
		}
		t++;
		if (t == 30)
		{
			t = 0;
			u++;
		}
	}
}

void Write_Txt()
{
	ofstream outfile;
	outfile.open("bitfile.txt");
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < n; j++)
		{
			outfile << common_data[i].bit[j];
		}
		outfile << endl;
	}
	outfile.close();
	outfile.open("flagfile.txt");
	for (int i = 0; i < N; i++)
	{
		outfile << common_data[i].flag;
		outfile << endl;
	}
	outfile.close();
}

void Read_Txt()
{
	char data[50];
	char flag;
	ifstream infile;
	infile.open("bitfile.txt");
	for (int i = 0; i < N; i++)
	{
		infile >> data;
		for (int j = 0; j < n; j++)
		{
			if (data[j] == '0')
			{
				common_data[i].bit[j] = 0;
			}
			else if(data[j] == '1')
			{
				common_data[i].bit[j] = 1;
			}
		}
	}
	infile.close();

	infile.open("flagfile.txt");
	for (int i = 0; i < N; i++)
	{
		infile >> flag;
		if (flag == '0')
		{
			common_data[i].flag = 0;
		}
		else if (flag == '1')
		{
			common_data[i].flag = 1;
		}
	}
	infile.close();
}

void print_bit_map()
{
	get_bit_map();
	int count = 0;
	cout << "位示图：" << endl;
	cout << "————————————————————————————————————————————" << endl;
	for (int i = 0; i < 30; i++)
	{
		for (int j = 0; j < 30; j++)
		{
			cout << bit_map[i][j] << "  ";
			if (bit_map[i][j] == 0)
			{
				count++;
			}
		}
		cout << endl;
	}
	cout << "————————————————————————————————————————————" << endl;
	cout << "空闲块还有" << count << "Byte" << endl;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
