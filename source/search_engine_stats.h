/*
	SEARCH_ENGINE_STATS.H
	---------------------
*/
#ifndef SEARCH_ENGINE_STATS_H_
#define SEARCH_ENGINE_STATS_H_

#include "time_stats.h"

/*
	class ANT_SEARCH_ENGINE_STATS
	-----------------------------
*/
class ANT_search_engine_stats : public ANT_time_stats
{
public:
	long long decompress_time;
	long long posting_read_time;
	long long rank_time;
	long long accumulator_init_time;
	long long sort_time;
	long long count_relevant_time;
	long long dictionary_time;
	long long stemming_time;				// time taken to do stemming (except re-convert into a postings list)
	long long stemming_reencode_time;		// time taken to convert from the stemming tf array into a postings list
	long long queries;						// total number of times stats have been added to this one
	long long disk_bytes_read_on_init;		// total number of bytes read from the disk on initialisation
	long long disk_bytes_read_on_search;	// total bytes read from the disk durin the search

public:
	ANT_search_engine_stats(ANT_memory *memory) ;
	virtual ~ANT_search_engine_stats() {}
	virtual void text_render(void);

	void initialise(void);
	void add(ANT_search_engine_stats *what);

	void add_posting_read_time(long long time) { posting_read_time += time; }
	void add_decompress_time(long long time) { decompress_time += time; }
	void add_rank_time(long long time) { rank_time += time; }
	void add_accumulator_init_time(long long time) { accumulator_init_time += time; }
	void add_sort_time(long long time) { sort_time += time; } 
	void add_count_relevant_documents(long long time) { count_relevant_time += time; }
	void add_dictionary_lookup_time(long long time) { dictionary_time += time; }
	void add_stemming_time(long long time) { stemming_time += time; }
	void add_stemming_reencode_time(long long time) { stemming_reencode_time += time; }
	void add_disk_bytes_read_on_init(long long bytes) { disk_bytes_read_on_init += bytes; }
	void add_disk_bytes_read_on_search(long long bytes) { disk_bytes_read_on_search += bytes; }
	long long get_cpu_time() { return accumulator_init_time + decompress_time + rank_time + sort_time + count_relevant_time; }
	long long get_cpu_time_ms() {return (long long)(get_cpu_time() / (get_clock_tick_frequency() / 1000.0)); }
	long long get_io_time() { return dictionary_time + posting_read_time; }
	long long get_io_time_ms() { return (long long)(get_io_time() / (get_clock_tick_frequency() / 1000.0)); }
} ;

#endif  /* SEARCH_ENGINE_STATS_H_ */
