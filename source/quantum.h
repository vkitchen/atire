/*
 * QUANTUM.H
 * ---------
 */

#ifndef QUANTUM_H_
#define QUANTUM_H_

#include "compress.h"
#include "search_engine_btree_leaf.h"
#include "ranking_function_quantum_parameters.h"

class ANT_quantum : public ANT_ranking_function_quantum_parameters
{
public:
	ANT_compressable_integer impact_value;
	ANT_compressable_integer doc_count;
	ANT_compressable_integer *offset_ptr;
};


#endif /* QUANTUM_H_ */
