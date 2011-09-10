/*
	SNIPPET_FACTORY.C
	-----------------
*/
#include <stdlib.h>
#include <stdio.h>
#include <new>
#include "snippet_factory.h"
#include "snippet_beginning.h"
#include "snippet_tag.h"
#include "snippet_tf.h"

/*
	ANT_SNIPPET_FACTORY::GET_SNIPPET_MAKER()
	----------------------------------------
*/
ANT_snippet *ANT_snippet_factory::get_snippet_maker(long type, long length_of_snippet, char *tag, long length_of_longest_document)
{
ANT_snippet *answer = NULL;

switch (type)
	{
	case SNIPPET_TITLE:
		answer = new (std::nothrow) ANT_snippet_tag(length_of_snippet, tag);
		break;
	case SNIPPET_BEGINNING:
		answer = new (std::nothrow) ANT_snippet_beginning(length_of_snippet, tag);
		break;
	case SNIPPET_TF:
		answer = new (std::nothrow) ANT_snippet_tf(length_of_snippet, length_of_longest_document);
		break;
	default:
		exit(printf("Unknown snippet algorithm, so cannot create a snippet generator\n"));
	}

return answer;
}