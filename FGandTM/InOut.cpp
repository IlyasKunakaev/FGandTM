#pragma region includes
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
#pragma endregion

fstream F("pasCodesAndList\\progPascal6.txt", ios::in);

#pragma region vars

const short MAX_IDENT = 15;
const short MAXINT = 32767;
const int MAXFLOAT = 170000000;
const short MAXLINE = 250;				//размер буфера ввода-вывода
const short ERRMAX = 10;				//макс. кол-во сохраняемых ошибок для текущей строки
short ErrInx = -1;						//кол-во найденных в текущей строке ошибок
bool ErrorOverFlow, haveError = false;		//флаги на переполнение ошибок и наличие ошибок в строке		
ofstream Flist;

unsigned SumErr = 1, lineOfCode = 1, sym, lname;
map <int, string> AllErrors;
map <int, string> ::iterator iter = AllErrors.begin();
bool stop = false, theend = false, AllOk = true, isComment = false, haveComma = false, theendComment = false, ff = false;
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
textposition token, token2;
unsigned LastInLine;
char* curLine;
void nextsym();


void mult();
void addend();
void simpleexpression();
void expression();
void cyclestatement();
void whilestatement();
void forstatement();
void repeatstatement();
void ifstatement();
void casestatement();
void statement();
void constant();
void type();

#pragma endregion

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
	A.open("Err.txt");
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

