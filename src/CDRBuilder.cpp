//#include "stdafx.h"
#ifdef _MSC_VER
/*
чтобы убрать проблемы под MS Visual Studio при линковке
nafxcwd.lib(afxmem.obj) : error LNK2005: "void * __cdecl operator new(unsigned int)" (??2@YAPAXI@Z) already defined in libcpmtd.lib(newop.obj)
nafxcwd.lib(afxmem.obj) : error LNK2005: "void __cdecl operator delete(void *)" (??3@YAXPAX@Z) already defined in LIBCMTD.lib(dbgdel.obj)
*/
#ifdef _DEBUG
  #pragma comment(lib, "nafxcwd.lib")
#else
  #pragma comment(lib, "nafxcw.lib")
#endif

#endif

//changed by pax
//#include ".\cdrbuilder.h"
//#include "errors.h"
#include "CDRBuilder.h"
#include "Errors.h"
#define _MSC_VER XXX;
//changed by pax

#include  <include/unipar.h>

#ifndef __FILE__
#error Undefined macro __FILE__
#endif

#ifndef __LINE__
#error Undefined macro __LINE__
#endif

#ifdef _MSC_VER
//если это MS VC++

#ifndef _DEBUG
//если Релизная версия
#define myASSERT(f)	if(!(f))			\
					{	\
						char sTmp[200];	\
						sprintf(sTmp, "WARNING! Вышлите по адресу roman@m-200.com данные: 'myASSERT : Файл %s Строка %d'", __FILE__, __LINE__);	\
						AddErrorString(sTmp); \
					}	
#else
//если Debug
#include "afxcom_.h"
#define myASSERT ASSERT

#include <atlbase.h>
#define myTRACE ATLTRACE
#endif

#endif

/*
int iCntMes = 0;
int iCntBytes = 0;
*/

CDRBuilding::CCDRBuilder::CCDRBuilder()
{
#ifdef CONTROL_CALLS_COUNT
	m_iCallsCount = 0;
	m_iMaxCallsCount = 0;
//	myTRACE("CallsCount - %d\n", m_iCallsCount);
#endif

	m_SSettings.bWriteBinary2 = false;
	m_SSettings.bWriteUnansweredCalls = true;
	m_SSettings.sNoNumberString[0] = 0;
	m_SSettings.btCDRStringFormat = FORMAT_CDR_EX;
	m_SSettings.bDecrementDuration = false;
	m_SSettings.sSampleString[0] = 0;

	m_pBegListCall = NULL;
	m_pEndListCall = NULL;
	m_dwLastError = ERROR_NONE;

	m_SJournalSettings.bEnable = FALSE;

	ResetModuleInfo(ALL_MODULES);
}

void CDRBuilding::CCDRBuilder::SetCommonSettings(const CDRBuilding::strCDRBuilderSettings* pSSettings)
{
	memcpy(&m_SSettings, pSSettings, sizeof(strCDRBuilderSettings));
	m_SSettings.sNoNumberString[MAX_NONUMBERSTRING_LENGTH] = 0;
}

CDRBuilding::CCDRBuilder::~CCDRBuilder(void)
{
	CleanCalls();
}


void CDRBuilding::CCDRBuilder::SetLocalNumbersSettings(const TListInterval& lstLocalNumbers,  const strPrefix* pSPrefix)
{
	//префикс
	if(pSPrefix)
	{
		memcpy(&m_SLocalNumbersSettings.SPrefix, pSPrefix, sizeof(strPrefix));
		m_SLocalNumbersSettings.SPrefix.sPrefix[MAX_PREFIX_LENGTH] = 0;
	}
	else
	{
		m_SLocalNumbersSettings.SPrefix.bCutPrefix = false;
		m_SLocalNumbersSettings.SPrefix.sPrefix[0] = 0;
	}
	//интервал
	m_SLocalNumbersSettings.lstInterval.clear();
	strInterval SI;
	TListInterval::const_iterator it, it_end = lstLocalNumbers.end();
	for(it = lstLocalNumbers.begin(); it != it_end; it++)
	{
		memcpy(&SI, &(*it), sizeof(strInterval));
		SI.beg[MAX_INTERVAL_LENGTH] = 0;
		SI.end[MAX_INTERVAL_LENGTH] = 0;
		m_SLocalNumbersSettings.lstInterval.push_back(SI);
	}
}

void CDRBuilding::CCDRBuilder::SetJournalSettings(const strJournalSettings* pSett)
{
	memcpy(&m_SJournalSettings, pSett, sizeof(strJournalSettings));
}

void CDRBuilding::CCDRBuilder::PutResiduaryCalls(const CDRBuilding::strModuleInfo SModuleInfoArray[MAX_MOD], const CDRBuilding::TListPSCallInfo& lstCallInfo)
{
	CleanCalls();
	memcpy(m_ModuleInfo, SModuleInfoArray, MAX_MOD*sizeof(strModuleInfo));
	strCallInfo* pCallInfo;
	TListPSCallInfo::const_iterator it;
	for(it = lstCallInfo.begin(); it != lstCallInfo.end(); it++)
	{
		pCallInfo = new strCallInfo;

#ifdef CONTROL_CALLS_COUNT
		m_iCallsCount++;
		/*
		myTRACE("CallsCount - %d\n", m_iCallsCount);
		if(m_iCallsCount > m_iMaxCallsCount)
		{
			m_iMaxCallsCount = m_iCallsCount;
			myTRACE("MaxCallsCount - %d\n", m_iMaxCallsCount);
		}
		*/
#endif

		memcpy(pCallInfo, *it, sizeof(strCallInfo));
		pCallInfo->pPrevCall = m_pEndListCall;
		pCallInfo->pNextCall = NULL;

        if(m_pBegListCall == NULL)
        {//если первый элемент
            m_pBegListCall = pCallInfo;
        }
        if(m_pEndListCall)
        {
            m_pEndListCall->pNextCall = pCallInfo;
        }
        m_pEndListCall = pCallInfo;
	}
}

void CDRBuilding::CCDRBuilder::GetResiduaryCalls(CDRBuilding::strModuleInfo SModuleInfoArray[MAX_MOD], TListPSCallInfo& lstCallInfo) const
{
	memcpy(SModuleInfoArray, m_ModuleInfo, MAX_MOD*sizeof(strModuleInfo));
	lstCallInfo.clear();
	strCallInfo* pTmp, *pCallInfo = m_pBegListCall;		  
	while(pCallInfo)
	{
		pTmp = new strCallInfo;
		memcpy(pTmp, pCallInfo, sizeof(strCallInfo));
		pTmp->pNextCall = NULL;
		pTmp->pPrevCall = NULL;
		lstCallInfo.push_back(pTmp);

		pCallInfo = pCallInfo->pNextCall;
	}
}


void CDRBuilding::CCDRBuilder::CleanCalls(void)
{
	//очистка звонков
	strCallInfo* pTmp, *pCallInfo = m_pBegListCall;		  
	while(pCallInfo)
	{
		pTmp = pCallInfo->pNextCall;
		DeleteCallFromList(pCallInfo);
		/*
		delete pCallInfo;
		*/
		pCallInfo = pTmp;
	}
	m_pBegListCall = NULL;
	m_pEndListCall = NULL;

	FreeListMem(m_lstCDR);
	FreeListMem(m_lstJour);
	FreeListMem(m_lstErr);
}

DWORD CDRBuilding::CCDRBuilder::GetLastError(void)
{
	return m_dwLastError;
}

BOOL CDRBuilding::CCDRBuilder::ProcessMessage(CMonMessageEx& mes)
{
	m_dwLastError = ERROR_NONE;
	JournalAnalysisMes(mes);
	/*
	iCntMes++;
	iCntBytes += iLen;
	*/
    switch(mes.message())
    {
    case MON_ALIVE:
        OnAlive(mes);
        break;
    case MON_COMOVERLOAD:
    case MON_WIZOVERLOAD:
    case MON_FATOVERLOAD:
#ifdef _DEBUG
	if(mes.time() == 203)
		int a = 2;
#endif
        OnComoverload(ALL_MODULES);
        break;
    case MON_SEIZURE:
        OnSeizure(mes);
        break;
    case MON_CALL:
        OnCall(mes);
        break;
    case MON_ACCEPT:
        OnAccept(mes);
        break;
    case MON_NUMBER:
        OnNumber(mes);
        break;
    case MON_ANSWER:
        OnAnswer(mes);
        break;
    case MON_SPIDER_MODULE_UP:
    case MON_SPIDER_MODULE_DOWN:
        OnSpiderModuleDown(mes);
        break;
    case MON_SPIDER_USER_STOP:
    case MON_SPIDER_START:
    case MON_SPIDER_TCP_UP:
        OnSpiderTcpDown();
        break;
    case MON_SPIDER_TCP_DOWN:
        //if( iLen == 2*sizeof(BYTE) + sizeof(TCallID)+sizeof(TClock) )
        if (mes.bufferSize() == 0)
            OnSpiderTcpDown();
        break;
    case MON_COMBINE:
        OnCombine(mes);
        break;
    case MON_RELEASE_IN:
    case MON_RELEASE_OUT:
        OnRelease(mes);
        break;
    case MON_DVO_REDIRECT:
        OnDvoRedirect(mes);
        break;
	case MON_INT_RLS_IN:
		if(mes.id() == 0x040A0109)
			int a = 2;
			break;
    case MON_TIMECHANGE:
    case MON_HOLDEND:
	case MON_INT_CALL_OUT:
	case MON_INT_CALL_IN:
	case MON_INT_RLS_OUT:
    case MON_LINK_STATE:
	case MON_PORT_FREE:
    case MON_SPIDER_INFO:
	case MON_SPIDER_INFO_EX:
		break;

    default:
        m_dwLastError = ERROR_UNKNOWN_MESSAGE;
    }
    return (m_dwLastError == ERROR_NONE);

}

void CDRBuilding::CCDRBuilder::GetCDRList(CDRBuilding::TPCharList& lst)
{
	if(!lst.empty())
	{
		myASSERT(FALSE);
		return;
	}
	CopyList(lst, m_lstCDR);
	FreeListMem(m_lstCDR);
}

void CDRBuilding::CCDRBuilder::GetJournalList(CDRBuilding::TPCharList& lst)
{
	if(!lst.empty())
	{
		myASSERT(FALSE);
		return;
	}
	CopyList(lst, m_lstJour);
	FreeListMem(m_lstJour);
}

void CDRBuilding::CCDRBuilder::GetErrorList(CDRBuilding::TPCharList& lst)
{
	if(!lst.empty())
	{
		myASSERT(FALSE);
		return;
	}
	CopyList(lst, m_lstErr);
	FreeListMem(m_lstErr);
}

void CDRBuilding::CCDRBuilder::JournalAnalysisMes(CMonMessageEx& mes)
{
	if(!m_SJournalSettings.bEnable)
        return;
    int readPos = 0;    //mes.clearReadPosition();
    char sInfo[1000];
	sInfo[0] = 0;
    switch(mes.message())
    {
    case MON_WIZOVERLOAD:
        {
            BYTE btMod=mes.getParameterByte(readPos);
			myASSERT(btMod < MAX_MOD);
            const TClock* pClock=mes.getParameterTimePtr(readPos);
            sprintf(sInfo, "[%02d-%02d-20%02d %02d:%02d:%02d]   Переполнение WizNet (модуль №%02d)",
                pClock->Date,pClock->Month,pClock->Year,
                pClock->Hours,pClock->Minutes,pClock->Seconds,btMod);
        }
        break;
    case MON_FATOVERLOAD:
        {
            BYTE btMod=mes.getParameterByte(readPos);
			myASSERT(btMod < MAX_MOD);
            const TClock* pClock=mes.getParameterTimePtr(readPos);
            sprintf(sInfo, "[%02d-%02d-20%02d %02d:%02d:%02d]   Переполнение FAT (модуль №%02d)",
                pClock->Date,pClock->Month,pClock->Year,
                pClock->Hours,pClock->Minutes,pClock->Seconds,btMod);
        }
        break;
	case MON_COMOVERLOAD:
        {
            BYTE btMod=mes.getParameterByte(readPos);
			myASSERT(btMod < MAX_MOD);
            const TClock* pClock=mes.getParameterTimePtr(readPos);
            sprintf(sInfo, "[%02d-%02d-20%02d %02d:%02d:%02d]   Переполнение Com-порта (модуль №%02d)",
                pClock->Date,pClock->Month,pClock->Year,
                pClock->Hours,pClock->Minutes,pClock->Seconds,btMod);
        }
        break;
	case MON_TIMECHANGE:
        {
            BYTE btMod=mes.getParameterByte(readPos);
			myASSERT(btMod < MAX_MOD);
            const TClock* pClock=mes.getParameterTimePtr(readPos);
            sprintf(sInfo, "[%02d-%02d-20%02d %02d:%02d:%02d]   Изменение времени в модуле №%02d",
                pClock->Date,pClock->Month,pClock->Year,
                pClock->Hours,pClock->Minutes,pClock->Seconds,btMod);
        }
        break;
		/*
    case MON_LINK_STATE:
        {
            BYTE btPcmBeq = mes.getParameterByte(readPos);
            btPcmBeq++;
            BYTE btPcmQnt = mes.getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  MON_LINK_STATE    [%02d:%02d]  ",
                        mes.id(),mes.time(),btPcmBeq,btPcmBeq+btPcmQnt-1);
            BYTE param[4];
            CString sTmp;
            for(int i = 0; i < btPcmQnt; i++)
            {
                for(int j = 0; j < 4; j++)
                {
                    param[j] =  mes.getParameterByte(readPos);
                }
                sTmp.Format("(%02d:%02d:%02d:%02d) ",param[0],param[1],param[2],param[3]);
                    sInfo += sTmp;
            }
        }
        break;
    case MON_SPIDER_MODULE_UP:
        {
            BYTE btNum = mes.getParameterByte(readPos);
            TClock clock = *(pMonMes->getParameterTime());
            sprintf(sInfo, "[%02d-%02d-20%02d %02d:%02d:%02d]   Появился модуль №%02d",
                clock.Date,clock.Month,clock.Year,clock.Hours,clock.Minutes,clock.Seconds,btNum);
        }
        break;
    case MON_SPIDER_MODULE_DOWN:
        {
            BYTE btNum = mes.getParameterByte(readPos);
            TClock clock = *(pMonMes->getParameterTime());
            sprintf(sInfo, "[%02d-%02d-20%02d %02d:%02d:%02d]   Потерян модуль №%02d",
                clock.Date,clock.Month,clock.Year,clock.Hours,clock.Minutes,clock.Seconds,btNum);
        }
        break;
		*/
    case MON_SPIDER_TCP_DOWN:
        {
            TClock clock = *(mes.getParameterTimePtr(readPos));
            sprintf(sInfo, "[%02d-%02d-20%02d %02d:%02d:%02d]   Прервано TCP соединение с SComm'ом",
                clock.Date,clock.Month,clock.Year,clock.Hours,clock.Minutes,clock.Seconds);
        }
        break;
    case MON_SPIDER_USER_STOP:
        {
            TClock clock = *(mes.getParameterTimePtr(readPos));
            sprintf(sInfo, "[%02d-%02d-20%02d %02d:%02d:%02d]   SMPSpider был остановлен пользователем",
                clock.Date,clock.Month,clock.Year,clock.Hours,clock.Minutes,clock.Seconds);
        }
        break;
    case MON_SPIDER_TCP_UP:
        {
            TClock clock = *(mes.getParameterTimePtr(readPos));
            sprintf(sInfo, "[%02d-%02d-20%02d %02d:%02d:%02d]   Восстановлено TCP соединение с SComm'ом",
                clock.Date,clock.Month,clock.Year,clock.Hours,clock.Minutes,clock.Seconds);
        }
        break;
    case MON_SPIDER_START:
        {
            TClock clock = *(mes.getParameterTimePtr(readPos));
            sprintf(sInfo, "[%02d-%02d-20%02d %02d:%02d:%02d]   SMPSpider был запущен",
                clock.Date,clock.Month,clock.Year,clock.Hours,clock.Minutes,clock.Seconds);
        }
        break;
    default: ;
                
    }
	if(sInfo[0])
		AddJournalString(sInfo);
}

void CDRBuilding::CCDRBuilder::ResetModuleInfo(int iModNumber)
{
	if(iModNumber == ALL_MODULES)
	{
		int i; //changed by pax
		for(/*int*/ i = 0; i < MAX_MOD; i++)
		{
			m_ModuleInfo[i].dwCounter = 0;
			m_ModuleInfo[i].LastAliveTime.clock.Control = CLOCK_UNINITIALIZED;
			m_ModuleInfo[i].btLastRelTime = 0;
			m_ModuleInfo[i].dwLastCheckComoverCallsGlTime = 0;
		}
		for(i = 0; i < MAX_MOD; i++)
			for(int j = 0; j < MAX_MOD; j++)
				if(i != j)
					m_ModDelta[i][j] = MOD_DELTA_UNINIT;
				else
					m_ModDelta[i][j] = 0;
	}
	else if(iModNumber <= MAX_MOD)
	{
		m_ModuleInfo[iModNumber].dwCounter = 0;
		m_ModuleInfo[iModNumber].LastAliveTime.clock.Control = CLOCK_UNINITIALIZED;
		m_ModuleInfo[iModNumber].btLastRelTime = 0;
		m_ModuleInfo[iModNumber].dwLastCheckComoverCallsGlTime = 0;

		for(int i = 0; i < MAX_MOD; i++)
		{
			if(i != iModNumber)
			{
				m_ModDelta[iModNumber][i] = MOD_DELTA_UNINIT;
				m_ModDelta[i][iModNumber] = MOD_DELTA_UNINIT;
			}
		}
	}
	else
	{
		myASSERT(FALSE);
	}
}

