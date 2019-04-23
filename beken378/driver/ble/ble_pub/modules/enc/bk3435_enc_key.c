
#include <string.h>
#include "common_bt.h"

#include "prf_types.h"               // Profile common types definition
#include "architect.h"                    // Platform Definitions
#include "prf.h"
#include "rf.h"
#include "prf_utils.h"

#include "uart.h"

/** **************************************************************************************
 *
 * @file bk3435_enc_key.c
 *
 * @brief bk3435 enc module application
 *
 * @auth  alen
 *
 * @date  2017.10.13
 *
 * Copyright (C) Beken 2009-2016
 *
 *
 ****************************************************************************************
 */


#define S_SWAP(a, b)   do {uint8_t t = S[a]; S[a] = S[b]; S[b] = t;} while(0)
	


/*----------------------------------------------------------------------------*
 *  NAME
 *		InsertSort
 *  DESCRIPTION
 *		从大到小排序算法
 *  PARAMETERS
 *
 *  RETURNS
 *---------------------------------------------------------------------------*/
static void Insert_Sort( uint8_t *arr, uint8_t n )
{
	int i, j, target;

	for( i = 1; i < n; i++ )
	{
		//key为要插入的元素
		target = arr[i];

		for( j=i; j>0 && arr[j-1] < target; j-- )
		{
			//移动元素的位置.供要插入元素使用
			arr[j] = arr[j-1];
		}
		//插入需要插入的元素
		arr[j] = target;
	}
}

/*----------------------------------------------------------------------------*
 *  NAME
 *		Rc4_Skip
 *  DESCRIPTION
 *
 *  PARAMETERS
 *
 *  RETURNS
 *---------------------------------------------------------------------------*/
static int Rc4_Skip( const uint8_t *key, 
		uint32_t keylen, uint32_t skip, uint8_t *data, uint32_t data_len )
{
	uint32_t  i, j, k;
	uint8_t   S[256], *pos;
	uint32_t  kpos;
	/* Setup RC4 state */
	for( i = 0; i < 256; i++ )
	{
		S[i] = i;
	}
	j = 0;
	kpos = 0;
	for( i = 0; i < 256; i++ )
	{
		j = ( j + S[i] + key[kpos] ) & 0xff;
		kpos++;
		if( kpos >= keylen )
		{
			kpos = 0;
		}
		S_SWAP( i, j );
	}
	/* Skip the start of the stream */
	i = j = 0;
	for( k = 0; k < skip; k++ )
	{
		i = ( i + 1 ) & 0xff;
		j = ( j + S[i] ) & 0xff;
		S_SWAP( i, j );
	}
	/* Apply RC4 to data */
	pos = data;
	for( k = 0; k < data_len; k++ )
	{
		i = ( i + 1 ) & 0xff;
		j = ( j + S[i] ) & 0xff;
		S_SWAP( i, j );
		*pos++ ^= S[( S[i] + S[j] ) & 0xff];
	}
	return 0;
}



/*----------------------------------------------------------------------------*
 *  NAME
 *   generate_key
 *  DESCRIPTION
 *
 *  PARAMETERS
 *
 *  RETURNS
 *---------------------------------------------------------------------------*/
void generate_key(uint8_t *mac, uint8_t *data, uint8_t *key )
{
	uint8_t   index;
	uint8_t   tempbuf[70];
	
	memset(tempbuf, 0, 70 );
	memcpy(tempbuf, mac, 6 );
	for(index = 0; index < 6; index++)
	{
		tempbuf[index] >>= 2;
	}
	Insert_Sort(tempbuf, sizeof(tempbuf));
	Rc4_Skip(tempbuf, 16, 0, data, 16 );

	memcpy(key, &data[0], 16);
}

