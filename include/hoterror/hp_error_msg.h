/**
 * Autogenerated by HotScript (0.0.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */

#ifndef _H_hoterror_hp_error_msg_hd_TYPES
#define _H_hoterror_hp_error_msg_hd_TYPES
#include "hotpot/hp_platform.h"
#include "hotpot/hp_error_code.h"
#define	MAX_ERROR_LIST_NUM 65536
#define	MAX_ERROR_MSG_LENGTH 1024
typedef struct _HP_ERROR_MSG
{
	HP_ERROR_CODE error_code;
	hpchar error_msg[MAX_ERROR_MSG_LENGTH];
}HP_ERROR_MSG;
typedef struct _HP_ERROR_MSG_LIBRARY
{
	hpuint32 error_list_num;
	HP_ERROR_MSG error_list[MAX_ERROR_LIST_NUM];
}HP_ERROR_MSG_LIBRARY;
#endif//_H_hoterror_hp_error_msg_hd_TYPES