void CDRBuilding::CCDRBuilder::OnAlive(CMonMessageEx& mes)
{
    int readPos = 0;    // mes.clearReadPosition();
    BYTE btMod = mes.getParameterByte(readPos);

	/*
	if(btMod == 1)
	{
		char sStrMessage[1500];
		TransformMessageToString(pMonMes, sStrMessage, 500);
		char sBuf[2000];
		sprintf(sBuf, "%s Mes:%4d  Bytes:%7d\n", sStrMessage, iCntMes, iCntBytes);
		iCntMes = 0;
		iCntBytes = 0;
		FILE* stream = fopen("d:\\3\\test.txt", "a+t");
		fputs(sBuf, stream);
		fclose(stream);
	}
	*/

	if(btMod >= MAX_MOD)
	{
		char str[500];
		sprintf(str, "В сообщении ALIVE неверный номер модуля - %d", btMod);
		AddErrorString(str);
		return;
	}
	myASSERT(btMod < MAX_MOD);
	CheckModuleInfo(btMod, mes);

    //readPos = 0;
    //btMod = mes.getParameterByte(readPos);//CheckModuleInfo изменяет позицию
	myASSERT(btMod < MAX_MOD);

    DWORD dwNewGlTime = GetGlobalTime(btMod, mes.time());
    const TClock* pClock = mes.getParameterTimePtr(readPos);

    if(!IsAcceptedTime(pClock))
    {
		char sStrMessage[1500];
		TransformMessageToString(mes, sStrMessage, 500);
		char sError[1500];
		strcpy(sError, "Неверное время - ");
		strcat(sError, sStrMessage);
		AddErrorString(sError);
    }
    else
    {
        if(m_ModuleInfo[btMod].LastAliveTime.clock.Control == CLOCK_UNINITIALIZED)
            m_ModuleInfo[btMod].LastAliveTime.clock.Control = 0;
        //присваиваем новые значения
        //глобальное время
        m_ModuleInfo[btMod].LastAliveTime.dwGlTime = dwNewGlTime;
        //реальное время    
        memcpy((void*)&m_ModuleInfo[btMod].LastAliveTime.clock, pClock, sizeof(TClock));
    }
}

void CDRBuilding::CCDRBuilder::CheckModuleInfo(BYTE btMod, CMonMessageEx& mes)
{
    //в ISUP иногда можен сначала придти 19 а потмо 18 и т.п.
	if(m_ModuleInfo[btMod].dwCounter!= 0 || m_ModuleInfo[btMod].btLastRelTime != 0)
	{//если уже были мессаги
		BYTE btDif = m_ModuleInfo[btMod].btLastRelTime - mes.time();
		if(btDif < 3)
			mes.time(m_ModuleInfo[btMod].btLastRelTime);
	}
    //проверка на истечении времени после COMOVERLOAD
    DWORD dwNewGlTime = GetGlobalTime(btMod, mes.time());
    if(dwNewGlTime - m_ModuleInfo[btMod].dwLastCheckComoverCallsGlTime > PERIOD_CHECK_COMOVER_CALLS)
    {
        ReleaseComoverCalls();
        //передвигаем счетчики
        for(int i = 1; i < MAX_MOD; i++)
        {
            m_ModuleInfo[i].dwLastCheckComoverCallsGlTime = GetGlobalTime(i, m_ModuleInfo[i].btLastRelTime);
        }
    }

    if(m_ModuleInfo[btMod].LastAliveTime.clock.Control != CLOCK_UNINITIALIZED)
    {//если ALIVE уже был
        //сравниваем разницу в относ. времени с передыдущей мессагой
        BYTE btBreak = mes.time() - m_ModuleInfo[btMod].btLastRelTime;
        if(btBreak > MAX_RELATIVE_TIME_BREAK)
        {//если имеет место разрыв
			char sStrMessage[1500];
			TransformMessageToString(mes, sStrMessage, 500);
			char sError[1500];
			sprintf(sError, "break-%03d  ", btBreak);
			strcat(sError, sStrMessage);
			AddErrorString(sError);
            OnComoverload(btMod);
        }
    }

//    if((m_ModuleInfo[btMod].btLastRelTime >= 240)&&(btRelTime <= 15))
    if(m_ModuleInfo[btMod].btLastRelTime > mes.time())
    {
        m_ModuleInfo[btMod].dwCounter ++;
    }

    m_ModuleInfo[btMod].btLastRelTime = mes.time();
//	mes.clearReadPosition();	//TransformMessageToString может сдвинуть позицию, поэтому лучше всегда устанавливать на начало
}

DWORD CDRBuilding::CCDRBuilder::GetGlobalTime(BYTE btMod, BYTE btRelTime)
{
    return m_ModuleInfo[btMod].dwCounter * 256 + btRelTime;
}

BOOL CDRBuilding::CCDRBuilder::TransformMessageToString(CMonMessageEx& mes, char* pBuffer, int iBufferSize)
{
	int readPos = 0;    // mes.clearReadPosition();
    char sInfo[500];
    switch(mes.message())
    {
    case MON_ALIVE:
        {
            BYTE btMod=mes.getParameterByte(readPos);
            const TClock* pClock=mes.getParameterTimePtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  ALIVE        (%02d)  [%02d-%02d-20%02d %02d:%02d:%02d]",
                mes.id(),mes.time(),btMod,pClock->Date,pClock->Month,pClock->Year,
                pClock->Hours,pClock->Minutes,pClock->Seconds);
        }
        break;
	case MON_COMOVERLOAD:
        {
            BYTE btMod=mes.getParameterByte(readPos);
            const TClock* pClock=mes.getParameterTimePtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  COMOVERLOAD  (%02d)  [%02d-%02d-20%02d %02d:%02d:%02d]",
                mes.id(),mes.time(),btMod,pClock->Date,pClock->Month,pClock->Year,
                pClock->Hours,pClock->Minutes,pClock->Seconds);
        }
        break;
	case MON_WIZOVERLOAD:
        {
            BYTE btMod=mes.getParameterByte(readPos);
            const TClock* pClock=mes.getParameterTimePtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  WIZOVERLOAD  (%02d)  [%02d-%02d-20%02d %02d:%02d:%02d]",
                mes.id(),mes.time(),btMod,pClock->Date,pClock->Month,pClock->Year,
                pClock->Hours,pClock->Minutes,pClock->Seconds);
        }
        break;
	case MON_FATOVERLOAD:
        {
            BYTE btMod=mes.getParameterByte(readPos);
            const TClock* pClock=mes.getParameterTimePtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  FATOVERLOAD  (%02d)  [%02d-%02d-20%02d %02d:%02d:%02d]",
                mes.id(),mes.time(),btMod,pClock->Date,pClock->Month,pClock->Year,
                pClock->Hours,pClock->Minutes,pClock->Seconds);
        }
        break;
	case MON_TIMECHANGE:
        {
            BYTE btMod=mes.getParameterByte(readPos);
            const TClock* pClock=mes.getParameterTimePtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  TIMECHANGE   (%02d)  [%02d-%02d-20%02d %02d:%02d:%02d]",
                mes.id(),mes.time(),btMod,pClock->Date,pClock->Month,pClock->Year,
                pClock->Hours,pClock->Minutes,pClock->Seconds);
        }
        break;
	case MON_INT_CALL_OUT:
		{
            BYTE btMod=mes.getParameterByte(readPos);
            BYTE btPcm=mes.getParameterByte(readPos);
            BYTE btKI=mes.getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  MON_INT_CALL_OUT  (%02d:%02d:%02d)",
                        mes.id(),mes.time(),btMod,btPcm,btKI);
		}
		break;
	case MON_INT_CALL_IN:
		{
            BYTE btMod=mes.getParameterByte(readPos);
            BYTE btPcm=mes.getParameterByte(readPos);
            BYTE btKI=mes.getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  MON_INT_CALL_IN   (%02d:%02d:%02d)",
                        mes.id(),mes.time(),btMod,btPcm,btKI);
		}
		break;
	case MON_INT_RLS_OUT:
		{
            BYTE btMod=mes.getParameterByte(readPos);
            BYTE btPcm=mes.getParameterByte(readPos);
            BYTE btKI=mes.getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  MON_INT_RLS_OUT   (%02d:%02d:%02d)",
                        mes.id(),mes.time(),btMod,btPcm,btKI);
		}
		break;
	case MON_INT_RLS_IN:
		{
            BYTE btMod=mes.getParameterByte(readPos);
            BYTE btPcm=mes.getParameterByte(readPos);
            BYTE btKI=mes.getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  MON_INT_RLS_IN    (%02d:%02d:%02d)",
                        mes.id(),mes.time(),btMod,btPcm,btKI);
		}
		break;
    case MON_LINK_STATE:
        {
            BYTE btPcmBeq = mes.getParameterByte(readPos);
            btPcmBeq++;
            BYTE btPcmQnt = mes.getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  MON_LINK_STATE    [%02d:%02d]  ",
                        mes.id(),mes.time(),btPcmBeq,btPcmBeq+btPcmQnt-1);
            BYTE param[4];
            char sTmp[500];
            for(int i = 0; i < btPcmQnt; i++)
            {
                for(int j = 0; j < 4; j++)
                {
                    param[j] =  mes.getParameterByte(readPos);
                }
                sprintf(sTmp, "(%02d:%02d:%02d:%02d) ",param[0],param[1],param[2],param[3]);
                strcat(sInfo, sTmp);
            }
        }
        break;
	case MON_PORT_FREE:
		{
            BYTE btMod=mes.getParameterByte(readPos);
            BYTE btPcm=mes.getParameterByte(readPos);
            BYTE btKI=mes.getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  MON_PORT_FREE     (%02d:%02d:%02d)",
                        mes.id(),mes.time(),btMod,btPcm,btKI);
		}
		break;
    case MON_DVO_REDIRECT:
        {
            BYTE btDvocode=mes.getParameterByte(readPos);
            const char* pNumFrom = mes.getParameterStringPtr(readPos);
            const char* pNumTo = mes.getParameterStringPtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  MON_DVO_REDIRECT  dvo:%03d num_from:%s num_to:%s)",
                        mes.id(),mes.time(),btDvocode,pNumFrom[0]?pNumFrom:"-",pNumTo[0]?pNumTo:"-");
        }
        break;
    case MON_HOLDEND:
        {
            sprintf(sInfo, "%08X  (%03d)  MON_HOLDEND",mes.id(),mes.time());
        }
        break;
    case MON_SEIZURE:
        {
            BYTE btSig=mes.getParameterByte(readPos);
            BYTE btMod=mes.getParameterByte(readPos);
            BYTE btSlotPcm=mes.getParameterByte(readPos);
            BYTE btPort=mes.getParameterByte(readPos);
            const char* pNum=mes.getParameterStringPtr(readPos);
            const char* pAon=mes.getParameterStringPtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  SEIZURE      %s  %s %s  (%02d:%02d:%02d)",
                        mes.id(),mes.time(),SigTypeToStr(btSig),pNum[0]?pNum:"-",pAon[0]?pAon:"-",btMod,btSlotPcm,btPort);
        }
        break;
    case MON_NUMBER:
        {
            const char* pNum=mes.getParameterStringPtr(readPos);
            const char* pAon=mes.getParameterStringPtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  NUMBER       %s %s",
                mes.id(),mes.time(),pNum[0]?pNum:"-",pAon[0]?pAon:"-");
        }
        break;
    case MON_ACCEPT:
            sprintf(sInfo, "%08X  (%03d)  ACCEPT       <%08X>",
                mes.id(),mes.time(),mes.getParameterDWord(readPos));
        break;
    case MON_CALL:
        {
            DWORD dwRemID=mes.getParameterDWord(readPos);
            BYTE btSig=mes.getParameterByte(readPos);
            BYTE btMod=mes.getParameterByte(readPos);
            BYTE btSlotPcm=mes.getParameterByte(readPos);
            BYTE btPort=mes.getParameterByte(readPos);
            const char* pNum=mes.getParameterStringPtr(readPos);
            const char* pAon=mes.getParameterStringPtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  CALL         %s  %s %s  (%02d:%02d:%02d)  <%08X>",
                        mes.id(),mes.time(),SigTypeToStr(btSig),pNum[0]?pNum:"-",pAon[0]?pAon:"-",btMod,btSlotPcm,btPort,dwRemID);
        }
        break;
    case MON_ANSWER:
            sprintf(sInfo, "%08X  (%03d)  ANSWER       ",mes.id(),mes.time());
        break;
    case MON_RELEASE_IN:
        {
            BYTE btReason = mes.getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  RELEASE_IN   (%03d)",mes.id(),mes.time(),btReason);
        }
        break;
    case MON_RELEASE_OUT:
        {
            // reason:BYTE
            BYTE btReason = mes.getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  RELEASE_OUT  (%03d)",mes.id(),mes.time(),btReason);
        }
        break;
    case MON_COMBINE:
            sprintf(sInfo, "%08X  (%03d)  COMBINE      <%08X>",mes.id(),mes.time(),mes.getParameterDWord(readPos));
        break;
    case MON_SPIDER_MODULE_DOWN:
        {
            BYTE btNum = mes.getParameterByte(readPos);
            TClock clock = *(mes.getParameterTimePtr(readPos));
            sprintf(sInfo, "SPIDER_MODULE_DOWN  [%02d-%02d-20%02d %02d:%02d:%02d]  mod:%02d",
                clock.Date,clock.Month,clock.Year,clock.Hours,clock.Minutes,clock.Seconds,btNum);
        }
        break;
    case MON_SPIDER_TCP_DOWN:
        {
            TClock clock = *(mes.getParameterTimePtr(readPos));
            sprintf(sInfo, "SPIDER_TCP_DOWN     [%02d-%02d-20%02d %02d:%02d:%02d]",
                clock.Date,clock.Month,clock.Year,clock.Hours,clock.Minutes,clock.Seconds);
        }
        break;
    case MON_SPIDER_USER_STOP:
        {
            TClock clock = *(mes.getParameterTimePtr(readPos));
            sprintf(sInfo, "SPIDER_USER_STOP    [%02d-%02d-20%02d %02d:%02d:%02d]",
                clock.Date,clock.Month,clock.Year,clock.Hours,clock.Minutes,clock.Seconds);
        }
        break;
    case MON_SPIDER_MODULE_UP:
        {
            BYTE btNum = mes.getParameterByte(readPos);
            TClock clock = *(mes.getParameterTimePtr(readPos));
            sprintf(sInfo, "SPIDER_MODULE_UP    [%02d-%02d-20%02d %02d:%02d:%02d]  mod:%02d",
                clock.Date,clock.Month,clock.Year,clock.Hours,clock.Minutes,clock.Seconds,btNum);
        }
        break;
    case MON_SPIDER_TCP_UP:
        {
            TClock clock = *(mes.getParameterTimePtr(readPos));
            sprintf(sInfo, "SPIDER_TCP_UP       [%02d-%02d-20%02d %02d:%02d:%02d]",
                clock.Date,clock.Month,clock.Year,clock.Hours,clock.Minutes,clock.Seconds);
        }
        break;
    case MON_SPIDER_START:
        {
            TClock clock = *(mes.getParameterTimePtr(readPos));
            sprintf(sInfo, "SPIDER_START        [%02d-%02d-20%02d %02d:%02d:%02d]",
                clock.Date,clock.Month,clock.Year,clock.Hours,clock.Minutes,clock.Seconds);
        }
        break;
    case MON_SPIDER_INFO:
        {
            DWORD dwOSver = mes.getParameterDWord(readPos);
            DWORD dwSPIDERver = mes.getParameterDWord(readPos);
            TClock clock = *(mes.getParameterTimePtr(readPos));

            char sSpiderVer[500];
            sprintf(sSpiderVer, "%d.%d.%d.%d",HIWORD(dwSPIDERver) >> 8,HIWORD(dwSPIDERver) & 0xFF,LOWORD(dwSPIDERver) >> 8,LOWORD(dwSPIDERver) & 0xFF);

            sprintf(sInfo, "SPIDER_INFO         OS:%08X Spider:%s  [%02d-%02d-20%02d %02d:%02d:%02d]",dwOSver, sSpiderVer,
                    clock.Date,clock.Month,clock.Year,clock.Hours,clock.Minutes,clock.Seconds);

        }
        break;
    case MON_SPIDER_INFO_EX:
        {
			const char* OSver = mes.getParameterStringPtr(readPos);
            DWORD dwSPIDERver = mes.getParameterDWord(readPos);
            TClock clock = *(mes.getParameterTimePtr(readPos));

            char sSpiderVer[500];
            sprintf(sSpiderVer, "%d.%d.%d.%d",HIWORD(dwSPIDERver) >> 8,HIWORD(dwSPIDERver) & 0xFF,LOWORD(dwSPIDERver) >> 8,LOWORD(dwSPIDERver) & 0xFF);

            sprintf(sInfo, "SPIDER_INFO_EX      OS:%s Spider:%s  [%02d-%02d-20%02d %02d:%02d:%02d]", OSver, sSpiderVer,
                    clock.Date,clock.Month,clock.Year,clock.Hours,clock.Minutes,clock.Seconds);

        }
        break;
    default:
            sprintf(sInfo, "UNKNOWN MESSAGE %d",mes.message());

    }

	BOOL bRes;
	if(iBufferSize > (int)strlen(sInfo))
	{
		strcpy(pBuffer, sInfo);
		bRes = TRUE;
	}
	else
	{
		pBuffer[0] = 0;
		bRes = FALSE;
	}
    return bRes;
}