#pragma region leks

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
		
		if (!F.eof())
		{
			printLine();
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
	while ((ch == '\0' || ch == '\t' || ch == ' ') && !stop)
		nextch();
	while (ch == ' ')
		nextch();
	token.lineNumber = positionnow.lineNumber;
	token.charNumber = positionnow.charNumber;
	if ((ch < 'А' || ch > 'Я') && (ch < 'а' || ch > 'я'))
		if (isalpha(ch))	//прочитали букву
			p = 1;
	else if (isdigit(ch))		//прочитали цифру
		p = 2;
	else				//прочитали спец. символ
		p = 3;
	int i, j;
	switch (p)
	{
	case (1): {
		lname = 0;
		while (((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_') && lname < MAX_IDENT)
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
		break; }
	case (2): {
		processingNumber();
		break; }
	case (3): {
		switch (ch)
		{
		case '<':  {
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
			break; }
		case '>':  {
			nextch();
			if (ch == '=')
			{
				sym = greaterequal;
				nextch();
			}
			else
				sym = greater;
			break; }
		case ':':  {
			nextch();
			if (ch == '=')
			{
				sym = assign;
				nextch();
			}
			else
				sym = colon;
			break; }
		case '+':  {
			sym = plus;
			nextch();
			break; }
		case '-':  {
			sym = minus;
			char tmp;
			nextch();
			tmp = ch;
			break; }
		case '*':  {
			nextch();
			if (ch == ')')
			{
				sym = rcomment;
				nextch();
			}
			else
				sym = star;
			break; }
		case '/':  {
			nextch();
			if (ch == '/')
			{
				sym = onecomment;
				while ((unsigned)positionnow.charNumber < LastInLine)
					nextch();
				nextch();
				while (sym == onecomment)
					nextsym();
			}
			else sym = slash;
			break; }
		case '=':  {
			sym = equal;
			nextch();
			break; }
		case '(':  {
			nextch();
			if (ch == '*')
			{
				sym = lcomment;
				do
				{
					nextchComment();
					if (ch == '*')
					{
						char tmp = ch;
						nextchComment();
						if (ch == ')' && tmp == '*')
						{
							/*while ((unsigned)positionnow.charNumber < LastInLine)
								nextchComment();*/
							nextchComment();
							while (sym == lcomment)
								nextsym();
							break;
						}
					}
				} while (ch != '*' && !theendComment);
				if (F.eof())
				{
					error(86, positionnow);
					theend = true;
					printLine();
					printErrors();
				}
			}
			else
				sym = leftpar;
			break; }
		case ')':  {
			sym = rightpar;
			nextch();
			break; }
		case '{':  {
			sym = flpar;
				do
				{
					nextchComment();
				} while (ch != '}' && !theendComment);
				if (F.eof())
				{
					error(86, positionnow);
					theend = true;
					printLine();
					printErrors();
				}
				else
				{
					while (sym == flpar)
						nextsym();
				}
			break; }
		case '}':  {
			nextch();
			break; }
		case '\'': {
			sym = charc;
			j = 0;
			char tmp = ch;
			nextch();
			if (ch == '\'')
			{
				error(75, positionnow);
				haveError = true;
				nextch();
				break;
			}
			while (ch != '\'' && !theendComment)
			{
				nextchComment();
				j++;
			}
			if (F.eof())
			{
				error(76, positionnow);
				haveError = true;
				theend = true;
				printErrors();
			}
			else
				nextch();
			break; }
		case '[':  {
			sym = lbracket;
			nextch();
			break; }
		case ']':  {
			sym = rbracket;
			nextch();
			break; }
		case '.':  {
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
				//printLine();
				if (haveError)
					printErrors();
			}
			break; }
		case ',':  {
			sym = comma;
			nextch();
			break; }
		case '^':  {
			sym = arrow;
			nextch();
			break; }
		case ';':  {
			sym = semicolon;
			nextch();
			break; }
		case '\0': {
			while (ch == '\0' && !stop)
				nextch();
			break; }
		default:   {
			error(6, positionnow);
			AllOk = false;
			haveError = true;
			nextch();
			break; }
		} }
	}
}

#pragma endregion

#pragma region Syntax

/*
--------------------ОБЩАЯ МИНИМАЛЬАНЯ ЧАСТЬ----------------------- 

Основные разделы программы: раздел описания переменных, 
							раздел операторов (Условный оператор (if). Оператор цикла с предусловием (while). Оператор выбора (case). 
							Оператор присваивания и составной оператор.) 

Переменные стандартных типов (Boolean, integer, real, char). 

Числовые константы. 

Арифметическое выражение (в выражении допустимы только константы, переменные, операции +, –, *, / и скобки + логические операции(сравнение и т.д.)) 
Выражение (полностью, включая арифметические, логические операции, сравнения и т.д.,
			но только над константами и простыми переменными и указателями (не индексированные, не поля записи)).

--------------------ОБЩАЯ ДОПОЛНИТЕЛЬНАЯ ЧАСТЬ--------------------

Раздел описания типов.
Описание типов (скалярный, перечислимый, ссылочный)
		

----------------------ИНДИВИДУАЛЬНАЯ ЧАСТЬ------------------------ 
12.	Раздел описания констант. 
	
*/

void accept(unsigned symbolexpected)
{
	if (sym == symbolexpected)
	{
		nextsym();
		if (sym == onecomment)
			nextsym();
	}
	else
	{
		error(symbolexpected, token);
		haveError = true;
	}
}

//ПЕРЕМЕННАЯ
void variable()
{
	accept(ident);
	while (sym == lbracket || sym == point || sym == arrow)
		switch (sym)
		{
		case lbracket:   // НУЖНО ЛИ??
			accept(lbracket);
			expression();
			while (sym == comma)
			{
				accept(comma);
				expression();
			}
			accept(rbracket);
			break;
		case point:		// НУЖНО ЛИ??
			accept(point);
			accept(ident);
			break;
		case arrow:
			accept(arrow);
			break;
		}
}

//МНОЖИТЕЛЬ
void mult()
{
	switch (sym)
	{
	case ident:
		accept(ident);
		if (sym == arrow)
			accept(arrow);
		break;
	case intc:
		if (nmb_int >= 0)
			accept(intc);
		else
		{
			error(50, token);
			nextsym();
		}
		break;
	case floatc:
		if (nmb_float >= 0)
			accept(floatc);
		else
		{
			error(50, token);
			nextsym();
		}
		break;
	case nilsy:
		accept(nilsy);
		break;
	case notsy:
		accept(notsy);
		mult();
		break;
	case leftpar:
		accept(leftpar);
		expression();
		accept(rightpar);
		break;
	default:
		error(322, token);
		nextsym();
	}
}

//СЛАГАЕМОЕ
void addend()
{
	mult();
	if (sym == star || sym == slash || sym == divsy || sym == modsy || sym == andsy)
		while (sym == star || sym == slash || sym == divsy || sym == modsy || sym == andsy)
		{
			switch (sym)
			{
			case star:
				accept(star);
				break;
			case slash:
				accept(slash);
				break;
			case divsy:
				accept(divsy);
				break;
			case modsy:
				accept(modsy);
				break;
			case andsy:
				accept(andsy);
				break;
			}
			mult();
		}
	else if (sym != semicolon && sym != plus && sym != minus && sym != orsy && 
		sym != later && sym != laterequal && sym != latergreater && sym != greater 
		&& sym != greaterequal)
		error(322, token); //  ------ ВЫБРАТЬ НОМЕР ОШИБКИ ------
}

//ВЫРАЖЕНИЕ             
void expression()
{
	simpleexpression();
	if (sym == equal || sym == latergreater || sym == later || sym == laterequal || sym == greaterequal ||
		sym == greater)
	{
		switch (sym)
		{
		case equal:
			accept(equal);
			break;
		case latergreater:
			accept(latergreater);
			break;
		case later:
			accept(later);
			break;
		case laterequal:
			accept(laterequal);
			break;
		case greaterequal:
			accept(greaterequal);
			break;
		case greater:
			accept(greater);
			break;
		}
		simpleexpression();
	}
}

//АРИФМЕТИЧЕСКОЕ ВЫРАЖЕНИЕ
void simpleexpression()
{
	if (sym == minus || sym == plus)
	{
		if (sym == minus)
			accept(minus);
		else
			accept(plus);
	}
	addend();
	while (sym == plus || sym == minus || sym == orsy)
	{
		if (sym == plus)
			accept(plus);
		else if (sym == minus)
			accept(minus);
		else
			accept(orsy);
		/*token2.charNumber = positionnow.charNumber;
		token2.lineNumber = positionnow.lineNumber;*/
		
		addend();

		/*nextsym();
		if (sym != plus || sym != minus || sym != star || sym != modsy || sym != divsy || sym != slash || sym != semicolon)
		{
			error(14, token2);
			nextsym();
		}*/
	}
}

//ЦИКЛ С ПРЕДУСЛОВИЕМ (+++)
void whilestatement()
{
	accept(whilesy);
	expression();
	accept(dosy);
	statement();
}

//УСЛОВНЫЙ ОПЕРАТОР (+++)
void ifstatement()
{
	accept(ifsy);
	expression();
	accept(thensy);
	statement();
	if (sym == elsesy)
	{
		accept(elsesy);
		statement();
	}
}

//ОПЕРАТОР
void statement()
{
	if (sym != endsy)
	{
		if (sym == ident)
		{
			accept(ident);
			if (sym == arrow)
				accept(arrow);
			if (sym == assign)
			{
				accept(assign);
				expression();
			}
			else
			{
				error(51, token);
				nextsym();
			}
		}
		else
		{
			switch (sym)
			{
			case beginsy:{
				accept(beginsy);
				statement();
				ff = (sym == endsy);
				while (sym == semicolon && !ff)
				{
					accept(semicolon);
					if (sym == endsy)
					{
						ff = true;
					}
					else
						statement();
					ff = (sym == endsy);
				}
				accept(endsy);
				break; }
			case ifsy: {
				ifstatement();
				break; }
			case casesy: {
				casestatement();
				break; }
			case whilesy: {
				whilestatement();
				break; }
			default: {
				error(322, token);
				break; }
			}
		}
	}
	else
		nextsym();
}

//СОСТАВНОЙ ОПЕРАТОР (+++)
void statementpart() 
{
	accept(beginsy);
	statement();
	while (sym == semicolon && stop == false)
	{
		accept(semicolon);
		if (sym != endsy)
			statement();
	}
	accept(endsy);
}

//ЭЛЕМЕНТ СПИСКА ВАРИАНТА (+++)
void caseelement()
{
	if (sym != endsy)
	{
		constant();
		while (sym == comma)
		{
			accept(comma);
			constant();
		}
		accept(colon);
		statement();
	}
}

//ОПЕРАТОР ВЫБОРА (+++)
void casestatement()
{
	accept(casesy);
	expression();
	accept(ofsy);
	caseelement();
	while (sym == semicolon)
	{
		accept(semicolon);
		caseelement();
	}
	accept(endsy);
}

//КОНСТАНТА 
void constant()
{
	if (sym == intc || sym == charc || sym == floatc)
	{
		switch (sym)
		{
		case intc:
			accept(intc);
			break;
		case charc:
			accept(charc);
			break;
		case floatc:
			accept(floatc);
			break;
		}
	}
	else
	{
		error(50, token); 
		nextsym();
	}
}

//ОПИСАНИЕ ПЕРЕМЕННЫХ (+++)
void vardeclaration()
{
	accept(ident);
	while (sym == comma)
	{
		accept(comma);
		accept(ident);
	}
	accept(colon);
	type();
}

//РАЗДЕЛ ОПИСАНИЯ ПЕРЕМЕННЫХ (+++)
void varpart()
{
	if (sym == varsy)
	{
		accept(varsy);
		do
		{
			vardeclaration();
			accept(semicolon);
		} while (sym == ident);
	}
}

//ПЕРЕЧИСЛИМЫЙ ТИП (+++)
void numtype()
{
	accept(leftpar);
	accept(ident);
	while (sym == comma)
	{
		accept(comma);
		accept(ident);
	}
	accept(rightpar);
}

//ОПИСАНИЕ ТИПОВ (+-)
void type()
{
	
	switch (sym)
	{
	case (ident):	//идентификатор, имя (скалярный тип)
		accept(ident);
		break;
	case (leftpar):	//перечислимый тип
		numtype();
		break;
	case (arrow): //ссылочный тип
		accept(arrow);
		accept(ident);
		break;
	}
}

//РАЗДЕЛ ТИПОВ (+++)
void typepart()
{
	if (sym == typesy)
	{
		accept(typesy);
		while (sym == ident)
		{
			accept(ident);
			accept(equal);
			type();
			accept(semicolon);
		}
	}
}

//РАЗДЕЛ КОНСТАНТ (+++)
void constpart()
{
	if (sym == constsy)
	{
		accept(constsy);
		while (sym == ident)
		{
			accept(ident);
			accept(equal);
			constant();
			accept(semicolon);
		}
	}
}

void block()
{
	constpart();		//раздел констант (инд)
	typepart();			//раздел типов (доп)
	varpart();			//раздел переменных (мин)
	statementpart();	//раздел операторов (мин)
}

void programme()
{
	if (sym == programsy)
	{
		accept(programsy);
		accept(ident);
		accept(semicolon);
	}
	block();
	accept(point);
}

#pragma endregion


void StartRead()
{
	char str[MAXLINE];
	F.getline(str, MAXLINE, '\n');
	positionnow.lineNumber = 0;
	positionnow.charNumber = -1;
	string s = curLine = str;
	LastInLine = s.length();
	nextsym();
	programme();
	Flist << "Компиляция окончена! Количество ошибок: " << SumErr - 1;
}


int main()
{
	setlocale(LC_ALL, "rus");
	tableOfAllError();
	Flist.open("pasCodesAndList\\list.txt");
	StartRead();
	Flist.close();
	return 0;
}