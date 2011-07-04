/*
	ATIRE_ENGINE_RESULT_SET.C
	-------------------------
*/
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include "atire_engine_result_set.h"
#include "atire_engine_result.h"
#include "str.h"
/*
	ATIRE_ENGINE_RESULT_SET::ATIRE_ENGINE_RESULT_SET()
	--------------------------------------------------
*/
ATIRE_engine_result_set::ATIRE_engine_result_set()
{
hits = 0;
length_of_results = 1024 * 1024;		// start with 1 million results (but unlikely to ever need this many)
results = new ATIRE_engine_result[(size_t)length_of_results];
}

/*
	ATIRE_ENGINE_RESULT_SET::~ATIRE_ENGINE_RESULT_SET()
	---------------------------------------------------
*/
ATIRE_engine_result_set::~ATIRE_engine_result_set()
{
delete [] results;
}

/*
	ATIRE_ENGINE_RESULT_SET::EXTEND()
	---------------------------------
*/
void ATIRE_engine_result_set::extend(long long new_size)
{
ATIRE_engine_result *replacement;

replacement = new ATIRE_engine_result[(size_t)new_size];
memcpy(replacement, results, (size_t)(sizeof(*results) * length_of_results));
delete [] results;
results = replacement;
length_of_results = new_size;
}

/*
	ATIRE_ENGINE_RESULT_SET::REWIND()
	---------------------------------
*/
void ATIRE_engine_result_set::rewind(void)
{
hits = 0;
}

/*
	ATIRE_ENGINE_RESULT_SET::ADD()
	------------------------------
*/
long long ATIRE_engine_result_set::add(char *source, long long docid_base)
{
long long found = 0;
char *pos;
char *rank, *id, *name, *rsv, *title, *end;
char *name_end, *title_end;

for (pos = strstr(source, "<hit>"); pos != NULL; pos = strstr(end, "<hit>"))
	{
	if ((end = strstr(pos, "</hit>")) == NULL)
		break;

	if ((rank = strstr(pos, "<rank>")) == NULL)
		break;
	rank += 6;

	if ((id = strstr(rank, "<id>")) == NULL)
		break;
	id += 4;

	if ((name = strstr(id, "<name>")) == NULL)
		break;
	name += 6;
	if ((name_end = strstr(name, "</name>")) == NULL)
		break;

	if ((rsv = strstr(name_end, "<rsv>")) == NULL)
		break;
	rsv += 5;

	if ((title = strstr(rsv, "<title>")) == NULL)
		break;
	title += 7;
	if ((title_end = strstr(title, "</title>")) == NULL)
		break;

	if (rank > end || id > end || name > end || name_end > end || rsv > end || title > end || title_end > end)
		break;

	*name_end = *title_end = '\0';

	if (hits + 1 > length_of_results)
		extend(length_of_results * expansion_factor);

	results[hits].rank = ANT_atoi64(rank);
	results[hits].id = ANT_atoi64(id) + docid_base;
	results[hits].name = name;
	results[hits].rsv = atof(rsv);
	results[hits].title = title;

	found++;
	hits++;
	}

return found;
}

/*
	ATIRE_ENGINE_RESULT_SET::CMP()
	------------------------------
*/
int ATIRE_engine_result_set::cmp(const void *a, const void *b)
{
ATIRE_engine_result *one, *two;

one = (ATIRE_engine_result *)a;
two = (ATIRE_engine_result *)b;

/*
	Highet RSV first
*/
if (one->rsv > two->rsv)
	return -1;
if (one->rsv < two->rsv)
	return 1;

/*
	Now we return the document with the lowest id base on global docid space.
*/
if (one->id < two->id)
	return -1;
if (one->id > two->id)
	return 1;

return 0;
}

/*
	ATIRE_ENGINE_RESULT_SET::SORT()
	-------------------------------
*/
void ATIRE_engine_result_set::sort(void) 
{
qsort(results, (size_t)hits, sizeof(*results), cmp);
}

/*
	ATIRE_ENGINE_RESULT_SET::SERIALISE()
	------------------------------------
*/
char *ATIRE_engine_result_set::serialise(char *query, long long overall_hits, long long time_taken, long long first, long long page_length)
{
long long current, from, to;
std::stringstream result;

result << "<ATIREsearch>\n";
result << "<query>" << query << "</query>";
result << "<numhits>" << overall_hits << "</numhits>";
result << "<time>" << time_taken << "</time>";

result << std::fixed << std::setprecision(2);

if (first < hits)
	{
	sort();

	from = first - 1;
	to = from + page_length < hits ? from + page_length : hits;
	result << "<hits>\n";
	for (current = from; current < to; current++)
		{
		result << "<hit>";
		result << "<rank>" << current + 1 << "</rank>";
		result << "<id>" << results[current].id << "</id>";
		result << "<name>" << results[current].name << "</name>";
		result << "<rsv>" << results[current].rsv << "</rsv>";
		result << "<title>" << results[current].title << "</title>";
		result << "</hit>\n";
		}
	result << "</hits>\n";
	}
result << "</ATIREsearch>";

return strnew(result.str().c_str());
}

/*
	ATIRE_ENGINE_RESULT_SET::SERIALISE_TREC()
	-----------------------------------------
*/
char *ATIRE_engine_result_set::serialise_TREC(long long topic_id, char *run_name, long long first, long long page_length)
{
long long current, from, to;
std::stringstream result;

result << std::fixed << std::setprecision(2);

if (first < hits)
	{
	sort();

	from = first - 1;
	to = from + page_length < hits ? from + page_length : hits;
	for (current = from; current < to; current++)
		{
		result << topic_id << " ";
		result << "Q0" << " ";
		result << results[current].name << " ";
		result << current + 1 << " ";
		result << results[current].rsv << " ";
		result << run_name << std::endl;
		}
	}

return strnew(result.str().c_str());
}