const char* CDRBuilding::CCDRBuilder::SigTypeToStr(BYTE sig)
{
        switch (sig)
        {
        case SIGNALLING_EXT:
            return "SIG_EXT";
        case SIGNALLING_CITY:
            return "SIG_CITY";
        case SIGNALLING_DSS1:
            return "SIG_DSS1";
        case SIGNALLING_CAS2_IN:
            return "SIG_CAS2_IN";
        case SIGNALLING_CAS2_OUT:
            return "SIG_CAS2_OUT";
        case SIGNALLING_V5_ANSA:
            return "SIG_V5_ANSA";
        case SIGNALLING_V5_ANPSTN:
            return "SIG_V5_ANPSTN";
        case SIGNALLING_V52AN_PCMPORT:
            return "SIG_V52AN_PCMPORT";
        case SIGNALLING_V52AN_SUBPORT:
            return "SIG_V52AN_SUBPORT";
        case SIGNALLING_V52LE_PCMPORT:
            return "SIG_V52LE_PCMPORT";
        case SIGNALLING_V52LE_SUBPORT:
            return "SIG_V52LE_SUBPORT";
        case SIGNALLING_CAS1:
            return "SIG_CAS1";
        case SIGNALLING_SORM:
            return "SIG_SORM";
        case SIGNALLING_DISA:
            return "SIG_DISA";
        case SIGNALLING_SERIAL:
            return "SIG_SERIAL";
        case SIGNALLING_ISUP:
            return "SIG_ISUP";
        case SIGNALLING_SPEAKER:
            return "SIG_SPEAKER";
        case SIGNALLING_PERMANENT:
            return "SIG_PERMANENT";
        case SIGNALLING_3SL_IN:
            return "SIG_3SL_IN";
        case SIGNALLING_3SL_OUT:
            return "SIG_3SL_OUT";
        case SIGNALLING_CAS_TEST:
            return "SIG_CAS_TEST";
        case SIGNALLING_INT:
            return "SIG_INT";
        case SIGNALLING_BUTTON:
            return "SIG_BUTTON";
        case SIGNALLING_ADASEPULT:
            return "SIG_ADASEPULT";
        case SIGNALLING_CAS1_NORKA_OUT:
            return "SIG_CAS1_NORKA_OUT";
        case SIGNALLING_CAS1_NORKA_IN:
            return "SIG_CAS1_NORKA_IN";
        case SIGNALLING_ISDN:
            return "SIG_ISDN";
        case SIGNALLING_EM:
            return "SIG_EM";

        default:
            return "???(SIG - undefined)";
        }
    return "??? (???)";
}
DWORD* p1;

void CDRBuilding::CCDRBuilder::OnComoverload(BYTE btMod)
{
    bool b;
	strCallInfo* pCall = m_pBegListCall;
    BYTE btLastRelTime;
    while(pCall)
    {
#ifdef _DEBUG
		if(pCall->InUnit.dwID == 15663393)
			int a = 2;
#endif
        if(btMod == ALL_MODULES)
            b = true;
        else
            b = (pCall->InUnit.btMod == btMod);
        if( b && (pCall->InUnit.btMod != 0) && (pCall->InUnit.dwComoverGlTime == 0) )
        {//если SEIZURE был
            btLastRelTime = m_ModuleInfo[pCall->InUnit.btMod].btLastRelTime;
            pCall->InUnit.dwComoverGlTime = GetGlobalTime(pCall->InUnit.btMod, btLastRelTime);
			if(pCall->InUnit.dwSeizGlTime == 0)
				pCall->InUnit.dwSeizGlTime = pCall->InUnit.dwComoverGlTime;
			else
				if(pCall->bTalk && pCall->InUnit.dwBegTalkGlTime == 0)
					pCall->InUnit.dwBegTalkGlTime = pCall->InUnit.dwComoverGlTime;
        }
        if(btMod == ALL_MODULES)
            b = true;
        else
            b = (pCall->OutUnit.btMod == btMod);
        if(b && (pCall->OutUnit.btMod != 0) && (pCall->OutUnit.dwComoverGlTime == 0) )
        {//если был CALL
            btLastRelTime = m_ModuleInfo[pCall->OutUnit.btMod].btLastRelTime;
            pCall->OutUnit.dwComoverGlTime = GetGlobalTime(pCall->OutUnit.btMod, btLastRelTime);
			if(pCall->OutUnit.dwSeizGlTime == 0)
				pCall->OutUnit.dwSeizGlTime = pCall->OutUnit.dwComoverGlTime;
			else
				if(pCall->bTalk && pCall->OutUnit.dwBegTalkGlTime == 0)
					pCall->OutUnit.dwBegTalkGlTime = pCall->OutUnit.dwComoverGlTime;
        }
        pCall = pCall->pNextCall;
    }
}

void CDRBuilding::CCDRBuilder::ReleaseComoverCalls(void)
{
    BYTE btReason = RELEASE_CAUSE_COMOVERLOAD;
    strCallInfo* pNext,*pCall = m_pBegListCall;
    DWORD pMasGlTime[MAX_MOD];
    BYTE btMod;
    for(int i = 0; i < MAX_MOD; i++)
        pMasGlTime[i] = 0;
    while((pCall)&&(GetLastError() == 0))
    {
        pNext = pCall->pNextCall;

        if(pCall->InUnit.dwSeizGlTime != 0 && pCall->InUnit.btMod != 0)
        {
            btMod = pCall->InUnit.btMod;
			myASSERT(btMod < MAX_MOD);
            if(pMasGlTime[btMod] == 0)
                pMasGlTime[btMod] = GetGlobalTime(btMod,m_ModuleInfo[btMod].btLastRelTime);
            if(pMasGlTime[btMod] - pCall->InUnit.dwSeizGlTime > MAX_TALK_DURATION)
			{
                ReleaseCall(pCall, btReason);
			}
        }
		else
        if(pCall->OutUnit.dwSeizGlTime != 0 && pCall->OutUnit.btMod != 0)
        {
            btMod = pCall->OutUnit.btMod;
			myASSERT(btMod < MAX_MOD);
            if(pMasGlTime[btMod] == 0)
                pMasGlTime[btMod] = GetGlobalTime(btMod,m_ModuleInfo[btMod].btLastRelTime);
            if(pMasGlTime[btMod] - pCall->OutUnit.dwSeizGlTime > MAX_TALK_DURATION)
			{
                ReleaseCall(pCall, btReason);
			}
        }
        pCall = pNext;
    }
}

bool CDRBuilding::CCDRBuilder::IsAcceptedTime(const TClock* pClock)
{
    bool bRes = true;
    if(pClock->Year > 38)//CTime больше не воспринимает
        bRes = false;
    if( (pClock->Month < 1) || (pClock->Month > 12) )
        bRes = false;
    if( (pClock->Date < 1) || (pClock->Date > 31) )
        bRes = false;
    if(pClock->Hours > 23)
        bRes = false;
    if(pClock->Minutes > 59)
        bRes = false;
    if(pClock->Seconds > 59)
        bRes = false;
    return bRes;
}

BYTE CDRBuilding::CCDRBuilder::ExtractModNumber(DWORD dwID)
{
	return HIBYTE(LOWORD(dwID));
}

CDRBuilding::strCallInfo* CDRBuilding::CCDRBuilder::FindCall(DWORD dwID)
{
    strCallInfo* pRes = NULL;
    strCallInfo* pCall = m_pBegListCall;
    while(pCall)
    {
        if(pCall->InUnit.dwID == dwID)
        {
            pRes = pCall;
            break;
        }
        if(pCall->OutUnit.dwID == dwID)
        {
            pRes = pCall;
            break;
        }
        pCall = pCall->pNextCall;
    }
    return pRes;
}

void CDRBuilding::CCDRBuilder::OnSeizure(CMonMessageEx& mes)
{
#ifdef _DEBUG
	if(mes.id() == 0xC038013C)
	{
		int a = 2;
	}
#endif
    // sig:BYTE module:BYTE slot/pcm:BYTE hole/ki:BYTE number:STRING aon:STRING
    //берем ID, Mod, Slot-PCM, KI-Port, Номер
    if(mes.id() == 0)
    {
        AddErrorString("SEIZURE : ID = 00000000");
        return;
    }

    //смотрим, чтоб не было до этого звонков с таким-же ID, может например, сначала Call придти
    strCallInfo* pCall = FindCall(mes.id());
	if(pCall)
	{
		//проверяем этот ID от Call'а или звонок с дубоирующимся CallID
		if( !(pCall->InUnit.dwID == mes.id() && pCall->InUnit.btPcmSlot == 0) )
		{
			ReleaseCall(pCall, RELEASE_CAUSE_DOUBLECALLID);
			pCall = FindCall(mes.id());
		}
	}

    if(pCall == NULL)
    {// мог CALL раньше придти
        pCall = AddNewCall();
#ifdef CONTROL_CALLS_COUNT
		m_iCallsCount++;
		/*
//		myTRACE("CallsCount - %d\n", m_iCallsCount);
		if(m_iCallsCount > m_iMaxCallsCount)
		{
			m_iMaxCallsCount = m_iCallsCount;
			myTRACE("MaxCallsCount - %d\n", m_iMaxCallsCount);
		}
		*/
#endif

    }

    //получаем параметры
    int readPos = 0;    // mes.clearReadPosition();
    BYTE btSig = mes.getParameterByte(readPos);
    BYTE btMod = mes.getParameterByte(readPos);
	myASSERT(btMod < MAX_MOD);
    BYTE btPcm = mes.getParameterByte(readPos);
    BYTE btKI = mes.getParameterByte(readPos);
    const char* pNum = mes.getParameterStringPtr(readPos);
    const char* pAon = mes.getParameterStringPtr(readPos);

    CheckModuleInfo(btMod, mes);

    //заполняем InUnit
    pCall->InUnit.btSig = btSig;
    pCall->InUnit.dwID = mes.id();
    pCall->InUnit.btMod = btMod;
    pCall->InUnit.btPcmSlot = btPcm;
    pCall->InUnit.btKIPort = btKI;

    // ставим глобальное время
    pCall->InUnit.dwSeizGlTime = GetGlobalTime(btMod, mes.time());
	WriteAliveTime(&pCall->InUnit);

    //вычисляем примерную дельту если Call пришел раньше 
	if(pCall->OutUnit.dwID != 0 && m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] == MOD_DELTA_UNINIT && pCall->InUnit.btMod != pCall->OutUnit.btMod)
	{

		m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] = pCall->OutUnit.dwSeizGlTime - pCall->InUnit.dwSeizGlTime;
		m_ModDelta[pCall->OutUnit.btMod][pCall->InUnit.btMod] = (-1)*m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod];
	}

    //абонентский номер
    strncpy(pCall->InUnit.acSubsNumber, pNum, MAX_LEN_NUMBER-1);
    pCall->InUnit.acSubsNumber[MAX_LEN_NUMBER-1] = 0;
    //аон
    strncpy(pCall->InUnit.acSubsAON, pAon, MAX_LEN_NUMBER-1);
    pCall->InUnit.acSubsAON[MAX_LEN_NUMBER-1] = 0;

	pCall->InUnit.dwComoverGlTime = 0; //мало ли при оверлоаде выставился, если CALL пришел быстрее
	
}

void CDRBuilding::CCDRBuilder::AddErrorString(const char* sStr)
{
	char* pTmp = new char[strlen(sStr)+1];
	strcpy(pTmp, sStr);
	m_lstErr.push_back(pTmp);
}

CDRBuilding::strCallInfo* CDRBuilding::CCDRBuilder::AddNewCall(void)
{
    strCallInfo* pCall = new strCallInfo;
    memset(pCall->abReserved,0,STRCALLINFO_RESERVED);
    memset(pCall->acRedirBuf,0,DVO_REDIR_BUFFER);
    //если Seizure не придет
    pCall->InUnit.dwID = 0;
    pCall->InUnit.btSig = 0;
    pCall->InUnit.btMod = 0;
    pCall->InUnit.btPcmSlot = 0;
    pCall->InUnit.btKIPort = 0;
    pCall->InUnit.acSubsNumber[0] = 0;
    pCall->InUnit.acSubsAON[0] = 0;
    pCall->InUnit.acCdPN[0] = 0;
    pCall->InUnit.acCgPN[0] = 0;
	pCall->InUnit.dwBegTalkGlTime = 0;
    pCall->InUnit.dwReleaseGlTime = 0;
    pCall->InUnit.dwSeizGlTime = 0;
    pCall->InUnit.dwComoverGlTime = 0;
    memset(&pCall->InUnit.clockSeiz, 0, sizeof(TClock));
    memset(&pCall->InUnit.AliveClock, 0, sizeof(TClock));
    memset(pCall->InUnit.abReserved, 0, STRCALLINFOUNIT_RESERVED);

    //если Call не придет
    pCall->OutUnit.btSig = 0;
    pCall->OutUnit.btMod = 0;
    pCall->OutUnit.btPcmSlot = 0;
    pCall->OutUnit.btKIPort = 0;
    pCall->OutUnit.dwID = 0;
    pCall->OutUnit.acSubsNumber[0] = 0;
    pCall->OutUnit.acSubsAON[0] = 0;
    pCall->OutUnit.acCdPN[0] = 0;
    pCall->OutUnit.acCgPN[0] = 0;
    pCall->OutUnit.dwBegTalkGlTime = 0;
    pCall->OutUnit.dwReleaseGlTime = 0;
    pCall->OutUnit.dwSeizGlTime = 0;
    pCall->OutUnit.dwComoverGlTime = 0;
    memset(&pCall->OutUnit.clockSeiz, 0, sizeof(TClock));
    memset(&pCall->OutUnit.AliveClock, 0, sizeof(TClock));
    memset(pCall->OutUnit.abReserved, 0, STRCALLINFOUNIT_RESERVED);

    pCall->bTalk = false;
    pCall->iDelta1 = 0;
    pCall->dwNextCombineCallID = 0;
    pCall->dwPrevCombineCallID = 0;

    pCall->pPrevCall = m_pEndListCall;
    //последнему элементу присваиваем указатель на него
    if(m_pEndListCall)
        m_pEndListCall->pNextCall = pCall;
    //указю на след - NULL
    pCall->pNextCall = NULL;
    //он становится последним
    m_pEndListCall = pCall;
    if(m_pBegListCall == NULL) //если список пустой
        m_pBegListCall = pCall;
    return pCall;

}

BOOL CDRBuilding::CCDRBuilder::TransformGlTimeToClock(BYTE btMod, DWORD dwGlTime, TClock& cl)
{
    if(m_ModuleInfo[btMod].LastAliveTime.clock.Control == CLOCK_UNINITIALIZED)
        return FALSE;
	int iDelta = dwGlTime - m_ModuleInfo[btMod].LastAliveTime.dwGlTime;
	cl = AddToClock(m_ModuleInfo[btMod].LastAliveTime.clock, iDelta);
    cl.Control = CLOCK_INITIALIZED_EX;
	return TRUE;
}

BYTE CDRBuilding::CCDRBuilder::GetDaysInMonth(BYTE btMonth, BYTE btYear)
{
	BYTE btRes = 0;
	switch(btMonth)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		btRes = 31;
		break;
	case 4:
	case 6:
	case 9:
	case 11:
		btRes = 30;
		break;
	case 2:
		btRes = (btYear % 4 == 0) ? 29 : 28;
		break;
	default:
		myASSERT(FALSE);
	}
	return btRes;
}

void CDRBuilding::CCDRBuilder::OnCall(CMonMessageEx& mes)
{
#ifdef _DEBUG
	if(mes.id() == 0xEE47323D)
		int a = 2;
#endif
    // remID:DWORD sig:BYTE module:BYTE slot/pcm:BYTE port/ki:BYTE number:STRING aon:STRING
    int readPos = 0;    // mes.clearReadPosition();
    if(mes.id() == 0)
    {
        AddErrorString("CALL : ID = 00000000");
        return;
    }
    strCallInfo* pCall;
	//смотрим нету ли звонков с таким ID, маловероятно, но могут быть
	pCall = FindCall(mes.id()); 
	if(pCall != NULL)
	{
		ReleaseCall(pCall, RELEASE_CAUSE_DOUBLECALLID);
	}

    //получаем параметры
    DWORD dwRemID = mes.getParameterDWord(readPos);
    BYTE btSig = mes.getParameterByte(readPos);
    BYTE btMod = mes.getParameterByte(readPos);
	myASSERT(btMod < MAX_MOD);
    BYTE btPcm = mes.getParameterByte(readPos);
    BYTE btKI = mes.getParameterByte(readPos);
    const char* pNum = mes.getParameterStringPtr(readPos);
    const char* pAon = mes.getParameterStringPtr(readPos);

    CheckModuleInfo(btMod, mes);

	bool bWasSeiz = true;
    if(dwRemID == 0)
    {
        char sTmp[100];
        sprintf(sTmp, "CALL :(mes.id()=%08X) remID = 00000000", mes.id());
        AddErrorString(sTmp);
        pCall = AddNewCall();
#ifdef CONTROL_CALLS_COUNT
		m_iCallsCount++;
		/*
//		myTRACE("CallsCount - %d\n", m_iCallsCount);
		if(m_iCallsCount > m_iMaxCallsCount)
		{
			m_iMaxCallsCount = m_iCallsCount;
			myTRACE("MaxCallsCount - %d\n", m_iMaxCallsCount);
		}
		*/
#endif

    }
    else
    {
        pCall = FindCall(dwRemID); 
        if(pCall == NULL)
        {
            //если небыло SEIZURE
			bWasSeiz = false;
            pCall = AddNewCall();

#ifdef CONTROL_CALLS_COUNT
			m_iCallsCount++;
			/*
	//		myTRACE("CallsCount - %d\n", m_iCallsCount);
			if(m_iCallsCount > m_iMaxCallsCount)
			{
				m_iMaxCallsCount = m_iCallsCount;
				myTRACE("MaxCallsCount - %d\n", m_iMaxCallsCount);
			}
			*/
#endif

            pCall->InUnit.dwID = dwRemID;
			pCall->InUnit.btMod = ExtractModNumber(pCall->InUnit.dwID);
			myASSERT(pCall->InUnit.btMod < MAX_MOD);
			if(pCall->InUnit.btMod < MAX_MOD)
			{
				//ставим время занятия - последнее время сообщения модуля
				pCall->InUnit.dwSeizGlTime = GetGlobalTime(pCall->InUnit.btMod, m_ModuleInfo[pCall->InUnit.btMod].btLastRelTime);
				WriteAliveTime(&pCall->InUnit);
			}
        }
    }

    //заполняем OutUnit
    pCall->OutUnit.btSig = btSig;
    pCall->OutUnit.dwID = mes.id();
    pCall->OutUnit.btMod = btMod;
    pCall->OutUnit.btPcmSlot = btPcm;
    pCall->OutUnit.btKIPort = btKI;

    // ставим глобальное время
    pCall->OutUnit.dwSeizGlTime = GetGlobalTime(btMod, mes.time());
	WriteAliveTime(&pCall->OutUnit);
	
    //вычисляем дельту
	if(bWasSeiz && m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] == MOD_DELTA_UNINIT  && pCall->InUnit.btMod != pCall->OutUnit.btMod)
	{
		m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] = pCall->OutUnit.dwSeizGlTime - pCall->InUnit.dwSeizGlTime;
		m_ModDelta[pCall->OutUnit.btMod][pCall->InUnit.btMod] = (-1)*m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod];
	}

    //абонентский номер
    strncpy(pCall->OutUnit.acSubsNumber,pNum,MAX_LEN_NUMBER-1);
    pCall->OutUnit.acSubsNumber[MAX_LEN_NUMBER-1] = 0;
    //аон
    strncpy(pCall->OutUnit.acSubsAON,pAon,MAX_LEN_NUMBER-1);
    pCall->OutUnit.acSubsAON[MAX_LEN_NUMBER-1] = 0;
}

