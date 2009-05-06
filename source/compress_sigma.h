/*
	COMPRESS_SIGMA.H
	----------------
*/

#ifndef __COMPRESS_SIGMA_H__
#define __COMPRESS_SIGMA_H__

#include "compress.h"

class  ANT_compress_sigma_frequency;

/*
	class ANT_COMPRESS_SIGMA
	------------------------
*/
class ANT_compress_sigma : public ANT_compress
{
protected:
	long threshold;
	ANT_compressable_integer *dictionary;
	long long dictionary_length;

protected:
	static int map_cmp(const void *a, const void *b);
	static int map_freq_cmp(const void *a, const void *b);
	static int long_cmp(const void *a, const void *b);

	ANT_compress_sigma_frequency *reorder(ANT_compress_sigma_frequency *map, ANT_compress_sigma_frequency *end, long uniques, long threshold, ANT_compressable_integer *uniques_over_threshold);

public:
	ANT_compress_sigma() { threshold = 1; dictionary_length = 0; dictionary = NULL; }
	virtual ~ANT_compress_sigma() { delete [] dictionary; }

	virtual long long compress(unsigned char *destination, long long destination_length, ANT_compressable_integer *source, long long source_integers);
	virtual void decompress(ANT_compressable_integer *destination, unsigned char *source, long long destination_integers);

} ;



#endif __COMPRESS_SIGMA_H__
