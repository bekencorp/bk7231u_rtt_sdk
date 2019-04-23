/**
 ****************************************************************************************
 *
 * @file ecc_p256.h
 *
 * @brief ECC function definitions for P256
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#if (SECURE_CONNECTIONS)

#include "ecc_p256.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "architect.h"

#include "kernel_mem.h"
#include "common_math.h"
#include "common_utils.h"
#include "common_list.h"
#include "kernel_event.h"

#include "dbg_swdiag.h"          // Software diag

/*
 * DEFINES
 ****************************************************************************************
 */

#define __INLINE__ __INLINE


/*********************************************************************************
 *  The length of P256 numbers stored.
 *  P256 numbers are stored in uint_32 arrays, with one more element than required.
 *  The extra element is used to store overflow's and thus simplify addition, subtraction
 *
 *********************************************************************************/
#if (ECC_MULT_ALGO_TYPE == 16)
#define MAX_OCTETS256  34
#define MAX_DIGITS_256  68 // ( (9+9) * 4 )
#define ELEMENTS_BIG_HEX256  (MAX_OCTETS256/2)  // 17
#define HIGHEST_INDEX_BIG_HEX256  ELEMENTS_BIG_HEX256 - 1 // 16
#elif (ECC_MULT_ALGO_TYPE == 32)
#define MAX_OCTETS256  36
#define MAX_DIGITS_256  72 // ( (9+9) * 4 )
#define ELEMENTS_BIG_HEX256  (MAX_OCTETS256/4)  // 9
#define HIGHEST_INDEX_BIG_HEX256  (MAX_OCTETS256/4 )- 1 // 8
#else
#error "ECC Multiplication Algorithm configuration Not Supported"
#endif // (ECC_MULT_ALGO_TYPE == 16)

/*
 * MACROS
 ****************************************************************************************
 */

/**********************************************************************
 * Typedef and macro for 64 bit number handling
 **********************************************************************/

#define _LL(x) x##ll

/************************************************************************************
 * Simple Macros to determine
 *                 a/ if a 'big hex number' is even.
 *                 b/ get the most significant uint32 element
 ************************************************************************************/
#define ISBIGHEXEVEN256(x) (!((x.num[HIGHEST_INDEX_BIG_HEX256]) & 0x01))

#if (ECC_MULT_ALGO_TYPE == 16)
#define GETMOSTSIGNIGICANTU16_256(tmpHexA)  tmpHexA->num[ELEMENTS_BIG_HEX256-tmpHexA->len]
#elif (ECC_MULT_ALGO_TYPE == 32)
#define GETMOSTSIGNIGICANTU32_256(tmpHexA)  tmpHexA->num[ELEMENTS_BIG_HEX256-tmpHexA->len]
#endif // (ECC_MULT_ALGO_TYPE == 16)


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

typedef unsigned long long int u64;

typedef uint8_t  u_int8;
typedef uint16_t u_int16;
typedef uint32_t u_int32;
typedef bool boolean;

/*
 * Signed types
 */

typedef signed char s_int8;
typedef signed short s_int16;
typedef signed int s_int32;

#if (ECC_MULT_ALGO_TYPE == 16)

typedef struct bigHex256
{
    u_int16 num[MAX_OCTETS256/2];
    u_int32  len;
    u_int32  sign;
} bigHex256;

typedef struct veryBigHex256
{
    u_int16 num[MAX_OCTETS256/1];
    u_int32  len;
    u_int32  sign;
} veryBigHex256;


#elif (ECC_MULT_ALGO_TYPE == 32)
typedef struct bigHex256
{
    u_int32 num[MAX_OCTETS256/4];
    u_int32  len;
    u_int32  sign;
} bigHex256;

typedef struct veryBigHex256
{
    u_int32 num[MAX_OCTETS256/2];
    u_int32  len;
    u_int32  sign;
} veryBigHex256;

#endif // (ECC_MULT_ALGO_TYPE == 16)

typedef struct ECC_Point256
{
    bigHex256 x;
    bigHex256 y;
} ECC_Point256;

typedef struct ECC_Jacobian_Point256
{
    bigHex256 x;
    bigHex256 y;
    bigHex256 z;
} ECC_Jacobian_Point256;



/// ECC multiplication element
struct ecc_elt_tag
{
    /// List element for chaining in the ECC environment
    struct common_list_hdr hdr;

    u_int32 Point_Mul_Word256;

    // ECC points Q and R in Jacobian format.
    ECC_Jacobian_Point256 Jacobian_PointQ256;
    ECC_Jacobian_Point256 Jacobian_PointR256;

    // The Private/Secret Key used during calculations
    bigHex256 Pk256;

    /// Return message identifier
    kernel_msg_id_t msg_id;
    /// Client task identifier
    kernel_task_id_t client_id;

    #if (ECC_MULT_ALGO_TYPE == 16)
    uint16_t current_val;
    uint16_t bit_cursor;
    #elif (ECC_MULT_ALGO_TYPE == 32)
    uint32_t current_val;
    uint32_t bit_cursor;
    #endif // (ECC_MULT_ALGO_TYPE == 16)
};


/// ECC environment structure
struct ecc_env_tag
{
    /// List of ongoing multiplications
    struct common_list ongoing_mul;
};


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */


/*****************************************************************************
 *  CONSTs REQUIRED FOR ECC P256
 ****************************************************************************/

// Base Points for the ECC P256 Curve. - LSB first
const u_int8 BasePoint_x_256[32]      = {0x96,0xC2,0x98,0xD8,0x45,0x39,0xA1,0xF4,0xA0,0x33,0xEB,0x2D,0x81,0x7D,0x03,0x77,0xF2,0x40,0xA4,0x63,0xE5,0xE6,0xBC,0xF8,0x47,0x42,0x2C,0xE1,0xF2,0xD1,0x17,0x6B};
const u_int8 BasePoint_y_256[32]      = {0xF5,0x51,0xBF,0x37,0x68,0x40,0xB6,0xCB,0xCE,0x5E,0x31,0x6B,0x57,0x33,0xCE,0x2B,0x16,0x9E,0x0F,0x7C,0x4A,0xEB,0xE7,0x8E,0x9B,0x7F,0x1A,0xFE,0xE2,0x42,0xE3,0x4F};
// Maximum Secret Key Value - LSB first
const u_int8 maxSecretKey_256[32]     = {0xA8,0x92,0x31,0x7E,0x61,0xe5,0xdc,0x79,0x42,0xcf,0x8b,0xd3,0x56,0x7d,0x73,0xde,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f,0x00,0x00,0x00,0x80,0xff,0xff,0xff,0x7f};
// Debug Public Keys - used when SSP debug mode is enabled - LSB first
const u_int8 DebugE256PublicKey_x[32] = {0xE6,0x9D,0x35,0x0E,0x48,0x01,0x03,0xCC,0xDB,0xFD,0xF4,0xAC,0x11,0x91,0xF4,0xEF,0xB9,0xA5,0xF9,0xE9,0xA7,0x83,0x2C,0x5E,0x2C,0xBE,0x97,0xF2,0xD2,0x03,0xB0,0x20};
const u_int8 DebugE256PublicKey_y[32] = {0x8B,0xD2,0x89,0x15,0xD0,0x8E,0x1C,0x74,0x24,0x30,0xED,0x8F,0xC2,0x45,0x63,0x76,0x5C,0x15,0x52,0x5A,0xBF,0x9A,0x32,0x63,0x6D,0xEB,0x2A,0x65,0x49,0x9C,0x80,0xDC};
// Debug Private Key - used when SSP debug mode is enabled - LSB first
const u_int8 DebugE256SecretKey[32] =   {0xBD,0x1A,0x3C,0xCD,0xA6,0xB8,0x99,0x58,0x99,0xB7,0x40,0xEB,0x7B,0x60,0xFF,0x4A,0x50,0x3F,0x10,0xD2,0xE3,0xB3,0xC9,0x74,0x38,0x5F,0xC5,0xA3,0xD4,0xF6,0x49,0x3F};

#if (ECC_MULT_ALGO_TYPE == 16)
 // Value P which is used in mod operations to reflect points back into the Galois Field of the curve.
 const bigHex256 bigHexP256 = {{ 0x0000,0xFFFF,0xFFFF,0x0000,0x0001,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF},
                              16,0};


 const veryBigHex256 veryBigHexP256 =
                            {{  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
                                0x0000,0xFFFF,0xFFFF,0x0000,0x0001,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF},
                                16,0};

 // Point of infinity in the Jacobian system
const  ECC_Jacobian_Point256 ecc_Jacobian_InfinityPoint256 = { {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000 },0x00,0x00},
                                                                  {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0001 },0x01,0x00},
                                                                  {{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000 },0x00,0x00}};


#elif (ECC_MULT_ALGO_TYPE == 32)
 // Value P which is used in mod operations to reflect points back into the Galois Field of the curve.
const bigHex256 bigHexP256 = {{ 0x00000000,0xFFFFFFFF,0x00000001,0x00000000,0x00000000,0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},
                              8,0};
const veryBigHex256 veryBigHexP256 =
                            {{  0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
                                0x00000000,0xFFFFFFFF,0x00000001,0x00000000,0x00000000,0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},
                                8,0};

// Point of infinity in the Jacobian system
const  ECC_Jacobian_Point256 ecc_Jacobian_InfinityPoint256 = { {{0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000 },0x00,0x00},
                                                           {{0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000001 },0x01,0x00},
                                                           {{0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000 },0x00,0x00}};
#endif // (ECC_MULT_ALGO_TYPE == 16)
// local environment variable
struct ecc_env_tag ecc_env;


/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */


/****************************************************************************
 *      Prototypes for local functions
 *      These perform the bulk of the maths for ECC
 ****************************************************************************/
