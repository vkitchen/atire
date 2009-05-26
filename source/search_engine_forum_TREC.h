/*
	SEARCH_ENGINE_FORUM_TREC.H
	--------------------------
*/

#ifndef __SEARCH_ENGINE_FORUM_TREC_H__
#define __SEARCH_ENGINE_FORUM_TREC_H__

#include "search_engine_forum.h"

#define MAX_RUN_NAME_LENGTH 1024

/*
	class ANT_SEARCH_ENGINE_FORUM_TREC
	----------------------------------
*/
class ANT_search_engine_forum_TREC : public ANT_search_engine_forum
{
private:
	char run_id[MAX_RUN_NAME_LENGTH];
public:
	ANT_search_engine_forum_TREC(char *filename, char *participant_id, char *run_id, char *task);
	void write(long topic_id, char **docids, long long hits);
} ;


#endif __SEARCH_ENGINE_FORUM_TREC_H__
