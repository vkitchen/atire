/*
	RANKING_FUNCTION_SIMILARITY.C
	-----------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "pragma.h"
#include "ranking_function_similarity.h"
#include "search_engine_btree_leaf.h"
#include "compress.h"
#include "search_engine_accumulator.h"

/*
	ANT_RANKING_FUNCTION_SIMILARITY::ANT_RANKING_FUNCTION_SIMILARITY()
	------------------------------------------------------------------
*/
ANT_ranking_function_similarity::ANT_ranking_function_similarity(ANT_search_engine *engine, double k1, double b) : ANT_ranking_function(engine)
{
const double one_minus_b = 1.0 - b;
long current;

this->k1 = k1;
this->b = b;

/*
	Precompute the length normalisation prior probability part of BM25
*/
document_prior_probability = new float [(size_t)documents_as_integer];
for (current = 0; current < documents_as_integer; current++)
	document_prior_probability[current] =  (float)(k1 * (one_minus_b + b * (document_lengths[current] / mean_document_length)));
}

/*
	ANT_RANKING_FUNCTION_SIMILARITY::~ANT_RANKING_FUNCTION_SIMILARITY()
	-------------------------------------------------------------------
*/
ANT_ranking_function_similarity::~ANT_ranking_function_similarity()
{
delete [] document_prior_probability;
}

/*
	ANT_RANKING_FUNCTION_SIMILARITY::RELEVANCE_RANK_TOP_K()
	-------------------------------------------------------
*/
#ifdef IMPACT_HEADER
void ANT_ranking_function_similarity::relevance_rank_top_k(ANT_search_engine_result *accumulator, ANT_search_engine_btree_leaf *term_details, ANT_impact_header *impact_header, ANT_compressable_integer *impact_ordering, long long trim_point, double prescalar, double postscalar) {
	const double k1 = this->k1;
	const double k1_plus_1 = k1 + 1.0;
	long long docid;
	double top_row, tf, idf;
	ANT_compressable_integer *current, *end;

	/*
		          N
		IDF = log -
		          n

		This variant of IDF is better than log((N - n + 0.5) / (n + 0.5)) on the 70 INEX 2008 Wikipedia topics
	*/
	idf = log((double)documents / (double)term_details->global_document_frequency);

	/*
		               tf(td) * (k1 + 1)
		rsv = ----------------------------------- * IDF
		                                  len(d)
		      tf(td) + k1 * (1 - b + b * --------)
	                                    av_len_d

		In this implementation we ignore k3 and the number of times the term occurs in the query.
	*/
	/*
		Alright, so, we have the trim_point at this point.  The trim point is an optimising factor.  It is the
		number of postings we should process before early-terminating.  As the postings are stored in impact
		order (typically from highest to lowest term frequency), the first posting is most probably the most
		relevant document and so on.  Oh, but as there will be multiple postings with the same impact score
		and we don't know which is "best", the early termination actually only happens at the end of processing
		one complete impact value's postings list.

		So how do we compute where to end?  The impacted list is of length:document frequencies plus the impacts
		plus the terminators.  So we can compute the ending point from document frequency if we add to that pointer
		for each impact and each zero.  Now that we can compare document frequency, we can compare to the trim_point
		measured in postings and early terminate after we have processed that "batch" of postings.
	*/
	impact_header->impact_value_ptr = impact_header->impact_value_start;
	impact_header->doc_count_ptr = impact_header->doc_count_start;
	current = impact_ordering;
	while(impact_header->doc_count_ptr < impact_header->doc_count_trim_ptr) {
		tf = *impact_header->impact_value_ptr;
		top_row = prescalar * tf * k1_plus_1;
		docid = -1;
		end = current + *impact_header->doc_count_ptr;
		while (current < end) {
			docid += *current++;
			/*
				This version uses the document prior probabilities computed in the constructor
				which takes more memory
			*/
			accumulator->add_rsv(docid, postscalar * idf * (top_row / (prescalar * tf + document_prior_probability[(size_t)docid])));
		}
		current = end;
		impact_header->impact_value_ptr++;
		impact_header->doc_count_ptr++;
	}
#pragma ANT_PRAGMA_UNUSED_PARAMETER
}
#else
void ANT_ranking_function_similarity::relevance_rank_top_k(ANT_search_engine_result *accumulator, ANT_search_engine_btree_leaf *term_details, ANT_compressable_integer *impact_ordering, long long trim_point, double prescalar, double postscalar)
{
const double k1 = this->k1;
const double k1_plus_1 = k1 + 1.0;
long long docid;
double top_row, tf, idf;
ANT_compressable_integer *current, *end;

/*
	          N
	IDF = log -
	          n

	This variant of IDF is better than log((N - n + 0.5) / (n + 0.5)) on the 70 INEX 2008 Wikipedia topics
*/
idf = log((double)documents / (double)term_details->global_document_frequency);

/*
	               tf(td) * (k1 + 1)
	rsv = ----------------------------------- * IDF
	                                  len(d)
	      tf(td) + k1 * (1 - b + b * --------)
                                    av_len_d

	In this implementation we ignore k3 and the number of times the term occurs in the query.
*/
/*
	Alright, so, we have the trim_point at this point.  The trim point is an optimising factor.  It is the
	number of postings we should process before early-terminating.  As the postings are stored in impact
	order (typically from highest to lowest term frequency), the first posting is most probably the most
	relevant document and so on.  Oh, but as there will be multiple postings with the same impact score
	and we don't know which is "best", the early termination actually only happens at the end of processing
	one complete impact value's postings list.

	So how do we compute where to end?  The impacted list is of length:document frequencies plus the impacts
	plus the terminators.  So we can compute the ending point from document frequency if we add to that pointer
	for each impact and each zero.  Now that we can compare document frequency, we can compare to the trim_point
	measured in postings and early terminate after we have processed that "batch" of postings.
*/
current = impact_ordering;
end = impact_ordering + (term_details->local_document_frequency >= trim_point ? trim_point : term_details->local_document_frequency);
while (current < end)
	{
	end += 2;		// account for the impact_order and the terminator
	tf = *current++;
	top_row = prescalar * tf * k1_plus_1;
	docid = -1;
	while (*current != 0)
		{
		docid += *current++;
		/*
			This version uses the document prior probabilities computed in the constructor
			which takes more memory
		*/
		accumulator->add_rsv(docid, postscalar * idf * (top_row / (prescalar * tf + document_prior_probability[(size_t)docid])));
		}
	current++;		// skip over the zero
	}
}
#endif

