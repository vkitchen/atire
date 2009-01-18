/*
	INEX_ASSESSMENT.C
	-----------------
*/
#include <string.h>
#include "INEX_assessment.h"
#include "relevant_document.h"
#include "memory.h"
#include "disk.h"

/*
	ANT_INEX_ASSESSMENT::CMP()
	--------------------------
*/
int ANT_INEX_assessment::cmp(const void *a, const void *b)
{
return *(long **)a - *(long **)b;
}

/*
	ANT_INEX_ASSESSMENT::MAX()
	--------------------------
*/
inline char *ANT_INEX_assessment::max(char *a, char *b, char *c)
{
char *thus_far;

thus_far = a;
if (b > thus_far)
	thus_far = b;
if (c > thus_far)
	thus_far = c;

return thus_far;
}

/*
	ANT_INEX_ASSESSMENT::ANT_INEX_ASSESSMENT()
	------------------------------------------
*/
ANT_INEX_assessment::ANT_INEX_assessment(ANT_memory *mem, char **docid_list, long documents)
{
char **current, *slish, *slosh, *slash, *start;
long *current_docid, **current_sorted_docid;

memory = mem;
this->documents = documents;
current_docid = numeric_docid_list = (long *)memory->malloc(sizeof(*numeric_docid_list) * documents);
current_sorted_docid = sorted_numeric_docid_list = (long **)memory->malloc(sizeof(*sorted_numeric_docid_list) * documents);
for (current = docid_list; *current != NULL; current++)
	{
	slash = strchr(*current, '/');
	slosh = strchr(*current, '\\');
	slish = *current;
	if ((start = max(slish, slash, slosh)) == NULL)
		break;
	*current_docid = atol(start);
	*current_sorted_docid = current_docid;
	current_docid++;
	current_sorted_docid++;
	}
qsort(sorted_numeric_docid_list, documents, sizeof(*sorted_numeric_docid_list), cmp);
}

/*
	ANT_INEX_ASSESSMENT::READ()
	---------------------------
*/
ANT_relevant_document *ANT_INEX_assessment::read(char *filename, long *reldocs)
{
ANT_disk disk;
char *file, **lines, **current;
long topic, document, document_length, relevant_characters, relevant_documents, *document_pointer, **found, lines_in_file;
ANT_relevant_document *current_assessment, *all_assessments;

/*
	load the assessment file into memory
*/
if ((file = disk.read_entire_file(filename)) == NULL)
	return NULL;
lines = disk.buffer_to_list(file, &lines_in_file);

/*
	count the number of relevant documents
*/
relevant_documents = 0;
for (current = lines; *current != 0; current++)
	{
	sscanf(*current, "%d %*s %d %d %d", &topic, &document, &relevant_characters, &document_length);
	if (relevant_characters != 0)
		relevant_documents++;
	}

/*
	allocate space for the (positive) assessments
*/
current_assessment = all_assessments = (ANT_relevant_document *)memory->malloc(sizeof(*all_assessments) * relevant_documents);

/*
	generate the list of relevant documents
*/
document_pointer = &document;
for (current = lines; *current != 0; current++)
	{
	sscanf(*current, "%d %*s %d %d %d", &topic, &document, &relevant_characters, &document_length);
	if (relevant_characters != 0)
		{
		found = (long **)bsearch(&document_pointer, sorted_numeric_docid_list, documents, sizeof(*sorted_numeric_docid_list), cmp);
		if (found == NULL)
			printf("DOC:%d is in the assessments, but not in the collection\n", document);
		else
			current_assessment->docid = numeric_docid_list - *found;		// the position in the list of documents is the internal docid used for computing precision
		current_assessment->topic = topic;
		current_assessment->rsv = (double)relevant_characters / (double)document_length;
		current_assessment++;
		}
	}

/*
	clean up
*/
delete [] file;
delete [] lines;

/*
	and return
*/
*reldocs = relevant_documents;
return all_assessments;
}