void CDRBuilding::CCDRBuilder::OnAccept(CMonMessageEx& mes)
{
    // remID:DWORD
    //int readPos = 0;    // mes.clearReadPosition();
	CheckModuleInfo(ExtractModNumber(mes.id()), mes);
    //пока ничего не делаем
}

void CDRBuilding::CCDRBuilder::OnNumber(CMonMessageEx& mes)
{
#ifdef _DEBUG
	if(mes.id() == 0xC038013C)
	{
		int a = 2;
//		return;
	}
#endif
    // number:STRING aon:STRING
    // здесь же будет ДВО
    int readPos = 0;    // mes.clearReadPosition();
    const char* pNum = mes.getParameterStringPtr(readPos);
    const char* pAon = mes.getParameterStringPtr(readPos);
    strCallInfo* pCall = FindCall(mes.id());
    if(pCall == NULL)
    {//левый NUMBER
        char sTmp[100];
        sprintf(sTmp, "NUMBER : Не найдена запись с ID = %08X", mes.id());
        AddErrorString(sTmp);
        return;
    }

    strCallInfoUnit* pCallUnit;
    if(pCall->InUnit.dwID == mes.id())
        pCallUnit = &pCall->InUnit;
    else
        pCallUnit = &pCall->OutUnit;
    BYTE btMod = pCallUnit->btMod;
	myASSERT(btMod < MAX_MOD);

	CheckModuleInfo(btMod, mes);
	WriteAliveTime(pCallUnit);
	pCallUnit->dwComoverGlTime = 0;
    //набранный номер
    strncpy(pCallUnit->acCdPN, pNum, MAX_LEN_NUMBER-1);
    pCallUnit->acCdPN[MAX_LEN_NUMBER-1] = 0;
    //АОН
    strncpy(pCallUnit->acCgPN, pAon, MAX_LEN_NUMBER-1);
    pCallUnit->acCgPN[MAX_LEN_NUMBER-1] = 0;

    /*
    if(pCall->InUnit.acCdPN[0] == '*')
    {//если ДВО формируем строку
        pCallUnit->dwReleaseGlTime = GetGlobalTime(btMod,mes.time());
        MakeExitString(pCall,16);
    }
    */
}

void CDRBuilding::CCDRBuilder::OnAnswer(CMonMessageEx& mes)
{
#ifdef _DEBUG
	if(mes.id() == 0x11780E1D)
		int a = 2;
#endif
	
    //без параметров
    if(mes.id() == 0)
    {
        AddErrorString("ANSWER : ID = 00000000");
        return;
    }
    //ищем запись с таким ID
    strCallInfo* pCall = FindCall(mes.id());
    if(pCall == NULL)
    {
        char sTmp[100];
        sprintf(sTmp, "ANSWER : Не найдена запись с ID = %08X", mes.id());
        AddErrorString(sTmp);
        return;
    }
	if(pCall->InUnit.dwBegTalkGlTime != 0 && pCall->OutUnit.dwBegTalkGlTime != 0)
	{
		//иногда бывает что левые АНВСВЕРЫ идут потом
		return;
	}

    if( pCall->InUnit.dwID == mes.id() )
    {
        BYTE btMod = pCall->InUnit.btMod;
		myASSERT(btMod < MAX_MOD);
		if(btMod != 0)
		{
            CheckModuleInfo(btMod, mes);
            DWORD dwGlTime = GetGlobalTime(btMod, mes.time());
			pCall->InUnit.dwBegTalkGlTime = dwGlTime;
			pCall->InUnit.dwComoverGlTime = 0;

			WriteAliveTime(&pCall->InUnit);

			if(pCall->bTalk && pCall->OutUnit.dwBegTalkGlTime != 0 && pCall->InUnit.btMod != pCall->OutUnit.btMod)//уточняем дельту
            {
                m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] = pCall->OutUnit.dwBegTalkGlTime - pCall->InUnit.dwBegTalkGlTime;
                m_ModDelta[pCall->OutUnit.btMod][pCall->InUnit.btMod] = (-1)*m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod];
				/*
				if(pCall->InUnit.btMod == 1 && pCall->OutUnit.btMod == 3 || pCall->InUnit.btMod == 3 && pCall->OutUnit.btMod == 1)
					myTRACE("%d\n", m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod]);
				*/
            }
			else
				pCall->bTalk = true;
            //если случай с COMBINE (когда переадресация по неответу) то для исход. юнита ANSWER может не придти
			while( !pCall->bTalk  && pCall->dwNextCombineCallID != 0 )
			{
				dwGlTime =  pCall->InUnit.dwBegTalkGlTime + m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] == MOD_DELTA_UNINIT ? 0 : m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod];
                pCall->OutUnit.dwBegTalkGlTime = dwGlTime;            
				pCall->OutUnit.dwComoverGlTime = 0;
                pCall = FindCall(pCall->dwNextCombineCallID);
				if(pCall == NULL)
					break;
				if(!pCall->bTalk)
				{
					pCall->InUnit.dwBegTalkGlTime = dwGlTime;
					pCall->InUnit.dwComoverGlTime = 0;
				}
			}

    	}
        else
            return;
    }
    else
    {
        BYTE btMod = pCall->OutUnit.btMod;
		myASSERT(btMod < MAX_MOD);
        CheckModuleInfo(btMod, mes);
        DWORD dwGlTime = GetGlobalTime(btMod, mes.time());
		pCall->OutUnit.dwBegTalkGlTime = dwGlTime;
		pCall->OutUnit.dwComoverGlTime = 0;
		WriteAliveTime(&pCall->OutUnit);
        if(pCall->bTalk && pCall->InUnit.dwBegTalkGlTime != 0 && pCall->InUnit.btMod != pCall->OutUnit.btMod)//уточняем дельту
        {
            m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] = pCall->OutUnit.dwBegTalkGlTime - pCall->InUnit.dwBegTalkGlTime;
            m_ModDelta[pCall->OutUnit.btMod][pCall->InUnit.btMod] = (-1)*m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod];
			/*
			if(m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] == 105)
				int a = 2;
			if(pCall->InUnit.btMod == 1 && pCall->OutUnit.btMod == 3 || pCall->InUnit.btMod == 3 && pCall->OutUnit.btMod == 1)
				myTRACE("%d\n", m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod]);
			*/
        }
		else
			pCall->bTalk = true;

        //если случай с COMBINE то для вход. юнита ANSWER может не придти
		while( !pCall->bTalk  && pCall->dwPrevCombineCallID != 0 )
		{
			dwGlTime =  pCall->OutUnit.dwBegTalkGlTime - m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] == MOD_DELTA_UNINIT ? 0 : m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod];
            pCall->InUnit.dwBegTalkGlTime = dwGlTime;            
			pCall->InUnit.dwComoverGlTime = 0;
            pCall = FindCall(pCall->dwPrevCombineCallID);
			if(pCall == NULL)
				break;
			if(!pCall->bTalk)
			{
				pCall->OutUnit.dwBegTalkGlTime = dwGlTime;
				pCall->OutUnit.dwComoverGlTime = 0;
			}
		}
	}
}

void CDRBuilding::CCDRBuilder::OnSpiderModuleDown(CMonMessageEx& mes)
{
    // module:BYTE  time:TClock
    int readPos = 0;    // mes.clearReadPosition();
    BYTE btMod = mes.getParameterByte(readPos);
#ifdef _DEBUG
	if(btMod == 26)
		int a = 2;
#endif
	myASSERT(btMod < MAX_MOD);
    ReleaseCalls(btMod);
	ResetModuleInfo(btMod);
}

void CDRBuilding::CCDRBuilder::ReleaseCalls(BYTE btMod)
{
    //отбиваем звонки в случае утери модуля или падения TCP

    //если btMod==255 то все модули
    BYTE btReason = RELEASE_CAUSE_TCP_MODULE_DOWN;
    strCallInfo* pNext, *pCall = m_pBegListCall;
    while((pCall)&&(GetLastError() == 0))
    {
        pNext = pCall->pNextCall;
        if( (pCall->InUnit.btMod != 0) && ( (btMod == ALL_MODULES) || (pCall->InUnit.btMod == btMod) ) ||
			(pCall->OutUnit.btMod != 0) && ( (btMod == ALL_MODULES) || (pCall->OutUnit.btMod == btMod) ) )
		{
#ifdef _DEBUG
			if(pCall->InUnit.dwID == 3545302907)
		int a = 2;
#endif
			if(pCall->InUnit.dwComoverGlTime == 0)
			    pCall->InUnit.dwComoverGlTime = GetGlobalTime(pCall->InUnit.btMod, m_ModuleInfo[pCall->InUnit.btMod].btLastRelTime);
			if(pCall->OutUnit.dwComoverGlTime == 0)
				pCall->OutUnit.dwComoverGlTime = GetGlobalTime(pCall->OutUnit.btMod, m_ModuleInfo[pCall->OutUnit.btMod].btLastRelTime);
			ReleaseCall(pCall, btReason);
		}


		/*
        if( (pCall->InUnit.btMod != 0) && ( (btMod == ALL_MODULES) || (pCall->InUnit.btMod == btMod) ) )
		{
			if(	(pCall->OutUnit.btMod != 0) && ( (btMod == ALL_MODULES) || (pCall->OutUnit.btMod == btMod) ) )
			{//отбиваем звонок
				if(pCall->InUnit.dwComoverGlTime == 0)
			        pCall->InUnit.dwComoverGlTime = GetGlobalTime(pCall->InUnit.btMod, m_ModuleInfo[pCall->InUnit.btMod].btLastRelTime);
				if(pCall->OutUnit.dwReleaseGlTime == 0)
					pCall->OutUnit.dwComoverGlTime = GetGlobalTime(pCall->OutUnit.btMod, m_ModuleInfo[pCall->OutUnit.btMod].btLastRelTime);
				ReleaseCall(pCall, btReason);
			}
			else
			{
				if(pCall->InUnit.dwComoverGlTime == 0)
			        pCall->InUnit.dwComoverGlTime = GetGlobalTime(pCall->InUnit.btMod, m_ModuleInfo[pCall->InUnit.btMod].btLastRelTime);
			}
		}
		else
			if(	(pCall->OutUnit.btMod != 0) && ( (btMod == ALL_MODULES) || (pCall->OutUnit.btMod == btMod) ) )
			{
				if(pCall->OutUnit.dwReleaseGlTime == 0)
					pCall->OutUnit.dwComoverGlTime = GetGlobalTime(pCall->OutUnit.btMod, m_ModuleInfo[pCall->OutUnit.btMod].btLastRelTime);
			}
		*/



		/*
        if( (pCall->InUnit.btMod != 0) && ( (btMod == ALL_MODULES) || (pCall->InUnit.btMod == btMod) ) )
        {//если SEIZURE был и звонок относится к нужному модулю
			if(pCall->InUnit.dwComoverGlTime == 0)
				pCall->InUnit.dwComoverGlTime = GetGlobalTime(pCall->InUnit.btMod, m_ModuleInfo[pCall->InUnit.btMod].btLastRelTime);
            ReleaseCall(pCall, btReason);
        }
        else
        {
	        if( (pCall->OutUnit.btMod != 0) && ( (btMod == ALL_MODULES) || (pCall->OutUnit.btMod == btMod) ) )
            {//если был только CALL и номер модуля - нужный
				if(pCall->OutUnit.dwReleaseGlTime == 0)
					pCall->OutUnit.dwComoverGlTime = GetGlobalTime(pCall->OutUnit.btMod, m_ModuleInfo[pCall->OutUnit.btMod].btLastRelTime);
                ReleaseCall(pCall, btReason);
            }
        }
		*/
        pCall = pNext;
    }
}

