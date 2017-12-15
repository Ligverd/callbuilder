#ifndef _CDRBUILDER_H_1C1CD0C0_20A0_4C3F_A39D_6DF29F450005_
#define _CDRBUILDER_H_1C1CD0C0_20A0_4C3F_A39D_6DF29F450005_
//added by pax
#define BOOL bool
#define FALSE false
#define TRUE true
//added by pax
#include <include/monmessage.h>

#ifdef _DEBUG
//	#define CONTROL_CALLS_COUNT
#endif

#include <list>
#include <string>

namespace CDRBuilding
{
//*****************************************************************************
// различные DEFINE'ы

//установки класса
#define FORMAT_CDR_TARIF2002     	        1
#define FORMAT_CDR_EX				        2
#define FORMAT_CDR_IGOR_DATAGRUPA           3
#define FORMAT_CDR_KOMLAIN_ED_SHASHIN		4
#define FORMAT_CDR_VADIM_ELTEL      		5
#define FORMAT_CDR_BARSUM_INNUMBERS         6
#define FORMAT_CDR_BARSUM_OUTNUMBERS        7
#define FORMAT_CDR_VADIM_ELTEL_NEW  		8
#define FORMAT_CDR_UNIVERSAL                9
#define FORMAT_CDR_FULLSTATISTIC            10
#define FORMAT_CDR_SHESTIVSKI				11
#define FROMAT_CDR_KHARITONOV				12
#define FROMAT_CDR_RASPOL					13
#define FROMAT_CDR_CERTIFICATION			14
#define FROMAT_CDR_RASPOL_OUT				15
#define FROMAT_CDR_RASPOL_INOUT				16
#define FROMAT_CDR_RASPOL_SUBSAON			17
#define FROMAT_CDR_SAMPLE_MODE				18

#define ALL_MODULES							255
#define PERIOD_CHECK_COMOVER_CALLS			3600
#define CLOCK_UNINITIALIZED					255
#define CLOCK_INITIALIZED_EX				22
#define MAX_RELATIVE_TIME_BREAK				12
#define MOD_DELTA_UNINIT					2000000000
#define MAX_TALK_DURATION					3600*10*2

#define PREFIX_SUBSCRIBER					"A"
#define PREFIX_TRUNK						"C"

// 133 - оборвалось подключение к scomm, часть сообщений потеряна и
//       рассчитать реальную длительность невозможно
// 134 - переполнение буфера передачи к scomm, часть сообщений потеряна и
//       рассчитать реальную длительность невозможно
// 135 - обнаружен двойной идентификатор вызова.
#define RELEASE_CAUSE_TCP_MODULE_DOWN		133
#define RELEASE_CAUSE_COMOVERLOAD			134		
#define RELEASE_CAUSE_DOUBLECALLID			135

//*****************************************************************************
//обозначения для настройки формата вывода
#define SAMPLE__CHANNEL_IN						"Ch_In"
#define SAMPLE__CHANNEL_IN__COMMENT				"Входящий транк"
#define SAMPLE__CGPN_IN							"CgPN_In"
#define SAMPLE__CGPN_IN__COMMENT				"Входящий АОН"
#define SAMPLE__CDPN_IN							"CdPN_In"
#define SAMPLE__CDPN_IN__COMMENT				"Входящий набранный номер"
#define SAMPLE__CHANNEL_OUT						"Ch_Out"
#define SAMPLE__CHANNEL_OUT__COMMENT			"Исходящий транк"
#define SAMPLE__CGPN_OUT						"CgPN_Out"
#define SAMPLE__CGPN_OUT__COMMENT				"АОН исходящий"
#define SAMPLE__CDPN_OUT						"CdPN_Out"
#define SAMPLE__CDPN_OUT__COMMENT				"Набранный номер исходящий"
#define SAMPLE__DATE_BEGTALK					"Date"
#define SAMPLE__DATE_BEGTALK__COMMENT			"Дата начала разговора"
#define SAMPLE__TIME_BEGTALK					"Time"
#define SAMPLE__TIME_BEGTALK__COMMENT			"Время начала разговора"
#define SAMPLE__DURATION						"Dur"
#define SAMPLE__DURATION__COMMENT				"Продолжительность"
#define SAMPLE__CV								"CV"
#define SAMPLE__CV__COMMENT						"Причина отбоя"

#define SAMPLE__DEFAULT_PREFIX					"%"
#define SAMPLE__DEFAULT_DIVIDER					" "
#define SAMPLE__DEFAULT_NODATASTR				"-"

//*****************************************************************************
//структуры и связанные с ними DEFINE'ы
//*****************************************************************************

// структуры-настройки класса
#define  MAX_NONUMBERSTRING_LENGTH      100
#define  MAX_SAMPLESTRING_LENGTH		1000
struct strCDRBuilderSettings
{
    bool bWriteUnansweredCalls;							//записывать неотвеченные вызовы
    bool bWriteBinary2;									//ставить бинарную двойку
    char sNoNumberString[MAX_NONUMBERSTRING_LENGTH+1];	//строка выводится, если номер не определился
	bool bDecrementDuration;							//уменьшать продолжительность разговора на 1 сек.
	BYTE btCDRStringFormat;								//формат CDR-строки
	char sSampleString[MAX_SAMPLESTRING_LENGTH+1];		//строка-шаблон CDR строк
};

//для внутренних номеров
//префикс
#define MAX_PREFIX_LENGTH               30
struct strPrefix
{
    BOOL bCutPrefix;									//отрезать префих у входящего АОНа, чтобы 
														//определить является ли номер внутренним
    char sPrefix[MAX_PREFIX_LENGTH+1];					//префикс
};
//диапозон внутренних номеров
#define MAX_INTERVAL_LENGTH             30
struct strInterval
{
	char beg[MAX_INTERVAL_LENGTH+1];					//первый номер диапозона
	char end[MAX_INTERVAL_LENGTH+1];					//последний номер диапозона
};
typedef std::list<strInterval> TListInterval;
//настройки журнала
struct strJournalSettings
{
	BOOL bEnable;										//вести журнал или нет
    int iMaxDuration;									//максимальная продол-ть разговора, после которой звонки попадают в журнал
};


//*****************************************************************************
//структуры, где хранится вся информация о вызове
#define MAX_LEN_NUMBER				30
#define STRCALLINFO_RESERVED		50-8
#define STRCALLINFOUNIT_RESERVED	50 - sizeof(TClock) - sizeof(DWORD) - sizeof(DWORD)  - sizeof(TClock) - sizeof(DWORD) 
#define DVO_REDIR_BUFFER			100

struct strCDRTrunkUnit
{
	char sTrunk[MAX_LEN_NUMBER]; 
	char sCdPN[MAX_LEN_NUMBER];  //набранный номер
	char sCgPN[MAX_LEN_NUMBER];  //АОН
} ;

struct strCDR
{
	strCDRTrunkUnit  STrunkInInfo;
	strCDRTrunkUnit  STrunkOutInfo;

