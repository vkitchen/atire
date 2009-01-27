/*
	TOPIC_TREE.C
	------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../source/str.h"
#include "../source/disk.h"
#include "../source/stop_word.h"
#include "../source/porter.h"

#define MAX_TERMS_PER_QUERY 20
#define MAX_QUERY_LENGTH 1024

#define BOOTSTRAP_ITERATIONS (1024 * 1024)

static char *SEPERATORS = " []()\"";

FILE *netfile, *statsfile;

ANT_porter stemmer;
ANT_stop_word stopper;

/*
	class ANT_QUERY
	---------------
*/
class ANT_query
{
public:
	long terms_in_query;
	char term_buffer[MAX_QUERY_LENGTH];
	char *term[MAX_TERMS_PER_QUERY];
} *term_list;

long term_list_length;

char stem_buffer[MAX_QUERY_LENGTH];
long query_length_stats[MAX_TERMS_PER_QUERY];

/*
	FIND_LINKS()
	------------
*/
long find_links(long from, long quiet)
{
long current;
char **current_term, **term;
long *simalarity_list;
long total_links;

simalarity_list = new long [term_list_length];
memset(simalarity_list, 0, sizeof(*simalarity_list) * term_list_length);

for (term = term_list[from].term; *term != NULL; term++)
	for (current = 0; current < term_list_length; current++)
		if (current != from)
			for (current_term = term_list[current].term; *current_term != NULL; current_term++)
				if (strcmp(*term, *current_term) == 0)
					simalarity_list[current]++;

total_links = 0;
for (current = 0; current < term_list_length; current++)
	if (simalarity_list[current] != 0)
		{
		total_links += simalarity_list[current];
		if (!quiet)
			fprintf(netfile, "%d %d %d w %d\n", from + 1, current + 1, simalarity_list[current], simalarity_list[current]);
		}

delete [] simalarity_list;
return total_links;
}

/*
	GENERATE_RANDOM_QUERY()
	-----------------------
*/
ANT_query *generate_random_query(ANT_query *query, long terms_in_query, long terms_in_dictionary, char **dictionary)
{
long term, random_term;
char *word, *into;

into = query->term_buffer;
query->terms_in_query = 0;
for (term = 0; term < terms_in_query; term++)
	{
	do
		{
		random_term = (long)((double)rand() / (double)RAND_MAX * terms_in_dictionary);
		word = dictionary[random_term];
		}
	while (stopper.isstop(word));

//printf("%s ", word);

	strcpy(into, word);
	stemmer.stem(into, stem_buffer);
	strcpy(into, stem_buffer);
	query->term[query->terms_in_query] = into;
	query->terms_in_query++;
	into += strlen(into);
	*into++ = '\0';
	}
query->terms_in_query = terms_in_query;
//puts("");

return query;
}

/*
	BOOTSTRAP_MAIN()
	----------------
	usage: topic_tree.exe <dictionary> <statsfile>
*/
int bootstrap_main(int argc, char *argv[])
{
ANT_disk disk;
char *file, **dictionary, *stats, **stats_list, **stats_line;
long terms_in_dictionary, queries, total_links;
long max_query_length, current_query, current, which;
long times, stats_length, len;
long *simalarity_list;

if (argc != 3)
	exit(printf("Usage:%s <dictionary> <statsfile>\n", argv[0]));

if ((file = disk.read_entire_file(argv[1])) == NULL)
	exit(printf("Cannot open dictionary:%s\n", argv[1]));

memset(query_length_stats, 0, sizeof(query_length_stats));
dictionary = disk.buffer_to_list(file, &terms_in_dictionary);

if ((stats = disk.read_entire_file(argv[2])) == NULL)
	exit(printf("Cannot open statsfile:%s\n", argv[2]));
stats_list = disk.buffer_to_list(stats, &stats_length);

max_query_length = queries = 0;
for (stats_line = stats_list; *stats_line != NULL; stats_line++)
	{
	times = len = 0;
	sscanf(*stats_line, "%d %d", &len, &times);
	if (times != 0)
		{
		query_length_stats[len] = times;
		queries += times;
		max_query_length = len;
		}
	}

/*
for (current = 0; current <= max_query_length; current++)
	if (query_length_stats[current] != 0)
		printf("len %d, %d times\n", current, query_length_stats[current]);
*/

term_list = new ANT_query[term_list_length = queries];
simalarity_list = new long [term_list_length];
memset(simalarity_list, 0, sizeof(*simalarity_list) * term_list_length);

for (times = 0; times < BOOTSTRAP_ITERATIONS; times++)
	{
	current_query = 0;
	for (current = 0; current <= max_query_length; current++)
		for (which = 0; which < query_length_stats[current]; which++)
				generate_random_query(&term_list[current_query++], current, terms_in_dictionary, dictionary);

	total_links = 0;
	for (current = 0; current < queries; current++)
		total_links += find_links(current, true);

//	if (total_links != 0)
//		{
		simalarity_list[total_links / 2]++;		 // because each link is represented in both directions
//		printf("Total Links:%d\n", total_links);
//		}
	}

for (current = 0; current < term_list_length; current++)
	printf("%d %d\n", current, simalarity_list[current]);

delete [] term_list;
delete [] simalarity_list;

return 0;
}