void CDRBuilding::CCDRBuilder::ReleaseCall(CDRBuilding::strCallInfo* pCall, BYTE btReason)
{
#ifdef _DEBUG
	if(pCall->InUnit.dwID == 0x8A810119)
		int a = 2;
#endif

	if( pCall->InUnit.acCdPN[0] == 0 && pCall->OutUnit.acCdPN[0] == 0)
	{
		DeleteCallFromList(pCall);
		return;
	}

	if(!m_SSettings.bWriteUnansweredCalls && !pCall->bTalk)
	{
		DeleteCallFromList(pCall);
		return;
	}
	//мало ли во время предыдущих сообщений не было ALIVE'ов
	WriteAliveTime(&pCall->InUnit);	
	WriteAliveTime(&pCall->OutUnit);	


	//если невозможно узнать время звонка из-за отсутствия ALIVE'ов
#define USE_NOUNITS			0
#define USE_INUNIT			1
#define USE_OUTUNIT			3
#define USE_BOTHUNITS		4
#define EXIT				5
	BYTE btUseUnit = 255;
	if(pCall->InUnit.AliveClock.Control != CLOCK_INITIALIZED_EX)
	{
		if(pCall->OutUnit.AliveClock.Control = CLOCK_INITIALIZED_EX)
			btUseUnit = USE_NOUNITS;
		else
			btUseUnit = USE_OUTUNIT;
	}
	else
	{
		if(pCall->OutUnit.AliveClock.Control != CLOCK_INITIALIZED_EX)
			btUseUnit = USE_INUNIT;
		else
			btUseUnit = USE_BOTHUNITS;
	}
	
	
	strCDR CDR;
	CDR.btReason = btReason;
	//заполняем CDR.STrunkInInfo
    if(pCall->InUnit.btSig == SIGNALLING_EXT || ((pCall->InUnit.btSig == SIGNALLING_DISA || pCall->InUnit.btSig == SIGNALLING_SERIAL) && pCall->InUnit.acSubsNumber[0]) )
    {
		sprintf(CDR.STrunkInInfo.sTrunk, "%s%s", PREFIX_SUBSCRIBER, pCall->InUnit.acSubsNumber);
    }
    else
    {
        char sTmp[MAX_LEN_NUMBER];
        sTmp[0] = 0;
        strcpy(sTmp, pCall->InUnit.acCgPN[0] ? pCall->InUnit.acCgPN : pCall->OutUnit.acCgPN);
        if(IsInnerNumber(sTmp, true))
            sprintf(CDR.STrunkInInfo.sTrunk, "%s%s", PREFIX_SUBSCRIBER, sTmp);
        else
            sprintf(CDR.STrunkInInfo.sTrunk, "%s%02d%02d%02d", PREFIX_TRUNK, pCall->InUnit.btMod,pCall->InUnit.btPcmSlot,pCall->InUnit.btKIPort);
    }

	strcpy(CDR.STrunkInInfo.sCdPN, pCall->InUnit.acCdPN);
	strcpy(CDR.STrunkInInfo.sCgPN, pCall->InUnit.acCgPN);
	/*
	strcpy(CDR.STrunkInInfo.sCdPN, pCall->InUnit.acCdPN[0]?pCall->InUnit.acCdPN:m_SSettings.sNoNumberString);
	strcpy(CDR.STrunkInInfo.sCgPN, pCall->InUnit.acCgPN[0]?pCall->InUnit.acCgPN:m_SSettings.sNoNumberString);
	*/

	//заполняем CDR.STrunkOutInfo
    if(pCall->OutUnit.btSig == SIGNALLING_EXT || ((pCall->OutUnit.btSig == SIGNALLING_DISA || pCall->OutUnit.btSig == SIGNALLING_SERIAL) && pCall->OutUnit.acSubsNumber[0]) )
    {
		sprintf(CDR.STrunkOutInfo.sTrunk, "%s%s", PREFIX_SUBSCRIBER, pCall->OutUnit.acSubsNumber);
    }
    else
    {
        char sTmp[MAX_LEN_NUMBER];
        sTmp[0] = 0;
        strcpy(sTmp, pCall->InUnit.acCdPN[0] ? pCall->InUnit.acCdPN : pCall->OutUnit.acCdPN);
        if(IsInnerNumber(sTmp, false))
            sprintf(CDR.STrunkOutInfo.sTrunk, "%s%s", PREFIX_SUBSCRIBER, sTmp);
        else
            sprintf(CDR.STrunkOutInfo.sTrunk, "%s%02d%02d%02d", PREFIX_TRUNK, pCall->OutUnit.btMod,pCall->OutUnit.btPcmSlot,pCall->OutUnit.btKIPort);
    }

	strcpy(CDR.STrunkOutInfo.sCdPN, pCall->OutUnit.acCdPN);
	strcpy(CDR.STrunkOutInfo.sCgPN, pCall->OutUnit.acCgPN);
	/*
	strcpy(CDR.STrunkOutInfo.sCdPN, pCall->OutUnit.acCdPN[0]?pCall->OutUnit.acCdPN:m_SSettings.sNoNumberString);
	strcpy(CDR.STrunkOutInfo.sCgPN, pCall->OutUnit.acCgPN[0]?pCall->OutUnit.acCgPN:m_SSettings.sNoNumberString);
	*/
	
	while(btUseUnit != EXIT)
	switch(btUseUnit)
	{
	case USE_NOUNITS:
		{
			char sTmp[200];
			sprintf(sTmp, "Не удалось определить время звонка. SeizID:<%08X> CallID:<%08X>", pCall->InUnit.dwID, pCall->OutUnit.dwID);
			AddErrorString(sTmp);
			DeleteCallFromList(pCall);
			return;
		}
		break;
	case USE_OUTUNIT:
		{
			//CALL значит был, и мы знаем как вх. так и исх. модули
			//данные будут одинаковые, будут браться из OutUnit
			if(!CheckCallsGlTime(pCall->OutUnit, pCall->InUnit, true))
			{
				char sTmp[200];
				sprintf(sTmp, "Не удалось расчитать длительность звонка. SeizID:<%08X> CallID:<%08X>", pCall->InUnit.dwID, pCall->OutUnit.dwID);
				AddErrorString(sTmp);
				DeleteCallFromList(pCall);
				return;
			}

			CDR.timeSeizOut = AddToClock(pCall->OutUnit.AliveClock, (int)(pCall->OutUnit.dwSeizGlTime - pCall->OutUnit.dwAliveGlTime));
			CDR.timeSeizIn = CDR.timeSeizOut;
			CDR.dwSeizDurationOut = pCall->OutUnit.dwReleaseGlTime - pCall->OutUnit.dwSeizGlTime;
			CDR.dwSeizDurationIn = CDR.dwSeizDurationOut;
			if(pCall->bTalk)
			{
				CDR.dwTalkDuration = pCall->OutUnit.dwReleaseGlTime - pCall->OutUnit.dwBegTalkGlTime;
				if(m_SSettings.bDecrementDuration && CDR.dwTalkDuration > 1)
					CDR.dwTalkDuration--;
			}
			else
				CDR.dwTalkDuration = 0;
			
			btUseUnit = EXIT;
			break;

			/*
			CDR.timeSeizOut = AddToClock(pCall->OutUnit.AliveClock, (int)(pCall->OutUnit.dwSeizGlTime - pCall->OutUnit.dwAliveGlTime));
			CDR.timeSeizIn = CDR.timeSeizOut;
			//проверяем чтоб было время RELEASE'а
			if(pCall->OutUnit.dwReleaseGlTime == 0)
			{
				//пытаемся восстановить по входящему
				if(pCall->InUnit.dwReleaseGlTime != 0 && m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] != MOD_DELTA_UNINIT)
				{
					pCall->OutUnit.dwReleaseGlTime = pCall->InUnit.dwReleaseGlTime + m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod];
				}
				else
				{//тогда должен быть COMOVERLOAD
					myASSERT(pCall->OutUnit.dwComoverGlTime != 0);
					pCall->OutUnit.dwReleaseGlTime = pCall->OutUnit.dwComoverGlTime;
				}
			}
//			myASSERT((int)(pCall->OutUnit.dwReleaseGlTime - pCall->OutUnit.dwSeizGlTime) > 0);
			CDR.dwSeizDurationIn = pCall->OutUnit.dwReleaseGlTime - pCall->OutUnit.dwSeizGlTime;
			CDR.dwSeizDurationOut = CDR.dwSeizDurationIn;
			//определяем продолжительность разговора
			if(pCall->bTalk)
			{
				if(pCall->OutUnit.dwBegTalkGlTime == 0)
				{//если не было ANSWER'а
					//пытаемся восстановить по входящему
					if(m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] != MOD_DELTA_UNINIT)
					{
						pCall->OutUnit.dwBegTalkGlTime = pCall->InUnit.dwBegTalkGlTime + m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod];
					}
					else
					{//тогда должен быть COMOVERLOAD
						myASSERT(pCall->OutUnit.dwComoverGlTime != 0);
						pCall->OutUnit.dwBegTalkGlTime = pCall->OutUnit.dwComoverGlTime;
					}
				}
//				CDR.dwTalkDuration = pCall->OutUnit.dwReleaseGlTime - pCall->InUnit.dwBegTalkGlTime;
				CDR.dwTalkDuration = (int)(pCall->OutUnit.dwReleaseGlTime - pCall->OutUnit.dwBegTalkGlTime) > 0 ? pCall->OutUnit.dwReleaseGlTime - pCall->OutUnit.dwBegTalkGlTime : 1;
				if(CDR.dwTalkDuration == 0)
					CDR.dwTalkDuration = 1;
			}
			else
				CDR.dwTalkDuration = 0;
			*/
			
		}
		break;
	case USE_INUNIT:
		{
			//1) был только SEIZURE
			//2) был только CALL, но время CALL'а не взялось из-за отсутствия ALIVE'ов
			//3) было и то и то, но время CALL'а не взялось из-за отсутствия ALIVE'ов
			if(!CheckCallsGlTime(pCall->InUnit, pCall->OutUnit, true))
			{
				char sTmp[200];
				sprintf(sTmp, "Не удалось расчитать длительность звонка. SeizID:<%08X> CallID:<%08X>", pCall->InUnit.dwID, pCall->OutUnit.dwID);
				AddErrorString(sTmp);
				DeleteCallFromList(pCall);
				return;
			}

			CDR.timeSeizIn = AddToClock(pCall->InUnit.AliveClock, (int)(pCall->InUnit.dwSeizGlTime - pCall->InUnit.dwAliveGlTime));
			CDR.timeSeizOut = CDR.timeSeizIn;
			CDR.dwSeizDurationIn = pCall->InUnit.dwReleaseGlTime - pCall->InUnit.dwSeizGlTime;
			CDR.dwSeizDurationOut = CDR.dwSeizDurationIn;
			if(pCall->bTalk)
			{
				CDR.dwTalkDuration = pCall->InUnit.dwReleaseGlTime - pCall->InUnit.dwBegTalkGlTime;
				if(m_SSettings.bDecrementDuration && CDR.dwTalkDuration > 1)
					CDR.dwTalkDuration--;
			}
			else
				CDR.dwTalkDuration = 0;
			btUseUnit = EXIT;
			break;

			/*

			//данные будут одинаковые, будут браться из InUnit
			if(pCall->InUnit.dwSeizGlTime == 0)
			{//по идее не может этого быть
				char sTmp[200];
				sprintf(sTmp, "Не удалось определить глобальное время занятия. SeizID:<%08X>", pCall->InUnit.dwID);
				AddErrorString(sTmp);
				if(pCall->OutUnit.btMod == 0)
				{
					DeleteCallFromList(pCall);
					return;
				}
				if(m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] != MOD_DELTA_UNINIT)
				{//по-любому есть pCall->OutUnit.dwSeizGlTime
					pCall->InUnit.dwSeizGlTime = pCall->OutUnit.dwSeizGlTime - m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod];
				}
				else
				{
					DeleteCallFromList(pCall);
					return;
				}
			}
			CDR.timeSeizIn = AddToClock(pCall->OutUnit.AliveClock, (int)(pCall->InUnit.dwSeizGlTime - pCall->InUnit.dwAliveGlTime));
			CDR.timeSeizOut = CDR.timeSeizIn;
			//проверяем чтоб было время RELEASE'а
			if(pCall->InUnit.dwReleaseGlTime == 0)
			{
				//пытаемся восстановить по исходящему
				if(pCall->InUnit.dwReleaseGlTime != 0 && m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] != MOD_DELTA_UNINIT)
				{
					pCall->InUnit.dwReleaseGlTime = pCall->OutUnit.dwReleaseGlTime - m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod];
				}
				else
				{//тогда должен быть COMOVERLOAD
//					myASSERT(pCall->OutUnit.dwComoverGlTime != 0);
					if(pCall->OutUnit.dwComoverGlTime == 0)
					{
						char sTmp[200];
						sprintf(sTmp, "Не удалось обсчитать звонок(Seiz:<%08X>) из-за отсутствия Release", pCall->InUnit.dwID);
						AddErrorString(sTmp);
						DeleteCallFromList(pCall);
						return;
					}
					pCall->InUnit.dwReleaseGlTime = pCall->InUnit.dwComoverGlTime;
				}
			}
//			myASSERT((int)(pCall->OutUnit.dwReleaseGlTime - pCall->OutUnit.dwSeizGlTime) > 0);
			CDR.dwSeizDurationIn = pCall->InUnit.dwReleaseGlTime - pCall->InUnit.dwSeizGlTime;
			CDR.dwSeizDurationOut = CDR.dwSeizDurationIn;
			//определяем продолжительность разговора
			if(pCall->bTalk)
			{
				if(pCall->InUnit.dwBegTalkGlTime == 0)
				{//если не было ANSWER'а
					//пытаемся восстановить по входящему
					if(m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] != MOD_DELTA_UNINIT)
					{
						pCall->InUnit.dwBegTalkGlTime = pCall->OutUnit.dwBegTalkGlTime - m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod];
					}
					else
					{//тогда должен быть COMOVERLOAD
						myASSERT(pCall->InUnit.dwComoverGlTime != 0);
						pCall->InUnit.dwBegTalkGlTime = pCall->InUnit.dwComoverGlTime;
					}
				}
				CDR.dwTalkDuration = (int)(pCall->InUnit.dwReleaseGlTime - pCall->InUnit.dwBegTalkGlTime) > 0 ? pCall->InUnit.dwReleaseGlTime - pCall->InUnit.dwBegTalkGlTime : 1;
//				CDR.dwTalkDuration = pCall->InUnit.dwReleaseGlTime - pCall->InUnit.dwBegTalkGlTime;
				if(CDR.dwTalkDuration == 0)
					CDR.dwTalkDuration = 1;
			}
			else
				CDR.dwTalkDuration = 0;
			*/
			
		}
		break;
	case USE_BOTHUNITS:
		{
#ifdef _DEBUG
	if(pCall->InUnit.dwID == 55247328)
		int a = 2;
#endif
			//1) был только CALL
			//2) было и то и то
			if(!CheckCallsGlTime(pCall->InUnit, pCall->OutUnit, false))
			{
				btUseUnit = USE_OUTUNIT;
				break;
			}
			if(!CheckCallsGlTime(pCall->OutUnit, pCall->InUnit, false))
			{
				btUseUnit = USE_INUNIT;
				break;
			}
			CDR.timeSeizIn = AddToClock(pCall->InUnit.AliveClock, (int)(pCall->InUnit.dwSeizGlTime - pCall->InUnit.dwAliveGlTime));
			CDR.timeSeizOut = AddToClock(pCall->OutUnit.AliveClock, (int)(pCall->OutUnit.dwSeizGlTime - pCall->OutUnit.dwAliveGlTime));
			CDR.dwSeizDurationIn = pCall->InUnit.dwReleaseGlTime - pCall->InUnit.dwSeizGlTime;
			CDR.dwSeizDurationOut = pCall->OutUnit.dwReleaseGlTime - pCall->OutUnit.dwSeizGlTime;
			/*
			if(CDR.dwSeizDurationIn < CDR.dwSeizDurationOut)
			{
				myASSERT(CDR.dwSeizDurationOut - CDR.dwSeizDurationIn == 1);
				CDR.dwSeizDurationIn = CDR.dwSeizDurationOut;
			}
			*/
			if(pCall->bTalk)
			{
				myASSERT(pCall->InUnit.dwReleaseGlTime - pCall->InUnit.dwBegTalkGlTime == pCall->OutUnit.dwReleaseGlTime - pCall->OutUnit.dwBegTalkGlTime);
				CDR.dwTalkDuration = pCall->InUnit.dwReleaseGlTime - pCall->InUnit.dwBegTalkGlTime;
				if(m_SSettings.bDecrementDuration && CDR.dwTalkDuration > 1)
					CDR.dwTalkDuration--;
			}
			else
				CDR.dwTalkDuration = 0;

			btUseUnit = EXIT;
			break;

			/*
			if(pCall->InUnit.dwSeizGlTime == 0)
			{//по идее не может этого быть
				char sTmp[200];
				sprintf(sTmp, "Не удалось определить глобальное время занятия. SeizID:<%08X>", pCall->InUnit.dwID);
				AddErrorString(sTmp);

				if(m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] != MOD_DELTA_UNINIT)
				{//по-любому есть pCall->OutUnit.dwSeizGlTime
					pCall->InUnit.dwSeizGlTime = pCall->OutUnit.dwSeizGlTime - m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod];
				}
				else
				{
					myASSERT(FALSE);
					DeleteCallFromList(pCall);
					return;

				}
			}
			CDR.timeSeizIn = AddToClock(pCall->InUnit.AliveClock, (int)(pCall->InUnit.dwSeizGlTime - pCall->InUnit.dwAliveGlTime));
			CDR.timeSeizOut = AddToClock(pCall->OutUnit.AliveClock, (int)(pCall->OutUnit.dwSeizGlTime - pCall->OutUnit.dwAliveGlTime));

			//проверяем чтоб было время RELEASE'а
			if(pCall->InUnit.dwReleaseGlTime == 0)
			{
				//пытаемся восстановить по исходящему
				if(pCall->OutUnit.dwReleaseGlTime != 0 && m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] != MOD_DELTA_UNINIT)
				{
					pCall->InUnit.dwReleaseGlTime = pCall->OutUnit.dwReleaseGlTime - m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod];
				}
				else
				{//тогда должен быть COMOVERLOAD
					myASSERT(pCall->OutUnit.dwComoverGlTime != 0);
					pCall->InUnit.dwReleaseGlTime = pCall->InUnit.dwComoverGlTime;

				}
			}
			if(pCall->OutUnit.dwReleaseGlTime == 0)
			{
				//пытаемся восстановить по входящему
				if(pCall->InUnit.dwReleaseGlTime != 0 && m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] != MOD_DELTA_UNINIT)
				{
					pCall->OutUnit.dwReleaseGlTime = pCall->InUnit.dwReleaseGlTime + m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod];
				}
				else
				{//тогда должен быть COMOVERLOAD
					myASSERT(pCall->OutUnit.dwComoverGlTime != 0);
					pCall->OutUnit.dwReleaseGlTime = pCall->OutUnit.dwComoverGlTime;
				}
			}
//			myASSERT((int)(pCall->OutUnit.dwReleaseGlTime - pCall->OutUnit.dwSeizGlTime) > 0);
			CDR.dwSeizDurationIn = pCall->OutUnit.dwReleaseGlTime - pCall->OutUnit.dwSeizGlTime;
			CDR.dwSeizDurationOut = CDR.dwSeizDurationIn;
			//определяем продолжительность разговора
			if(pCall->bTalk)
			{
				if(pCall->InUnit.dwBegTalkGlTime == 0)
				{//если не было ANSWER'а
					//пытаемся восстановить по входящему
					if(m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] != MOD_DELTA_UNINIT)
					{
						pCall->InUnit.dwBegTalkGlTime = pCall->OutUnit.dwBegTalkGlTime - m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod];
					}
					else
					{//тогда должен быть COMOVERLOAD
						myASSERT(pCall->InUnit.dwComoverGlTime != 0);
						pCall->InUnit.dwBegTalkGlTime = pCall->InUnit.dwComoverGlTime;
					}
				}
				if(pCall->OutUnit.dwBegTalkGlTime == 0)
				{//если не было ANSWER'а
					//пытаемся восстановить по входящему
					if(m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod] != MOD_DELTA_UNINIT)
					{
						pCall->OutUnit.dwBegTalkGlTime = pCall->InUnit.dwBegTalkGlTime + m_ModDelta[pCall->InUnit.btMod][pCall->OutUnit.btMod];
					}
					else
					{//тогда должен быть COMOVERLOAD
						myASSERT(pCall->OutUnit.dwComoverGlTime != 0);
						pCall->OutUnit.dwBegTalkGlTime = pCall->OutUnit.dwComoverGlTime;
					}
				}
				int iInTalkDur = (int)(pCall->InUnit.dwReleaseGlTime - pCall->InUnit.dwBegTalkGlTime);
				if(iInTalkDur <= 0)
					iInTalkDur = 1;
				int iOutTalkDur = (int)(pCall->OutUnit.dwReleaseGlTime - pCall->OutUnit.dwBegTalkGlTime);
				if(iOutTalkDur <= 0)
					iOutTalkDur = 1;
				CDR.dwTalkDuration = iInTalkDur > iOutTalkDur ? iInTalkDur : iOutTalkDur;

				if(CDR.dwTalkDuration == 0)
					CDR.dwTalkDuration = 1;
			}
			else
				CDR.dwTalkDuration = 0;
				*/
		break;
		}
	default:
		myASSERT(FALSE);
	}
	//проверка
	myASSERT((int)CDR.dwSeizDurationIn >= 0);
	myASSERT((int)CDR.dwSeizDurationOut >= 0);
	/*
	if(CDR.dwSeizDurationIn < CDR.dwSeizDurationOut)
	{
		//myASSERT(CDR.dwSeizDurationIn + 1 == CDR.dwSeizDurationOut);
	}
	*/
	myASSERT((int)CDR.dwTalkDuration >= 0);
	if(CDR.dwTalkDuration == 3602)
		CDR.dwTalkDuration = 3601;

	if(CDR.dwSeizDurationIn > (DWORD)m_SJournalSettings.iMaxDuration || CDR.dwSeizDurationOut > (DWORD)m_SJournalSettings.iMaxDuration || CDR.dwTalkDuration > (DWORD)m_SJournalSettings.iMaxDuration)
    {
		char sTmp[500];
		sprintf(sTmp, "Большая продолжительность занятия - InDur:%d, CallDur:%d, TalkDur%d. SeizID:<%08X> CallID:<%08X>",CDR.dwSeizDurationIn, CDR.dwSeizDurationOut, CDR.dwTalkDuration, pCall->InUnit.dwID, pCall->OutUnit.dwID);
		AddJournalString(sTmp);
    }
	//если была переадресация
	//сохраняем
	strCDRTrunkUnit tmpStr = CDR.STrunkOutInfo;
    char* pToken = strtok(pCall->acRedirBuf,"[|");
    while(pToken)
    {
        sprintf(CDR.STrunkOutInfo.sTrunk, "%s%s", PREFIX_SUBSCRIBER, pToken);
		sprintf(CDR.STrunkOutInfo.sCdPN, "%s", pToken);
		strcpy(CDR.STrunkOutInfo.sCgPN, CDR.STrunkInInfo.sCgPN);
		MakeCDR(CDR);

        pToken = strtok(NULL,"|]");
		strcpy(CDR.STrunkInInfo.sTrunk, CDR.STrunkOutInfo.sTrunk);
		strcpy(CDR.STrunkInInfo.sCdPN, pToken);
		strcpy(CDR.STrunkInInfo.sCgPN, CDR.STrunkOutInfo.sCgPN);

        if(pToken)
            pToken = strtok(NULL,"[|");
        
    }

	CDR.STrunkOutInfo = tmpStr;
	MakeCDR(CDR);
    DeleteCallFromList(pCall);
}