/*
	ANT_RANKING_FUNCTION_SIMILARITY::RELEVANCE_RANK_TF()
	----------------------------------------------------
	This is the variant that gets called for stemming.
*/
void ANT_ranking_function_similarity::relevance_rank_tf(ANT_search_engine_result *accumulator, ANT_search_engine_btree_leaf *term_details, ANT_weighted_tf *tf_array, long long trim_point, double prescalar, double postscalar)
{
const double k1 = this->k1;
const double b = this->b;
const double k1_plus_1 = k1 + 1.0;
const double one_minus_b = 1.0 - b;
double tf, idf, top_row;
ANT_weighted_tf *current, *end;
long long docid;

compute_term_details(term_details, tf_array);	// get document_frequency;

idf = log((double)(documents) / (double)term_details->global_document_frequency);

current = tf_array;
end = tf_array + (term_details->local_document_frequency >= trim_point ? trim_point : term_details->local_document_frequency);

for (current = tf_array; current < end; current++)
	if (*current != 0)
		{
		tf = *current >= 0x100 ? 0xFF : *current;
		docid = current - tf_array;
		top_row = prescalar * tf * k1_plus_1;

		accumulator->add_rsv(docid, postscalar * idf * (top_row / (prescalar * tf + k1 * (one_minus_b + b * (document_lengths[(size_t)docid] / mean_document_length)))));
		}

#ifdef IMPACT_HEADER
relevance_rank_top_k(accumulator, term_details, the_impact_header, decompress_buffer, trim_point, prescalar, postscalar);
#else
relevance_rank_top_k(accumulator, term_details, decompress_buffer, trim_point, prescalar, postscalar);
#endif
}

/*
	ANT_RANKING_FUNCTION_SIMILARITY::RANK()
	---------------------------------------
*/
double ANT_ranking_function_similarity::rank(ANT_compressable_integer docid, ANT_compressable_integer length, unsigned char term_frequency, long long collection_frequency, long long document_frequency)
{
exit(printf("Cannot pre-compute the impact of the stemming similarity function (it is meaningless)\n"));
#pragma ANT_PRAGMA_UNUSED_PARAMETER
}
