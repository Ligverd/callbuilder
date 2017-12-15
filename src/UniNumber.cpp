/**************************************************************************
Codepage : Win-1251
File     : UniNumber.cpp
Project  : SMP
Desc     :
           Interface.

AUTHOR   : $Author: maksim $
           Alexander Zuykov, MTA
ID       : $Id: UniNumber.cpp,v 1.40.4.3 2009/11/26 12:04:18 maksim Exp $
MODIFIED : $Date: 2009/11/26 12:04:18 $
***************************************************************************


**************************************************************************/

#include  <string.h>
#include  "callbuilder.h"
#include  "UniNumber.h"



namespace UniN {


Number::Number() //uc numberType)
{
    numberType = 0xFE;

    numberComplete   = false;
    len              = 0;
    memset(number, 0x00, sizeof(number));


    /*
    // ‘лаги
    switch(numberType) {
        case NbType_CalledPartyNumber:
            flags.init_CalledPartyNumber();
            break;

        case NbType_CallingPartyNumber:
            flags.init_CallingPartyNumber();
            break;

        case NbType_ConnectedNumber:
            flags.init_ConnectedNumber();
            break;

        case NbType_RedirectingNumber:
            flags.init_RedirectingNumber();
            break;

        case NbType_RedirectionNumber:
            flags.init_RedirectonNumber();
            break;

        case NbType_ISUP_OriginalCalledNumber:
            flags.init_OriginalCalledNumber();
            break;

        default:
            WARNING;
    }
    */
}

Number::Number(const Number* src)
{
    if (src)
    {
        numberType      = src->numberType;
        numberComplete  = src->numberComplete;
        len             = src->len;
        memcpy(number, src->number, len+1);
        memcpy(&flags, &src->flags, sizeof(flags));
        //memcpy(this, src, sizeof(Number));   нельз€, т.к. src м.б. неполным и иканчиватьс€ пустотой
    }
    else
    {
        numberType = 0xFE;

        numberComplete   = false;
        len              = 0;
        memset(number, 0x00, sizeof(number));

        flags.setnone();
    }
}

void  Number::setNumber(const char* digits, bool numberComplete)
{
    len = 0;
    this->numberComplete = numberComplete;
    char ch;
    if (digits)
    {
        while((ch = *digits++)) {
            if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F')) {
                number[len++] = ch;
            }

            else if (ch == '*' || ch == '%' || ch == '=') {
                number[len++] = ch;
            }

            else if (ch == '#')
            {
                if (len) {
                    numberComplete = true;
                    break;
                }
                else
                    number[len++] = ch;
            }

            else
                WARNING;

            if (len >= MAX_NUMBER_LENGTH) {
                WARNING;
                len = 0;
                number[0] = '\0';
                return;
            }
        }
    }

    number[len] = '\0';
}

void Number::setNumber(const char* digits, bool numberComplete, int len )
{
    char tmp[50];
    if (digits)
    {
        strncpy(tmp, digits, len);
        tmp[len] = 0;
    }
    else
        tmp[0] = 0;
    setNumber(tmp, numberComplete);
}

void Number::clearNumber()
{
    len = 0;
    numberComplete = false;
    memset(number, 0x00, sizeof(number));   // number[0] = '\0';
}
void Number::clear()
{
    numberType = 0xFE;
    clearNumber();
}

void  Number::addDigit(char digit)
{
    if (len >= MAX_NUMBER_LENGTH-1) {
        WARNING;
        return;
    }
    number[len++] = digit;
    number[len] = '\0';
}

bool  Number::insertDigit(size_t pos, char digit)
{
    if (pos > len) return false;
    if (len+1 > MAX_NUMBER_LENGTH) return false;

    memmove(number+pos+1, number+pos, len-pos+1);   // +1 - '\0'
    number[pos] = digit;

    len++;
    return true;
}
bool  Number::insertDigits(size_t pos, const char* digits)
{
    if (pos > len) return false;

    size_t l = strlen(digits);
    if (len+l > MAX_NUMBER_LENGTH) return false;

    memmove(number+pos+l, number+pos, len-pos+1);   // +1 - '\0'
    memcpy(number+pos, digits, l);
    len += l;

    return true;
}

bool  Number::erase(size_t pos, size_t count)
{
    if (pos > len) return false;
    if (pos+count > len) return false;

    memmove(number+pos, number+pos+count, len-pos-count+1); // +1 - '\0'
    len -= count;
    return true;
}

void  Number::applyFlags ( NumberFlags& xflags )
{
    if (xflags.nbplan != 0xFF)
        flags.nbplan = xflags.nbplan;
    if (xflags.typenb != 0xFF)
        flags.typenb = xflags.typenb;
    if (xflags.scrind != 0xFF)
        flags.scrind = xflags.scrind;
    if (xflags.presind != 0xFF)
        flags.presind = xflags.presind;
    if (xflags.innind != 0xFF)
        flags.innind = xflags.innind;
    if (xflags.niind != 0xFF)
        flags.niind = xflags.niind;
}


void  Number::operator=(const Number& src)
{
    memcpy(this, &src, src.realSize());
}

};  // namespace UniN
