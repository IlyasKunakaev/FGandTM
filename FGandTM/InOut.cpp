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

fstream F("pasCodesAndList\\3.txt", ios::in);

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
bool ifFlag = false;
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


void mult(unsigned *followers);
void addend(unsigned *followers);
void simpleexpression(unsigned *followers);
void expression(unsigned *followers);
void whilestatement(unsigned *followers);
void ifstatement(unsigned *followers);
void casestatement(unsigned *followers);
void statement(unsigned *followers);
void constant();
void type(unsigned *followers);

#pragma endregion

#pragma region ST_FOL
unsigned
*st_all,
*m_scolon,
*af_headfunproc,
*m_colon,
*m_fpar,
*m_comma,
*idstarters,
*begpart,
*rpar,
*st_constpart,
*st_typepart,
*st_varpart,
*st_statpart,
*st_typ,
*st_statement,
*st_startstatement,
*st_express,
*st_termfact;

unsigned
*blockfol,
*af_3const1,
*af_sameparam,
*af_iftrue,
*af_whilefor,
*af_constident,
*af_constident2,
*af_other,
*af_codesForAdd;

unsigned
*op_rel,
*op_add,
*op_mult;

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
	haveError = false;
	for (int k = 0; k <= ErrInx; k++)
	{
		ErrList[k].code = NULL;
		ErrList[k].tp.charNumber = NULL;
		ErrList[k].tp.lineNumber = NULL;
	}
	ErrInx = -1;
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
			printErrors();
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
	bool errNumber = false, errInt = false, haveErrorNumber = false;

	textposition errNum;
	errNum.charNumber = positionnow.charNumber;
	errNum.lineNumber = positionnow.lineNumber;
	while (ch >= '0' && ch <= '9' && !haveErrorNumber)
	{
		digit = ch - '0';
		if (nmb_int < MAXINT / 10 || (nmb_int == MAXINT / 10 && nmb_int <= MAXINT % 10))
			nmb_int = 10 * nmb_int + digit;
		else
		{
			nmb_int = 10 * nmb_int + digit;
			errNumber = true;
			haveError = true;
			haveErrorNumber = true;
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
				haveErrorNumber = false;
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
				haveErrorNumber = true;
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
				haveErrorNumber = true;
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
			haveErrorNumber = true;
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
				//if (haveError)
					//printErrors();
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
			if (stop)
				sym = endoffile;
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

bool belong(unsigned element, unsigned *set)
{
	unsigned word_number, bit_number;
	word_number = element / WORDLENGTH;
	bit_number = element % WORDLENGTH;
	return(set[word_number] & (1 << (WORDLENGTH - 1 - bit_number)));
}

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

void set_disjunct(unsigned set1[], unsigned set2[], unsigned set3[])
{
	for (unsigned i = 0; i < SET_SIZE; ++i) 
		set3[i] = set1[i] | set2[i];
}

void skipto(unsigned *set)
{
	while (!belong(sym, set) && !stop)
		nextsym();
}

void skipto2(unsigned *set1, unsigned *set2)
{
	while (!belong(sym, set1) && !belong(sym, set2) && !stop)
		nextsym();
}

//ПЕРЕМЕННАЯ
void variable(unsigned *followers)
{
	accept(ident);
	while (sym == lbracket || sym == point || sym == arrow)
		switch (sym)
		{
		case lbracket:   // НУЖНО ЛИ??
			accept(lbracket);
			expression(followers);
			while (sym == comma)
			{
				accept(comma);
				expression(followers);
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
void mult(unsigned *followers)
{
	//follower: endsy, elsesy, untilsy, ident, beginsy, ifsy, thensy, whilesy, forsy, casesy, 
	//			semicolon, later, laterequal, greater, greaterequal, equal, 
	//			latergreater, insy, eolint, point, orsy, endoffile
	//			plus, minus, leftpar, lbracket, notsy, ident,
	//			intc, floatc, charc, stringc, nilsy,
	//			star, slash, divsy, modsy, andsy
	//st_termfact: ident, leftpar, lbracket, notsy, intc, floatc, charc, stringc, nilsy, eolint

	//followers from if->expr->simpleexpr->addend->MULT->expression->simpleexpression->addend->mult:	
	//		twopoints, comma, rbracket, rightpar
	//		endsy, elsesy, untilsy, ident, beginsy, ifsy, thensy, whilesy, forsy, casesy, 
	//		semicolon, later, laterequal, greater, greaterequal, equal, 
	//		latergreater, insy, eolint, point, orsy, endoffile
	//		plus, minus, leftpar, lbracket, notsy, ident,
	//		intc, floatc, charc, stringc, nilsy,
	//		star, slash, divsy, modsy, andsy

	unsigned ptra[SET_SIZE];
	if (!belong(sym, st_termfact))
	{
		error(42, token);
		haveError = true;
		skipto2(st_termfact,m_scolon);
	}
	if (belong(sym, st_termfact))
	{
		switch (sym)
		{
		case ident: {
			accept(ident);
			if (sym == arrow)
				accept(arrow);
			break; }
		case intc:{
			if (nmb_int >= 0)
				accept(intc);
			else
			{
				error(50, token);
				haveError = true;
				nextsym();
			}
			break;	}
		case floatc: {
			if (nmb_float >= 0)
				accept(floatc);
			else
			{
				error(50, token);
				haveError = true;
				nextsym();
			}
			break;	}
		case nilsy: {
			accept(nilsy);
			break; }
		case notsy: {
			accept(notsy);
			mult(followers);
			break; }
		case leftpar: {
			accept(leftpar);
			set_disjunct(followers, af_3const1, ptra);
			//af_3const1:	twopoints, comma, rbracket, eolint
			
			//ptra: twopoints, comma, rbracket, rightpar
			//		endsy, elsesy, untilsy, ident, beginsy, ifsy, thensy, whilesy, forsy, casesy, 
			//		semicolon, later, laterequal, greater, greaterequal, equal, 
			//		latergreater, insy, eolint, point, orsy, endoffile
			//		plus, minus, leftpar, lbracket, notsy, ident,
			//		intc, floatc, charc, stringc, nilsy,
			//		star, slash, divsy, modsy, andsy
			set_disjunct(ptra, rpar, ptra);
			//rpar:			rightpar, eolint
			expression(ptra);
			if (sym == rightpar)
				accept(rightpar);
			else
				skipto(m_scolon);
			break; }
		default: {
			error(322, token);
			haveError = true;
			nextsym(); }
		}
	}
}

//СЛАГАЕМОЕ
void addend(unsigned *followers)
{
	//followers: endsy, elsesy, untilsy, ident, beginsy, ifsy, thensy, whilesy, forsy, casesy, 
	//			semicolon, later, laterequal, greater, greaterequal, equal, 
	//			latergreater, insy, eolint, eolint, point, plus, minus, orsy, endoffile
	//st_express: plus, minus, leftpar, lbracket, notsy, ident,
	//			  intc, floatc, charc, stringc, nilsy, eolint
	//st_termfact: ident, leftpar, lbracket, notsy, intc, floatc, charc, stringc, nilsy, eolint

	//followers from if->expr->simpleexpr->addend->MULT->expression->simpleexpression->addend:	
	//		twopoints, comma, rbracket, rightpar
	//		endsy, elsesy, untilsy, ident, beginsy, ifsy, thensy, whilesy, forsy, casesy, 
	//		semicolon, later, laterequal, greater, greaterequal, equal, 
	//		latergreater, insy, eolint, point, orsy, endoffile
	//		plus, minus, leftpar, lbracket, notsy, ident,
	//		intc, floatc, charc, stringc, nilsy,
	//		star, slash, divsy, modsy, andsy

	unsigned ptra[SET_SIZE];
	if (!belong(sym, st_termfact))
	{
		error(42, token);
		haveError = true;
		skipto2(st_termfact, followers);
	}
	if (belong(sym, st_termfact))
	{
		set_disjunct(followers, st_express, ptra);
		set_disjunct(ptra, op_mult, ptra);
		//ptra: endsy, elsesy, untilsy, ident, beginsy, ifsy, thensy, whilesy, forsy, casesy, 
		//			semicolon, later, laterequal, greater, greaterequal, equal, 
		//			latergreater, insy, eolint, point, orsy, endoffile
		//			plus, minus, leftpar, lbracket, notsy, ident,
		//			intc, floatc, charc, stringc, nilsy,
		//			star, slash, divsy, modsy, andsy

		mult(ptra);
		//op_mult: star, slash, divsy, modsy, andsy, eolint
		while (belong(sym, op_mult) && !F.eof())
		{
			nextsym();
			mult(ptra);
		}
		if (!belong(sym, followers))
		{
			error(6, token);
			haveError = true;
			if (!ifFlag)
				skipto(followers);
			//af_other: beginsy, endsy, ifsy, thensy, whilesy, forsy, 
			//			casesy, semicolon, endoffile, eolint
			else skipto(af_codesForAdd);
		}
	}
}

//ВЫРАЖЕНИЕ             
void expression(unsigned *followers)
{
	//followers: endsy, elsesy, untilsy, ident, beginsy,
	//			 ifsy, thensy, whilesy, forsy, casesy, semicolon, eolint, point, endoffile
	//
	//st_express: plus, minus, leftpar, lbracket, notsy, ident,
	//			  intc, floatc, charc, stringc, nilsy, eolint

	//followers from if->expr->simpleexpr->addend->MULT:	
	//		twopoints, comma, rbracket, rightpar
	//		endsy, elsesy, untilsy, ident, beginsy, ifsy, thensy, whilesy, forsy, casesy, 
	//		semicolon, later, laterequal, greater, greaterequal, equal, 
	//		latergreater, insy, eolint, point, orsy, endoffile
	//		plus, minus, leftpar, lbracket, notsy, ident,
	//		intc, floatc, charc, stringc, nilsy,
	//		star, slash, divsy, modsy, andsy

	unsigned ptra[SET_SIZE];
	if (!belong(sym, st_express))
	{
		error(42, token);
		haveError = true;
		skipto2(st_express, followers);
	}
	if (belong(sym, st_express))
	{
		//op_rel: later, laterequal, greater, greaterequal, equal, latergreater, insy, eolint
		set_disjunct(op_rel, followers, ptra);
		//ptra: endsy, elsesy, untilsy, ident, beginsy, ifsy, thensy, whilesy, forsy, casesy, 
		//		semicolon, later, laterequal, greater, greaterequal, equal, 
		//		latergreater, insy, eolint, point, endoffile
		simpleexpression(ptra);

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
			simpleexpression(followers);
		}
		if (!belong(sym, followers))
		{
			error(6, token);
			haveError = true;
			skipto(followers);
		}
	}
}

//АРИФМЕТИЧЕСКОЕ ВЫРАЖЕНИЕ
void simpleexpression(unsigned *followers)
{
	//followers: endsy, elsesy, untilsy, ident, beginsy, ifsy, thensy, whilesy, forsy, casesy, 
	//		     semicolon, later, laterequal, greater, greaterequal, equal, 
	//		     latergreater, insy, eolint, point, endoffile

	//st_express: plus, minus, leftpar, lbracket, notsy, ident,
	//			  intc, floatc, charc, stringc, nilsy, eolint

	//followers from if->expr->simpleexpr->addend->MULT->expression->simpleexpression:	
	//		twopoints, comma, rbracket, rightpar
	//		endsy, elsesy, untilsy, ident, beginsy, ifsy, thensy, whilesy, forsy, casesy, 
	//		semicolon, later, laterequal, greater, greaterequal, equal, 
	//		latergreater, insy, eolint, point, orsy, endoffile
	//		plus, minus, leftpar, lbracket, notsy, ident,
	//		intc, floatc, charc, stringc, nilsy,
	//		star, slash, divsy, modsy, andsy

	unsigned ptra[SET_SIZE];
	if (!belong(sym, st_express))
	{
		error(42, token);
		haveError = true;
		skipto2(st_express, followers);
	}

	if (belong(sym, st_express))
	{
		//op_add: plus, minus, orsy, eolint

		//ptra: endsy, elsesy, untilsy, ident, beginsy, ifsy, thensy, whilesy, forsy, casesy, 
		//		semicolon, later, laterequal, greater, greaterequal, equal, 
		//		latergreater, insy, eolint, eolint, point, plus, minus, orsy, endoffile
		set_disjunct(op_add, followers, ptra);
		if (belong(sym, op_add))
			nextsym();
		addend(ptra);
		while ((sym == plus || sym == minus || sym == orsy))
		{
			if (sym == plus)
				accept(plus);
			else if (sym == minus)
				accept(minus);
			else
				accept(orsy);
			addend(ptra);
		}
		if (!belong(sym, followers))
		{
			error(6, token);
			haveError = true;
			skipto(followers);
		}
	}
}

//ЦИКЛ С ПРЕДУСЛОВИЕМ (+++)
void whilestatement(unsigned *followers)
{
	accept(whilesy);
	if (belong(sym, st_express))
	{
		expression(followers);
		if (sym == dosy)
			nextsym();
		else {
			accept(dosy);
			skipto(st_startstatement);
		}
	}
	statement(followers);
}

//УСЛОВНЫЙ ОПЕРАТОР (+++)
void ifstatement(unsigned *followers)
{

	//followers: endsy, elsesy, untilsy, ident, beginsy, 
	//			 ifsy, thensy, whilesy, forsy, casesy, semicolon, eolint, point, endoffile
	accept(ifsy);
	ifFlag = true;
	//st_express: plus, minus, leftpar, lbracket, notsy, ident,
	//			  intc, floatc, charc, stringc, nilsy, eolint
	if (belong(sym, st_express))
	{
		expression(followers);
		ifFlag = false;
		if (sym == thensy)
			nextsym();
		else 
		{
			accept(thensy);
			//st_startstatement: beginsy, ifsy, whilesy, forsy, casesy, semicolon, eolint
			skipto(st_startstatement);
		}
	}
	statement(followers);
	if (sym == elsesy)
	{
		nextsym();
		statement(followers);
	}
	
}

//ОПЕРАТОР
void statement(unsigned *followers)
{
	//followers: endsy, elsesy, untilsy, ident, beginsy, ifsy, 
	//               whilesy, forsy, casesy, semicolon, eolint, point, endoffile

	unsigned ptra[SET_SIZE];
	//st_statement: endsy, elsesy, untilsy, ident, beginsy, ifsy, whilesy, 
	//              forsy, casesy, semicolon, eolint
	if (!belong(sym, st_statement))
	{
		error(113, token);
		haveError = true;
		skipto2(st_statement, followers);
	}

	if (belong(sym, st_statement))
	{
		set_disjunct(followers, st_statement, ptra);
		//ptra: endsy, elsesy, untilsy, ident, beginsy, 
		//      ifsy, whilesy, forsy, casesy, semicolon, eolint, point, endoffile

		switch (sym)
		{
		case whilesy:
			//af_whilefor: dosy, eolint
			set_disjunct(ptra, af_whilefor, ptra);
			whilestatement(ptra);
			break;
		case ifsy:
			//af_iftrue: thensy, eolint
			set_disjunct(ptra, af_iftrue, ptra);
			ifstatement(ptra);
			break;
		case casesy:
			casestatement(ptra);
			break;
		case beginsy:
			accept(beginsy);
			statement(ptra);
			ff = (sym == endsy);
			while (sym == semicolon && !ff)
			{
				accept(semicolon);
				if (sym == endsy)
				{
					ff = true;
				}
				else
					statement(followers); //followers: point, endoffile
				ff = (sym == endsy);
			}
			accept(endsy);
			break;
		case ident:
			accept(ident);
			switch (sym)
			{
			case arrow:
					accept(arrow);
					break;
			case assign: {
				accept(assign);
				expression(followers);
				break; } //followers: point, endoffile}
			default:
				{
					error(51, token);
					haveError = true;
					skipto(m_scolon);
				}
			}
			break;
		case endsy:		
			break;
		}
		if (!belong(sym, followers)) //followers: point, endoffile
		{
			error(6, token);
			haveError = true;
			skipto(followers);
		}
	}
}

//СОСТАВНОЙ ОПЕРАТОР (+++)
void statementpart(unsigned *followers)
{
	//followers: point, endoffile

	unsigned ptra[SET_SIZE];

	if (!belong(sym, st_statpart))//st_statpart: beginsy, eolint
	{
		error(17, token);
		skipto2(st_statpart, followers);
	}
	if (belong(sym, st_statpart))//st_statpart
	{
		accept(beginsy);
		set_disjunct(followers, st_statement, ptra); 
		// st_statement: endsy, elsesy, untilsy, ident, beginsy, ifsy, 
		//               whilesy, forsy, casesy, semicolon, eolint
		//		   ptra: +point,+endoffile
		while (sym != endsy && !stop) {
			statement(ptra);
			accept(semicolon);
		}
		accept(endsy);
		if (!belong(sym, followers))
		{
			error(61, token);
			haveError = true;
			skipto(followers);
		}
	}
}

//ЭЛЕМЕНТ СПИСКА ВАРИАНТА (+++)
void caseelement(unsigned *followers)
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
		statement(followers);
	}
}

//ОПЕРАТОР ВЫБОРА (+++)
void casestatement(unsigned *followers)
{
	accept(casesy);
	expression(followers);
	accept(ofsy);
	caseelement(followers);
	while (sym == semicolon)
	{
		accept(semicolon);
		caseelement(followers);
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
		haveError = true;
		nextsym();
	}
}

//ОПИСАНИЕ ПЕРЕМЕННЫХ (+++)
void vardeclaration(unsigned *followers)
{
	bool err1stIdent = false;
	//followers: varsy, functionsy, proceduresy, beginsy, point, endoffile, rightpar, semicolon, eolint
	if (!belong(sym, idstarters))
	{
		error(2, token);
		haveError = true;
		err1stIdent = true;
		skipto2(idstarters, followers);
	}
	if (sym == ident)
	{
		nextsym();
		if (!err1stIdent)
		{
			while (sym == comma)
			{
				nextsym();
				accept(ident);
			}
			accept(colon);
			type(followers);
			if (!belong(sym, followers))
			{
				error(14, token);
				haveError = true;
				skipto2(followers,idstarters);
			}
		}
	}
}

//РАЗДЕЛ ОПИСАНИЯ ПЕРЕМЕННЫХ (+++)
void varpart(unsigned *followers)
{
	//followers: varsy, functionsy, proceduresy, beginsy, point, endoffile
	unsigned ptra[SET_SIZE];
	//st_varpart: varsy,functionsy,proceduresy,beginsy
	if (!belong(sym, st_varpart))
	{
		error(18, token);
		haveError = true;
		skipto2(st_varpart, followers);
	}
	if (sym == varsy)
	{
		accept(varsy);
		//af_sameparam: rightpar, semicolon, eolint
		set_disjunct(af_sameparam, followers, ptra);
		do
		{
			vardeclaration(ptra);
			accept(semicolon);
			if (sym != ident)
				if (!belong(sym, followers))
				{
					error(2, token);
					haveError = true;
					skipto(ptra);
					nextsym();
					printErrors();
				}
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
void type(unsigned *followers)
{
	//follow: varsy, functionsy, proceduresy, beginsy, point, endoffile, rightpar, semicolon, eolint
	//st_typ: arrow, arraysy, filesy, setsy, recordsy, plus, minus, ident, leftpar, intc, charc, stringc, eolint
	if (!belong(sym, st_typ))
	{
		error(10, token);
		haveError = true;
		skipto2(st_typ, followers);
	}
	if (belong(sym, st_typ))
	{
		if (sym == ident || sym == leftpar ||  sym == arrow )
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
	}
}

//РАЗДЕЛ ТИПОВ (+++)
void typepart(unsigned *followers)
{
	//followers: varsy, functionsy, proceduresy, beginsy, point, endoffile
	unsigned ptra[SET_SIZE];
	//st_typepart: typesy,varsy,functionsy,proceduresy,beginsy
	if (!belong(sym, st_typepart))
	{
		error(1, token);
		haveError = true;
		skipto2(st_typepart, followers);
	}
	if (sym == typesy)
	{
		nextsym();
		//af_sameparam: rightpar, semicolon, eolint
		set_disjunct(af_sameparam, followers, ptra);
		while (!belong(sym, followers)) 
		{
			if (!belong(sym, idstarters))
			{
				error(2, token);
				haveError = true;
				skipto2(idstarters, ptra);
			}
			if (sym == ident) 
			{
				nextsym();
				accept(equal);
			}
			if (!belong(sym, idstarters))
			{
				skipto2(idstarters, ptra);
			}
			type(ptra);
			accept(semicolon);
		}
	}
}

//РАЗДЕЛ КОНСТАНТ (+++)
void constpart(unsigned *followers)
{
	unsigned ptra[SET_SIZE];
	//followers: constsy,typesy,varsy,functionsy,proceduresy,beginsy,point,endoffile
	if (!belong(sym, st_constpart))
	{
		error(18, token);
		haveError = true;
		skipto2(st_constpart, idstarters);
	}
	if (sym == constsy)
	{
		nextsym();
		set_disjunct(af_constident, followers, ptra);
		
		while (!belong(sym, followers)) 
		{
			if (!belong(sym, idstarters))
			{
				error(2, token);
				haveError = true;
				skipto2(idstarters, followers);
				printErrors();
			}
			
			if (sym == ident) 
			{
				nextsym();
				accept(equal);
				//af_constident: ident, equal, intc, floatc, semicolon
				if (!belong(sym, af_constident))
				{
					skipto2(idstarters, ptra);
				}
				//af_constident2: intc, floatc, semicolon
				if (belong(sym, af_constident2))
				{
					if (sym != semicolon)
						nextsym();
					accept(semicolon);
				}
			}
		}
	}
}

void block(unsigned *followers)  //followers: point, endoffile
{
	unsigned ptra[SET_SIZE];

	//begpart: labelsy,constsy,typesy,varsy,functionsy,proceduresy,beginsy
	if (!belong(sym, begpart))
	{
		error(18,token);
		haveError = true;
		skipto2(begpart, followers);
		printErrors();
	}
	if (belong(sym, begpart))
	{
		//st_typepart: typesy,varsy,functionsy,proceduresy,beginsy
		set_disjunct(st_typepart, followers, ptra);
		constpart(ptra); //исправить старт_фоллоу
		//st_varpart: varsy,functionsy,proceduresy,beginsy
		set_disjunct(st_varpart, followers, ptra);
		typepart(ptra);
		varpart(ptra);
		statementpart(followers);
		if (!belong (sym, followers))
		{
			if (!stop)
				error(6,token);
			//haveError = true;
			
			skipto(followers);
			printErrors();
		}
	}
}

void programme(unsigned *followers)
{
	//followers: point, endoffile, eolint
	//begpart: labelsy, constsy, typesy, varsy, functionsy, proceduresy, beginsy, eolint
	//st_all: programsy, eolint
	//af_headfunproc: varsy, constsy, labelsy, typesy, beginsy, point, semicolon, endoffile, eolint
	unsigned ptra[SET_SIZE];
	set_disjunct(st_all, af_headfunproc, ptra);
	bool prog = true;
	if (!belong(sym, ptra))
	{
		error(3, token);
		haveError = true;
		nextsym();
		if (sym != programsy)
		{
			prog = false;
			skipto2(ptra, idstarters);
		}
	}

	if (belong(sym, ptra) || belong(sym,idstarters))
	{  
		if (prog)
			accept(programsy);
		if (!belong(sym, idstarters))
		{
			haveError = true;
			set_disjunct(m_scolon, af_headfunproc, ptra);
			skipto(ptra);
		}
		accept(ident);
		if (!belong(sym, m_scolon))
		{
			error(14, token);
			haveError = true;
			skipto(af_headfunproc);
		}
		while(sym==semicolon)
			accept(semicolon);
		block(blockfol);
		accept(point);
	}
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
	programme(blockfol);
	Flist << "Компиляция окончена! Количество ошибок: " << SumErr - 1;
}


unsigned *Convert_To_Bits(unsigned *intstr)
{
	unsigned *set, *str, wordnum, bitnum;
	str = intstr; 
	set = (unsigned*)calloc(8, sizeof(unsigned));
	while (*str != eolint)	
	{
		wordnum = *str / WORDLENGTH;	
		bitnum = *str % WORDLENGTH;	
		set[wordnum] |= (1 << (WORDLENGTH - 1 - bitnum));
		str++;
	}
	return((unsigned*)set);
}


/**********************************************************************************************************************************/
/**********************************************************************************************************************************/
/*                                                         ГЛАВНАЯ ФУНКЦИЯ                                                        */
/**********************************************************************************************************************************/
/**********************************************************************************************************************************/

void st_follow()
{
#pragma region start_follow
	unsigned
		codes_st_all[] = { programsy, eolint },
		codes_idstart[] = { ident, eolint },
		codes_block[] = { labelsy, constsy, typesy, varsy, functionsy, proceduresy, beginsy, eolint },
		codes_rightpar[] = { rightpar, eolint },
		codes_constant[] = { plus, minus, charc, stringc, ident, intc, floatc, eolint },
		codes_typ[] = { arrow, arraysy, filesy, setsy, recordsy, plus, minus, ident, leftpar, intc, charc, stringc, eolint },
		codes_statement[] = { endsy, elsesy, untilsy, ident, beginsy, ifsy, whilesy, forsy, casesy, ofsy, semicolon, eolint },
		codes_express[] = { plus, minus, leftpar, lbracket, notsy, ident,intc, floatc, charc, stringc, nilsy, eolint },
		codes_termfact[] = { ident, leftpar, lbracket, notsy, intc, floatc, charc, stringc, nilsy, eolint };

	unsigned
		acodes_block[] = { point, endoffile, eolint },
		acodes_typ[] = { endsy, rightpar, semicolon, eolint },
		acodes_3const[] = { twopoints, comma, rbracket, eolint },
		acodes_iftrue[] = { thensy, eolint },
		acodes_wiwifor[] = { comma,dosy, eolint },
		acodes_constident[] = { ident, equal, intc, floatc, semicolon, eolint },
		acodes_constident2[] = { intc, floatc, semicolon, eolint },
		acodes_AddendSkip[] = { endsy, elsesy, untilsy, ident, beginsy, ifsy, thensy, whilesy, forsy, casesy,
					semicolon, later, laterequal, greater, greaterequal, equal,
					latergreater, insy, point, plus, minus, orsy, endoffile, eolint },
		acodes_other[] = { beginsy, endsy, ifsy, thensy, whilesy, forsy, casesy, semicolon, endoffile, eolint };
	
	unsigned
		codes_rel[] = { later, laterequal, greater, greaterequal, equal, latergreater, insy, eolint },
		codes_add[] = { plus, minus, orsy, eolint },
		codes_mult[] = { star, slash, divsy, modsy, andsy, eolint };

	unsigned
		colon_m[] = { colon, semicolon, eolint },
		af_head[] = { varsy, constsy, labelsy, typesy, beginsy, point, semicolon, endoffile, eolint },
		comma_m[] = { comma, eolint },
		semcol_m[] = { semicolon, eolint };

#pragma endregion


#pragma region

	st_all = Convert_To_Bits(codes_st_all);
	m_scolon = Convert_To_Bits(semcol_m);
	m_comma = Convert_To_Bits(comma_m),
	af_headfunproc = Convert_To_Bits(af_head),
	m_colon = Convert_To_Bits(colon_m);

	idstarters = Convert_To_Bits(codes_idstart);
	begpart = Convert_To_Bits(codes_block);
	st_constpart = Convert_To_Bits(codes_block + 1);
	st_typepart = Convert_To_Bits(codes_block + 2);
	st_varpart = Convert_To_Bits(codes_block + 3);
	st_statpart = Convert_To_Bits(codes_block + 6);
	st_typ = Convert_To_Bits(codes_typ);
	st_startstatement = Convert_To_Bits(codes_statement + 4);
	st_statement = Convert_To_Bits(codes_statement);

	st_express = Convert_To_Bits(codes_express);
	st_termfact = Convert_To_Bits(codes_termfact);

	blockfol = Convert_To_Bits(acodes_block);
	rpar = Convert_To_Bits(codes_rightpar);
	af_3const1 = Convert_To_Bits(acodes_3const);
	af_sameparam = Convert_To_Bits(acodes_typ + 1);
	af_other = Convert_To_Bits(acodes_other);
	af_codesForAdd = Convert_To_Bits(acodes_AddendSkip);

	af_iftrue = Convert_To_Bits(acodes_iftrue);
	af_whilefor = Convert_To_Bits(acodes_wiwifor + 1);

	af_constident = Convert_To_Bits(acodes_constident);
	af_constident2 = Convert_To_Bits(acodes_constident2);

	op_rel = Convert_To_Bits(codes_rel);
	op_add = Convert_To_Bits(codes_add);
	op_mult = Convert_To_Bits(codes_mult);
#pragma endregion

}

int main()
{
	setlocale(LC_ALL, "rus");
	tableOfAllError();
	Flist.open("pasCodesAndList\\list.txt");
	st_follow();
	StartRead();
	Flist.close();
	return 0;
}