void AddBigHex256(const bigHex256 *bigHexA,const bigHex256 *bigHexB, bigHex256 *BigHexResult);
void SubtractBigHex256( const bigHex256 *bigHexA, const bigHex256 *bigHexB, bigHex256 *BigHexResult);
void bigHexInversion256( bigHex256* bigHexA, bigHex256* pResult);
void Add2SelfBigHex256( bigHex256 *bigHexA,const bigHex256 *bigHexB);
void AddP256( bigHex256 *bigHexA);
void SubtractBigHexMod256(const bigHex256 *bigHexA, const bigHex256 *bigHexB, bigHex256 *BigHexResult);
void SubtractFromSelfBigHex256(bigHex256 *bigHexA,const bigHex256 *bigHexB);
void SubtractFromSelfBigHexSign256( bigHex256 *bigHexA, const bigHex256 *bigHexB);
void MultiplyBigHexModP256(const bigHex256 *bigHexA,const bigHex256 *bigHexB, bigHex256 *BigHexResult);
#if (ECC_MULT_ALGO_TYPE == 16)
void MultiplyByU16ModP256(const u_int16 N,bigHex256* result);
#elif (ECC_MULT_ALGO_TYPE == 32)
void MultiplyByU32ModP256(const u_int32 N,bigHex256* result);
#endif // (ECC_MULT_ALGO_TYPE == 16)
void specialModP256(bigHex256 *tmpHexA);
void AddPdiv2_256(bigHex256 *bigHexA);

static int isValidSecretKey_256(u_int8* secretKey);

/****************************************************************************
 *      Prototypes for local inline functions
 *      There are small functions which are frequently called
 ****************************************************************************/
__INLINE__ void initBigNumber256(bigHex256 *BigHexResult);
__INLINE__ void setBigNumberLength256( bigHex256 *BigHexResult);
__INLINE__ void setVeryBigNumberLength256(veryBigHex256 *BigHexResult);
__INLINE__ void copyBigHex256(const bigHex256 *source,bigHex256 *destination);
__INLINE__ void shiftLeftOneArrayElement256(bigHex256 *input);
__INLINE__ void divideByTwo256(bigHex256* A);

__INLINE__ int isGreaterThan256(const bigHex256 *bigHexA,const bigHex256 *bigHexB);
__INLINE__ int isGreaterThanOrEqual256(const bigHex256 *bigHexA,const bigHex256 *bigHexB);
__INLINE__ int isVeryBigHexGreaterThanOrEqual256(const veryBigHex256 *bigHexA,const veryBigHex256 *bigHexB);

__INLINE__ void GF_Setup_Jacobian_Infinity_Point256(ECC_Jacobian_Point256 *infinity);
__INLINE__ void GF_Affine_To_Jacobian_Point_Copy256(const ECC_Point256 *source,ECC_Jacobian_Point256 *destination);
__INLINE__ void GF_Jacobian_Point_Copy256(const ECC_Jacobian_Point256 *source,ECC_Jacobian_Point256 *destination);




// Galois Field Operands
int GF_Jacobian_Point_Double256(const ECC_Jacobian_Point256* pPointP, ECC_Jacobian_Point256* pResultPoint);

int isValidSecretKey_256(u_int8* secretKey)
{
    // First check that the secret key is not all zeros.
    //
    int i;

    for(i=31;i>=0;i--)
    {
        if (secretKey[i] != 0)
            break;
    }

    if ((i==-1) && (secretKey[0] == 0))
        return 0;

    for(i=31;i>=0;i--)
    {
        if (secretKey[i] > maxSecretKey_256[i])
            return 0;
        else if (secretKey[i] < maxSecretKey_256[i])
            return 1;
    }
    return 1;
}

/******************************************************************************
 * Section of functions for BASIC big number handling
 *   These do not consider the Finite Field or ECC
 *
 *  Mod P is not performed
 *  Sign is not considered -- all inputs and results  are assumed to be positive
 *
 *  AddBigHex            - This adds two 15*u16 positive numbers.
 *  AddVeryBigHex        - This adds two 30*u16 positive numbers.
 *
 *  SubtractBigHex       - This subtracts two 15*u16 positive numbers
 *                         To ensure a +ive result we assume A >= B
 *  SubtractVeryBigHex   - This subtracts two 30*u16 positive numbers
 *                         To ensure a +ive result we assume A >= B
 *  MultiplyBigHex       - This multiplies two 15*u16 positive numbers to produce
 *                         a 30*u16 positive result
 *
 ******************************************************************************/

//
//
// Inputs - Format of the inputs that they begin with the LEAST significant bytes in the 0 entry of the array.
//
void AddBigHex256( const bigHex256 *bigHexA, const bigHex256 *bigHexB, bigHex256 *BigHexResult)
{
    u_int32 carry = 0;
    s_int32 i;
    #if (ECC_MULT_ALGO_TYPE == 16)
    u_int32 tmp;
    const u_int16 *numA = bigHexA->num;
    const u_int16 *numB = bigHexB->num;
    u_int16 *result = BigHexResult->num;
    #elif (ECC_MULT_ALGO_TYPE == 32)
    u64 tmp;
    const u_int32 *numA = bigHexA->num;
    const u_int32 *numB = bigHexB->num;
    u_int32 *result = BigHexResult->num;
    #endif // (ECC_MULT_ALGO_TYPE == 16)

    for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
    {
        #if (ECC_MULT_ALGO_TYPE == 16)
        tmp = (u_int32)((u_int32)(numA[i]) + (u_int32)(numB[i]) + carry);
        if(tmp & 0x10000)
            carry = 0x01;
        else
            carry = 0x00;

        result[i] = (tmp & 0xFFFF);
        #elif (ECC_MULT_ALGO_TYPE == 32)
        tmp = (u64)((u64)(numA[i]) + (u64)(numB[i]) + carry);
        if(tmp & _LL(0x100000000))
            carry = 0x01;
        else
            carry = 0x00;

        result[i] = (tmp & 0xFFFFFFFF);
        #endif // (ECC_MULT_ALGO_TYPE == 16)
    }
    setBigNumberLength256(BigHexResult);
    BigHexResult->sign = 0;
}

void AddBigHexModP256( const bigHex256 *bigHexA, const bigHex256 *bigHexB, bigHex256 *BigHexResult)
{
    AddBigHex256(bigHexA,bigHexB,BigHexResult);

    if (BigHexResult->sign == 0x00)   // Positive Number
    {
        if(isGreaterThanOrEqual256(BigHexResult,&bigHexP256))
        {
            //SubtractFromSelfBigHexSign(BigHexResult,&bigHexP);
            SubtractFromSelfBigHex256(BigHexResult, &bigHexP256);
        }
    }
    else
    {
        AddP256(BigHexResult);
    }
}
// A = A + B
void Add2SelfBigHex256( bigHex256 *bigHexA,const bigHex256 *bigHexB)
{
    #if (ECC_MULT_ALGO_TYPE == 16)
    u_int32 tmp;
    u_int16 carry = 0;
    s_int32 i;
    u_int16 *numA = bigHexA->num;
    const u_int16 *numB = bigHexB->num;

    for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
    {
        tmp = (u_int32)((u_int32)(numA[i]) + (u_int32)(numB[i]) + carry);
        if(tmp & 0x10000)
            carry = 0x01;
        else
            carry = 0x00;

        numA[i] = (tmp & 0xFFFF);
    }
    #elif (ECC_MULT_ALGO_TYPE == 32)
    u64 tmp;
    u_int32 carry = 0;
    s_int32 i;
    u_int32 *numA = bigHexA->num;
    const u_int32 *numB = bigHexB->num;

    for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
    {
        tmp = (u64)((u64)(numA[i]) + (u64)(numB[i]) + carry);
        if(tmp & _LL(0x100000000))
            carry = 0x01;
        else
            carry = 0x00;

        numA[i] = (tmp & 0xFFFFFFFF);
    }
    #endif // (ECC_MULT_ALGO_TYPE == 16)
    setBigNumberLength256(bigHexA);
}

void SubtractBigHex256(const bigHex256 *bigHexA, const bigHex256 *bigHexB, bigHex256 *BigHexResult)
{
    #if (ECC_MULT_ALGO_TYPE == 16)
    // Due to the nature of the GF we have to assume both inputs are +ive.
    u_int16 borrow=0;
    s_int32 i;
    const u_int16* pBigNum = bigHexA->num;
    const u_int16* pSmallNum = bigHexB->num;
    u_int16* pResult = BigHexResult->num;

    for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
    {
         if (((u_int32)((u_int32)(pSmallNum[i])+(u_int32)borrow)) > (pBigNum[i]))
        {
            pResult[i] = (((u_int32)(pBigNum[i] + 0x10000))- (pSmallNum[i]+borrow)) & 0xFFFF;
            borrow = 0x01;
        }
        else
        {
            pResult[i] = ((pBigNum[i])- (pSmallNum[i]+borrow)) & 0xFFFF;
            borrow = 0x00;
        }
    }
    #elif (ECC_MULT_ALGO_TYPE == 32)
    // Due to the nature of the GF we have to assume both inputs are +ive.
    u_int32 borrow=0;
    s_int32 i;
    const u_int32* pBigNum = bigHexA->num;
    const u_int32* pSmallNum = bigHexB->num;
    u_int32* pResult = BigHexResult->num;

    for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
    {
         if (((u64)((u64)(pSmallNum[i])+(u64)borrow)) > (pBigNum[i]))
        {
            pResult[i] = (((u64)(pBigNum[i] + _LL(0x100000000)))- (pSmallNum[i]+borrow)) & 0xFFFFFFFF;
            borrow = 0x01;
        }
        else
        {
            pResult[i] = ((pBigNum[i])- (pSmallNum[i]+borrow)) & 0xFFFFFFFF;
            borrow = 0x00;
        }
    }
    #endif // (ECC_MULT_ALGO_TYPE == 16)
    setBigNumberLength256(BigHexResult);
}

// A = A - B
void SubtractFromSelfBigHex256(bigHex256 *bigHexA,const bigHex256 *bigHexB)
{
    #if (ECC_MULT_ALGO_TYPE == 16)
    u_int16 borrow=0;
    s_int32 i;
    u_int16* pBigNum = bigHexA->num;
    const u_int16* pSmallNum = bigHexB->num;

    for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
    {
         if (((u_int32)((u_int32)(pSmallNum[i])+(u_int32)borrow)) > (pBigNum[i]))
        {
            pBigNum[i]  = (((u_int32)(pBigNum[i] + 0x10000))- (pSmallNum[i]+borrow)) & 0xFFFF;
            borrow = 0x01;
        }
        else
        {
            pBigNum[i] = ((pBigNum[i])- (pSmallNum[i]+borrow)) & 0xFFFF;
            borrow = 0x00;
        }
    }
    #elif (ECC_MULT_ALGO_TYPE == 32)
    // Due to the nature of the GF we have to assume both inputs are +ive.
    u_int32 borrow=0;
    s_int32 i;
    u_int32* pBigNum = bigHexA->num;
    const u_int32* pSmallNum = bigHexB->num;

    for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
    {
         if (((u64)((u64)(pSmallNum[i])+(u64)borrow)) > (pBigNum[i]))
        {
            pBigNum[i]  = (((u64)(pBigNum[i] + _LL(0x100000000)))- (pSmallNum[i]+borrow)) & 0xFFFFFFFF;
            borrow = 0x01;
        }
        else
        {
            pBigNum[i] = ((pBigNum[i])- (pSmallNum[i]+borrow)) & 0xFFFFFFFF;
            borrow = 0x00;
        }
    }
    #endif // (ECC_MULT_ALGO_TYPE == 16)
    setBigNumberLength256(bigHexA);
}

