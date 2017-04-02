#ifndef __HAVE_CONFIG_H
#define __HAVE_CONFIG_H

#include <stdint.h>

#define DEFINE_STRUCT_ITEM_WITH_DEFAULT(type, name, default_value) type name;
#define COLLECT_STRUCT_ITEM_NAME_WITH_DEFAULT(type, name, default_value) #name,
#define COLLECT_STRUCT_ITEM_TYPE_WITH_DEFAULT(type, name, default_value) #type,
#define COLLECT_STRUCT_ITEM_DEFAULT_VALUE(type, name, default_value) default_value,

#define DEFINE_KV_ENUM_ITEM(key, value) key = value,
#define COLLECT_KV_ENUM_ITEM_NAME(key, value) #key,
#define COLLECT_KV_ENUM_ITEM_VALUE(key, value) value,

#define CONFIG_FOREACH(ITEM)				\
	ITEM(int8_t, x_direction,		-1)		\
	ITEM(int8_t, y_direction,		1)		\
	ITEM(int8_t, scroll_direction,	-1)		\
	ITEM(int8_t, scroll_maxspeed,	5)		\
	ITEM(int8_t, mls_enabled,		0)		\
	ITEM(int8_t, reserve_1,			0)		\
	ITEM(int16_t, reserve_2,		0)		\
	ITEM(float,  scale_left,		1.0f)	\
	ITEM(float,  scale_right,		1.0f)	\
	ITEM(float,  scale_up,			1.0f)	\
	ITEM(float,  scale_down,		1.0f)	\
	/*-------------------------------------*/

struct Config
{
	CONFIG_FOREACH(DEFINE_STRUCT_ITEM_WITH_DEFAULT)
};

#define OPCODE_FOREACH(ITEM)				\
	ITEM(OPCODE_GET_CONFIG,			1)		\
	ITEM(OPCODE_SET_CONFIG,			2)		\
	ITEM(OPCODE_ECHO,				3)		\
	ITEM(OPCODE_START_TP_DUMP,		4)		\
	ITEM(OPCODE_STOP_TP_DUMP,		5)		\
	ITEM(OPCODE_TP_DATA,			6)		\
	/*-------------------------------------*/ 

enum OPCODE
{
	OPCODE_FOREACH(DEFINE_KV_ENUM_ITEM)
};

struct TrackPointDumpData
{
	uint16_t timestamp;
	uint8_t  x;
	uint8_t  y;
};


#endif		/* #ifndef __HAVE_CONFIG_H */

