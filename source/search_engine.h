/*
	SEARCH_ENGINE.H
	---------------
*/
#ifndef __SEARCH_ENGINE_H__
#define __SEARCH_ENGINE_H__

#include "search_engine_posting.h"

class ANT_memory;
class ANT_file;
class ANT_search_engine_btree_node;
class ANT_search_engine_btree_leaf;
class ANT_search_engine_accumulator;

class ANT_search_engine
{
private:
	ANT_memory *memory;
	ANT_file *index;
	ANT_search_engine_btree_node *btree_root;
	long btree_nodes, documents;
	long *document_lengths;
	double mean_document_length;
	unsigned char *btree_leaf_buffer, *postings_buffer;
	ANT_search_engine_accumulator *accumulator;
	ANT_search_engine_posting posting;

private:
	long long get_long_long(unsigned char *from) { return *((long long *)from); }
	long get_long(unsigned char *from) { return *((long *)from); }

	void bm25_rank(ANT_search_engine_btree_leaf *leaf, ANT_search_engine_posting *postings);
public:
	ANT_search_engine(ANT_memory *memory);
	~ANT_search_engine();

	void init_accumulators(void);
	long long get_btree_leaf_position(char *term, long long *length, long *exact_match);
	ANT_search_engine_btree_leaf *get_postings_details(char *term, ANT_search_engine_btree_leaf *term_details);
	unsigned char *get_postings(ANT_search_engine_btree_leaf *term_details, unsigned char *destination);
	void decompress(unsigned char *start, unsigned char *end, long *into);
	void process_one_search_term(char *term);
	ANT_search_engine_accumulator *generate_results_list(long *hits);
	long document_count(void) { return documents; }
};

#endif __SEARCH_ENGINE_H__