/**********************************************************************************
 * This section extends of the basic functions provided previously to support
 * sign. We concentrate on the Add and Subtract functions for both 15*u16 and 30*u16
 * numbers.
 *
 * AddBigHexSign         - Adds two 15*u16 numbers considering the sign.
 * SubtractBigHexSign    - Subtracts two 15*u16 numbers considering the sign
 * AddVeryBigHexSign
 * SubtractVeryBigHexSign
 * MultiplyBigHexSign    - Multiplies two 15*u16 numbers considering the sign.
 **********************************************************************************/

// AddP is only invoked when the sign of A is -ive
// A is always less than P
//
void AddP256( bigHex256 *bigHexA)
{
    bigHex256 BigHexResult;
    SubtractBigHex256(&bigHexP256, bigHexA,&BigHexResult);
    copyBigHex256(&BigHexResult,bigHexA);
    bigHexA->sign = 0;
}

void AddPdiv2_256( bigHex256 *bigHexA)
{
    #if (ECC_MULT_ALGO_TYPE == 16)
    u_int16 *numA = bigHexA->num;
    const u_int16 *numB = bigHexP256.num;

    if (bigHexA->sign == 0)
    {
        u_int32 tmp;
        u_int16 carry;
        s_int32 i;

        carry = 0;

        for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
        {
            tmp = (u_int32)((u_int32)(numA[i]) + (u_int32)(numB[i]) + carry);
            if(tmp & 0x10000)
                carry = 0x01;
            else
                carry = 0x00;

            numA[i] = (tmp & 0xFFFF);
        }
    }
    else // if (bigHexA->sign == 1)
    {
        if ( isGreaterThan256(bigHexA, &bigHexP256))
        {
            // Due to the nature of the GF we have to assume both inputs are +ive.
            u_int16 borrow=0;
            s_int32 i;

            for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
            {
                if (((u_int32)((u_int32)(numB[i])+(u_int32)borrow)) > (numA[i]))
                {
                    numA[i]  = (((u_int32)(numA[i] + 0x10000))- (numB[i]+borrow)) & 0xFFFF;
                    borrow = 0x01;
                }
                else
                {
                    numA[i] = ((numA[i])- (numB[i]+borrow)) & 0xFFFF;
                    borrow = 0x00;
                }
            }
        }
        else
        {
            bigHex256 BigHexResult;

            SubtractBigHex256(&bigHexP256,bigHexA,&BigHexResult);
            copyBigHex256(&BigHexResult,bigHexA);
            bigHexA->sign = 0;
        }
    }
    // divide by 2
    // divideByTwo(bigHexA);
    {
        u_int16 rem=0;
        u_int32 val;
        u_int32 i;

        for(i=0; i < (ELEMENTS_BIG_HEX256); i++)
        {
            val = (u_int32)((u_int32)(numA[i]) + ((u_int32)rem * 0x10000));

            numA[i] = (u_int16)(val>>1);
            rem = (u_int32)(val - ((val>>1)<<1));
        }

        // We need to re-calculate the length.
        setBigNumberLength256(bigHexA);
    }
    #elif (ECC_MULT_ALGO_TYPE == 32)
    u_int32 *numA = bigHexA->num;
    const u_int32 *numB = bigHexP256.num;

    if (bigHexA->sign == 0)
    {
        u64 tmp;
        u_int32 carry;
        s_int32 i;

        carry = 0;

        for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
        {
            tmp = (u64)((u64)(numA[i]) + (u64)(numB[i]) + carry);
            if(tmp & _LL(0x100000000))
                carry = 0x01;
            else
                carry = 0x00;

            numA[i] = (tmp & 0xFFFFFFFF);
        }
    }
    else // if (bigHexA->sign == 1)
    {
        if ( isGreaterThan256(bigHexA, &bigHexP256))
        {

            // Due to the nature of the GF we have to assume both inputs are +ive.
            u_int32 borrow=0;
            s_int32 i;

            for (i=(ELEMENTS_BIG_HEX256-1);i >= 0; i--)
            {
                if (((u64)((u64)(numB[i])+(u64)borrow)) > (numA[i]))
                {
                    numA[i]  = (((u64)(numA[i] + _LL(0x100000000)))- (numB[i]+borrow)) & 0xFFFFFFFF;
                    borrow = 0x01;
                }
                else
                {
                    numA[i] = ((numA[i])- (numB[i]+borrow)) & 0xFFFFFFFF;
                    borrow = 0x00;
                }
            }
        }
        else
        {
            bigHex256 BigHexResult;

            SubtractBigHex256(&bigHexP256,bigHexA,&BigHexResult);
            copyBigHex256(&BigHexResult,bigHexA);
            bigHexA->sign = 0;
        }
    }
    // divide by 2
    // divideByTwo(bigHexA);
    {
        u_int32 rem=0;
        u64 u64val;
        u_int32 i;

        for(i=0; i < (ELEMENTS_BIG_HEX256); i++)
        {
            u64val = (u64)((u64)(numA[i]) + ((u64)rem * _LL(0x100000000)));

            numA[i] = (u_int32)(u64val>>1);
            rem = (u64)(u64val - ((u64val>>1)<<1));
        }

        // We need to re-calculate the length.
        setBigNumberLength256(bigHexA);
    }
    #endif // (ECC_MULT_ALGO_TYPE == 16)
}

void SubtractFromSelfBigHexSign256( bigHex256 *bigHexA, const bigHex256 *bigHexB)
{
    // This function uses use the basis AddBigHex and SubtractBigHex to implement a full
    // Addition which considers sign and mod.
    //
    if (bigHexA->sign == 0)
    {
        if (bigHexB->sign == 0)
        {
            if ( isGreaterThanOrEqual256(bigHexA,bigHexB))
            {
                SubtractFromSelfBigHex256(bigHexA, bigHexB);
            }
            else
            {
                bigHex256 BigHexResult;

                SubtractBigHex256(bigHexB, bigHexA,&BigHexResult);
                copyBigHex256(&BigHexResult,bigHexA);
                bigHexA->sign = 1;
            }
        }
        else
        {
            // 3/  if A is + and B is -   C =   A+B  Mod
            Add2SelfBigHex256(bigHexA, bigHexB);
        }
    }
    else  // if (bigHexA->sign == 1)
    {
        if (bigHexB->sign == 0)
        {
            Add2SelfBigHex256(bigHexA, bigHexB);
        }
        else
        {
            if ( isGreaterThanOrEqual256(bigHexB,bigHexA))
            {
                bigHex256 BigHexResult;

                SubtractBigHex256(bigHexB, bigHexA,&BigHexResult);
                copyBigHex256(&BigHexResult,bigHexA);
                bigHexA->sign = 0;
            }
            else
            {
                SubtractFromSelfBigHex256(bigHexA, bigHexB);
            }
        }
    }
}
/*****************************************************************************
 * Following functions further extend on the basic functions to include Mod P.
 *
 * AddBigHexMod  -- This function takes A and B which can be signed and output
 *                  a result C which is less than P.
 *                  It call AddBigHexSign and then perform Mod P on the output.
 *                  If the output is -ive it recursively add P until we get a +ive
 *                  number which is less than P.
 *                  If the output is +ive it recursively subtracts P until we get
 *                  a +number which is less than P.
 *
 * SubtractBigHexMod  -- This function takes A and B which can be signed and output
 *                  a result C which is less than P.
 *                  It calls SubtractBigHexSign and then performs Mod P on the output.
 *                  If the output is -ive it recursively add P until we get a +ive
 *                  number which is less than P.
 *                  If the output is +ive it recursively subtracts P until we get
 *                  a +number which is less than P.
 *
 * MultiplyBigHexMod
 ****************************************************************************/
void SubtractBigHexMod256(const bigHex256 *bigHexA, const bigHex256 *bigHexB, bigHex256 *BigHexResult)
{
    if (bigHexA->sign == 0)
    {
        if (bigHexB->sign == 0)
        {
            if ( isGreaterThanOrEqual256(bigHexA,bigHexB))
            {
                SubtractBigHex256(bigHexA, bigHexB,BigHexResult);
                BigHexResult->sign = 0;
            }
            else
            {
                SubtractBigHex256(bigHexB, bigHexA,BigHexResult);
                BigHexResult->sign = 1;
            }
        }
        else
        {
            // 3/  if A is + and B is -   C =   A+B  Mod
            AddBigHex256(bigHexA, bigHexB,BigHexResult);
            BigHexResult->sign = 0;
        }
    }
    else  // if (bigHexA->sign == 1)
    {
        if (bigHexB->sign == 0)
        {
            AddBigHex256(bigHexA, bigHexB,BigHexResult);
            BigHexResult->sign = 1;
        }
        else
        {
            if ( isGreaterThanOrEqual256(bigHexB,bigHexA))
            {
                SubtractBigHex256(bigHexB, bigHexA,BigHexResult);
                BigHexResult->sign = 0;
            }
            else
            {
                SubtractBigHex256(bigHexA, bigHexB,BigHexResult);
                BigHexResult->sign = 1;
            }
        }
    }

    if (BigHexResult->sign == 0x00)   // Positive Number
    {
        if(isGreaterThanOrEqual256(BigHexResult,&bigHexP256))
        {
            //SubtractFromSelfBigHexSign(BigHexResult,&bigHexP);
            SubtractFromSelfBigHex256(BigHexResult, &bigHexP256);
        }
    }
    else
    {
        AddP256(BigHexResult);
    }
}

// From curren t usage we can safely assume both inputs are positive.