	TClock	timeSeizIn;
	TClock	timeSeizOut;
	DWORD	dwSeizDurationIn;
	DWORD	dwSeizDurationOut;
	DWORD	dwTalkDuration;
	BYTE	btReason;
} ;



struct strTime
{
    DWORD dwGlTime;
    TClock clock;
} ;

struct strCallInfoUnit
{
	DWORD dwID;
	BYTE btSig;
	BYTE btMod;
	BYTE btPcmSlot;						   
	BYTE btKIPort;
    //если это абонент, то инфа о нем
	char acSubsNumber[MAX_LEN_NUMBER];//если SIG_EXT то указывается абонент, кто звонил или кому звонили
	char acSubsAON[MAX_LEN_NUMBER];//если SIG_EXT то АОН абонента
    //набранный номер и АОН
	char acCdPN[MAX_LEN_NUMBER]; //набранный номер
	char acCgPN[MAX_LEN_NUMBER];//АОН
    
    DWORD dwBegTalkGlTime;      //точка начала разговора
    DWORD dwReleaseGlTime;      //точка конца разговора

    //новые поля
    TClock  clockSeiz;			//не используется
    DWORD   dwSeizGlTime;       //точка начала занятия
	DWORD	dwComoverGlTime;	//время COMOVERLOAD'а

    TClock  AliveClock;			//последний ALIVE
    DWORD   dwAliveGlTime;      //глобальное время ALIVE'а
	//резерв
    BYTE abReserved[STRCALLINFOUNIT_RESERVED];
};


struct strCallInfo
{
	strCallInfoUnit InUnit;
	strCallInfoUnit OutUnit;
	bool bTalk;					  //был разговор или нет
    int iDelta1;                   //InUnit.dwBegGlTime + iDelta = OutUnit.dwBegGlTime  

    DWORD dwNextCombineCallID;
    DWORD dwPrevCombineCallID;

    char acRedirBuf[DVO_REDIR_BUFFER];
    //резерв
    BYTE abReserved[STRCALLINFO_RESERVED];

	strCallInfo* pNextCall;          
	strCallInfo* pPrevCall;
} ;
typedef std::list<strCallInfo*> TListPSCallInfo;

//информация о модулях, необходимая при обработке сообщений
#ifndef MAX_MOD
#define MAX_MOD	64
#endif

#define STRMODULEINFO_RESERVED			50
struct strModuleInfo
{
    BYTE    btLastRelTime;      // относительное время последнего сообщения модуля
    DWORD   dwCounter;          // счетчик модуля, инкрементируется когда относительное время переходит на след. круг
    strTime LastAliveTime;      // последнее глобальное время ALIVE'а и его реальное время

