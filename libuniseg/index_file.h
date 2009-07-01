/*
 * index_file.h
 *
 *  Created on: Oct 26, 2008
 *      Author: monfee
 */

#ifndef INDEX_FILE_H_
#define INDEX_FILE_H_

#include "file.h"
#include "freq.h"
#include "address.h"

class IndexFile : public File {

public:
	typedef Freq::string_type						string_type;
	typedef Freq::string_array						string_array;

public:
	static const int UINT_SIZE = sizeof(unsigned int);
	static const int ADDRESSES_NUMBER = 3;
	static const int ADDRESSES_LENGTH = ADDRESSES_NUMBER * UINT_SIZE;
	static const int UNICODE_CHAR_LENGTH = 4;
	//static const std::string EXT_NAME;

private:
	//Freq& 			freq_;
	int				wlen_;
	address_array	aa_;

public:
	IndexFile(std::string name/*, Freq& freq*/) :
		File::File(name)/*, freq_(freq) */{ init(); }
	IndexFile(std::string name, unsigned int size/*, Freq& freq*/) :
		File::File(name, size)/*, freq_(freq) */{ init(); }
	virtual ~IndexFile();

	void write();
	void read();
	unsigned int cal(File::array_type& arr);
	void alloc(File::array_type& arr);

	int wlen() { return wlen_; }
	void wlen(int len) { wlen_ = len; }

	address_array& aa() { return aa_; }

private:
	void init();
};

//const std::string IndexFile::EXT_NAME = ".idx";

#endif /* INDEX_FILE_H_ */
