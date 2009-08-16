/*
 * PLUGIN.H
 * --------
 *
 *  Created on: Jul 2, 2009
 *      Author: monfee
 */

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

/*
	class ANT_PLUGIN
	----------------
*/
class ANT_plugin
{
public:
	enum plugin_id { SEGMENTATION  = 0 };

public:
	virtual const unsigned char *do_segmentation(unsigned char *c, int length);
	virtual const unsigned char *get_input();
	virtual const unsigned char *get_output();
	virtual int get_count();
};

/*
	ANT_PLUGIN::DO_SEGMENTATION()
	-----------------------------
*/
#pragma warning (suppress: 4100)
inline const unsigned char *ANT_plugin::do_segmentation(unsigned char *c, int length) { return 0; }

/*
	ANT_PLUGIN::GET_INPUT()
	-----------------------
*/
inline const unsigned char *ANT_plugin::get_input() { return NULL; }

/*
	ANT_PLUGIN::GET_OUTPUT()
	------------------------
*/
inline const unsigned char *ANT_plugin::get_output() { return NULL; }

/*
	ANT_PLUGIN::GET_COUNT()
	-----------------------
*/
inline int ANT_plugin::get_count() { return 0; }

typedef ANT_plugin *maker_t();

/*
	struct ANT_PLUGIN_MAKER
	-----------------------
*/
struct ANT_plugin_maker
{
	ANT_plugin		*plugin;
	maker_t			*maker;

	void			*dlib; // dynamic library handle loaded by name
};

extern ANT_plugin_maker **plugin_factory;

#endif __PLUGIN_H__
