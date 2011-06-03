/*
	PARSER.H
	--------
*/
#ifndef PARSER_H_
#define PARSER_H_

#include "string_pair.h"
#include "parser_token.h"
#include "ctypes.h"
#include "unicode_tables.h"
#include "unicode.h"

#ifndef FALSE
	#define FALSE 0
#endif

#ifndef TRUE
	#define TRUE (!FALSE)
#endif

#ifdef _MSC_VER
	#define inline __forceinline
#endif

/*
	class ANT_PARSER
	----------------
*/
class ANT_parser
{
public:
	/*
		This might be confusing. For Chinese, no segmentation means documents are indexed by characters only;
		DOUBLE_SEGMENTATION means string is segmented into words and also single characters for indexing
		ONFLY_SEGMENTATION means segment the sentence using the Chinese segmentation module when parsing the document
		If ONFLY_SEGMENTATION is not set, but SHOULD_SEGMENT is still set that means the text is segmented.
	 */
	enum { NOSEGMENTATION = 0, SHOULD_SEGMENT = 1, ONFLY_SEGMENTATION = 2, DOUBLE_SEGMENTATION = 4, BIGRAM_SEGMENTATION = 8 };

protected:
	unsigned char *document;
	unsigned char *current;
	ANT_parser_token current_token;
	unsigned char *segmentation;
	long should_segment;

/*
protected:
	static int isheadchar(unsigned char val) { return ANT_isalnum(val) || val == '<' || val == '\0'; }
*/

public:
	ANT_parser(long should_segment = NOSEGMENTATION);
	virtual ~ANT_parser();

/*
	static int isXMLnamestartchar(unsigned char val) { return ANT_isalpha(val) || val == ':' || val == '_'; }		// see http://www.w3.org/TR/REC-xml/#NT-NameStartChar
	static int isXMLnamechar(unsigned char val) { return isXMLnamestartchar(val) || ANT_isdigit(val) || val == '.' || val == '-'; } // see http://www.w3.org/TR/REC-xml/#NT-NameChar
*/

	virtual void segment(unsigned char *start, long length);

	void set_document(unsigned char *document);
	virtual ANT_parser_token *get_next_token(void);
} ;

#endif  /* PARSER_H_ */


