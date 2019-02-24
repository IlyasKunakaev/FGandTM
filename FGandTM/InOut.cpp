#include <iostream>
#include <fstream>
#include <locale>
#include <string>
using namespace std;

const short MAXLINE = 250;				//размер буфера ввода-вывода
const short ERRMAX = 10;				//макс. кол-во сохраняемых ошибок для текущей строки
short ErrInx = -1;						//кол-во найденных в текущей строке ошибок
bool ErrorOverFlow, haveError = true;		//флаги на переполнение ошибок и наличие ошибок в строке		
ofstream Flist;
fstream F("F:\\FGMT\\progPascal6.txt", ios::in);
unsigned AllErr = 1, lineOfCode = 1;

typedef struct {
	int lineNumber;
	int charNumber;
}textposition;

typedef struct {
	unsigned code;
	textposition tp;
}ErrListStruct[ERRMAX];

ErrListStruct ErrList;
textposition positionnow;
char ch;
unsigned LastInLine;
char* curLine;
char str[MAXLINE];

unsigned readNextLine()
{

	F.getline(str, MAXLINE, '\n');
	string s = curLine = str;
	return s.length();
}

void printErrors()
{
	for (int i = 0; i <= ErrInx; i++)
	{
		Flist << fixed;
		Flist << "**";
		Flist.width(2);
		Flist << AllErr;
		Flist << "**";
		Flist.width(ErrList[i].tp.charNumber + 1);
		Flist << "^";
		Flist << "  Ошибка!Код:" << ErrList[i].code << "\n";
		Flist << "******"; //<< TableCodeErr[ErrList[i].code] << " ";
		Flist << endl;
		AllErr++;
	}
}

void printLine()
{
	Flist.width(4);
	Flist << lineOfCode++;
	Flist.width(3);
	Flist << " " << curLine << "\n";
}

void nextch()
{
	if (positionnow.charNumber == LastInLine)
	{
		printLine();
		if (haveError)
			printErrors();
		LastInLine = readNextLine();
		positionnow.lineNumber++;
		positionnow.charNumber = 0;
	}
	else
		positionnow.charNumber++;
	ch = curLine[positionnow.charNumber];
}

void error(int code, textposition tp)	//формирование таблицы ошибок текущей строки
{
	if (ErrInx == ERRMAX)
		ErrorOverFlow = true;
	else
	{
		++ErrInx;
		ErrList[ErrInx].code = code;
		ErrList[ErrInx].tp.lineNumber = tp.lineNumber;
		ErrList[ErrInx].tp.charNumber = tp.charNumber;
	}
}

void createError()
{
	fstream A("F:\\FGMT\\CoordErr6.txt", ios::in);
	while (!A.eof())
	{
		++ErrInx;
		A >> ErrList[ErrInx].tp.lineNumber;
		A >> ErrList[ErrInx].tp.charNumber;
		A >> ErrList[ErrInx].code;
	}
}

void StartRead()
{
	char str[MAXLINE];
	createError();
	F.getline(str, MAXLINE, '\n');
	positionnow.lineNumber = 0;
	positionnow.charNumber = -1;
	string s = curLine = str;
	LastInLine = s.length();
	while (!F.eof())
		nextch();
}

int main()
{
	Flist.open("F:\\FGMT\\list.txt");
	StartRead();
	Flist.close();
	//cin.get();
	return 0;
}