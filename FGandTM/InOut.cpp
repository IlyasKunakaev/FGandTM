#define _CRT_SECURE_NO_WARNINGS
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
const int MAXFLOAT = 170000000;
const short MAXLINE = 250;				//размер буфера ввода-вывода
const short ERRMAX = 10;				//макс. кол-во сохраняемых ошибок для текущей строки
short ErrInx = -1;						//кол-во найденных в текущей строке ошибок
bool ErrorOverFlow, haveError = false;		//флаги на переполнение ошибок и наличие ошибок в строке		
ofstream Flist;
fstream F("E:\\FGMT\\PascalCode\\progPascal6.txt", ios::in);
ofstream ListOfCode;
unsigned SumErr = 1, lineOfCode = 1, sym, lname;
map <int, string> AllErrors;
map <int, string> ::iterator iter = AllErrors.begin();
bool stop = false, theend = false, AllOk = true, isComment = false, haveComma = false, theendComment = false;
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
	A.open("E:\\FGMT\\PascalCode\\Err.txt");
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

void nextchComment()
{
	if (positionnow.charNumber == LastInLine)
	{
		printLine();
		if (!F.eof())
		{
			LastInLine = readNextLine();
			haveError = false;
			positionnow.lineNumber++;
			positionnow.charNumber = 0;
		}
		else
		{
			theendComment = true;
			stop = true;
		}
			
	}
	else
		positionnow.charNumber++;
	ch = curLine[positionnow.charNumber];
}

void nextch()
{
	if (positionnow.charNumber == LastInLine)
	{
		printLine();
		if (!isComment)
			ListOfCode << endl;
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
	}
	else
		positionnow.charNumber++;
	ch = curLine[positionnow.charNumber];
}

void processingNumber()
{
	int digit;
	nmb_int = 0;
	nmb_float = 0;
	bool errNumber = false, errInt = false;
	textposition errNum;
	errNum.charNumber = positionnow.charNumber;
	errNum.lineNumber = positionnow.lineNumber;
	while (ch >= '0' && ch <= '9' && !haveError)
	{
		digit = ch - '0';
		if (nmb_int < MAXINT / 10 || (nmb_int == MAXINT / 10 && nmb_int <= MAXINT % 10))
			nmb_int = 10 * nmb_int + digit;
		else
		{
			nmb_int = 10 * nmb_int + digit;
			errNumber = true;
			haveError = true;
			errInt = true;
			nmb_float = (float)nmb_int;
		}
		nextch();
	}
	
	if (ch >= '0' && ch <= '9')
	{
		errInt = false;
		while (ch >= '0' && ch <= '9')
		{
			digit = ch - '0';
			if (nmb_float < MAXFLOAT / 10 || (nmb_float == MAXFLOAT / 10 && nmb_float <= MAXFLOAT % 10))
			{
				nmb_float = 10 * nmb_float + digit;
				errNumber = false;
				haveError = false;
			}
			else
			{
				while (ch >= '0' && ch <= '9')
				{
					nextch();
					haveComma = true;
				}
				if (ch == '.')
				{		
					sym = floatc;
					error(207, errNum);
					nextch();
				}
				else 
				{
					sym = intc;
					error(203, errNum);
					nextch();
				}
				nmb_float = 0;
				nmb_int = 0;
				errNumber = true;
				haveError = true;
				while (ch >= '0' && ch <= '9')
					nextch();
			}
			if (!haveComma)
				nextch();
		}
		if (ch == '.')
		{
			nextch();
			if (ch < '0' || ch > '9')
			{
				errNumber = true;
				haveError = true;
				error(201, errNum);
			}
			while (ch >= '0' && ch <= '9')
				nextch();
			sym = floatc;
		}
	}
	else if (ch == '.')
	{
		nextch();
		if (ch < '0' || ch > '9')
		{
			errNumber = true;
			haveError = true;
			error(201, errNum);
		}
		while (ch >= '0' && ch <= '9')
			nextch();
		sym = floatc;
	}
	else
	{
		sym = intc;
		if (errInt)
			error(203, errNum);
	}
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
		for (unsigned int k = 0; k < lname; k++)
			name[k] = NULL;
		break;
	case (2):
		processingNumber();
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
			char tmp;
			nextch();
			tmp = ch;
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
				isComment = true;
				AllOk = false;
				while ((unsigned)positionnow.charNumber < LastInLine)
				{
					nextch();
				}
				nextch();
				ListOfCode << endl;
				isComment = false;
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
				AllOk = false;
				sym = lcomment;
				isComment = true;
				do
				{
					nextchComment();
					if (ch == '*')
					{
						tmp = ch;
						nextchComment();
						if (ch == ')' && tmp == '*')
						{
							while ((unsigned)positionnow.charNumber < LastInLine)
								nextchComment();
							nextchComment();
							break;
						}
					}
				} while (ch != '*' && !theendComment);
				if (F.eof())
				{
					error(86, positionnow);
					theend = true;
					printErrors();
				}
					
				isComment = false;
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
			AllOk = false;
			isComment = true;
			do
			{
				nextchComment();
			} while (ch != '}' && !theendComment);
			if (F.eof())
			{
				error(86, positionnow);
				theend = true;
				printErrors();
			}
			else 
			{
				while ((unsigned)positionnow.charNumber < LastInLine)
					nextchComment();
				nextchComment();
			}
			isComment = false;
		}
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
		case '\0':
			while (ch == '\0')
				nextch();
			break;
		default:
			error(6, positionnow);
			AllOk = false;
			haveError = true;
			nextch();
			break;
		}
	}
}

void StartRead()
{
	char str[MAXLINE];
	ListOfCode.open("E:\\fgmt\\ListOfCode.txt");
	F.getline(str, MAXLINE, '\n');
	positionnow.lineNumber = 0;
	positionnow.charNumber = -1;
	string s = curLine = str;
	LastInLine = s.length();
	while (!F.eof() || !theend)
	{
		AllOk = true;
		nextsym();
		if (AllOk)
			ListOfCode << sym << " ";
	}
	ListOfCode.close();
	Flist << "Компиляция окончена! Количество ошибок: " << SumErr-1;
}

int main()
{
	setlocale(LC_ALL, "rus");
	tableOfAllError();
	Flist.open("E:\\fgmt\\list.txt");
	StartRead();
	Flist.close();
	//cin.get();
	return 0;
}