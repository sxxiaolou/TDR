#include <stdio.h>

#include "hotpot/hp_platform.h"
#include "hotpot/hp_error.h"
#include "json_y.h"
#include "json_parser.h"

#define YY_USER_ACTION																	\
	yylloc->first_line = yylloc->last_line = yylineno;									\
	yylloc->first_column = yycolumn;													\
	yylloc->last_column = yycolumn + yyleng - 1;										\
	yycolumn += yyleng;																	
		

static void hotscript_reserved_keyword(char* keyword)
{
	//yyerror(&yylloc, hotscript_parameter, "Cannot use reserved language keyword: \"%s\"\n", keyword);
}



static void hexToDigit(unsigned int * val, const unsigned char * hex)
{
    unsigned int i;
    for (i=0;i<4;i++) {
        unsigned char c = hex[i];
        if (c >= 'A') c = (c & ~0x20) - 7;
        c -= '0';
        *val = (*val << 4) | c;
    }
}

static size_t Utf32toUtf8(unsigned int codepoint, char * utf8Buf) 
{
    if (codepoint < 0x80) {
        utf8Buf[0] = (char) codepoint;
        utf8Buf[1] = 0;
        return 1;
    } else if (codepoint < 0x0800) {
        utf8Buf[0] = (char) ((codepoint >> 6) | 0xC0);
        utf8Buf[1] = (char) ((codepoint & 0x3F) | 0x80);
        utf8Buf[2] = 0;
        return 2;
    } else if (codepoint < 0x10000) {
        utf8Buf[0] = (char) ((codepoint >> 12) | 0xE0);
        utf8Buf[1] = (char) (((codepoint >> 6) & 0x3F) | 0x80);
        utf8Buf[2] = (char) ((codepoint & 0x3F) | 0x80);
        utf8Buf[3] = 0;
        return 3;
    } else if (codepoint < 0x200000) {
        utf8Buf[0] =(char)((codepoint >> 18) | 0xF0);
        utf8Buf[1] =(char)(((codepoint >> 12) & 0x3F) | 0x80);
        utf8Buf[2] =(char)(((codepoint >> 6) & 0x3F) | 0x80);
        utf8Buf[3] =(char)((codepoint & 0x3F) | 0x80);
        utf8Buf[4] = 0;
        return 4;
    } else {
        utf8Buf[0] = '?';
        utf8Buf[1] = 0;
        //error?
        return 2;
    }
}

#define YYCTYPE   unsigned char
#define YYFILL(n) 
#define YYCURSOR  jp->yy_cursor
#define YYLIMIT   jp->yy_limit
#define yytext jp->yy_text
#define yyleng jp->yy_leng

#define YYGETCONDITION()  jp->yy_state
#define YYSETCONDITION(s) jp->yy_state = s

#define STATE(name)  yyc##name
#define BEGIN(state) YYSETCONDITION(STATE(state))
#define YYSTATE      YYGETCONDITION()

hpint32 json_lex_scan(JSON_PARSER *jp, YYSTYPE * yylval)
{
restart:
	if(YYCURSOR >= YYLIMIT)
	{
		return 0;
	}
	//记录开始匹配时候的位置
	jp->yy_text = YYCURSOR;
/*!re2c
re2c:yyfill:check = 0;


newline			("\r"|"\n"|"\r\n")
identifier		[a-zA-Z_][a-zA-Z_0-9]*
whitespace		[ \t\r\n]*
symbols			[,:\[\]\{\}]
string_begin	['\"']

<!*> := yyleng = YYCURSOR - jp->yy_text;

<INITIAL>{symbols} {
	return *jp->yy_text;
}

<INITIAL>{identifier}	{
	yylval->type = E_HP_STRING;
	yylval->val.str.ptr = jp->yy_text;
	yylval->val.str.len = yyleng;
	return tok_identifier;
}
	
<INITIAL>{string_begin} {
  char mark = *jp->yy_text;
  //YYCURSOR只比yy_text大1哦~

  //最大字符串的限制
  yylval->val.str.ptr = malloc(1024);
  yylval->type = E_HP_STRING;
  yylval->val.str.len = 0;  
  while (YYCURSOR < YYLIMIT)
  {	
	unsigned char ch = *YYCURSOR;
	++YYCURSOR;
    switch (ch)
    {
      case '\\':
		++YYCURSOR;
        switch (*YYCURSOR)
        {
          case '"':
			yylval->val.str.ptr[(yylval->val.str.len)++] = '"';
            continue;
          case '\\':
			yylval->val.str.ptr[(yylval->val.str.len)++] = '\\';
            continue;
          case '/':
			yylval->val.str.ptr[(yylval->val.str.len)++] = '/';
            continue;
          case 'b':
			yylval->val.str.ptr[(yylval->val.str.len)++] = '\b';
            continue;
          case 'f':
			yylval->val.str.ptr[(yylval->val.str.len)++] = '\f';
            continue;
          case 'n':
			yylval->val.str.ptr[(yylval->val.str.len)++] = '\n';
            continue;
          case 'r':
			yylval->val.str.ptr[(yylval->val.str.len)++] = '\r';
            continue;
          case 't':
			yylval->val.str.ptr[(yylval->val.str.len)++] = '\t';
            continue;
		  case 'u':
		  {
			char hex_number[5];
			unsigned int i;
			unsigned int d;
			for(i = 0;i < 4; ++i)
			{
				++YYCURSOR;
				if (YYCURSOR >= YYLIMIT) {
					return 0;
				}
				hex_number[i] = *YYCURSOR;
			}
			//这里要进行错误处理
			hexToDigit(&d, hex_number);
			yylval->val.str.len += Utf32toUtf8(d, yylval->val.str.ptr + (yylval->val.str.len));
			
			continue;
		  }
          default:
            //yyerror("bad escape character\n");
            return -1;
        }
        break;
      default:
        if (ch == mark)
        {
			yylval->val.str.ptr[yylval->val.str.len] = 0;
			yylval->type = E_HP_STRING;
			return tok_string;
        }
        else
        {
          yylval->val.str.ptr[(yylval->val.str.len)++] = ch;
        }
    }
  }
}

<INITIAL>. | '\n' {
	goto restart;
}

*/
return 0;
}
