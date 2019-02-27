#include <iostream>
#include <fstream>
#include <locale>
#include <string>
#include <map>
#include "DECW.h"
#include "DECKEY.h"
#include "StructKey.h"
using namespace std;

const short MAX_IDENT = 15;
const short MAXINT = 32767;
const short MAXLINE = 250;				//размер буфера ввода-вывода
const short ERRMAX = 10;				//макс. кол-во сохраняемых ошибок для текущей строки
short ErrInx = -1;						//кол-во найденных в текущей строке ошибок
bool ErrorOverFlow, haveError = false;		//флаги на переполнение ошибок и наличие ошибок в строке		
ofstream Flist;
fstream F("F:\\FGMT\\progPascal6.txt", ios::in);
unsigned SumErr = 1, lineOfCode = 1, sym, lname;
map <int, string> AllErrors;
map <int, string> ::iterator iter = AllErrors.begin();
bool stop = false, theend = false;
int p, nmb_int;
float nmb_float;
char one_symbol, ch, str[MAXLINE], *addrname, name[MAX_IDENT];

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
textposition token;
unsigned LastInLine;
char* curLine;

void nextsym();

#pragma region Errors

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

void tableOfAllError()	//таблица со всеми возможными ошибками
{
	ifstream A;
	A.open("F:\\FGMT\\Err.txt");
	int key;
	char value[85];
	while (!A.eof())
	{
		A >> key;
		A.get();
		if (!A.eof())
			A.getline(value, 85);
		AllErrors[key] = value;
	}
	A.close();
}

void printErrors()
{
	for (int i = 0; i <= ErrInx; i++)
	{
		Flist << fixed;
		Flist << "**";
		Flist.width(2);
		Flist << SumErr;
		Flist << "**";
		Flist.width(ErrList[i].tp.charNumber + 2);
		Flist << "^";
		Flist << "  Ошибка! Код:" << ErrList[i].code << "\n";
		iter = AllErrors.find(ErrList[i].code);
		Flist << "******" << " " << iter->second;
		Flist << endl;
		SumErr++;
	}
}

#pragma endregion


void printLine()
{
	Flist.width(4);
	Flist << lineOfCode++;
	Flist.width(3);
	Flist << " " << curLine << "\n";
}

unsigned readNextLine()
{

	F.getline(str, MAXLINE, '\n');
	string s = curLine = str;
	return s.length();
}

void nextch()
{
	if (positionnow.charNumber == LastInLine)
	{
		printLine();
		if (haveError)
		{
			printErrors();
			for (int k = 0; k <= ErrInx; k++)
			{
				ErrList[k].code = NULL;
				ErrList[k].tp.charNumber = NULL;
				ErrList[k].tp.lineNumber = NULL;
			}
			haveError = false;
			ErrInx = -1;
		}
		if (!F.eof())
		{
			LastInLine = readNextLine();
			haveError = false;
			positionnow.lineNumber++;
			positionnow.charNumber = 0;
		}
		else
			stop = true;
		/*if (F.eof())
			while (!stop)
				nextch();*/
	}
	else
		positionnow.charNumber++;
	ch = curLine[positionnow.charNumber];
	//return ch;
}