void SubtractBigHexUint32_256(const bigHex256 *bigHexA, const u_int32 numB, bigHex256 *BigHexResult)
{
    initBigNumber256(BigHexResult);

    if (bigHexA->num[HIGHEST_INDEX_BIG_HEX256] >= numB)
    {
        copyBigHex256(bigHexA,BigHexResult);
        BigHexResult->num[ELEMENTS_BIG_HEX256-1] = bigHexA->num[ELEMENTS_BIG_HEX256-1]-numB;
    }
    else
    {
        bigHex256 N;

        initBigNumber256(&N);

        N.len = 1;
        N.num[HIGHEST_INDEX_BIG_HEX256] = numB;
        SubtractBigHexMod256(bigHexA,&N,BigHexResult);
    }
}


void MultiplyBigHexModP256(const bigHex256 *bigHexA, const bigHex256 *bigHexB, bigHex256 *BigHexResult)
{
    veryBigHex256 tmpResult;
    {
        s_int32 k;
        #if (ECC_MULT_ALGO_TYPE == 16)
        const u_int16 *numA = bigHexA->num;
        const u_int16 *numB = bigHexB->num;
        u_int16 *result = tmpResult.num;
        #elif (ECC_MULT_ALGO_TYPE == 32)
        const u_int32 *numA = bigHexA->num;
        const u_int32 *numB = bigHexB->num;
        u_int32 *result = tmpResult.num;
        #endif // (ECC_MULT_ALGO_TYPE == 16)
        memset(tmpResult.num,0,sizeof(tmpResult.num));
        tmpResult.len = 0;
        tmpResult.sign = 0;
        //
        // Below Optimisations seem to degrade performance on Windows by a factor of 2.
        //
        for(k=(ELEMENTS_BIG_HEX256-1);k >= 0;k--)
        {
            u_int32 mcarry = 0;
            s_int32 j = 0;

            // The inner loop multiplies each byte of HexB by a single byte of
            // HexA
            for(j=(ELEMENTS_BIG_HEX256-1);j >=0;j--)
            {
                #if (ECC_MULT_ALGO_TYPE == 16)
                u_int32 val;

                val = (((u_int32)(numA[k]) ) * ((u_int32)(numB[j]))) + result[j+k+1] + mcarry;
                result[j+k+1] = (val & 0xFFFF);
                mcarry = (u_int16)( val >> 16);
                #elif (ECC_MULT_ALGO_TYPE == 32)
                u64 val;

                val = (((u64)(numA[k]) ) * ((u64)(numB[j]))) + result[j+k+1] + mcarry;
                result[j+k+1] = (val & 0xFFFFFFFF);
                mcarry = (u_int32)( val >> 32);
                #endif // (ECC_MULT_ALGO_TYPE == 16)
            }
        }
        setVeryBigNumberLength256(&tmpResult);
        tmpResult.sign = (bigHexA->sign != bigHexB->sign);
    }

    {
        u_int32 i=0;
        bigHex256 tmpResult2;

        while ((tmpResult.num[i] == 0x00) && (i<(ELEMENTS_BIG_HEX256)))
            i++;

        // Take the next 13 elements
        //
        tmpResult2.sign = tmpResult.sign;

        if (isVeryBigHexGreaterThanOrEqual256(&tmpResult,&veryBigHexP256))
        {
            while(tmpResult.num[i] == 0x00)
                i++;
        }
        memcpy(tmpResult2.num,tmpResult.num+i,sizeof(tmpResult2.num));
        setBigNumberLength256(&tmpResult2);

        #if (ECC_MULT_ALGO_TYPE == 16)
        while((i+ELEMENTS_BIG_HEX256) < (MAX_OCTETS256))
        #elif (ECC_MULT_ALGO_TYPE == 32)
        while((i+ELEMENTS_BIG_HEX256) < (MAX_OCTETS256/2))
        #endif // (ECC_MULT_ALGO_TYPE == 16)
        {
            specialModP256(&tmpResult2);
            shiftLeftOneArrayElement256(&tmpResult2);

            // Add the next byte from A in left_over;
            tmpResult2.num[HIGHEST_INDEX_BIG_HEX256] = tmpResult.num[i+ELEMENTS_BIG_HEX256];
            i++;
            setBigNumberLength256(&tmpResult2);
        }

        specialModP256(&tmpResult2);
        copyBigHex256(&tmpResult2,BigHexResult);
    }
}


void MultiplyBigHexByUint32_256(const bigHex256 *bigHexA, const u_int32 numB, bigHex256 *BigHexResult)
{
    #if (ECC_MULT_ALGO_TYPE == 16)
    s_int32 k;
    const u_int16 *numA = bigHexA->num;
    u_int16 *result = BigHexResult->num;
    u_int16 mcarry = 0;

    //
    // Below Optimisations seem to degrade performance on Windows by a factor of 2.
    //
    for(k=HIGHEST_INDEX_BIG_HEX256;k >= 0;k--)
    {
        u_int32 val;

        val = (((u_int32)(numA[k]) ) * ((u_int32)numB)) + mcarry;
        result[k] = (val & 0xFFFF);
        mcarry = (u_int16)( val >> 16);

    }
    #elif (ECC_MULT_ALGO_TYPE == 32)
    s_int32 k;
    const u_int32 *numA = bigHexA->num;
    u_int32 *result = BigHexResult->num;
    u_int32 mcarry = 0;

    //
    // Below Optimisations seem to degrade performance on Windows by a factor of 2.
    //
    for(k=HIGHEST_INDEX_BIG_HEX256;k >= 0;k--)
    {
        u64 val;

        val = (((u64)(numA[k]) ) * ((u64)numB)) + mcarry;
        result[k] = (val & 0xFFFFFFFF);
        mcarry = (u_int32)( val >> 32);
    }
    #endif // (ECC_MULT_ALGO_TYPE == 16)
    setBigNumberLength256(BigHexResult);
    BigHexResult->sign = bigHexA->sign;
    specialModP256(BigHexResult);
}


__INLINE__ void shiftLeftOneArrayElement256(bigHex256 *input)
{
    memcpy(input->num,(input->num+1), (8*sizeof(u_int32)));
}

//
// This function adds N * Mod P to a number.
//
// The left_over = tmpHexA % P

void specialModP256(bigHex256 *tmpHexA)
{
    bigHex256 NModP;
    #if (ECC_MULT_ALGO_TYPE == 16)
    u_int16 SignificantU16;
    #elif (ECC_MULT_ALGO_TYPE == 32)
    u_int32 SignificantU32;
    #endif // (ECC_MULT_ALGO_TYPE == 16)

   initBigNumber256(&NModP);
   if(((tmpHexA->sign == 0) && isGreaterThanOrEqual256(tmpHexA,&bigHexP256)) ||
       (tmpHexA->sign == 1))
   {
       // If I multiply the most significant u_int16 of A by the Mod P and then subtract from A
       // this is equivalent to an iteractive subtraction - but much faster.
       if (tmpHexA->len > bigHexP256.len)
       {
           // approach
            #if (ECC_MULT_ALGO_TYPE == 16)
           SignificantU16 = GETMOSTSIGNIGICANTU16_256(tmpHexA);
           MultiplyByU16ModP256(SignificantU16,&NModP);
            #elif (ECC_MULT_ALGO_TYPE == 32)
           SignificantU32 = GETMOSTSIGNIGICANTU32_256(tmpHexA);
           MultiplyByU32ModP256(SignificantU32,&NModP);
            #endif // (ECC_MULT_ALGO_TYPE == 16)
           //tmpHexA->num[0] = 0x00;

           if (tmpHexA->sign == 0)
           {
               SubtractFromSelfBigHexSign256(tmpHexA,&NModP);
           }
           else
           {
               // Change sign of A to +ive
               // Perform Subtraction
               // Change sign of A back to -ive
               tmpHexA->sign = 0;
               SubtractFromSelfBigHexSign256(tmpHexA,&NModP);
               tmpHexA->sign = 1;
           }
       }


       if (((tmpHexA->sign == 0) && isGreaterThanOrEqual256(tmpHexA,&bigHexP256)) ||
            (tmpHexA->sign == 1))
       {
         // It is implicit that the sign is negative.
         // so we can remove the check.
         //
           if (tmpHexA->sign == 1)
           {
               AddP256(tmpHexA);
           }
           else
           {
               // Can this be replaced by Adding the invert of P
               SubtractFromSelfBigHex256(tmpHexA, &bigHexP256);
           }
       }
   }
}
#if (ECC_MULT_ALGO_TYPE == 32)
void MultiplyByU32ModP256(const u_int32 N,bigHex256* result)
{
    u_int32 tempVal = (0xFFFFFFFF - N) + 1;

    result->num[0] = N - 1;
    result->num[1] = (u_int32)tempVal;
    result->num[2] = N ;
    result->num[3] = 0x00 ;
    result->num[4] = 0x00 ;
    result->num[5] = N-1;
    result->num[6] = 0xFFFFFFFF;
    result->num[7] = 0xFFFFFFFF;
    result->num[8] = (u_int32)tempVal;

    result->sign = 0x00;
    if (result->num[0])
        result->len = 0x9;
    else
        result->len = 0x8;
}
#endif

#if (ECC_MULT_ALGO_TYPE == 16)
void MultiplyByU16ModP256(const u_int16 N,bigHex256* result)
{

    result->num[0] = N-1;
    result->num[1] = 0xFFFF;
    result->num[2] = 0xFFFF - (N-1);
    result->num[3] = 0;
    result->num[4] = N;
    result->num[5] = 0;
    result->num[6] = 0;
    result->num[7] = 0;
    result->num[8] = 0;
    result->num[9] = 0;
    result->num[10] = N-1;
    result->num[11] = 0xFFFF;
    result->num[12] = 0xFFFF;
    result->num[13] = 0xFFFF;
    result->num[14] = 0xFFFF;
    result->num[15] = 0xFFFF;
    result->num[16] = 0xFFFF - (N-1);

    result->sign = 0x00;
    if (result->num[0])
        result->len = 17;
    else
        result->len = 16;
}
#endif // (ECC_MULT_ALGO_TYPE == 16)

__INLINE__ void copyBigHex256(const bigHex256 *source,bigHex256 *destination)
{
    memcpy(destination->num,source->num,sizeof(destination->num)); //for P256 max_octets = 36

    destination->len = source->len;
    destination->sign = source->sign;
}