BOOL CDRBuilding::CCDRBuilder::CheckCallsGlTime(CDRBuilding::strCallInfoUnit& MainCallUnit, CDRBuilding::strCallInfoUnit& SubsCallUnit, bool bLastCheck)
{
	//смотрим можно ли определить продолжительность разговора
	DWORD dwTalkDuration = 0;
	bool bTalk = MainCallUnit.dwBegTalkGlTime != 0 || SubsCallUnit.dwBegTalkGlTime;
	if(bTalk)
	{
		DWORD dwTalkDurationIn = 0;
		DWORD dwTalkDurationOut = 0;
		if(MainCallUnit.dwBegTalkGlTime != 0 && MainCallUnit.dwReleaseGlTime && MainCallUnit.dwReleaseGlTime >= MainCallUnit.dwBegTalkGlTime)
		{
			dwTalkDurationIn = MainCallUnit.dwReleaseGlTime - MainCallUnit.dwBegTalkGlTime;
			if(dwTalkDurationIn == 0)
				dwTalkDurationIn = 1;
		}
		if(SubsCallUnit.dwBegTalkGlTime != 0 && SubsCallUnit.dwReleaseGlTime && SubsCallUnit.dwReleaseGlTime >= SubsCallUnit.dwBegTalkGlTime)
		{
			dwTalkDurationOut = SubsCallUnit.dwReleaseGlTime - SubsCallUnit.dwBegTalkGlTime;
			if(dwTalkDurationOut == 0)
				dwTalkDurationOut = 1;
		}
		dwTalkDuration = dwTalkDurationOut > dwTalkDurationIn ? dwTalkDurationOut : dwTalkDurationIn;
	}

	//смотрим время занятия
	if(MainCallUnit.dwSeizGlTime == 0)
	{//если был только Call, но по вх.юниту пытаемся всё вычислить
		//вообщето этого не должно быть
		if(m_ModDelta[SubsCallUnit.btMod][MainCallUnit.btMod] != MOD_DELTA_UNINIT)
		{
			//всяко должно быть время занятия 
			MainCallUnit.dwSeizGlTime = SubsCallUnit.dwSeizGlTime + m_ModDelta[SubsCallUnit.btMod][MainCallUnit.btMod];
		}
		else
			return FALSE;
	}

	//время начала разговора
	if(bTalk && MainCallUnit.dwBegTalkGlTime == 0)
	{//если был разговор
		if(SubsCallUnit.btMod == MainCallUnit.btMod && SubsCallUnit.dwBegTalkGlTime != 0)
		{
			MainCallUnit.dwBegTalkGlTime =  SubsCallUnit.dwBegTalkGlTime;
		}
		else
		if(MainCallUnit.dwReleaseGlTime != 0 && dwTalkDuration != 0)
		{//если знаем продолжительность разговора и время конца разговора
			MainCallUnit.dwBegTalkGlTime =  MainCallUnit.dwReleaseGlTime - dwTalkDuration;
		}
		else
		{
			if(!bLastCheck)
				return FALSE;//нужно поробовать по другому юниту

			DWORD dwBegTalkGlTime1 = 0, dwBegTalkGlTime2 = 0;
			if(m_ModDelta[SubsCallUnit.btMod][MainCallUnit.btMod] != MOD_DELTA_UNINIT)
			{//пытаемся восстановить по вспомогательному юниту
				dwBegTalkGlTime1 = SubsCallUnit.dwBegTalkGlTime + m_ModDelta[SubsCallUnit.btMod][MainCallUnit.btMod];
			}
			if(MainCallUnit.dwComoverGlTime != 0)
			{//должен быть комоверлоад
				//по комоверу в самую последнюю очередю определять
				//например в 100 - Сейз, 101 - Кол, 102 - комовер, 110-ансвер_исх, 110 - комовер. запомнится то 102
				dwBegTalkGlTime2 = MainCallUnit.dwComoverGlTime;
			}
			if(dwBegTalkGlTime1 == 0 && dwBegTalkGlTime2 == 0)
				return FALSE;
			DWORD dwReleaseGlTime = MainCallUnit.dwReleaseGlTime != 0 ? MainCallUnit.dwReleaseGlTime : 0xFFFFFFFF;
			if(dwBegTalkGlTime1 >= MainCallUnit.dwSeizGlTime && dwBegTalkGlTime1 <= dwReleaseGlTime)
				MainCallUnit.dwBegTalkGlTime = dwBegTalkGlTime1;
			else
				if(dwBegTalkGlTime2 >= MainCallUnit.dwSeizGlTime && dwBegTalkGlTime2 <= dwReleaseGlTime)
					MainCallUnit.dwBegTalkGlTime = dwBegTalkGlTime2;
				else
					return FALSE;
		}

		/*
		if(m_ModDelta[SubsCallUnit.btMod][MainCallUnit.btMod] != MOD_DELTA_UNINIT)
		{//пытаемся восстановить по вспомогательному юниту
			MainCallUnit.dwBegTalkGlTime = SubsCallUnit.dwBegTalkGlTime + m_ModDelta[SubsCallUnit.btMod][MainCallUnit.btMod];
		}
		else
		if(MainCallUnit.dwComoverGlTime != 0)
		{//должен быть комоверлоад
			//по комоверу в самую последнюю очередю определять
			//например в 100 - Сейз, 101 - Кол, 102 - комовер, 110-ансвер_исх, 110 - комовер. запомнится то 102
			MainCallUnit.dwBegTalkGlTime = MainCallUnit.dwComoverGlTime;
		}
		else
		{
			myASSERT(FALSE);
			MainCallUnit.dwBegTalkGlTime = MainCallUnit.dwSeizGlTime;
		}
		*/
	}

	//проверка
	if(MainCallUnit.dwSeizGlTime > MainCallUnit.dwBegTalkGlTime && bTalk)
	{
		/*
		if(SubsCallUnit.dwSeizGlTime != 0 && SubsCallUnit.dwBegTalkGlTime != 0 && SubsCallUnit.dwBegTalkGlTime > SubsCallUnit.dwSeizGlTime)
			MainCallUnit.dwBegTalkGlTime = MainCallUnit.dwSeizGlTime + SubsCallUnit.dwBegTalkGlTime - SubsCallUnit.dwSeizGlTime;
		else
		*/
			MainCallUnit.dwBegTalkGlTime = MainCallUnit.dwSeizGlTime;
	}

	//время конца занятия
	if(dwTalkDuration != 0)
	{
		MainCallUnit.dwReleaseGlTime = MainCallUnit.dwBegTalkGlTime + dwTalkDuration;
	}
	else
	{
		if(MainCallUnit.dwReleaseGlTime == 0)
		{
			//должен быть комоверлоад
			if(SubsCallUnit.btMod == MainCallUnit.btMod && SubsCallUnit.dwReleaseGlTime != 0)
			{
				MainCallUnit.dwReleaseGlTime =  SubsCallUnit.dwReleaseGlTime;
			}
			else
			if(MainCallUnit.dwReleaseGlTime != 0 && m_ModDelta[SubsCallUnit.btMod][MainCallUnit.btMod] != MOD_DELTA_UNINIT)
			{
				MainCallUnit.dwReleaseGlTime = SubsCallUnit.dwReleaseGlTime + m_ModDelta[SubsCallUnit.btMod][MainCallUnit.btMod];
			}
			else
			{
				if(!bLastCheck)
					return FALSE;//нужно поробовать по другому юниту
				DWORD dwReleaseGlTime1 = 0, dwReleaseGlTime2 = 0;
				if(m_ModDelta[SubsCallUnit.btMod][MainCallUnit.btMod] != MOD_DELTA_UNINIT && SubsCallUnit.dwReleaseGlTime != 0)
				{//пытаемся восстановить по вспомогательному юниту
					dwReleaseGlTime1 = SubsCallUnit.dwReleaseGlTime + m_ModDelta[SubsCallUnit.btMod][MainCallUnit.btMod];
				}
				if(MainCallUnit.dwComoverGlTime != 0)
				{//должен быть комоверлоад
					//по комоверу в самую последнюю очередю определять
					//например в 100 - Сейз, 101 - Кол, 102 - комовер, 110-ансвер_исх, 110 - комовер. запомнится то 102
					dwReleaseGlTime2 = MainCallUnit.dwComoverGlTime;
				}
				if(dwReleaseGlTime1 == 0 && dwReleaseGlTime2 == 0)
				{
					if(bTalk)
					{
						MainCallUnit.dwReleaseGlTime = MainCallUnit.dwBegTalkGlTime + 1;
					}
					else
					{
						MainCallUnit.dwReleaseGlTime = MainCallUnit.dwSeizGlTime;
					}
				}

				
				if(dwReleaseGlTime1 >= (bTalk ? MainCallUnit.dwBegTalkGlTime : MainCallUnit.dwSeizGlTime))
					MainCallUnit.dwReleaseGlTime = dwReleaseGlTime1;
				else
					if(dwReleaseGlTime2 >= (bTalk ? MainCallUnit.dwBegTalkGlTime : MainCallUnit.dwSeizGlTime))
						MainCallUnit.dwReleaseGlTime = dwReleaseGlTime2;
					else
						return FALSE;
			}

			/*
			if(MainCallUnit.dwComoverGlTime != 0)
			{
				MainCallUnit.dwReleaseGlTime = MainCallUnit.dwComoverGlTime;
			}
			else
			{
				myASSERT(FALSE);
				return FALSE;
			}
			{//пытаемся восстановить по вспомогательному юниту
				if(MainCallUnit.dwReleaseGlTime != 0 && m_ModDelta[SubsCallUnit.btMod][MainCallUnit.btMod] != MOD_DELTA_UNINIT)
				{
					MainCallUnit.dwReleaseGlTime = SubsCallUnit.dwReleaseGlTime + m_ModDelta[SubsCallUnit.btMod][MainCallUnit.btMod];
				}
				else
				{
					myASSERT(FALSE);
					return FALSE;
				}
			}
			if(bTalk)
			{
				if(MainCallUnit.dwReleaseGlTime >= MainCallUnit.dwBegTalkGlTime)
				{
					if(MainCallUnit.dwReleaseGlTime - MainCallUnit.dwBegTalkGlTime == 0)
						MainCallUnit.dwReleaseGlTime++;
				}
				else
				{
					myASSERT(FALSE);
					return FALSE;
				}
			}
			*/
		}

		//проверка
		if(bTalk)
		{
			if(MainCallUnit.dwReleaseGlTime <= MainCallUnit.dwBegTalkGlTime)
			{
				MainCallUnit.dwReleaseGlTime = MainCallUnit.dwBegTalkGlTime + 1;
			}
		}
		else
		{
			if(MainCallUnit.dwReleaseGlTime < MainCallUnit.dwSeizGlTime)
			{
				MainCallUnit.dwReleaseGlTime = MainCallUnit.dwSeizGlTime;
			}
		}
	}
	return TRUE;		
}

void CDRBuilding::CCDRBuilder::DeleteCallFromList(CDRBuilding::strCallInfo* pDelCall)
{
    //если первый или последний
    if(m_pBegListCall == pDelCall)
        m_pBegListCall = pDelCall->pNextCall;
    if(m_pEndListCall == pDelCall)
        m_pEndListCall = pDelCall->pPrevCall;

    if(pDelCall->pPrevCall)
        pDelCall->pPrevCall->pNextCall = pDelCall->pNextCall;
    if(pDelCall->pNextCall)
        pDelCall->pNextCall->pPrevCall = pDelCall->pPrevCall;
    delete pDelCall;
#ifdef CONTROL_CALLS_COUNT
	m_iCallsCount--;
//	myTRACE("CallsCount - %d\n", m_iCallsCount);
#endif
}

TClock CDRBuilding::CCDRBuilder::AddToClock(const TClock& clock, int iDelta)
{
	TClock cl = clock;
	int dt = 1;
	if(iDelta < 0)
	{
		dt = -1;
		iDelta *= -1;
	}
	
	for(int i = 0; i < iDelta; i++)
	//секунды
	if((int)cl.Seconds + dt == (-1) || cl.Seconds + dt == 60)
	{
		//минуты
		if((int)cl.Minutes + dt == (-1) || cl.Minutes + dt == 60)
		{
			//часы
			if((int)cl.Hours + dt == (-1) || cl.Hours + dt == 24)
			{
				//дни
				if((int)cl.Date + dt == 0 || cl.Date + dt == GetDaysInMonth(cl.Month, cl.Year)+1)
				{
					//месяцы
					if((int)cl.Month + dt == 0 || cl.Month + dt == 13)
					{
						//года
						cl.Year += dt;

						cl.Month = dt > 0 ? 1 : 12;
					}
					else
						cl.Month += dt;
					cl.Date = dt > 0 ? 1 : GetDaysInMonth(cl.Month, cl.Year);
				}
				else
					cl.Date += dt;
				cl.Hours = dt > 0 ? 0 : 23;
			}
			else
				cl.Hours += dt;
			cl.Minutes = dt > 0 ? 0 : 59;
		}
		else
			cl.Minutes += dt;
		cl.Seconds = dt > 0 ? 0 : 59;
	}
	else
		cl.Seconds += dt;
	return cl;
}

void CDRBuilding::CCDRBuilder::OnSpiderTcpDown(void)
{
	ReleaseCalls(ALL_MODULES);
	ResetModuleInfo(ALL_MODULES);
}

void CDRBuilding::CCDRBuilder::OnCombine(CMonMessageEx& mes)
{
    // secID:DWORD

    if(mes.id() == 0)
    {
        AddErrorString("COMBINE : ID = 00000000");
        return;
    }
    //ищем запись с таким ID
    strCallInfo* pCall = FindCall(mes.id());
    if(pCall == NULL)
    {
        char sTmp[100];
        sprintf(sTmp, "COMBINE : Не найдена запись с ID=%08X", mes.id());
        AddErrorString(sTmp);
        return;
    }

    int readPos = 0;    // mes.clearReadPosition();
    DWORD dwSecID = mes.getParameterDWord(readPos);

    BYTE btMod;
    if(pCall->InUnit.dwID == mes.id())
    {
        btMod = pCall->InUnit.btMod;
		myASSERT(btMod < MAX_MOD);
        pCall->dwPrevCombineCallID = dwSecID;
		WriteAliveTime(&pCall->InUnit);
    }
    else
    {
        btMod = pCall->OutUnit.btMod;
		myASSERT(btMod < MAX_MOD);
        pCall->dwNextCombineCallID = dwSecID;
		WriteAliveTime(&pCall->OutUnit);
    }

    CheckModuleInfo(btMod, mes);

}

