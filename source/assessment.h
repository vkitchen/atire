/*
	ASSESSMENT.H
	------------
*/

#ifndef __ASSESSMENT_H__
#define __ASSESSMENT_H__

class ANT_memory;
class ANT_relevant_document; 

/*
	class ANT_ASSESSMENT
	--------------------
*/
class ANT_assessment
{
friend class ANT_assessment_factory;

protected:
	ANT_memory *memory;
	long long documents;
	long *numeric_docid_list;
	long **sorted_numeric_docid_list;

protected:
	ANT_assessment() {}
	void copy(ANT_assessment *what_to_copy);

	char *max(char *a, char *b, char *c);
	static int cmp(const void *a, const void *b);

public:
	ANT_assessment(ANT_memory *mem, char **docid_list, long long documents);
	virtual ANT_relevant_document *read(char *filename, long long *reldocs) = 0;
} ;

#endif __ASSESSMENT_H__
