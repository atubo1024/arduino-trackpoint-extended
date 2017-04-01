#include <string.h>
#include "../main.h"
#include "../serialframe.h"
#include "helper.h"

static char* CONFIG_ITEM_NAMES[]  = { CONFIG_FOREACH(COLLECT_STRUCT_ITEM_NAME_WITH_DEFAULT) };
static char* CONFIG_ITEM_TYPES[]  = { CONFIG_FOREACH(COLLECT_STRUCT_ITEM_TYPE_WITH_DEFAULT) };
static char* OPCODE_ITEM_NAMES[]  = { OPCODE_FOREACH(COLLECT_KV_ENUM_ITEM_NAME) };
static int   OPCODE_ITEM_VALUES[] = { OPCODE_FOREACH(COLLECT_KV_ENUM_ITEM_VALUE) };

int GetConfigItemCount(void)
{
	return sizeof(CONFIG_ITEM_NAMES) / sizeof(char *);
}
char** GetConfigItemNames(void)
{
	return CONFIG_ITEM_NAMES;
}
char** GetConfigItemTypes(void)
{
	return CONFIG_ITEM_TYPES;
}

int GetOpCodeItemCount(void)
{
	return sizeof(OPCODE_ITEM_NAMES) / sizeof(char *);
}
char** GetOpCodeItemNames(void)
{
	return OPCODE_ITEM_NAMES;
}
int* GetOpCodeItemValues(void)
{
	return OPCODE_ITEM_VALUES;
}

