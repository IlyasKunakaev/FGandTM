#include "DECKEY.h"
#include "DECW.h"
struct key
{
	unsigned codekey;
	char     namekey[15];
};
static struct key  keywords[64] =
/* таблица ключевых слов и их кодов 		*/
{
{ ident," "},
{ dosy, "do"},
{ ifsy, "if"},
{ insy, "in"},
{ ofsy, "of"},
{ orsy, "or"},
{ tosy, "to"},
{ ident,"  "},
{ andsy,"and"},
{ divsy,"div"},
{ endsy,"end"},
{ forsy,"for"},
{ modsy,"mod"},
{ nilsy,"nil"},
{ notsy,"not"},
{ setsy,"set"},
{ varsy,"var"},
{ ident,"   "},
{ casesy,"case"},
{ elsesy,"else"},
{ filesy,"file"},
{ gotosy,"goto"},
{ onlysy,"only"},
{ thensy,"then"},
{ typesy,"type"},
{ unitsy,"unit"},
{ usessy,"uses"},
{ withsy,"with"},
{ ident, "    "},
{ arraysy,"array"},
{ beginsy,"begin"},
{ constsy,"const"},
{ labelsy,"label"},
{ untilsy,"until"},
{ whilesy,"while"},
{ ident,  "     "},
{ exportsy,"export"},
{ importsy,"import"},
{ downtosy,"downto"},
{ modulesy,"module"},
{ packedsy,"packed"},
{ recordsy,"record"},
{ repeatsy,"repeat"},
{ vectorsy,"vector"},
{ stringsy,"string"},
{ ident,   "      "},
{ forwardsy,"forward"},
{ processsy,"process"},
{ programsy,"program"},
{ segmentsy,"segment"},
{ ident,    "       "},
{ functionsy,"function"},
{ separatesy,"separate"},
{ ident,     "        "},
{ interfacesy,"interface"},
{ proceduresy,"procedure"},
{ qualifiedsy,"qualified"},
{ ident,      "         "},
{ ident,      "          "},
{ ident,      "           "},
{ ident,      "            "},
{ ident,      "             "},
{ implementationsy,"implementation"},
{ ident,        "              "}
};

/* массив номеров строк с кодом ident: 	*/
unsigned short last[15] =
{
-1,0,7,17,28,35,45,50,53,57,58,59,60,61,63
};