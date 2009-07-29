/*
	PARSER_READABILITY.C
	--------------------
*/
#include "parser_readability.h"

/*
	ANT_PARSER_READABILITY::GET_NEXT_TOKEN()
	----------------------------------------
*/
ANT_string_pair *ANT_parser_readability::get_next_token(void)
{
unsigned char *start;

while (!isheadchar(*current) && !issentenceend(*current))
	current++;

if (issentenceend(*current))
	{
	start = current++;
	while (issentenceend(*current))
		current++;
	
	current_token.start = (char *)start;
	current_token.string_length = current - start;
	}
else if (ANT_isalpha(*current))
	{
	start = current++;
	while (ANT_isalpha(*current))
		current++;
	while (issentenceend(*current))
		current++;
	
	current_token.start = (char *)start;
	current_token.string_length = current - start;
	}
else if (ANT_isdigit(*current))
	{
	start = current++;
	while (ANT_isdigit(*current))
		current++;
	while (issentenceend(*current))
		current++;
	
	current_token.start = (char *)start;
	current_token.string_length = current - start;
	}
else if (*current == '\0')						// end of string
	return NULL;
else											// everything else (that starts with a '<')
	{
	start = current++; // keep the < so readability can know it's a tag
	if (isXMLnamestartchar(*current))
		{
		while (isXMLnamechar(*current))
			{
			*current = ANT_toupper(*current);
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
				while (!((*current == '-') && (*(current + 1) == '-') && (*(current + 2) == '>')))
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
