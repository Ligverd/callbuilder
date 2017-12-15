// UniPar platform function

#include  <include/unipar.h>
#include  <string.h>

#ifdef _ATS_
#include  <include/task.h>
#endif

const char* getSignallingText(TSignalling  sig)
{
    switch(sig) {
        case SIGNALLING_NONE            : return "NONE";            //   0;  
        case SIGNALLING_EXT             : return "EXT";             //   1;  
        case SIGNALLING_DSS1            : return "DSS1";            //   2;  
        case SIGNALLING_CAS2_IN         : return "CAS2_IN";         //   3;  
        case SIGNALLING_CAS2_OUT        : return "CAS2_OUT";        //   4;  
        case SIGNALLING_V5_ANSA         : return "V5_ANSA";         //   5;  // AN Subscriber adapter
        case SIGNALLING_V5_ANPSTN       : return "V5_ANPSTN";       //   6;  // AN PSTN protocol
        case SIGNALLING_V52AN_PCMPORT   : return "V52AN_PCMPORT";   //   7;  // v52 pcm
        case SIGNALLING_V52AN_SUBPORT   : return "V52AN_SUBPORT";   //   8;  // v52 sub(real to hardware)
        case SIGNALLING_V52LE_PCMPORT   : return "V52LE_PCMPORT";   //   9;  // v52 pcm
        case SIGNALLING_V52LE_SUBPORT   : return "V52LE_SUBPORT";   //  10;  // v52 sub (virtual to ext)
        case SIGNALLING_CITY            : return "CITY";            //  11;  // 2-ух проводка
        case SIGNALLING_CAS1            : return "CAS1";            //  12;  // 1 ВСК на 4/6 проводке и ИКМ
        case SIGNALLING_SORM            : return "SORM";            //  13;  // SORM-KGB
        case SIGNALLING_DISA            : return "DISA";            //  14;  
        case SIGNALLING_SERIAL          : return "SERIAL";          //  15;  
        case SIGNALLING_ISUP            : return "ISUP";            //  16;  
        case SIGNALLING_SPEAKER         : return "SPEAKER";         //  17;  
        case SIGNALLING_PERMANENT       : return "PERMANENT";       //  18;  // постоянная коммутация каналов
        case SIGNALLING_3SL_IN          : return "3SL_IN";          //  19;  // 3-х проводка входяща
        case SIGNALLING_3SL_OUT         : return "3SL_OUT";         //  20;  // 3-х проводка исходяща
        case SIGNALLING_2600            : return "2600";            //  21;  // сигнализация 2600
        case SIGNALLING_DSS1SUBSCRIBER  : return "DSS1SUBSCRIBER";  //  22;  // Порт абонентского выноса по DSS1
        case SIGNALLING_CAS_TEST        : return "CAS_TEST";        //  23;  // Tester
        case SIGNALLING_INT             : return "INT";             //  24;  // Внутренние лини
        case SIGNALLING_CONTROL         : return "CONTROL";         //  25;  // Контрольный порт
        case SIGNALLING_MTP             : return "MTP";             //  26;  // Сигнализация для внутреннего обмена по MTP
        case SIGNALLING_ISDN            : return "ISDN";            //  27;  
        case SIGNALLING_CALLGEN         : return "CALLGEN";         //  28;  // Генератор вызовов
        case SIGNALLING_ADASE           : return "ADASE";           //  29;  // Сигнализация АДАСЭ 1200&1600
        case SIGNALLING_CAS1_NORKA_OUT  : return "CAS1_NORKA_OUT";  //  30;  // 1 ВСК "норка" на ИКМ, исх
        case SIGNALLING_CAS1_NORKA_IN   : return "CAS1_NORKA_IN";   //  31;  // 1 ВСК "норка" на ИКМ, вх
        case SIGNALLING_CALLBACK        : return "CALLBACK";        //  32;  // Обратный вызов
        case SIGNALLING_BUTTON          : return "BUTTON";          //  33;  
        case SIGNALLING_ADASEPULT       : return "ADASEPULT";       //  34;  
        case SIGNALLING_2100            : return "2100";            //  35;  // Сигнализация 2100 для АДАСЭ
        case SIGNALLING_EM              : return "EM";              //  36;
        case SIGNALLING_V52AN_ISDNPORT  : return "V52LE_ISDNPORT";  //  37;
        case SIGNALLING_V52LE_ISDNPORT  : return "V52LE_ISDNPORT";  //  38;
        default                         : return "UNKNOWN";
    }
}
TSignalling getSignallingBin(const char* sig)
{
    if (!strcmp(sig, "NONE"))               return SIGNALLING_NONE;
    if (!strcmp(sig, "EXT"))                return SIGNALLING_EXT;
    if (!strcmp(sig, "DSS1"))               return SIGNALLING_DSS1;
    if (!strcmp(sig, "CAS2_IN"))            return SIGNALLING_CAS2_IN;
    if (!strcmp(sig, "CAS2_OUT"))           return SIGNALLING_CAS2_OUT;
    if (!strcmp(sig, "V5_ANSA"))            return SIGNALLING_V5_ANSA;
    if (!strcmp(sig, "V5_ANPSTN"))          return SIGNALLING_V5_ANPSTN;
    if (!strcmp(sig, "V52AN_PCMPORT"))      return SIGNALLING_V52AN_PCMPORT;
    if (!strcmp(sig, "V52AN_SUBPORT"))      return SIGNALLING_V52AN_SUBPORT;
    if (!strcmp(sig, "V52LE_PCMPORT"))      return SIGNALLING_V52LE_PCMPORT;
    if (!strcmp(sig, "V52LE_SUBPORT"))      return SIGNALLING_V52LE_SUBPORT;
    if (!strcmp(sig, "CITY"))               return SIGNALLING_CITY;
    if (!strcmp(sig, "CAS1"))               return SIGNALLING_CAS1;
    if (!strcmp(sig, "SORM"))               return SIGNALLING_SORM;
    if (!strcmp(sig, "DISA"))               return SIGNALLING_DISA;
    if (!strcmp(sig, "SERIAL"))             return SIGNALLING_SERIAL;
    if (!strcmp(sig, "ISUP"))               return SIGNALLING_ISUP;
    if (!strcmp(sig, "SPEAKER"))            return SIGNALLING_SPEAKER;
    if (!strcmp(sig, "PERMANENT"))          return SIGNALLING_PERMANENT;
    if (!strcmp(sig, "3SL_IN"))             return SIGNALLING_3SL_IN;
    if (!strcmp(sig, "3SL_OUT"))            return SIGNALLING_3SL_OUT;
    if (!strcmp(sig, "2600"))               return SIGNALLING_2600;
    if (!strcmp(sig, "DSS1SUBSCRIBER"))     return SIGNALLING_DSS1SUBSCRIBER;
    if (!strcmp(sig, "CAS_TEST"))           return SIGNALLING_CAS_TEST;
    if (!strcmp(sig, "INT"))                return SIGNALLING_INT;
    if (!strcmp(sig, "CONTROL"))            return SIGNALLING_CONTROL;
    if (!strcmp(sig, "MTP"))                return SIGNALLING_MTP;
    if (!strcmp(sig, "ISDN"))               return SIGNALLING_ISDN;
    if (!strcmp(sig, "CALLGEN"))            return SIGNALLING_CALLGEN;
    if (!strcmp(sig, "ADASE"))              return SIGNALLING_ADASE;
    if (!strcmp(sig, "CAS1_NORKA_OUT"))     return SIGNALLING_CAS1_NORKA_OUT;
    if (!strcmp(sig, "CAS1_NORKA_IN"))      return SIGNALLING_CAS1_NORKA_IN;
    if (!strcmp(sig, "CALLBACK"))           return SIGNALLING_CALLBACK;
    if (!strcmp(sig, "BUTTON"))             return SIGNALLING_BUTTON;
    if (!strcmp(sig, "ADASEPULT"))          return SIGNALLING_ADASEPULT;
    if (!strcmp(sig, "2100"))               return SIGNALLING_2100;
    if (!strcmp(sig, "EM"))                 return SIGNALLING_EM;
    if (!strcmp(sig, "V52LE_ISDNPORT"))     return SIGNALLING_V52AN_ISDNPORT;
    if (!strcmp(sig, "V52LE_ISDNPORT"))     return SIGNALLING_V52LE_ISDNPORT;

#ifdef  _ATS_
    WARNING;
#endif
    return SIGNALLING_NONE;
}
