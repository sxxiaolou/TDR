/**
 * Autogenerated by HotScript (0.0.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */

#ifndef _H_hotpot_hp_config_hd_TYPES
#define _H_hotpot_hp_config_hd_TYPES
#include "hotpot/hp_platform.h"
#define	HOTPOT_VERSION "0.0.1"
#define	HP_INVALID_INDEX -1
typedef enum _hpbool
{
	hptrue = 1,
	hpfalse = 0,
}hpbool;
typedef enum _HPType
{
	E_HP_INT8 = 0,
	E_HP_INT16 = 1,
	E_HP_INT32 = 2,
	E_HP_INT64 = 3,
	E_HP_UINT8 = 4,
	E_HP_UINT16 = 5,
	E_HP_UINT32 = 6,
	E_HP_UINT64 = 7,
	E_HP_CHAR = 8,
	E_HP_BYTES = 9,
	E_HP_STRING = 10,
	E_HP_DOUBLE = 11,
	E_HP_BOOL = 12,
	E_HP_NULL = 13,
	E_HP_VECTOR = 14,
	E_HP_OBJECT = 15,
}HPType;
#endif//_H_hotpot_hp_config_hd_TYPES