/*
	STATS_MAIN()
	------------
*/
int stats_main(int argc, char *argv[])
{
ANT_disk disk;
char *file, **line_list;
char *term;
long lines, current_term, current, total_links;
char *netfilename, *statsfilename;

netfilename = argv[2];
statsfilename = argv[3];
if (argc != 4)
	exit(printf("usage:%s <queryfile> <netfile> <statsfile>\n", argv[0]));

if ((file = disk.read_entire_file(argv[1])) == NULL)
	exit(printf("Cannot open topic file\n"));
line_list = disk.buffer_to_list(file, &lines);

netfile = fopen(netfilename, "wb");
statsfile = fopen(statsfilename, "wb");

term_list = new ANT_query[lines];
for (current = 0; current < lines; current++)
	{
	if (strlen(line_list[current]) >= MAX_QUERY_LENGTH)
		exit(printf("Line %d: Query too long (exceeds %d chars)\n", current, MAX_QUERY_LENGTH));
	strcpy(term_list[current].term_buffer, line_list[current]);
	current_term = 0;
	for (term = strtok(line_list[current], SEPERATORS); term != NULL; term = strtok(NULL, SEPERATORS))
		{
		if (current_term > MAX_TERMS_PER_QUERY)
			exit(printf("Line %d: Too many search terms (exceeds %d)\n", current, current_term));
		if (stopper.isstop(term))		// drop stop words
			continue;
		strlwr(term);
		stemmer.stem(term, stem_buffer);
		strcpy(term, stem_buffer);
		term_list[current].term[current_term] = term;
		current_term++;
		}
	term_list[current].terms_in_query = current_term;
	term_list[current].term[current_term] = NULL;
	}
term_list_length = current;

/*
	Node list
*/
memset(query_length_stats, 0, sizeof(query_length_stats));
fprintf(netfile, "*Vertices %d\n", lines);
for (current = 0; current < lines; current++)
	{
	fprintf(netfile, "%d \"%d\" x_fact %d y_fact %d\n", current + 1, current + 1, term_list[current].terms_in_query, term_list[current].terms_in_query);
	query_length_stats[term_list[current].terms_in_query]++;
	}

total_links = 0;
fprintf(netfile, "*Arcs\n");
for (current = 0; current < lines; current++)
	total_links += find_links(current, false);

fprintf(statsfile, "Len Queries\n");
for (current = 0; current < MAX_TERMS_PER_QUERY; current++)
	if (query_length_stats[current] != 0)
		fprintf(statsfile, "%3d %d\n", current - 1, query_length_stats[current]);

fprintf(statsfile, "Total Links:%d\n", total_links);
fclose(netfile);
fclose(statsfile);
return 0;
}

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
{
srand((unsigned int)time(0));		// see the random number generator

if (argc == 4)
	return stats_main(argc, argv);
else
	return bootstrap_main(argc, argv);
}