__INLINE__ void initBigNumber256(bigHex256 *BigHexResult)
{
    memset(BigHexResult->num,0,sizeof(BigHexResult->num));

    BigHexResult->len = 0;
    BigHexResult->sign = 0;
}

__INLINE__ void setBigNumberLength256(bigHex256 *BigHexResult)
{
    int i;

    for(i=0;i<(ELEMENTS_BIG_HEX256);i++)
    {
        if(BigHexResult->num[i] != 0x00)
           break;
    }
    BigHexResult->len = (ELEMENTS_BIG_HEX256)-i;
}

__INLINE__ void setVeryBigNumberLength256(veryBigHex256 *BigHexResult)
{
    int i;

    #if (ECC_MULT_ALGO_TYPE == 16)
    for(i=0;i<(MAX_OCTETS256);i++)
    #elif (ECC_MULT_ALGO_TYPE == 32)
    for(i=0;i<(MAX_OCTETS256/2);i++)
    #endif // (ECC_MULT_ALGO_TYPE == 16)
    {
        if(BigHexResult->num[i] != 0x00)
           break;
    }
    #if (ECC_MULT_ALGO_TYPE == 16)
    BigHexResult->len = (MAX_OCTETS256)-i;
    #elif (ECC_MULT_ALGO_TYPE == 32)
    BigHexResult->len = (MAX_OCTETS256/2)-i;
    #endif // (ECC_MULT_ALGO_TYPE == 16)
}

//
// if A > B return TRUE
//

__INLINE__ int isGreaterThan256(const bigHex256 *bigHexA,const bigHex256 *bigHexB)
{

   u_int32 i;
   u_int32 A_len = bigHexA->len;
   u_int32 B_len = bigHexB->len;

    if(A_len > B_len)
        return 1;
    else if(A_len < B_len)
        return 0;

    // Now we have two equal sized arrays which we need to compare.
    // .
    for (i=((ELEMENTS_BIG_HEX256)-A_len); i <(ELEMENTS_BIG_HEX256);i++)
    {
        if (bigHexB->num[i] > bigHexA->num[i])
        {
            return 0;
        }
        else if (bigHexB->num[i] < bigHexA->num[i])
        {
            return 1;
        }
    }
    return 0;
}


__INLINE__ int isGreaterThanOrEqual256(const bigHex256 *bigHexA,const bigHex256 *bigHexB)
{
   u_int32 i;
   u_int32 A_len = bigHexA->len;
   u_int32 B_len = bigHexB->len;

    if(A_len > B_len)
        return 1;
    else if(A_len < B_len)
        return 0;

    // Now we have two equal sized arrays which we need to compare.
    // .

    for (i=((ELEMENTS_BIG_HEX256)-A_len); i <(ELEMENTS_BIG_HEX256);i++)
    {
        if (bigHexB->num[i] > bigHexA->num[i])
        {
            return 0;
        }
        else if (bigHexB->num[i] < bigHexA->num[i])
        {
            return 1;
        }
    }
    return 1;
}


__INLINE__ int isVeryBigHexGreaterThanOrEqual256(const veryBigHex256 *bigHexA,const veryBigHex256 *bigHexB)
{
    int i;
    u_int32 A_len = bigHexA->len;
    u_int32 B_len = bigHexB->len;

    if(A_len > B_len)
        return 1;
    else if(A_len < B_len)
        return 0;

    // Now we have two equal sized arrays which we need to compare.
    //
    #if (ECC_MULT_ALGO_TYPE == 16)
    for (i=((MAX_OCTETS256)-A_len); i <(MAX_OCTETS256);i++)
    #elif (ECC_MULT_ALGO_TYPE == 32)
    for (i=((MAX_OCTETS256/2)-A_len); i <(MAX_OCTETS256/2);i++)
    #endif // (ECC_MULT_ALGO_TYPE == 16)
    {

        if (bigHexB->num[i] != bigHexA->num[i])
        {
            return (bigHexB->num[i] < bigHexA->num[i]);
        }

    }
    return 1;
}


/*__INLINE__*/ int notEqual256(const bigHex256 *bigHexA,const bigHex256 *bigHexB)
{
    u_int32 i;

    for(i=0;i<(ELEMENTS_BIG_HEX256);i++)
    {
        if(bigHexA->num[i] != bigHexB->num[i])
        {
            return 1;
        }
    }
    return 0;
}

__INLINE__ int Is_Infinite256(const ECC_Jacobian_Point256* pPointP)
{
    return ( (!notEqual256(&pPointP->x,&ecc_Jacobian_InfinityPoint256.x)) && (!notEqual256(&pPointP->y,&ecc_Jacobian_InfinityPoint256.y)) && (!notEqual256(&pPointP->z,&ecc_Jacobian_InfinityPoint256.z)));
}

//isLessThan
__INLINE__ int isLessThan256(const bigHex256 *bigHexA,const bigHex256 *bigHexB)
{
    u_int32 i;

    if(bigHexA->len < bigHexB->len)
        return 1;
    else if(bigHexA->len > bigHexB->len)
        return 0;

    // Now we have two equal sized arrays which we need to compare.
    // Start at the last entry is each array and compare.

    for (i=bigHexA->len; i!=0;i--)
    {
        if (bigHexB->num[i] < bigHexA->num[i])
            return 0;
            break;
    }
    return 1;
}

/*********** JACOBIAN ******/
int GF_Jacobian_Point_Addition256(const ECC_Jacobian_Point256* pPointP,const ECC_Jacobian_Point256* pPointQ, ECC_Jacobian_Point256* pResultPoint)
{

// This should be 12 Multiplications and 2 Squares
// currently implemented as 12 Muls and 2 Squares and one cube
//  Let (X1, Y1, Z1) and (X2, Y2, Z2) be two points (both unequal to the 'point at infinity') represented in 'Standard Projective Coordinates'. Then the sum (X3, Y3, Z3) can be calculated by
//
//  U1 = Y2*Z1
//  U2 = Y1*Z2
//  V1 = X2*Z1
//  V2 = X1*Z2
//  if (V1 == V2)
//     if (U1 != U2)
//         return POINT_AT_INFINITY
//      else
//         return POINT_DOUBLE(X1, Y1, Z1)
// U = U1 - U2
// V = V1 - V2
// W = Z1*Z2

// A = U^2*W - V^3 - 2*V^2*V2
// X3 = V*A
// Y3 = U*(V^2*V2 - A) - V^3*U2
// Z3 = V^3*W
//  return (X3, Y3, Z3)

    bigHex256 jac_U1;
    bigHex256 jac_U2;
    bigHex256 jac_V1;
    bigHex256 jac_V2;
    bigHex256 jac_U;
    bigHex256 jac_V;
    bigHex256 jac_A;
    bigHex256 jac_W;
    bigHex256 V_sqr;
    bigHex256 V_cube;
    bigHex256 U_sqr;
     bigHex256 V_sqr_mult_by_V2;

    const bigHex256* pJac_PointX1 = &pPointP->x;
    const bigHex256* pJac_PointY1 = &pPointP->y;
    const bigHex256* pJac_PointZ1 = &pPointP->z;

    const bigHex256* pJac_PointX2 = &pPointQ->x;
    const bigHex256* pJac_PointY2 = &pPointQ->y;
    const bigHex256* pJac_PointZ2 = &pPointQ->z;

    initBigNumber256(&pResultPoint->x);
    initBigNumber256(&pResultPoint->y);
    initBigNumber256(&pResultPoint->z);

    initBigNumber256(&jac_U1);
    initBigNumber256(&jac_U2);
    initBigNumber256(&jac_V1);
    initBigNumber256(&jac_V2);

    initBigNumber256(&jac_U);
    initBigNumber256(&jac_V);
    initBigNumber256(&jac_A);
    initBigNumber256(&jac_W);

    initBigNumber256(&V_sqr);
    initBigNumber256(&V_cube);
    initBigNumber256(&U_sqr);
    initBigNumber256(&V_sqr_mult_by_V2);


    // First Handle points of infinity
/*
P1 and P2 are both infinite: P3=infinite.
P1 is infinite: P3=P2.
P2 is infinite: P3=P1.
P1 and P2 have the same x coordinate, but different y coordinates or both y coordinates equal 0: P3=infinite.

*/
    if (Is_Infinite256(pPointP))
    {
        if(Is_Infinite256(pPointQ))
        {
            // Result = Infinity
            //pResultPoint = Infinity
            GF_Jacobian_Point_Copy256(&ecc_Jacobian_InfinityPoint256,pResultPoint);
            return 0;
        }
        else
        {
            // Result = pPointQ
            // pResultPoint = pPointQ
            GF_Jacobian_Point_Copy256(pPointQ,pResultPoint);
            return 0;
        }
    }
    else if (Is_Infinite256(pPointQ))
    {
        // Result = pPointP
        // pResultPoint = pPointP;
        GF_Jacobian_Point_Copy256(pPointP,pResultPoint);
        return 0;
    }

    if (!notEqual256(&pPointQ->x,&pPointP->x)) // Same X coordinate
    {
        if (notEqual256(&pPointQ->y,&pPointP->y)) // Different Y coordinates
        {
            // Result = Infinity
            GF_Jacobian_Point_Copy256(&ecc_Jacobian_InfinityPoint256,pResultPoint);
            return 0;

        }
        else if ((pPointQ->y.len <= 1) && (pPointQ->y.num[HIGHEST_INDEX_BIG_HEX256] == 0)) // Y co-ords = 0
        {
                    // Result = Infinity
            GF_Jacobian_Point_Copy256(&ecc_Jacobian_InfinityPoint256,pResultPoint);
            return 0;
        }
    }

//  U1 = Y2*Z1
//  U2 = Y1*Z2
//  V1 = X2*Z1
//  V2 = X1*Z2

    MultiplyBigHexModP256(pJac_PointY2,pJac_PointZ1,&jac_U1);
    MultiplyBigHexModP256(pJac_PointY1,pJac_PointZ2,&jac_U2);
    MultiplyBigHexModP256(pJac_PointX2,pJac_PointZ1,&jac_V1);
    MultiplyBigHexModP256(pJac_PointX1,pJac_PointZ2,&jac_V2);

//  if (V1 == V2)
//     if (U1 != U2)
//         return POINT_AT_INFINITY
//      else
//         return POINT_DOUBLE(X1, Y1, Z1)

    if (!notEqual256(&jac_V1,&jac_V2))
    {
        if (notEqual256(&jac_U1,&jac_U2))
        {
            // Result = Infinity
            GF_Jacobian_Point_Copy256(&ecc_Jacobian_InfinityPoint256,pResultPoint);
            return 0;
        }
        else
        {
            GF_Jacobian_Point_Double256(pPointP,pResultPoint);
            return 0; //POINT_DOUBLE(&jac_PointX1,&jac_PointY1,&jac_PointZ1);
        }
    }

// U = U1 - U2
// V = V1 - V2
// W = Z1*Z2   ( 5 muls to here )
// A = U^2*W - V^3 - 2*V^2*V2

    SubtractBigHexMod256(&(jac_U1),&(jac_U2),&jac_U);
    SubtractBigHexMod256(&(jac_V1),&(jac_V2),&jac_V);

    MultiplyBigHexModP256(pJac_PointZ1,pJac_PointZ2,&jac_W);

    {
        // Determine A
        // A = U^2*W - V^3 - 2*V^2*V2
        // V_sqr = V^2
        // V_cube = V^3
        // U_sqr = U^2
        // int1 = V_sqr * V2
        // int2 = 2 * int1  --  (2*V^2*V2)
        // int3 = U_sqr * W
        // int4 = int3 - V_cube
        // A = int4 - int2

        // X3 = D^2 - int3

        bigHex256 int1;
        bigHex256 int3;
        bigHex256 int4;
        bigHex256 Double_V_sqr_mult_by_V2;

        initBigNumber256(&int1);
        initBigNumber256(&int3);
        initBigNumber256(&int4);
        initBigNumber256(&Double_V_sqr_mult_by_V2);

        MultiplyBigHexModP256(&jac_V,&jac_V,&V_sqr);
        MultiplyBigHexModP256(&V_sqr,&jac_V,&V_cube);
        MultiplyBigHexModP256(&jac_U,&jac_U,&U_sqr);

        MultiplyBigHexModP256(&V_sqr,&jac_V2,&V_sqr_mult_by_V2);  // V^2 * V2

        MultiplyBigHexByUint32_256(&V_sqr_mult_by_V2,0x02,&Double_V_sqr_mult_by_V2); // 2 * V^2 * V2
        MultiplyBigHexModP256(&U_sqr,&jac_W,&int3);    // U^2 * W
        SubtractBigHexMod256(&int3,&V_cube,&int4);   // (U^2 * W) - V^3
        SubtractBigHexMod256(&int4,&Double_V_sqr_mult_by_V2,&jac_A);  // A = U^2*W - V^3 - 2*V^2*V2
    }

    // Determine X3
    {
        // X3 = V*A

        MultiplyBigHexModP256(&jac_V,&jac_A,&(pResultPoint->x));
    }
    // Determine Y3
    {
       // Y3 = U*(V^2*V2 - A) - V^3*U2
       // int1 = (V^2*V2 - A)
       // int2 = U*(V^2*V2 - A)
       // int3 =  V^3*U2
        bigHex256 int1;
        bigHex256 int2;
        bigHex256 int3;

        initBigNumber256(&int1);
        initBigNumber256(&int2);
        initBigNumber256(&int3);

        SubtractBigHexMod256(&V_sqr_mult_by_V2,&jac_A,&int1); // int1 =( V^2*V2 - A)
        // Below line was incorrect

         MultiplyBigHexModP256(&jac_U,&int1,&int2);  // int2 = U * ( V^2*V2 - A)
        //
        MultiplyBigHexModP256(&V_cube,&jac_U2,&int3); // int3 = V^3 * U2
        SubtractBigHexMod256(&int2,&int3,&(pResultPoint->y));
    }


    // Determine Z3
    {
        // Z3 = V^3*W
        MultiplyBigHexModP256(&V_cube,&jac_W,&(pResultPoint->z));
    }

    return 1;
}