    DWORD   dwLastCheckComoverCallsGlTime;      //глобальное время для проверки, есть ли зависшие звонки из-за COMOVERLOAD'а
    //резерв
    BYTE abReserved[STRMODULEINFO_RESERVED];
};

typedef std::list<char*> TPCharList;

struct strLocalNumbersSettings
{
	strPrefix SPrefix;
	TListInterval lstInterval;
};


//*****************************************************************************
//сам класс-обработчик
class CCDRBuilder
{
public:
#ifdef CONTROL_CALLS_COUNT
	int m_iCallsCount;
	int m_iMaxCallsCount;
#endif

private:
	//поля
	strCDRBuilderSettings m_SSettings;
	strLocalNumbersSettings m_SLocalNumbersSettings;

    strModuleInfo m_ModuleInfo[MAX_MOD]; 
	strCallInfo* m_pBegListCall;		  
	strCallInfo* m_pEndListCall;	

	DWORD m_dwLastError;

	strJournalSettings m_SJournalSettings;

	TPCharList m_lstCDR;
	TPCharList m_lstJour;
	TPCharList m_lstErr;

    int m_ModDelta[MAX_MOD][MAX_MOD]; //таблица примерных дельт, по которым из глобального времени одного
	                                  //модуля можно вычислить время другого


	//методы

public:
	CCDRBuilder();
	~CCDRBuilder(void);
	
	//настройки 
	void SetCommonSettings(const strCDRBuilderSettings* pSSettings);
	void SetLocalNumbersSettings(const TListInterval& lstLocalNumbers,  const strPrefix* pSPrefix = NULL);
	void SetJournalSettings(const strJournalSettings* pSett);
	//закрузка и выгрузка звонков, оставшихся после обработки файла
	void PutResiduaryCalls(const strModuleInfo SModuleInfoArray[MAX_MOD], const TListPSCallInfo& lstCallInfo);
	void GetResiduaryCalls(strModuleInfo SModuleInfoArray[MAX_MOD], TListPSCallInfo& lstCallInfo) const;

	DWORD GetLastError(void);
	//обработка сообщения 
	BOOL ProcessMessage(CMonMessageEx& mes);
	//получение результата
	void GetCDRList(TPCharList& lst);
	void GetJournalList(TPCharList& lst);
	void GetErrorList(TPCharList& lst);
	//очистка звонков
	void CleanCalls(void);
	void OnComoverload(BYTE btMod);
	static BOOL TransformMessageToString(CMonMessageEx& mes, char* pBuffer, int iBufferSize);
	static const char* SigTypeToStr(BYTE sig);

private:
	std::string GetStringParameterBySample(const char* sSample, const CDRBuilding::strCDR& CDR);
	BOOL CheckCallsGlTime(strCallInfoUnit& MainCallUnit, strCallInfoUnit& SubsCallUnit, bool bLastCheck);
	void JournalAnalysisMes(CMonMessageEx& mes);
	void ResetModuleInfo(int iModNumber);
	void OnAlive(CMonMessageEx& mes);
	void CheckModuleInfo(BYTE btMod, CMonMessageEx& mes);
	DWORD GetGlobalTime(BYTE btMod, BYTE btRelTime);
	void ReleaseComoverCalls(void);
	BYTE ExtractModNumber(DWORD dwID);
	void OnSeizure(CMonMessageEx& mes);
	void AddErrorString(const char* sStr);
	strCallInfo* AddNewCall(void);
	BYTE GetDaysInMonth(BYTE btMonth, BYTE btYear);
	void OnCall(CMonMessageEx& mes);
	void OnAccept(CMonMessageEx& mes);
	void OnNumber(CMonMessageEx& mes);
	void OnAnswer(CMonMessageEx& mes);
	void OnSpiderModuleDown(CMonMessageEx& mes);
	void ReleaseCalls(BYTE btMod);
	void ReleaseCall(strCallInfo* pCall, BYTE btReason);
	void DeleteCallFromList(strCallInfo* pDelCall);
	TClock AddToClock(const TClock& clock, int iDelta);
	void OnSpiderTcpDown(void);
	void OnCombine(CMonMessageEx& mes);
	void MakeCDR(const strCDR& CDR);
	void AddJournalString(const char* sStr);
	void AddCDRString(const char* sStr);
	void OnRelease(CMonMessageEx& mes);
	void OnDvoRedirect(CMonMessageEx& mes);
public:
	strCallInfo* GetPBegListCall(void);
	bool IsAcceptedTime(const TClock* pClock);
	BOOL TransformGlTimeToClock(BYTE btMod, DWORD dwGlTime, TClock& cl);
	strCallInfo* FindCall(DWORD dwID);
	void FreeListMem(TPCharList& lst);

private:
	void WriteAliveTime(strCallInfoUnit* pCallUnit);
	bool IsInnerNumber(char* pNum, bool bIsAon);
	void CopyList(TPCharList& lstDest, const TPCharList& lstSrc);
};
}

#endif
