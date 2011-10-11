/*
	MEMORY_FILE_LINE.H
	------------------
*/
#ifndef MEMORY_FILE_LINE_H_
#define MEMORY_FILE_LINE_H_

/*
	class ANT_MEMORY_FILE_LINE
	--------------------------
*/
class ANT_memory_file_line
{
friend
	class ANT_memory_file_line_iterator;

private:
	char *contents;						// the current file as read from disk
	char **line;						// an array of pointers into the in-memory version of the disk file
	long long current_line;				// line at the top of the current display page
	char **current_line_pointer;		// pointer to the line of text at the top of the current display page
	long long lines_in_file;			// number of lines in the file
	long long page_size;				// number of lines on a display page

public:
	ANT_memory_file_line();
	virtual ~ANT_memory_file_line();

	/*
		Read a file into memory and return the number of lines read
	*/
	long long read_file(char *filename);

	/*
		Return a pointer to the line of text that should be at line display_line on the output device
	*/
	char **get_current_line(void) 			{ return current_line_pointer; }

	/*
		Return stats about the current document
	*/
	long long get_current_line_number(void) { return current_line; }
	long long get_page_size(void) 			{ return page_size; }
	long long get_lines_in_file(void) 		{ return lines_in_file; }

	/*
		Navigate around the document
	*/
	long long line_up(void);
	long long line_down(void);
	long long page_up(void);
	long long page_down(void);
	long long file_start(void);
	long long file_end(void);
	long long goto_line(long long line);

	/*
		Set the current page length
	*/
	long long set_page_size(long long to) { return page_size = to; }
} ;

#endif /* MEMORY_FILE_LINE_H_ */


