int GF_Jacobian_Point_Double256(const ECC_Jacobian_Point256* pPointP, ECC_Jacobian_Point256* pResultPoint)
{
 //if (Y == 0)
 // return POINT_AT_INFINITY
 //W = a*Z^2 + 3*X^2
 // if a = -3, then W can also be calculated as W = 3*(X + Z)*(X - Z), saving 2 field squarings
 // THUS
 //W = 3*(X+Z)*(X-Z)
 //S = Y*Z
 //B = X*Y*S
 //H = W^2 - 8*B
 //X' = 2*H*S
 //Y' = W*(4*B - H) - 8*Y^2*S^2
 //Z' = 8*S^3
 //return (X', Y', Z')

    const bigHex256* pJac_PointX1 = &pPointP->x;
    const bigHex256* pJac_PointY1 = &pPointP->y;
    const bigHex256* pJac_PointZ1 = &pPointP->z;

    bigHex256 W;
    bigHex256 S;
    bigHex256 B;
    bigHex256 H;
    bigHex256 S_sqr;

    if (((pJac_PointY1->len <= 1) && (pJac_PointY1->num[HIGHEST_INDEX_BIG_HEX256] == 0x00)) ||
    	Is_Infinite256(pPointP))
    {
        // Point at Infinity
        // pResultPoint = Infinity
        GF_Jacobian_Point_Copy256(&ecc_Jacobian_InfinityPoint256,pResultPoint);
        return 0;
    }

    // Determine W
    {
        bigHex256 X_plus_Z;
        bigHex256 X_minus_Z;
        bigHex256 int1;

        // Need to convert below to AddBigHexModP to consider sign and mod..
        AddBigHexModP256(pJac_PointX1,pJac_PointZ1,&X_plus_Z);
        SubtractBigHexMod256(pJac_PointX1,pJac_PointZ1,&X_minus_Z);
        MultiplyBigHexModP256(&X_plus_Z,&X_minus_Z,&int1);
        MultiplyBigHexByUint32_256(&(int1),0x03,&W);
    }
    // Determine S
    {
        MultiplyBigHexModP256(pJac_PointY1,pJac_PointZ1,&S);
    }
    // Determine B
    {
        bigHex256 int1;

        //B = X*Y*S
        MultiplyBigHexModP256(pJac_PointY1,&S,&int1);
        MultiplyBigHexModP256(pJac_PointX1,&int1,&B);
    }

    // Determine H
    {
        //H = W^2 - 8*B
         bigHex256 W_sqr;
         bigHex256 Eight_B;

         MultiplyBigHexModP256(&W,&W,&W_sqr);
        MultiplyBigHexByUint32_256(&B,8,&Eight_B);

        SubtractBigHexMod256(&W_sqr,&Eight_B,&H);
    }
    // Determine X
    //X = 2*H*S
    {
        bigHex256 int1;

        MultiplyBigHexModP256(&H,&S,&int1);
        MultiplyBigHexByUint32_256((const bigHex256*)(&int1),0x02,&pResultPoint->x);

    }
    // Determine Y
    // Y' = W*(4*B - H) - 8*Y^2*S^2

    {
        bigHex256 Y_sqr;
        bigHex256 int1;
        bigHex256 int2; // 8*Y^2*S^2
        bigHex256 int3;
        bigHex256 int4;
        bigHex256 int5;

        // 8*Y^2*S^2
        MultiplyBigHexModP256(pJac_PointY1,pJac_PointY1,&Y_sqr);
        MultiplyBigHexModP256(&S,&S,&S_sqr);
        MultiplyBigHexModP256(&Y_sqr,&S_sqr,&int1);
        MultiplyBigHexByUint32_256(&(int1),0x08,&int2);

        // 4 * B
        MultiplyBigHexByUint32_256(&B,0x04,&int3);
        // (4*B - H)
        SubtractBigHexMod256(&int3,&H,&int4);
        // W * (4*B - H)
        MultiplyBigHexModP256(&W,&int4,&int5);

        SubtractBigHexMod256((const bigHex256*)&int5,(const bigHex256*)&int2,&pResultPoint->y);

    }

    // Determine Z
    {
          //Z1 = 8*S^3
          bigHex256 int1;

          MultiplyBigHexModP256(&S_sqr,&S,&int1);
          MultiplyBigHexByUint32_256(&int1,0x08,&pResultPoint->z);
    }
    return 0;
}

void GF_Point_Jacobian_To_Affine256(ECC_Jacobian_Point256* pJacPoint,bigHex256* pX_common_ord_affine,bigHex256* pY_common_ord_affine )
{
    bigHex256 inverted_Z_jacobian;

    // x_affine = X_jacobian/Z_jacobian
    // y_affine = Y_jacobian/Z_jacobian

    DBG_SWDIAG(ECDH, INV, 1);
    // First Determine 1/Z_jacobian
    bigHexInversion256(&pJacPoint->z,&inverted_Z_jacobian);
    DBG_SWDIAG(ECDH, INV, 0);

    MultiplyBigHexModP256(&pJacPoint->x,&inverted_Z_jacobian,pX_common_ord_affine);
    MultiplyBigHexModP256(&pJacPoint->y,&inverted_Z_jacobian,pY_common_ord_affine);

}


__INLINE__ void GF_Point_Copy256(const ECC_Point256 *source,ECC_Point256 *destination)
{
    copyBigHex256(&source->x,&destination->x);
    copyBigHex256(&source->y,&destination->y);
}


__INLINE__ void GF_Jacobian_Point_Copy256(const ECC_Jacobian_Point256 *source,ECC_Jacobian_Point256 *destination)
{
    copyBigHex256(&source->x,&destination->x);
    copyBigHex256(&source->y,&destination->y);
    copyBigHex256(&source->z,&destination->z);
}

__INLINE__ void GF_Affine_To_Jacobian_Point_Copy256(const ECC_Point256 *source,ECC_Jacobian_Point256 *destination)
{
    bigHex256 BigHex_1;

    BigHex_1.len = 0x01;
    BigHex_1.sign = 0; // Positive Number
    memset(BigHex_1.num,0, sizeof(BigHex_1.num) );
    BigHex_1.num[HIGHEST_INDEX_BIG_HEX256] = 0x01;
    copyBigHex256(&source->x,&destination->x);
    copyBigHex256(&source->y,&destination->y);
    copyBigHex256(&BigHex_1,&destination->z);
}