void nextsym()
{
	while (ch == '\0')
		nextch();
	while (ch == ' ')
		nextch();
	token.lineNumber = positionnow.lineNumber;
	token.charNumber = positionnow.charNumber;
	if (isalpha(ch))	//прочитали букву
		p = 1;
	else if (isdigit(ch))		//прочитали цифру
		p = 2;
	else				//прочитали спец. символ
		p = 3;
	char tmp;
	int i, j;
	switch (p)
	{
	case (1):
		lname = 0;
		while (((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) && lname < MAX_IDENT)
		{
			name[lname++] = ch;
			nextch();
		}
		strcpy_s(keywords[last[lname]].namekey, name);
		if (lname > 1)
			i = last[lname - 1] + 1;
		else
			i = 0;
		while (strcmp(keywords[i].namekey, name) != 0)
			i++;
		sym = keywords[i].codekey;
		for (int k = 0; k < MAX_IDENT; k++)
			name[k] = NULL;
		break;
	case (2):
		int digit;
		nmb_int = 0;
		while (ch >= '0' && ch <= '9')
		{
			digit = ch - '0';
			if (nmb_int < MAXINT / 10 || (nmb_int == MAXINT / 10 && nmb_int <= MAXINT % 10))
				nmb_int = 10 * nmb_int + digit;
			else
			{
				error(203, positionnow);
				haveError = true;
				nmb_int = 0;
			}
			nextch();
		}
		sym = intc;
		break;
	case (3):
		switch (ch)
		{
		case '<':
			nextch();
			if (ch == '=')
			{
				sym = laterequal;
				nextch();
			}
			else
				if (ch == '>')
				{
					sym = latergreater;
					nextch();
				}
				else
					sym = later;
			break;

		case '>':
			nextch();
			if (ch == '=')
			{
				sym = greaterequal;
				nextch();
			}
			else
				sym = greater;
			break;

		case ':':
			nextch();
			if (ch == '=')
			{
				sym = assign;
				nextch();
			}
			else
				sym = colon;
			break;

		case '+':
			sym = plus;
			nextch();
			break;

		case '-':
			sym = minus;
			nextch();
			break;

		case '*':
			nextch();
			if (ch == ')')
			{
				sym = rcomment;
				nextch();
			}
			else
				sym = star;
			break;

		case '/':
			nextch();
			if (ch == '/')
			{
				while (positionnow.charNumber <= LastInLine)
					nextch();
			}
			else sym = slash;
			break;

		case '=':
			sym = equal;
			nextch();
			break;

		case '(':
			nextch();
			if (ch == '*')
			{
				sym = lcomment;
				do 
				{
					nextch();
					if (ch == '*')
					{
						tmp = ch;
						nextch();
						if (ch == ')' && tmp == '*')
						{
							nextch();
							break;
						}
					}
				} while (ch != '*');
			}
			else
				sym = leftpar;
			break;

		case ')':
			sym = rightpar;
			nextch();
			break;

		case '{':
		{
			while (ch != '}')
				nextch();
		}
			nextch();
			break;

		case '}':
			nextch();
			break;

		case '\'':
			sym = charc;
			j = 0;
			tmp = ch;
			nextch();
			if (ch == '\'')
			{
				error(75, positionnow);
				haveError = true;
				nextch();
				break;
			}
			while (ch != '\'' && j <= MAX_IDENT)
			{
				nextch();
				j++;
			}
			if (j > MAX_IDENT)
			{
				error(75, positionnow);
				haveError = true;
				nextch();
			}
			nextch();
			break;

		case '[':
			sym = lbracket;
			nextch();
			break;

		case ']':
			sym = rbracket;
			nextch();
			break;

		case '.':
			nextch();
			if (ch == '.')
			{
				sym = twopoints;
				nextch();
			}
			else
			{
				sym = point;
				theend = true;
				printLine();
				if (haveError)
					printErrors();
			}
			break;

		case ',':
			sym = comma;
			nextch();
			break;

		case '^':
			sym = arrow;
			nextch();
			break;

		case ';':
			sym = semicolon;
			nextch();
			break;
		default:
			error(6, positionnow);
			haveError = true;
			nextch();
			break;
		}
	}
}

void StartRead()
{
	char str[MAXLINE];
	//createError();
	F.getline(str, MAXLINE, '\n');
	positionnow.lineNumber = 0;
	positionnow.charNumber = -1;
	string s = curLine = str;
	LastInLine = s.length();
	while (!F.eof() || !theend)
		nextsym();
		

}

int main()
{
	setlocale(LC_ALL, "rus");
	tableOfAllError();
	Flist.open("f:\\fgmt\\list.txt");
	StartRead();
	Flist.close();
	cin.get();
	return 0;
}