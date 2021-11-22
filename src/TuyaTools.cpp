/*
 * @FileName: TuyaTools.cpp
 * @Author: Tuya
 * @Email: 
 * @LastEditors: Tuya
 * @Date: 2021-11-16 11:25:32
 * @LastEditTime: 2021-11-18 18:02:59
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company:  http://www.tuya.com
 * @Description: Some necessary tools
 */


#include "TuyaTools.h"
#include "TuyaDefs.h"


TuyaTools::TuyaTools(void)
{
}

TuyaTools::~TuyaTools(void)
{
}

/**
 * @description: copy count bytes data from src to dest
 * @param[in] {src}    srouce address
 * @param[in] {dest}   destination address
 * @param[in] {count}  length of copy data
 * @return void
 */
unsigned char TuyaTools::hex_to_bcd(unsigned char Value_H, unsigned char Value_L)
{
    unsigned char bcd_value;

    if ((Value_H >= '0') && (Value_H <= '9'))
        Value_H -= '0';
    else if ((Value_H >= 'A') && (Value_H <= 'F'))
        Value_H = Value_H - 'A' + 10;
    else if ((Value_H >= 'a') && (Value_H <= 'f'))
        Value_H = Value_H - 'a' + 10;

    bcd_value = Value_H & 0x0f;

    bcd_value <<= 4;
    if ((Value_L >= '0') && (Value_L <= '9'))
        Value_L -= '0';
    else if ((Value_L >= 'A') && (Value_L <= 'F'))
        Value_L = Value_L - 'a' + 10;
    else if ((Value_L >= 'a') && (Value_L <= 'f'))
        Value_L = Value_L - 'a' + 10;

    bcd_value |= Value_L & 0x0f;

    return bcd_value;
}

/**
* @description: get string len 
* @param[in] {str} higher bits data 
* @return string len 
*/
unsigned long TuyaTools::my_strlen(unsigned char *str)
{
    unsigned long len = 0;
    if (str == NULL)
    {
        return 0;
    }

    for (len = 0; *str++ != '\0';)
    {
        len++;
    }

    return len;
}

/**
* @description: assign ch to the first count bytes of the memory address src
* @param[in] {src}    srouce address
* @param[in] {ch}     set value 
* @param[in] {count}  length of set address 
* @return void
*/
void *TuyaTools::my_memset(void *src, unsigned char ch, unsigned short count)
{
    unsigned char *tmp = (unsigned char *)src;

    if (src == NULL)
    {
        return NULL;
    }

    while (count--)
    {
        *tmp++ = ch;
    }

    return src;
}

/**
* @description: copy count bytes data from src to dest
* @param[in] {src}    srouce address
* @param[in] {dest}   destination address
* @param[in] {count}  length of copy data
* @return void
*/
void *TuyaTools::my_memcpy(void *dest, const void *src, unsigned short count)
{
    unsigned char *pdest = (unsigned char *)dest;
    const unsigned char *psrc = (const unsigned char *)src;
    unsigned short i;

    if (dest == NULL || src == NULL)
    {
        return NULL;
    }

    if ((pdest <= psrc) || (pdest > psrc + count))
    {
        for (i = 0; i < count; i++)
        {
            pdest[i] = psrc[i];
        }
    }
    else
    {
        for (i = count; i > 0; i--)
        {
            pdest[i - 1] = psrc[i - 1];
        }
    }

    return dest;
}

/**
* @description: copy string src to string dest
* @param[in] {src}    srouce address
* @param[in] {dest}   destination address
* @return the tail of destination 
*/
char *TuyaTools::my_strcpy(char *dest, const char *src)
{
    if ((NULL == dest) || (NULL == src))
    {
        return NULL;
    }

    char *p = dest;
    while (*src != '\0')
    {
        *dest++ = *src++;
    }
    *dest = '\0';
    return p;
}

/**
* @description: compare string s1 with string s2
* @param[in] {s1}  srouce address
* @param[in] {s2}  destination address
* @return compare result
*/
int TuyaTools::my_strcmp(char *s1, char *s2)
{
    while (*s1 && *s2 && *s1 == *s2)
    {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

/**
* @description: int translate to byte
* @param[in] {number} int data 
* @param[out] {value} the result array     
* @return void
*/
void TuyaTools::int_to_byte(unsigned long number, unsigned char value[4])
{
    value[0] = number >> 24;
    value[1] = number >> 16;
    value[2] = number >> 8;
    value[3] = number & 0xff;
}

/**
* @description: byte data translate to int 
* @param[in] {value}  the byte array 
* @return result of int data 
*/
unsigned long TuyaTools::byte_to_int(const unsigned char value[4])
{
    unsigned long nubmer = 0;

    nubmer = (unsigned long)value[0];
    nubmer <<= 8;
    nubmer |= (unsigned long)value[1];
    nubmer <<= 8;
    nubmer |= (unsigned long)value[2];
    nubmer <<= 8;
    nubmer |= (unsigned long)value[3];

    return nubmer;
}

/**
* @description: computing checksum
* @param[in] {pack}  Data source pointer
* @param[in] {pack_len}  Computes the checksum length
* @return check_sum
*/
unsigned char TuyaTools::get_check_sum(unsigned char *pack, unsigned short pack_len)
{
  unsigned short i;
  unsigned char check_sum = 0;
  
  for(i = 0; i < pack_len; i ++)
  {
    check_sum += *pack ++;
  }
  
  return check_sum;
}