__INLINE__ void GF_Setup_Jacobian_Infinity_Point256(ECC_Jacobian_Point256 *infinity)
{
    bigHex256 BigHex_1;

    BigHex_1.len = 0x01;
    BigHex_1.sign = 0; // Positive Number
    memset(BigHex_1.num,0, sizeof(BigHex_1.num) );
    BigHex_1.num[HIGHEST_INDEX_BIG_HEX256] = 0x00;
    copyBigHex256(&BigHex_1,&infinity->x);
    BigHex_1.num[HIGHEST_INDEX_BIG_HEX256] = 0x01;
    copyBigHex256(&BigHex_1,&infinity->y);
    BigHex_1.num[HIGHEST_INDEX_BIG_HEX256] = 0x00;
    copyBigHex256(&BigHex_1,&infinity->z);

}

/**************************************************************
 *  Function :- bigHexInversion
 *
 *  Parameters :-  bigHexA - a pointer to bigHex - the input
 *                 pResult - a pointer to bigHex - the output
 *
 *  Description
 *  This function performs the inversion of a bigHex number. The output multiplied
 *  by the input will result in '1', as the output is Moded by P.
 *
 *            ( bigHexA * pResult ) % P = 1.
 *
 * Ref " Software Implementation of the NIST Elliptical "
 *  Input : Prime P (bigHexP), a (bigHexA) E [1, p-1]
 *  Output : a^-1 mod P
 ************************************************************************/

void bigHexInversion256( bigHex256* bigHexA,bigHex256* pResult)
{
    bigHex256 u;
    bigHex256 v;
    bigHex256 A;
    bigHex256 C;

    // Change the sign to positive
    bigHexA->sign = 0;

    // Step 1
    // u <-- a, v <-- p, A <-- 1, C <-- 0.
    //
    initBigNumber256(&A);
    initBigNumber256(&C);

    copyBigHex256(bigHexA,&u);
    copyBigHex256(&bigHexP256,&v);

    A.num[HIGHEST_INDEX_BIG_HEX256] = 1;
    C.num[HIGHEST_INDEX_BIG_HEX256] = 0;
    A.len = 1;
    C.len = 1;

    //
    // Step 2.
    //   While u != 0 do
    //   2.1  While u is even do :
    //           u <-- u/2. If A is even then A <-- A/2; else A <-- (A+P)/2
    //   2.2  While v is even do :
    //           v <-- v/2. If C is even then C <-- C/2; else C <-- (C+P)/2
    //   2.3  If u >= v then : u <-- u - v, A <-- A - C; else v <-- v - u, C <-- C - A
    //
    while(u.len!=0)
    {
        while(ISBIGHEXEVEN256(u))
        {
            divideByTwo256(&u);
            if (ISBIGHEXEVEN256(A))
            {
                // A = A/2
                divideByTwo256(&A);
            }
            else
            {
                AddPdiv2_256(&A);
            }
        }
        while(ISBIGHEXEVEN256(v))
        {
            divideByTwo256(&v);
            if (ISBIGHEXEVEN256(C))
            {
                divideByTwo256(&C);
            }
            else
            {
                AddPdiv2_256(&C);
            }
        }
        if (isGreaterThanOrEqual256(&u,&v))
        {
            SubtractFromSelfBigHex256(&u,&v);
            SubtractFromSelfBigHexSign256(&A,&C);
        }
        else
        {
            SubtractFromSelfBigHex256(&v,&u);
            SubtractFromSelfBigHexSign256(&C,&A);
        }
    }
    // Step 3 :- Now perform Mod P
    // pResult = C % P
    {
        // If the Mod P is greater than bigHexA then return with
        // C unmodified.

        if (C.sign == 0)
        {
            if (isGreaterThan256(&bigHexP256,&C))
            {
                copyBigHex256(&C,pResult);
                return;
            }
            else // Positive Number thus subtract P iteratively.
            {
                specialModP256(&C);
            }
        }
        else // Negative Number
        {
            specialModP256(&C);
        }
        copyBigHex256(&C,pResult);
    }
}

/*******************************************************************
 * Funcion :- divideByTwo
 *
 * Parameters :- A - a bigHex pointer - which is the divided by two
 *
 *******************************************************************/

void divideByTwo256(bigHex256* A)
{
    #if (ECC_MULT_ALGO_TYPE == 16)
    u_int16 rem=0;
    u_int32 val;
    u_int32 i;

    for(i=0; i < (ELEMENTS_BIG_HEX256); i++)
    {
        val = (u_int32)((u_int32)(A->num[i]) + ((u_int32)(rem << 16)));

        A->num[i] = (u_int16)(val>>1);
        rem = (u_int32)(val - ((val>>1)<<1));
    }
    #elif (ECC_MULT_ALGO_TYPE == 32)
    u_int32 rem=0;
    u64 u64val;
    u_int32 i;

    for(i=0; i < (ELEMENTS_BIG_HEX256); i++)
    {
        u64val = (u64)((u64)(A->num[i]) + ((u64)rem * _LL(0x100000000)));

        A->num[i] = (u_int32)(u64val>>1);
        rem = (u64)(u64val - ((u64val>>1)<<1));
    }
    #endif // (ECC_MULT_ALGO_TYPE == 16)
    // We need to re-calculate the length.
    setBigNumberLength256(A);
}

//    Q = kP
//
/********************************************************
 * Function :- ECC_Point_Multiplication
 *
 * Parameters :- pk       - a pointer to a bigHex which the point should be multiplied by.
 *               pPointP  - pointer to the point which is to be multiplied by pk
 *
 * Description
 * This function performs ECC point multiplication. It uses the Scalar Multiplication
 * algorithm. Scalar multiplication works is way through the bits of a BigHex starting with the LSB.
 * For each bit (irrespective of Value) a point Doubling is performed, if the bit has a value = 1 then
 * a point addition is also performed.
 *
 * Scalar Multiplication: LSB first
 *   ?Require k=(km-1,km-2,?k0)2, km=1
 *   ?Compute Q=kP
 * ?Q=0, R=P
 * ?For i=0 to m-1
 *   ?If ki=1 then
 *      ?Q=Q+R
 *   ?End if
 *   ?R=2R
 * ?End for
 * ?Return Q
 ******************************************************************************************/
/**
 ****************************************************************************************
 * @brief Outer function for ECC point multiplications, both multiply an ECC point by 8 bits.
 *
 * Used to continue ECC P256 algorithm.
 ****************************************************************************************
 */
static void ecc_point_multiplication_uint8_256(struct ecc_elt_tag* ecc_elt)
{
    ECC_Jacobian_Point256 tmpResultPoint;
    ECC_Jacobian_Point256 tmpResultPoint2;
    ECC_Jacobian_Point256* jPointQ256 = &(ecc_elt->Jacobian_PointQ256);
    ECC_Jacobian_Point256* jPointR256 = &(ecc_elt->Jacobian_PointR256);

    if(ecc_elt->bit_cursor == 0)
    {
        ecc_elt->current_val = ecc_elt->Pk256.num[ecc_elt->Point_Mul_Word256-1];
        ecc_elt->bit_cursor = ECC_MULT_ALGO_TYPE;
    }

    // check if bit in current val is set
    if (ecc_elt->current_val & 0x1)
    {
        // Q = Q + R
        GF_Jacobian_Point_Addition256(jPointQ256, jPointR256, &tmpResultPoint);
        // Copy ResultPoint to PointQ
        GF_Jacobian_Point_Copy256(&tmpResultPoint, jPointQ256);
    }
    // Point Doubling
    // R = 2R
    GF_Jacobian_Point_Double256(jPointR256, &tmpResultPoint2);
    // Copy Result point to PointR
    GF_Jacobian_Point_Copy256(&tmpResultPoint2, jPointR256);

    // update current value to check next bit
    ecc_elt->current_val = ecc_elt->current_val >> 1;
    ecc_elt->bit_cursor--;

    // check if a full word has been calculated
    if(ecc_elt->bit_cursor == 0)
    {
        // decrement the counter
        ecc_elt->Point_Mul_Word256--;
    }
}

/**
 ****************************************************************************************
 * @brief ECC ecc_multiplication event handler
 ****************************************************************************************
 */
static void ecc_multiplication_event_handler(void)
{
    DBG_SWDIAG(ECDH, COMPUTE, 1);
    // Clear the event
    kernel_event_clear(KERNEL_EVENT_ECC_MULTIPLICATION);

    // Take the next multiplication
    struct ecc_elt_tag* ecc_elt = (struct ecc_elt_tag*) common_list_pop_front(&ecc_env.ongoing_mul);


    // Check if element is present
    if(ecc_elt != NULL)
    {
        // Check that current multiplication is not finished
        if(ecc_elt->Point_Mul_Word256 > 0)
        {
            DBG_SWDIAG(ECDH, MULT, 1);
            // Execute 1 multiplication step
            ecc_point_multiplication_uint8_256(ecc_elt);

            // Re-insert the multiplication at the end of the list
            common_list_push_back(&ecc_env.ongoing_mul, &ecc_elt->hdr);
            DBG_SWDIAG(ECDH, MULT, 0);
        }
        else
        {
            DBG_SWDIAG(ECDH, END, 1);
            // Multiplication completed, report to the client task
            if(ecc_elt->client_id != TASK_BLE_NONE)
            {
                int32_t i, j;
                ECC_Point256 pointQ256;

                struct ecc_result_ind* ind = KERNEL_MSG_ALLOC(ecc_elt->msg_id, ecc_elt->client_id, TASK_BLE_NONE, ecc_result_ind);

                initBigNumber256(&pointQ256.x);
                initBigNumber256(&pointQ256.y);

                GF_Point_Jacobian_To_Affine256(&(ecc_elt->Jacobian_PointQ256), &pointQ256.x, &pointQ256.y);

                // Free the memory previously allocated for Jacobian points Q and R and the private key
                kernel_free(ecc_elt);

                // Copy result keys X coordinate # LSB first
                for(i = 31, j=1; i>=0;) // Keys Res is MSB - make it in LSB
                {
                    #if (ECC_MULT_ALGO_TYPE == 16)
                    ind->key_res_x[i] =   ((pointQ256.x.num[j] & 0xFF00) >> 8);
                    ind->key_res_x[i-1] =  (pointQ256.x.num[j] & 0x00FF);
                    i-=2;
                    j++;
                    #elif (ECC_MULT_ALGO_TYPE == 32)
                    ind->key_res_x[i]   = ((pointQ256.x.num[j] & 0xFF000000) >> 24);
                    ind->key_res_x[i-1] = ((pointQ256.x.num[j] & 0x00FF0000) >> 16);
                    ind->key_res_x[i-2] = ((pointQ256.x.num[j] & 0x0000FF00) >> 8);
                    ind->key_res_x[i-3] = ( pointQ256.x.num[j] & 0x000000FF);
                    i-=4;
                    j++;
                    #endif // (ECC_MULT_ALGO_TYPE == 16)
                }

                // Copy result keys Y coordinate
                for(i = 31, j=1; i>=0;) // Keys Res is MSB - make it in LSB
                {
                    #if (ECC_MULT_ALGO_TYPE == 16)
                    ind->key_res_y[i]  = ((pointQ256.y.num[j] & 0xFF00) >> 8);
                    ind->key_res_y[i-1] = (pointQ256.y.num[j] & 0x00FF);

                    i-=2;
                    j++;
                    #elif (ECC_MULT_ALGO_TYPE == 32)
                    ind->key_res_y[i]   = ((pointQ256.y.num[j] & 0xFF000000) >> 24);
                    ind->key_res_y[i-1] = ((pointQ256.y.num[j] & 0x00FF0000) >> 16);
                    ind->key_res_y[i-2] = ((pointQ256.y.num[j] & 0x0000FF00) >> 8);
                    ind->key_res_y[i-3] = ( pointQ256.y.num[j] & 0x000000FF);
                    i-=4;
                    j++;
                    #endif // (ECC_MULT_ALGO_TYPE == 16)
                }

                kernel_msg_send(ind);
            }

            DBG_SWDIAG(ECDH, END, 0);
        }
    }
    else
    {
        ASSERT_ERR(0);
    }

    // Restart the event in case there is multiplication to perform
    if(!common_list_is_empty(&ecc_env.ongoing_mul))
    {
        kernel_event_set(KERNEL_EVENT_ECC_MULTIPLICATION);
    }
    else
    {
        DBG_SWDIAG(ECDH, BUSY, 0);
      
    }
    DBG_SWDIAG(ECDH, COMPUTE, 0);
}