void CDRBuilding::CCDRBuilder::MakeCDR(const CDRBuilding::strCDR& CDR)
{
	char sStr[1000];
	//формируем дату и время начала звонка
	TClock cl;
	if(CDR.dwTalkDuration == 0)
		cl = CDR.timeSeizIn;
	else
		cl = AddToClock(CDR.timeSeizIn, (int)(CDR.dwSeizDurationIn - CDR.dwTalkDuration));
	char sBegTalkDateTime[100];
	sprintf(sBegTalkDateTime, "%02d-%02d-%02d %02d:%02d:%02d", cl.Date, cl.Month, cl.Year, cl.Hours, cl.Minutes, cl.Seconds);

#define PCGPN_IN	CDR.STrunkInInfo.sCgPN[0] ? CDR.STrunkInInfo.sCgPN : m_SSettings.sNoNumberString
#define PCDPN_IN	CDR.STrunkInInfo.sCdPN[0] ? CDR.STrunkInInfo.sCdPN : m_SSettings.sNoNumberString
#define PCGPN_OUT	CDR.STrunkOutInfo.sCgPN[0] ? CDR.STrunkOutInfo.sCgPN : m_SSettings.sNoNumberString
#define PCDPN_OUT	CDR.STrunkOutInfo.sCdPN[0] ? CDR.STrunkOutInfo.sCdPN : m_SSettings.sNoNumberString

	const char* pCgPN, *pCdPN;
	switch(m_SSettings.btCDRStringFormat)
    {
    case FORMAT_CDR_EX:
		pCgPN = CDR.STrunkInInfo.sCgPN[0] ? CDR.STrunkInInfo.sCgPN :(CDR.STrunkOutInfo.sCgPN[0] ? CDR.STrunkOutInfo.sCgPN : m_SSettings.sNoNumberString);
		pCdPN = CDR.STrunkInInfo.sCdPN[0] ? CDR.STrunkInInfo.sCdPN :(CDR.STrunkOutInfo.sCdPN[0] ? CDR.STrunkOutInfo.sCdPN : m_SSettings.sNoNumberString);
		sprintf(sStr, "%s%s %s %s %s %s %d %d", m_SSettings.bWriteBinary2 ? "\x02" : "", 
			CDR.STrunkInInfo.sTrunk, pCgPN, CDR.STrunkOutInfo.sTrunk, pCdPN,
			sBegTalkDateTime, CDR.dwTalkDuration, CDR.btReason);
#ifdef _DEBUG
		if(strcmp(sStr+1, "C345321 33442946002 C335804 73534257541 28-05-06 13:58:31 0 133") == 0)
			int a = 2;
#endif
		AddCDRString(sStr);
        break;
    case FORMAT_CDR_TARIF2002:
		pCgPN = CDR.STrunkInInfo.sCgPN[0] ? CDR.STrunkInInfo.sCgPN :(CDR.STrunkOutInfo.sCgPN[0] ? CDR.STrunkOutInfo.sCgPN : m_SSettings.sNoNumberString);
		pCdPN = CDR.STrunkInInfo.sCdPN[0] ? CDR.STrunkInInfo.sCdPN :(CDR.STrunkOutInfo.sCdPN[0] ? CDR.STrunkOutInfo.sCdPN : m_SSettings.sNoNumberString);
		sprintf(sStr, "%s%s %s %s %s %d %d", m_SSettings.bWriteBinary2 ? "\x02" : "", 
			CDR.STrunkInInfo.sTrunk, CDR.STrunkOutInfo.sTrunk, pCdPN,
			sBegTalkDateTime, CDR.dwTalkDuration, CDR.btReason);
		AddCDRString(sStr);
        break;
		/*
    case FORMAT_CDR_IGOR_DATAGRUPA:
        GetFormatTarifIgorDatagrupaString(CDR, sStr, 1000);
        break;
		*/
    case FORMAT_CDR_KOMLAIN_ED_SHASHIN:
	    //строка для КомЛайн (Челябинск) Эдуард Шашин
		//номер_МР_порта_откуда АОН АОН_тр номер_МР_порта_куда DIAL DIAL_тр дд-мм-гг чч:мм:сс duration CV
		// 0x02C015105 83519061245 83512689495  C025510 781615 73512781615 13-02-04 12:52:47 300  16
		sprintf(sStr, "%s%s %s %s %s %s %s %s %d %d", m_SSettings.bWriteBinary2 ? "\x02" : "", 
			CDR.STrunkInInfo.sTrunk, PCGPN_IN, PCGPN_OUT,  
			CDR.STrunkOutInfo.sTrunk, PCDPN_IN, PCDPN_OUT,
			sBegTalkDateTime, CDR.dwTalkDuration, CDR.btReason);
		AddCDRString(sStr);
        break;
    case FORMAT_CDR_VADIM_ELTEL:
		pCgPN = CDR.STrunkOutInfo.sCgPN[0] ? CDR.STrunkOutInfo.sCgPN :(CDR.STrunkInInfo.sCgPN[0] ? CDR.STrunkInInfo.sCgPN : m_SSettings.sNoNumberString);
		pCdPN = CDR.STrunkOutInfo.sCdPN[0] ? CDR.STrunkOutInfo.sCdPN :(CDR.STrunkInInfo.sCdPN[0] ? CDR.STrunkInInfo.sCdPN : m_SSettings.sNoNumberString);
		sprintf(sStr, "%s%s %s %s %s %s %d %d", m_SSettings.bWriteBinary2 ? "\x02" : "", 
			CDR.STrunkInInfo.sTrunk, pCgPN, CDR.STrunkOutInfo.sTrunk, pCdPN,
			sBegTalkDateTime, CDR.dwTalkDuration, CDR.btReason);
		AddCDRString(sStr);
        break;
    case FORMAT_CDR_BARSUM_INNUMBERS:
		{
			pCgPN = CDR.STrunkInInfo.sCgPN[0] ? CDR.STrunkInInfo.sCgPN :(CDR.STrunkOutInfo.sCgPN[0] ? CDR.STrunkOutInfo.sCgPN : m_SSettings.sNoNumberString);
			pCdPN = CDR.STrunkInInfo.sCdPN[0] ? CDR.STrunkInInfo.sCdPN :(CDR.STrunkOutInfo.sCdPN[0] ? CDR.STrunkOutInfo.sCdPN : m_SSettings.sNoNumberString);
			char sCh_In[100], sCh_Out[100], sCgPN[100], sCdPN[100];
			if(strncmp(CDR.STrunkInInfo.sTrunk, PREFIX_SUBSCRIBER, 1) == 0)
			{
				strcpy(sCh_In, "       ");
				strcpy(sCgPN, CDR.STrunkInInfo.sTrunk+1);
			}
			else
			{
				strcpy(sCh_In, CDR.STrunkInInfo.sTrunk);
				strcpy(sCgPN, pCgPN);
			}
			if(strncmp(CDR.STrunkOutInfo.sTrunk, PREFIX_SUBSCRIBER, 1) == 0)
			{
				strcpy(sCh_Out, "       ");
				strcpy(sCdPN, CDR.STrunkOutInfo.sTrunk+1);
			}
			else
			{
				strcpy(sCh_Out, CDR.STrunkOutInfo.sTrunk);
				strcpy(sCdPN, pCdPN);
			}
			sprintf(sStr, "%03d %s %20s %20s %s %s %6d",
				CDR.btReason, sCh_In, sCgPN, sCdPN, sCh_Out, sBegTalkDateTime, CDR.dwTalkDuration);
			AddCDRString(sStr);
		}
        break;
    case FORMAT_CDR_BARSUM_OUTNUMBERS:
		{
			pCgPN = CDR.STrunkOutInfo.sCgPN[0] ? CDR.STrunkOutInfo.sCgPN :(CDR.STrunkInInfo.sCgPN[0] ? CDR.STrunkInInfo.sCgPN : m_SSettings.sNoNumberString);
			pCdPN = CDR.STrunkOutInfo.sCdPN[0] ? CDR.STrunkOutInfo.sCdPN :(CDR.STrunkInInfo.sCdPN[0] ? CDR.STrunkInInfo.sCdPN : m_SSettings.sNoNumberString);
			char sCh_In[100], sCh_Out[100], sCgPN[100], sCdPN[100];
			if(strncmp(CDR.STrunkInInfo.sTrunk, PREFIX_SUBSCRIBER, 1) == 0)
			{
				strcpy(sCh_In, "       ");
				strcpy(sCgPN, CDR.STrunkInInfo.sTrunk+1);
			}
			else
			{
				strcpy(sCh_In, CDR.STrunkInInfo.sTrunk);
				strcpy(sCgPN, pCgPN);
			}
			if(strncmp(CDR.STrunkOutInfo.sTrunk, PREFIX_SUBSCRIBER, 1) == 0)
			{
				strcpy(sCh_Out, "       ");
				strcpy(sCdPN, CDR.STrunkOutInfo.sTrunk+1);
			}
			else
			{
				strcpy(sCh_Out, CDR.STrunkOutInfo.sTrunk);
				strcpy(sCdPN, pCdPN);
			}
			sprintf(sStr, "%03d %s %20s %20s %s %s %6d",
				CDR.btReason, sCh_In, sCgPN, sCdPN, sCh_Out, sBegTalkDateTime, CDR.dwTalkDuration);
			AddCDRString(sStr);
		}
        break;
    case FORMAT_CDR_VADIM_ELTEL_NEW:
		{
			DWORD dwDuration = CDR.dwTalkDuration;
			BYTE btHours = (BYTE)(dwDuration / 3600);
			dwDuration -= btHours * 3600;
			BYTE btMinutes = (BYTE)(dwDuration / 60);
			dwDuration -= btMinutes * 60;
			BYTE btSeconds = (BYTE)dwDuration;

			sprintf(sStr, "IC  %s %s %s %02d:%02d:%02d %s %s %d", sBegTalkDateTime, PCGPN_IN, PCDPN_IN,
				btHours, btMinutes, btSeconds, CDR.STrunkInInfo.sTrunk, CDR.STrunkOutInfo.sTrunk, CDR.btReason);
			AddCDRString(sStr);
			sprintf(sStr, "OG %s  %s %s %02d:%02d:%02d %s %s %d", sBegTalkDateTime, PCGPN_OUT, PCDPN_OUT,
				btHours, btMinutes, btSeconds, CDR.STrunkInInfo.sTrunk, CDR.STrunkOutInfo.sTrunk, CDR.btReason);
			AddCDRString(sStr);
		}
        break;
    case FORMAT_CDR_UNIVERSAL:
		sprintf(sStr, "%s %s %s %s %s %s %s %d %d", CDR.STrunkInInfo.sTrunk, PCGPN_IN, PCDPN_IN, 
			CDR.STrunkOutInfo.sTrunk, PCGPN_OUT, PCDPN_OUT,
			sBegTalkDateTime, CDR.dwTalkDuration, CDR.btReason);
		AddCDRString(sStr);
        break;
    case FORMAT_CDR_FULLSTATISTIC:
		char sSeizDateTime[100];
		sprintf(sSeizDateTime, "%02d-%02d-%02d %02d:%02d:%02d", CDR.timeSeizIn.Date, CDR.timeSeizIn.Month, CDR.timeSeizIn.Year, CDR.timeSeizIn.Hours, CDR.timeSeizIn.Minutes, CDR.timeSeizIn.Seconds);
		sprintf(sStr, "%s %s %s %s %s %s %s %d %d %d", CDR.STrunkInInfo.sTrunk, PCGPN_IN, PCDPN_IN, 
			CDR.STrunkOutInfo.sTrunk, PCGPN_OUT, PCDPN_OUT,
			sSeizDateTime, CDR.dwSeizDurationIn, CDR.dwTalkDuration, CDR.btReason);
		AddCDRString(sStr);
        break;
	case FORMAT_CDR_SHESTIVSKI:
		{
			if(strncmp(CDR.STrunkInInfo.sTrunk, PREFIX_SUBSCRIBER, strlen(PREFIX_SUBSCRIBER)) == 0)
			{//Шестивский сказал что ему нужны только исходящие вызовы
				char sDateTime[100];
				const char* pSpace6 = "      ";
				const char* pSpace16 = "                ";
				sprintf(sDateTime, "%02d%02d20%02d%02d%02d%02d", cl.Date, cl.Month, cl.Year, cl.Hours, cl.Minutes, cl.Seconds);
				pCgPN = CDR.STrunkInInfo.sCgPN[0] ? CDR.STrunkInInfo.sCgPN :(CDR.STrunkOutInfo.sCgPN[0] ? CDR.STrunkOutInfo.sCgPN : m_SSettings.sNoNumberString);
				pCdPN = CDR.STrunkInInfo.sCdPN[0] ? CDR.STrunkInInfo.sCdPN :(CDR.STrunkOutInfo.sCdPN[0] ? CDR.STrunkOutInfo.sCdPN : m_SSettings.sNoNumberString);
				sprintf(sStr, "%s%s 1%s%s%s%06d", 
					pCgPN, strlen(pCgPN) >= 6 ? "" : pSpace6+strlen(pCgPN),  pCdPN, strlen(pCdPN) >= 16 ? "" : pSpace16+strlen(pCdPN), sDateTime, CDR.dwTalkDuration);
				AddCDRString(sStr);
			}
		}
		break;
	case FROMAT_CDR_KHARITONOV:
		{
			//формируем дату и время конца звонка
			if(CDR.dwTalkDuration == 0)
				cl = CDR.timeSeizIn;
			else
				cl = AddToClock(CDR.timeSeizIn, (int)(CDR.dwSeizDurationIn));
			char sEndTalkDateTime[100];
			sprintf(sEndTalkDateTime, "%02d-%02d-%02d %02d:%02d:%02d", cl.Date, cl.Month, cl.Year, cl.Hours, cl.Minutes, cl.Seconds);

			pCgPN = CDR.STrunkOutInfo.sCgPN[0] ? CDR.STrunkOutInfo.sCgPN :(CDR.STrunkInInfo.sCgPN[0] ? CDR.STrunkInInfo.sCgPN : m_SSettings.sNoNumberString);
			pCdPN = CDR.STrunkInInfo.sCdPN[0] ? CDR.STrunkInInfo.sCdPN :(CDR.STrunkOutInfo.sCdPN[0] ? CDR.STrunkOutInfo.sCdPN : m_SSettings.sNoNumberString);
			sprintf(sStr, "%s%s %s %s %s %s %d %d", m_SSettings.bWriteBinary2 ? "\x02" : "", 
				CDR.STrunkInInfo.sTrunk, pCgPN, CDR.STrunkOutInfo.sTrunk, pCdPN,
				sEndTalkDateTime, CDR.dwTalkDuration, CDR.btReason);
			AddCDRString(sStr);
		}
		break;
	case FROMAT_CDR_RASPOL:
		//12.01.2007;16:00:01;015101;015224;3311456;-;8495464372;00:00:12;16
		{
			pCgPN = CDR.STrunkInInfo.sCgPN[0] ? CDR.STrunkInInfo.sCgPN :(CDR.STrunkOutInfo.sCgPN[0] ? CDR.STrunkOutInfo.sCgPN : m_SSettings.sNoNumberString);
			pCdPN = CDR.STrunkInInfo.sCdPN[0] ? CDR.STrunkInInfo.sCdPN :(CDR.STrunkOutInfo.sCdPN[0] ? CDR.STrunkOutInfo.sCdPN : m_SSettings.sNoNumberString);
			char sCh_In[100], sCh_Out[100], sCgPN[100], sCdPN[100],sCateg[100];
			strcpy(sCateg, m_SSettings.sNoNumberString);
			if(strncmp(CDR.STrunkInInfo.sTrunk, PREFIX_SUBSCRIBER, 1) == 0)
			{
				strcpy(sCh_In, m_SSettings.sNoNumberString);
				strcpy(sCgPN, CDR.STrunkInInfo.sTrunk+1);
			}
			else
			{
				strcpy(sCh_In, CDR.STrunkInInfo.sTrunk+1);
				strcpy(sCgPN, pCgPN);
			}
			if(strncmp(CDR.STrunkOutInfo.sTrunk, PREFIX_SUBSCRIBER, 1) == 0)
			{
				strcpy(sCh_Out, m_SSettings.sNoNumberString);
				strcpy(sCdPN, CDR.STrunkOutInfo.sTrunk+1);
			}
			else
			{
				strcpy(sCh_Out, CDR.STrunkOutInfo.sTrunk+1);
				strcpy(sCdPN, pCdPN);
			}
			sprintf(sBegTalkDateTime, "%02d.%02d.20%02d;%02d:%02d:%02d", cl.Date, cl.Month, cl.Year, cl.Hours, cl.Minutes, cl.Seconds);
			DWORD dwDuration = CDR.dwTalkDuration;
			BYTE btHours = (BYTE)(dwDuration / 3600);
			dwDuration -= btHours * 3600;
			BYTE btMinutes = (BYTE)(dwDuration / 60);
			dwDuration -= btMinutes * 60;
			BYTE btSeconds = (BYTE)dwDuration;

			sprintf(sStr, "%s;%s;%s;%s;%s;%s;%02d:%02d:%02d;%d", 
				sBegTalkDateTime, sCh_In, sCh_Out, sCgPN, sCateg, sCdPN, btHours, btMinutes, btSeconds, CDR.btReason);
			AddCDRString(sStr);
		}
		break;
	case FROMAT_CDR_CERTIFICATION:
		{
			//045_3145281_153_2245689_04-03-2003_16:55:39_265
			//045_3145281_153_2245689_04-03-2003_16:55:39_X02
			pCgPN = CDR.STrunkInInfo.sCgPN[0] ? CDR.STrunkInInfo.sCgPN :(CDR.STrunkOutInfo.sCgPN[0] ? CDR.STrunkOutInfo.sCgPN : m_SSettings.sNoNumberString);
			pCdPN = CDR.STrunkOutInfo.sCdPN[0] ? CDR.STrunkOutInfo.sCdPN :(CDR.STrunkInInfo.sCdPN[0] ? CDR.STrunkInInfo.sCdPN : m_SSettings.sNoNumberString);
			char sCh_In[100], sCh_Out[100], sCgPN[100], sCdPN[100], sDuration[100];
			if(strncmp(CDR.STrunkInInfo.sTrunk, PREFIX_SUBSCRIBER, 1) == 0)
			{
				strcpy(sCh_In, "000");
				strcpy(sCgPN, CDR.STrunkInInfo.sTrunk+1);
			}
			else
			{
				char sTmp[100];
				strcpy(sTmp, CDR.STrunkInInfo.sTrunk);
				int iCh = atoi(sTmp+5);
				sTmp[5] = 0;
				iCh += ((atoi(sTmp+3)>50?atoi(sTmp+3)-50:atoi(sTmp+3))-1)*30;
				sTmp[3] = 0;
				iCh += (atoi(sTmp+1)-1)*480;
				sprintf(sCh_In, "%03d", iCh);
				strcpy(sCgPN, pCgPN);
			}
			if(strncmp(CDR.STrunkOutInfo.sTrunk, PREFIX_SUBSCRIBER, 1) == 0)
			{
				strcpy(sCh_Out, "000");
				strcpy(sCdPN, CDR.STrunkOutInfo.sTrunk+1);
			}
			else
			{
				char sTmp[100];
				strcpy(sTmp, CDR.STrunkOutInfo.sTrunk);
				int iCh = atoi(sTmp+5);
				sTmp[5] = 0;
				iCh += ((atoi(sTmp+3)>50?atoi(sTmp+3)-50:atoi(sTmp+3))-1)*30;
				sTmp[3] = 0;
				iCh += (atoi(sTmp+1)-1)*480;
				sprintf(sCh_Out, "%03d", iCh);
				strcpy(sCdPN, pCdPN);
			}
			if(CDR.dwTalkDuration != 0)
				sprintf(sDuration, "%03d", CDR.dwTalkDuration);
			else
				sprintf(sDuration, "X%02X", CDR.btReason);

			sprintf(sBegTalkDateTime, "%02d-%02d-20%02d_%02d:%02d:%02d", cl.Date, cl.Month, cl.Year, cl.Hours, cl.Minutes, cl.Seconds);
			sprintf(sStr, "%s_%s_%s_%s_%s_%s",
				sCh_In, sCgPN, sCh_Out, sCdPN, sBegTalkDateTime, sDuration);
			AddCDRString(sStr);
		}
		break;
	case FROMAT_CDR_RASPOL_OUT:
		//12.01.2007;16:00:01;015101;015224;3311456;-;8495464372;00:00:12;16
		{
			pCgPN = CDR.STrunkOutInfo.sCgPN[0] ? CDR.STrunkOutInfo.sCgPN :(CDR.STrunkInInfo.sCgPN[0] ? CDR.STrunkInInfo.sCgPN : m_SSettings.sNoNumberString);
			pCdPN = CDR.STrunkOutInfo.sCdPN[0] ? CDR.STrunkOutInfo.sCdPN :(CDR.STrunkInInfo.sCdPN[0] ? CDR.STrunkInInfo.sCdPN : m_SSettings.sNoNumberString);
			char sCh_In[100], sCh_Out[100], sCgPN[100], sCdPN[100],sCateg[100];
			strcpy(sCateg, m_SSettings.sNoNumberString);
			if(strncmp(CDR.STrunkInInfo.sTrunk, PREFIX_SUBSCRIBER, 1) == 0)
			{
				strcpy(sCh_In, m_SSettings.sNoNumberString);
				strcpy(sCgPN, CDR.STrunkInInfo.sTrunk+1);
			}
			else
			{
				strcpy(sCh_In, CDR.STrunkInInfo.sTrunk+1);
				strcpy(sCgPN, pCgPN);
			}
			if(strncmp(CDR.STrunkOutInfo.sTrunk, PREFIX_SUBSCRIBER, 1) == 0)
			{
				strcpy(sCh_Out, m_SSettings.sNoNumberString);
				strcpy(sCdPN, CDR.STrunkOutInfo.sTrunk+1);
			}
			else
			{
				strcpy(sCh_Out, CDR.STrunkOutInfo.sTrunk+1);
				strcpy(sCdPN, pCdPN);
			}
			sprintf(sBegTalkDateTime, "%02d.%02d.20%02d;%02d:%02d:%02d", cl.Date, cl.Month, cl.Year, cl.Hours, cl.Minutes, cl.Seconds);
			DWORD dwDuration = CDR.dwTalkDuration;
			BYTE btHours = (BYTE)(dwDuration / 3600);
			dwDuration -= btHours * 3600;
			BYTE btMinutes = (BYTE)(dwDuration / 60);
			dwDuration -= btMinutes * 60;
			BYTE btSeconds = (BYTE)dwDuration;

			sprintf(sStr, "%s;%s;%s;%s;%s;%s;%02d:%02d:%02d;%d", 
				sBegTalkDateTime, sCh_In, sCh_Out, sCgPN, sCateg, sCdPN, btHours, btMinutes, btSeconds, CDR.btReason);
			AddCDRString(sStr);
		}
		break;
	case FROMAT_CDR_RASPOL_INOUT:
		//АОН исходящий, набранный номер - входящий
		//12.01.2007;16:00:01;015101;015224;3311456;-;8495464372;00:00:12;16
		{
			pCgPN = CDR.STrunkOutInfo.sCgPN[0] ? CDR.STrunkOutInfo.sCgPN :(CDR.STrunkInInfo.sCgPN[0] ? CDR.STrunkInInfo.sCgPN : m_SSettings.sNoNumberString);
			pCdPN = CDR.STrunkInInfo.sCdPN[0] ? CDR.STrunkInInfo.sCdPN :(CDR.STrunkOutInfo.sCdPN[0] ? CDR.STrunkOutInfo.sCdPN : m_SSettings.sNoNumberString);
			char sCh_In[100], sCh_Out[100], sCgPN[100], sCdPN[100],sCateg[100];
			strcpy(sCateg, m_SSettings.sNoNumberString);
			if(strncmp(CDR.STrunkInInfo.sTrunk, PREFIX_SUBSCRIBER, 1) == 0)
			{
				strcpy(sCh_In, m_SSettings.sNoNumberString);
				strcpy(sCgPN, CDR.STrunkInInfo.sTrunk+1);
				//strcpy(sCgPN, pCgPN);
			}
			else
			{
				strcpy(sCh_In, CDR.STrunkInInfo.sTrunk+1);
				strcpy(sCgPN, pCgPN);
			}
			if(strncmp(CDR.STrunkOutInfo.sTrunk, PREFIX_SUBSCRIBER, 1) == 0)
			{
				strcpy(sCh_Out, m_SSettings.sNoNumberString);
				strcpy(sCdPN, CDR.STrunkOutInfo.sTrunk+1);
			}
			else
			{
				strcpy(sCh_Out, CDR.STrunkOutInfo.sTrunk+1);
				strcpy(sCdPN, pCdPN);
			}
			sprintf(sBegTalkDateTime, "%02d.%02d.20%02d;%02d:%02d:%02d", cl.Date, cl.Month, cl.Year, cl.Hours, cl.Minutes, cl.Seconds);
			DWORD dwDuration = CDR.dwTalkDuration;
			BYTE btHours = (BYTE)(dwDuration / 3600);
			dwDuration -= btHours * 3600;
			BYTE btMinutes = (BYTE)(dwDuration / 60);
			dwDuration -= btMinutes * 60;
			BYTE btSeconds = (BYTE)dwDuration;

			sprintf(sStr, "%s;%s;%s;%s;%s;%s;%02d:%02d:%02d;%d", 
				sBegTalkDateTime, sCh_In, sCh_Out, sCgPN, sCateg, sCdPN, btHours, btMinutes, btSeconds, CDR.btReason);
			AddCDRString(sStr);
		}
		break;
	case FROMAT_CDR_RASPOL_SUBSAON:
		//при исходящем вызове от абонента ставится его АОН, а не NumberA абонента
		//при входящем вызове на абонента ставится входящий набранный номер, а не а не NumberA абонента
		//12.01.2007;16:00:01;015101;015224;3311456;-;8495464372;00:00:12;16
		{
			pCgPN = CDR.STrunkInInfo.sCgPN[0] ? CDR.STrunkInInfo.sCgPN :(CDR.STrunkOutInfo.sCgPN[0] ? CDR.STrunkOutInfo.sCgPN : m_SSettings.sNoNumberString);
			pCdPN = CDR.STrunkInInfo.sCdPN[0] ? CDR.STrunkInInfo.sCdPN :(CDR.STrunkOutInfo.sCdPN[0] ? CDR.STrunkOutInfo.sCdPN : m_SSettings.sNoNumberString);
			char sCh_In[100], sCh_Out[100], sCgPN[100], sCdPN[100],sCateg[100];
			strcpy(sCateg, m_SSettings.sNoNumberString);
			if(strncmp(CDR.STrunkInInfo.sTrunk, PREFIX_SUBSCRIBER, 1) == 0)
			{
				strcpy(sCh_In, m_SSettings.sNoNumberString);
//				strcpy(sCgPN, CDR.STrunkInInfo.sTrunk+1);
				strcpy(sCgPN, pCgPN);
			}
			else
			{
				strcpy(sCh_In, CDR.STrunkInInfo.sTrunk+1);
				strcpy(sCgPN, pCgPN);
			}
			if(strncmp(CDR.STrunkOutInfo.sTrunk, PREFIX_SUBSCRIBER, 1) == 0)
			{
				strcpy(sCh_Out, m_SSettings.sNoNumberString);
//				strcpy(sCdPN, CDR.STrunkOutInfo.sTrunk+1);
				strcpy(sCdPN, pCdPN);
			}
			else
			{
				strcpy(sCh_Out, CDR.STrunkOutInfo.sTrunk+1);
				strcpy(sCdPN, pCdPN);
			}
			sprintf(sBegTalkDateTime, "%02d.%02d.20%02d;%02d:%02d:%02d", cl.Date, cl.Month, cl.Year, cl.Hours, cl.Minutes, cl.Seconds);
			DWORD dwDuration = CDR.dwTalkDuration;
			BYTE btHours = (BYTE)(dwDuration / 3600);
			dwDuration -= btHours * 3600;
			BYTE btMinutes = (BYTE)(dwDuration / 60);
			dwDuration -= btMinutes * 60;
			BYTE btSeconds = (BYTE)dwDuration;

			sprintf(sStr, "%s;%s;%s;%s;%s;%s;%02d:%02d:%02d;%d", 
				sBegTalkDateTime, sCh_In, sCh_Out, sCgPN, sCateg, sCdPN, btHours, btMinutes, btSeconds, CDR.btReason);
			AddCDRString(sStr);
		}
		break;
	case FROMAT_CDR_SAMPLE_MODE:
		{
			char* s = m_SSettings.sSampleString;
		}
		break;
    default:
		if(! m_dwLastError && ERROR_UNKNOWN_CDR_FORMAT )
			m_dwLastError += ERROR_UNKNOWN_CDR_FORMAT;
        AddErrorString("Неизвестный формат выходного файла");
    }

}

