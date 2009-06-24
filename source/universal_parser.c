/*
	UNIVERSAL_PARSER.C
	------------------
	Created on: Jun 20, 2009
	Author: monfee
*/

#include "universal_parser.h"

/*
	ANT_UNIVERSAL_PARSER::ANT_UNIVERSAL_PARSER()
	--------------------------------------------
*/
ANT_universal_parser::ANT_universal_parser(ANT_encoding_factory::encoding what_encoding, bool by_char_or_word) : ANT_parser(), tokentype(by_char_or_word), lang(lang = ANT_encoding::UNKNOWN)
{
enc = ANT_encoding_factory::gen_encoding_scheme(what_encoding);
}

/*
	ANT_UNIVERSAL_PARSER::ANT_UNIVERSAL_PARSER()
	--------------------------------------------
*/
ANT_universal_parser::ANT_universal_parser() :	ANT_parser(), tokentype(true), lang(lang = ANT_encoding::UNKNOWN)
{
enc = ANT_encoding_factory::gen_encoding_scheme(ANT_encoding_factory::ASCII);
}

/*
	ANT_UNIVERSAL_PARSER::ANT_UNIVERSAL_PARSER()
	--------------------------------------------
*/
ANT_universal_parser::~ANT_universal_parser()
{
delete enc;
}

/*
	ANT_UNIVERSAL_PARSER::STORE_TOKEN()
	-----------------------------------
*/
void ANT_universal_parser::store_token(unsigned char *start)
{
if (tokentype || enc->is_english())
	{
/*
	TODO Chinese segmentation
*/
//		if (enc->lang() == ANT_encoding::CHINESE) {
//
//		}
	while (enc->is_valid_char(current) && lang == enc->lang())
		{
		lang = enc->lang();
		enc->tolower(current);
		move2nextchar();
		}
	}

current_token.start = (char *)start;
current_token.string_length = current - start;
}

/*
	ANT_UNIVERSAL_PARSER::GET_NEXT_TOKEN()
	--------------------------------------
*/
ANT_string_pair *ANT_universal_parser::get_next_token(void)
{
unsigned char *start;
int ret = NOTHEADCHAR;

while ((ret = isheadchar(current)) == NOTHEADCHAR)
	current++;

lang = enc->lang();

if (ret == ALPHACHAR)				// alphabetic-like strings for all languages
	{
	enc->tolower(current);
	start = current;
	move2nextchar();
	store_token(start);
	}
else if (ret == NUMBER)				// numbers
	{
	start = current++;
	while (ANT_isdigit(*current))
		current++;

	current_token.start = (char *)start;
	current_token.string_length = current - start;
	}
else if (ret == END)						// end of string
	return NULL;
else											// everything else (that starts with a '<')
	{
	start = ++current;
	if (isXMLnamestartchar(*current))
		{
		while (isXMLnamechar(*current))
			{
			enc->toupper(current);
			current++;
			}
		current_token.start = (char *)start;
		current_token.string_length = current - start;

		while (*current != '>')
			{
			if (*current == '"')
				while (*current != '"')
					current++;
			else if (*current == '\'')
				while (*current != '\'')
					current++;
			current++;
			}
		}
	else
		{
		if (*current == '/')					// </tag>	(XML Close tag)
			while (*current != '>')
				current++;
		else if (*current == '?')					// <? ... ?> (XML Processing Instructions)
			{
			current++; // current has to move to next character before we do the comparison again
			while (*current != '?' && *(current + 1) != '>')
				current++;
			}
		else if (*current == '!')				// <! ... > (
			{
			if (*(current + 1) == '-' && *(current + 2) == '-')		// <!-- /// --> (XML Comment)
				while (*current != '-' && *(current + 1) != '-' && *(current + 2) != '>')
					current++;
			else								// nasty XML stuff like <![CDATA[<greeting>Hello, world!</greeting>]]>
				while (*current != '>')
					current++;
			}
		return get_next_token();		// ditch and return the next character after where we are
		}
	}

return &current_token;
}