/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

void ecc_init(bool reset)
{
    if(reset)
    {
        // Empty multiplications list
        while(!common_list_is_empty(&ecc_env.ongoing_mul))
        {
            // Free allocated memory
            struct ecc_elt_tag* elt = (struct ecc_elt_tag*) common_list_pop_front(&ecc_env.ongoing_mul);
            kernel_free(elt);
        }
       
    }

    // Initialize multiplications list
    common_list_init(&ecc_env.ongoing_mul);

    // Register event to handle multiplication steps
    kernel_event_callback_set(KERNEL_EVENT_ECC_MULTIPLICATION, &ecc_multiplication_event_handler);
}

void ecc_generate_key256(const u_int8* secret_key, const u_int8* public_key_x, const u_int8* public_key_y, kernel_msg_id_t msg_id, kernel_task_id_t task_id)
{
    u_int32 big_num_offset=1;
    int32_t i, j;
    bigHex256 PrivateKey256;
    ECC_Point256 PublicKey256;

    // Now Copy the Public Key and Secret Key coordinates to ECC point format.
    PrivateKey256.num[0] = 0;
    PublicKey256.x.num[0] = 0;
    PublicKey256.y.num[0] = 0;

    DBG_SWDIAG(ECDH, BUSY, 1);

    for (i=31, j=big_num_offset;i>=0;) // Public and Private Keys Are LSB - make it in MSB
    {
        #if (ECC_MULT_ALGO_TYPE == 16)
        PrivateKey256.num[j] = (u_int32)
                                                   ((((*(secret_key+i   )) <<  8) & 0xFF00) +
                                                    (((*(secret_key+(i-1) ))      & 0x00FF)));

        PublicKey256.x.num[j] = (u_int32)
                                                  ( (((*(public_key_x+i   )) <<  8) & 0xFF00) +
                                                    (((*(public_key_x+(i-1) ))      & 0x00FF)));

        PublicKey256.y.num[j] = (u_int32)
                                                  ( (((*(public_key_y+i   )) <<  8) & 0xFF00) +
                                                    (((*(public_key_y+(i-1) ))      & 0x00FF)));

        i -= 2;
        j++;
        #elif (ECC_MULT_ALGO_TYPE == 32)
        PrivateKey256.num[j] = (u_int32)
                                            ((((*(secret_key+i    )) << 24) & 0xFF000000) +
                                           (((*(secret_key+(i-1))) << 16) & 0x00FF0000) +
                                           (((*(secret_key+(i-2))) <<  8) & 0x0000FF00) +
                                           (( *(secret_key+(i-3)))        & 0x000000FF));

        PublicKey256.x.num[j] = (u_int32)
                                            ((((*(public_key_x+i    )) << 24) & 0xFF000000) +
                                           (((*(public_key_x+(i-1))) << 16) & 0x00FF0000) +
                                           (((*(public_key_x+(i-2))) <<  8) & 0x0000FF00) +
                                           (( *(public_key_x+(i-3)))        & 0x000000FF));

        PublicKey256.y.num[j] = (u_int32)
                                           ((((*(public_key_y+i    )) << 24) & 0xFF000000) +
                                           (((*(public_key_y+(i-1))) << 16) & 0x00FF0000) +
                                           (((*(public_key_y+(i-2))) <<  8) & 0x0000FF00) +
                                           (( *(public_key_y+(i-3)))        & 0x000000FF));
        i -= 4;
        j++;
        #endif // (ECC_MULT_ALGO_TYPE == 16)
    }

    setBigNumberLength256(&PrivateKey256);
    setBigNumberLength256(&PublicKey256.x);
    setBigNumberLength256(&PublicKey256.y);
    PublicKey256.x.sign = 0;
    PublicKey256.y.sign = 0;

//    ECC_Point_Multiplication256(&PrivateKey256,&PublicKey256,blocking);
    {
        ECC_Jacobian_Point256 PointP_Jacobian;
        ECC_Jacobian_Point256* pPointP_Jacobian = &PointP_Jacobian;

        // Allocate Memory for Jacobian Points P, Q, R
        struct ecc_elt_tag* ecc_elt = (struct ecc_elt_tag*) kernel_malloc(sizeof(struct ecc_elt_tag), KERNEL_MEM_NON_RETENTION);

        // Store client message/task ID
        ecc_elt->msg_id = msg_id;
        ecc_elt->client_id = task_id;

        // Need to map from Affine Point to Jacobian Point
        GF_Affine_To_Jacobian_Point_Copy256(&PublicKey256, pPointP_Jacobian);

        GF_Jacobian_Point_Copy256(pPointP_Jacobian, &(ecc_elt->Jacobian_PointR256));

        copyBigHex256(&PrivateKey256, &(ecc_elt->Pk256));

        GF_Setup_Jacobian_Infinity_Point256(&(ecc_elt->Jacobian_PointQ256));

        // Initialize cursor
        ecc_elt->current_val = 0;
        ecc_elt->bit_cursor  = 0;
        ecc_elt->Point_Mul_Word256 = ELEMENTS_BIG_HEX256;
        
        // Check if the client task is expecting a return message
        if (ecc_elt->client_id == TASK_BLE_NONE)
        {
            // Execute all the multiplication steps
            while (ecc_elt->Point_Mul_Word256 > 0)
            {
                ecc_point_multiplication_uint8_256(ecc_elt);
            }
        }
        else
        {
            // Insert the multiplication at the end of the list
            common_list_push_back(&ecc_env.ongoing_mul, &ecc_elt->hdr);

            // Start the event
            kernel_event_set(KERNEL_EVENT_ECC_MULTIPLICATION);
        }
    }
}

void ecc_abort_key256_generation(kernel_task_id_t task_id)
{
    struct ecc_elt_tag* prev = NULL;
    struct ecc_elt_tag* elt = (struct ecc_elt_tag*) common_list_pick(&ecc_env.ongoing_mul);

    // Empty multiplications list
    while(elt != NULL)
    {
        // Check if the element correspond to the abort request
        if(elt->client_id == task_id)
        {
            // Extract element from the list
            common_list_extract_after(&ecc_env.ongoing_mul, &prev->hdr, &elt->hdr);

            // Free allocated memory
            kernel_free(elt);

            // Check if list is empty
            if(common_list_is_empty(&ecc_env.ongoing_mul))
            {
                // Clear the event
                kernel_event_clear(KERNEL_EVENT_ECC_MULTIPLICATION);
            }

            break;
        }

        // Jump to next
        prev = elt;
        elt = (struct ecc_elt_tag*) common_list_next(&elt->hdr);
    }
}

void ecc_gen_new_public_key(u_int8* secret_key, kernel_msg_id_t msg_id, kernel_task_id_t task_id)
{
    ecc_generate_key256(secret_key, BasePoint_x_256, BasePoint_y_256, msg_id, task_id);
}


void ecc_gen_new_secret_key(uint8_t* secret_key256, bool forced)
{
    // Secure key is forced
    if(forced)
    {
        // check its validity
//        if(!isValidSecretKey_256(secret_key256))
//        {
//            ASSERT_ERR(0);
//        }
    }
    else
    {
        // Need to get a 32 Byte random number - which we will use as the secret key.
        // We then ECC multiply this by the Base Points, to get a new Public Key.
        //
        do
        {
            common_write32p(&secret_key256[0],  common_rand_word());
            common_write32p(&secret_key256[4],  common_rand_word());
            common_write32p(&secret_key256[8],  common_rand_word());
            common_write32p(&secret_key256[12], common_rand_word());
            common_write32p(&secret_key256[16], common_rand_word());
            common_write32p(&secret_key256[20], common_rand_word());
            common_write32p(&secret_key256[24], common_rand_word());
            common_write32p(&secret_key256[28], common_rand_word());
        }
        while(!isValidSecretKey_256(secret_key256));
    }
}

void ecc_get_debug_Keys(uint8_t*secret_key, uint8_t* pub_key_x, uint8_t* pub_key_y)
{
    int8_t i;

    for (i=31; i>=0; i--)
    {
        pub_key_x[i]  = DebugE256PublicKey_x[i];
        pub_key_y[i]  = DebugE256PublicKey_y[i];
        secret_key[i] = DebugE256SecretKey[i];
    }
}
#endif // (SECURE_CONNECTIONS)
