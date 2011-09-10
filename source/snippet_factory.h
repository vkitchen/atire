/*
	SNIPPET_FACTORY.H
	-----------------
*/
#ifndef SNIPPET_FACTORY_H_
#define SNIPPET_FACTORY_H_

class ANT_snippet;

/*
	class ANT_SNIPPET_FACTORY
	-------------------------
*/
class ANT_snippet_factory
{
public:
	enum { SNIPPET_TITLE = 1, SNIPPET_BEGINNING, SNIPPET_TF };

public:
	ANT_snippet_factory() {}
	virtual ~ANT_snippet_factory() {}

	static ANT_snippet *get_snippet_maker(long type, long length_of_snippet, char *tag, long length_of_longest_document);
} ;

#endif /* SNIPPET_FACTORY_H_ */