std::string CDRBuilding::CCDRBuilder::GetStringParameterBySample(const char* sSample, const CDRBuilding::strCDR& CDR)
{
	return (std::string) "";
}

void CDRBuilding::CCDRBuilder::AddJournalString(const char* sStr)
{
	if(!m_SJournalSettings.bEnable)
		return;
	char* pTmp = new char[strlen(sStr)+1];
	strcpy(pTmp, sStr);
	m_lstJour.push_back(pTmp);
}

void CDRBuilding::CCDRBuilder::AddCDRString(const char* sStr)
{
	char* pTmp = new char[strlen(sStr)+1];
	strcpy(pTmp, sStr);
	m_lstCDR.push_back(pTmp);
}

void CDRBuilding::CCDRBuilder::OnRelease(CMonMessageEx& mes)
{
#ifdef _DEBUG
	
	if(mes.id() == 0xC038013C)
	{
		int a = 2;
	}
#endif
    // reason:BYTE
    if(mes.id() == 0)
    {
        AddErrorString("RELEASE : ID = 00000000");
        return;
    }

	// int readPos = 0;    // mes.clearReadPosition();
    strCallInfo* pCall = FindCall(mes.id());
    if(pCall == NULL)
        return;

	BYTE btModReleaseGlTime;
	DWORD dwReleaseGlTime;
    if(pCall->InUnit.dwID == mes.id())
    {
        CheckModuleInfo(pCall->InUnit.btMod, mes);
		pCall->InUnit.dwReleaseGlTime = GetGlobalTime(pCall->InUnit.btMod, mes.time());
		dwReleaseGlTime = pCall->InUnit.dwReleaseGlTime;
		btModReleaseGlTime = pCall->InUnit.btMod;
		WriteAliveTime(&pCall->InUnit);		
	}
	else
	{
        CheckModuleInfo(pCall->OutUnit.btMod, mes);
		pCall->OutUnit.dwReleaseGlTime = GetGlobalTime(pCall->OutUnit.btMod, mes.time());
		dwReleaseGlTime = pCall->OutUnit.dwReleaseGlTime;
		btModReleaseGlTime = pCall->OutUnit.btMod;
		WriteAliveTime(&pCall->OutUnit);
	}
	/*
	pCall->InUnit.dwComoverGlTime = 0;
	pCall->OutUnit.dwComoverGlTime = 0;
	*/

	int readPos = 0;    // mes.clearReadPosition();
    BYTE btReason = mes.getParameterByte(readPos);

    //bool bTalk = pCall->bTalk;
    DWORD dwNextID = pCall->dwNextCombineCallID;
    DWORD dwPrevID = pCall->dwPrevCombineCallID;

	if(dwNextID != 0 || dwPrevID != 0)
	    ReleaseCall(pCall, btReason);
	else
		if(pCall->InUnit.dwReleaseGlTime != 0 && (pCall->OutUnit.dwReleaseGlTime != 0 || pCall->OutUnit.dwID == 0))
		{
			ReleaseCall(pCall, btReason);
		}
		else
			return;

    //если этот звонок является началом цепочки
    //все время перескок с Call на Seizure
    while(dwNextID != 0)
    {
        pCall = FindCall(dwNextID);
        //проверки 
        if(pCall == NULL)
        {
			break;
        }
		myASSERT(pCall->InUnit.dwID == dwNextID);
		if(m_ModDelta[btModReleaseGlTime][pCall->InUnit.btMod] != MOD_DELTA_UNINIT)
			pCall->InUnit.dwReleaseGlTime = dwReleaseGlTime + m_ModDelta[btModReleaseGlTime][pCall->InUnit.btMod];
		else
			if(m_ModDelta[btModReleaseGlTime][pCall->OutUnit.btMod] != MOD_DELTA_UNINIT)
				pCall->OutUnit.dwReleaseGlTime = dwReleaseGlTime + m_ModDelta[btModReleaseGlTime][pCall->OutUnit.btMod];
			else
				pCall->InUnit.dwReleaseGlTime = GetGlobalTime(pCall->InUnit.btMod, m_ModuleInfo[pCall->InUnit.btMod].btLastRelTime);

		/*
		pCall->InUnit.dwComoverGlTime = 0;
		pCall->OutUnit.dwComoverGlTime = 0;
		*/

        dwNextID = pCall->dwNextCombineCallID;
        ReleaseCall(pCall, btReason);
    }

    //если этот звонок является концом цепочки
    //все время перескок с Seizure на Call
    while(dwPrevID != 0)
    {
        pCall = FindCall(dwPrevID);
        //проверки 
        if(pCall == NULL)
        {
			break;
        }
		if(pCall->InUnit.dwID == dwPrevID)
		{//если чел позвонил двум и объединил их
			if(m_ModDelta[btModReleaseGlTime][pCall->InUnit.btMod] != MOD_DELTA_UNINIT)
				pCall->InUnit.dwReleaseGlTime = dwReleaseGlTime + m_ModDelta[btModReleaseGlTime][pCall->InUnit.btMod];
			else
				if(m_ModDelta[btModReleaseGlTime][pCall->OutUnit.btMod] != MOD_DELTA_UNINIT)
					pCall->OutUnit.dwReleaseGlTime = dwReleaseGlTime + m_ModDelta[btModReleaseGlTime][pCall->OutUnit.btMod];
				else
					pCall->InUnit.dwReleaseGlTime = GetGlobalTime(pCall->InUnit.btMod, m_ModuleInfo[pCall->InUnit.btMod].btLastRelTime);
		}
		else
		{
			if(m_ModDelta[btModReleaseGlTime][pCall->OutUnit.btMod] != MOD_DELTA_UNINIT)
				pCall->OutUnit.dwReleaseGlTime = dwReleaseGlTime + m_ModDelta[btModReleaseGlTime][pCall->OutUnit.btMod];
			else
				if(m_ModDelta[btModReleaseGlTime][pCall->InUnit.btMod] != MOD_DELTA_UNINIT)
					pCall->InUnit.dwReleaseGlTime = dwReleaseGlTime + m_ModDelta[btModReleaseGlTime][pCall->InUnit.btMod];
				else
					pCall->OutUnit.dwReleaseGlTime = GetGlobalTime(pCall->OutUnit.btMod, m_ModuleInfo[pCall->OutUnit.btMod].btLastRelTime);
		}

//		myASSERT(pCall->OutUnit.dwID == dwPrevID);

		if(m_ModDelta[btModReleaseGlTime][pCall->OutUnit.btMod] != MOD_DELTA_UNINIT)
		{
			pCall->OutUnit.dwReleaseGlTime = dwReleaseGlTime + m_ModDelta[btModReleaseGlTime][pCall->OutUnit.btMod];
		}
		else
			if(m_ModDelta[btModReleaseGlTime][pCall->InUnit.btMod] != MOD_DELTA_UNINIT)
			{
				pCall->InUnit.dwReleaseGlTime = dwReleaseGlTime + m_ModDelta[btModReleaseGlTime][pCall->InUnit.btMod];
			}
			else
			{
				pCall->OutUnit.dwReleaseGlTime = GetGlobalTime(pCall->OutUnit.btMod, m_ModuleInfo[pCall->OutUnit.btMod].btLastRelTime);
			}

		/*
		pCall->InUnit.dwComoverGlTime = 0;
		pCall->OutUnit.dwComoverGlTime = 0;
		*/

        dwPrevID = pCall->dwPrevCombineCallID;
        ReleaseCall(pCall, btReason);
    }

}

void CDRBuilding::CCDRBuilder::OnDvoRedirect(CMonMessageEx& mes)
{
    if(mes.id() == 0)
        return;

    strCallInfo* pCall = FindCall(mes.id());
    if(pCall == NULL)
    {
        char sTmp[200];
        sprintf(sTmp, "MON_DVO_REDIRECT : Не найдена запись с ID =%08X ", mes.id());
        AddErrorString(sTmp);
        return;
    }

    int readPos = 0;    // mes.clearReadPosition();
    BYTE btDvoCode = mes.getParameterByte(readPos);
    if(btDvoCode == 0x21 || btDvoCode == 0x29)
    {
        const char* pNumFrom = mes.getParameterStringPtr(readPos);
        const char* pNumTo = mes.getParameterStringPtr(readPos);
        char sNumStr[200];
        sprintf(sNumStr, "[%s|%s]", pNumFrom, pNumTo);

        int iBeg = (int)strlen(pCall->acRedirBuf);
        if(iBeg + strlen(sNumStr) + 1 <= DVO_REDIR_BUFFER)
        {
            strcpy(pCall->acRedirBuf + iBeg, sNumStr);
        }
    }
}

CDRBuilding::strCallInfo* CDRBuilding::CCDRBuilder::GetPBegListCall(void)
{
	return m_pBegListCall;
}

void CDRBuilding::CCDRBuilder::WriteAliveTime(CDRBuilding::strCallInfoUnit* pCallUnit)
{
	if(pCallUnit->AliveClock.Control == CLOCK_INITIALIZED_EX)
		return;
	if(pCallUnit->btMod == 0)
		return;
	if(m_ModuleInfo[pCallUnit->btMod].LastAliveTime.clock.Control == CLOCK_UNINITIALIZED)
		return;
	pCallUnit->dwAliveGlTime = m_ModuleInfo[pCallUnit->btMod].LastAliveTime.dwGlTime;
	pCallUnit->AliveClock = m_ModuleInfo[pCallUnit->btMod].LastAliveTime.clock;
	pCallUnit->AliveClock.Control = CLOCK_INITIALIZED_EX;
}

bool CDRBuilding::CCDRBuilder::IsInnerNumber(char* pNum, bool bIsAon)
{
    bool bRes = false;
    TListInterval::iterator it;
    char sNumber[100];
    strcpy(sNumber, pNum);
	//проверка на префикс
	// проверяется только у АОНов если m_bCutPrefix = true;
	int iPrefixLen = strlen(m_SLocalNumbersSettings.SPrefix.sPrefix);
	if((bIsAon) && m_SLocalNumbersSettings.SPrefix.bCutPrefix && iPrefixLen != 0)
    {//если стоит настройка обрезать префикс
		if(iPrefixLen < (int)strlen(pNum) && iPrefixLen != 0)
			if(strncmp(m_SLocalNumbersSettings.SPrefix.sPrefix, pNum, iPrefixLen) == 0)
				strcpy(sNumber, pNum+iPrefixLen);
    }

	for(it = m_SLocalNumbersSettings.lstInterval.begin(); it != m_SLocalNumbersSettings.lstInterval.end(); it++)
    {
        if(strlen((*it).beg) == strlen(sNumber))
        {
            if( (strcmp((*it).beg, sNumber) <= 0) && (strcmp((*it).end, sNumber) >=0 ) )
            {
                bRes = true;
                strcpy(pNum, sNumber);//если был обрезан префикс, то так и должно быть
                break;
            }
        }
    }
    return bRes;
}

void CDRBuilding::CCDRBuilder::FreeListMem(CDRBuilding::TPCharList& lst)
{
	CDRBuilding::TPCharList::iterator it;
	for(it = lst.begin(); it != lst.end(); it++)
	{
		delete *it;
	}
	lst.clear();
}

void CDRBuilding::CCDRBuilder::CopyList(CDRBuilding::TPCharList& lstDest, const CDRBuilding::TPCharList& lstSrc)
{
	lstDest.clear();
	CDRBuilding::TPCharList::const_iterator it;
	char* pTmp;
	for(it = lstSrc.begin(); it != lstSrc.end(); it++)
	{
		pTmp = new char[strlen(*it)+1];
		strcpy(pTmp, *it);
		lstDest.push_back(pTmp);

	}
}

