#ifndef _CDRBUILDER_H_
#define _CDRBUILDER_H_

#include <monmessage.h>
#include <list>
#include <string>


namespace CDRBuilding
{
//*****************************************************************************
// ��������� DEFINE'�

//��������� ������
#define FORMAT_CDR_TARIF2002                1
#define FORMAT_CDR_EX                       2
#define FORMAT_CDR_IGOR_DATAGRUPA           3
#define FORMAT_CDR_KOMLAIN_ED_SHASHIN       4
#define FORMAT_CDR_VADIM_ELTEL              5
#define FORMAT_CDR_BARSUM_INNUMBERS         6
#define FORMAT_CDR_BARSUM_OUTNUMBERS        7
#define FORMAT_CDR_VADIM_ELTEL_NEW          8
#define FORMAT_CDR_UNIVERSAL                9
#define FORMAT_CDR_FULLSTATISTIC            10
#define FORMAT_CDR_SHESTIVSKI               11
#define FORMAT_CDR_KHARITONOV               12
#define FORMAT_CDR_RASPOL                   13
#define FORMAT_CDR_CERTIFICATION            14
#define FORMAT_CDR_RASPOL_OUT               15
#define FORMAT_CDR_RASPOL_INOUT             16
#define FORMAT_CDR_RASPOL_SUBSAON           17
#define FORMAT_CDR_SAMPLE_MODE              18
#define FORMAT_CDR_TARIFF2000               19
#define FORMAT_CDR_BGBILLING                20

#define ALL_MODULES                         255
#define PERIOD_CHECK_COMOVER_CALLS          3600
#define CLOCK_UNINITIALIZED                 255
#define CLOCK_INITIALIZED_EX                22
#define MAX_RELATIVE_TIME_BREAK             12
#define MOD_DELTA_UNINIT                    2000000000
#define MAX_TALK_DURATION                   3600*10*2*10

#define PREFIX_SUBSCRIBER                   "A"
#define PREFIX_TRUNK                        "C"

// 133 - ���������� ����������� � scomm, ����� ��������� �������� �
//       ���������� �������� ������������ ����������
// 134 - ������������ ������ �������� � scomm, ����� ��������� �������� �
//       ���������� �������� ������������ ����������
// 135 - ��������� ������� ������������� ������.
#define RELEASE_CAUSE_TCP_MODULE_DOWN       133
#define RELEASE_CAUSE_COMOVERLOAD           134
#define RELEASE_CAUSE_DOUBLECALLID          135

//*****************************************************************************
//����������� ��� ��������� ������� ������
#define SAMPLE__CHANNEL_IN                      "Ch_In"
#define SAMPLE__CHANNEL_IN__COMMENT             "�������� �����"
#define SAMPLE__CGPN_IN                         "CgPN_In"
#define SAMPLE__CGPN_IN__COMMENT                "�������� ���"
#define SAMPLE__CDPN_IN                         "CdPN_In"
#define SAMPLE__CDPN_IN__COMMENT                "�������� ��������� �����"
#define SAMPLE__CHANNEL_OUT                     "Ch_Out"
#define SAMPLE__CHANNEL_OUT__COMMENT            "��������� �����"
#define SAMPLE__CGPN_OUT                        "CgPN_Out"
#define SAMPLE__CGPN_OUT__COMMENT               "��� ���������"
#define SAMPLE__CDPN_OUT                        "CdPN_Out"
#define SAMPLE__CDPN_OUT__COMMENT               "��������� ����� ���������"
#define SAMPLE__DATE_BEGTALK                    "Date"
#define SAMPLE__DATE_BEGTALK__COMMENT           "���� ������ ���������"
#define SAMPLE__TIME_BEGTALK                    "Time"
#define SAMPLE__TIME_BEGTALK__COMMENT           "����� ������ ���������"
#define SAMPLE__DURATION                        "Dur"
#define SAMPLE__DURATION__COMMENT               "�����������������"
#define SAMPLE__CV                              "CV"
#define SAMPLE__CV__COMMENT                     "������� �����"

#define SAMPLE__DEFAULT_PREFIX                  "%"
#define SAMPLE__DEFAULT_DIVIDER                 " "
#define SAMPLE__DEFAULT_NODATASTR               "-"

//*****************************************************************************
//��������� � ��������� � ���� DEFINE'�
//*****************************************************************************

// ���������-��������� ������
#define  MAX_NONUMBERSTRING_LENGTH      128
#define  MAX_SAMPLESTRING_LENGTH        1024
struct strCDRBuilderSettings
{
    std::string sA164;
    std::string sB164;
    char sSampleString[MAX_SAMPLESTRING_LENGTH];        //������-������ CDR �����
    char sNoNumberString[MAX_NONUMBERSTRING_LENGTH];    //������ ���������, ���� ����� �� �����������
    bool bDeleteFirstDigitFromAON;                      //������� ������ ����� �� ����
    bool bBill2000;                                     //����� ������������� � �������2000
    bool bWriteUnansweredCalls;                         //���������� ������������ ������
    bool bWriteBinary2;                                 //������� �������� ������
    bool bDecrementDuration;                            //��������� ����������������� ��������� �� 1 ���.
    BYTE btCDRStringFormat;                             //������ CDR-������
    BYTE dummy[2];
};

//��� ���������� �������
//�������
#define MAX_PREFIX_LENGTH               32
struct strPrefix
{
    bool bCutPrefix;                                    //�������� ������ � ��������� ����, �����
    bool dummy[3];
                                                        //���������� �������� �� ����� ����������
    char sPrefix[MAX_PREFIX_LENGTH];                    //�������
};
//�������� ���������� �������
#define MAX_INTERVAL_LENGTH             32
struct strInterval
{
    char beg[MAX_INTERVAL_LENGTH];                  //������ ����� ���������
    char end[MAX_INTERVAL_LENGTH];                  //��������� ����� ���������
};
typedef std::list<strInterval> TListInterval;
//��������� �������
struct strJournalSettings
{
    int iMaxDuration;                               //������������ ������-�� ���������, ����� ������� ������ �������� � ������
    bool bEnable;                                   //����� ������ ��� ���
    bool dummy[3];
};


//*****************************************************************************
//���������, ��� �������� ��� ���������� � ������
#define MAX_LEN_NUMBER              32
#define STRCALLINFO_RESERVED        48
#define STRCALLINFOUNIT_RESERVED    24
#define DVO_REDIR_BUFFER            128


struct strCDRTrunkUnit
{
    char sTrunk[MAX_LEN_NUMBER];
    char sCdPN[MAX_LEN_NUMBER];  //��������� �����
    char sCgPN[MAX_LEN_NUMBER];  //���
};


struct strCDR
{
    strCDRTrunkUnit  STrunkInInfo;
    strCDRTrunkUnit  STrunkOutInfo;

    TClock  timeSeizIn;
    TClock  timeSeizOut;
    DWORD   dwSeizDurationIn;
    DWORD   dwSeizDurationOut;
    DWORD   dwTalkDuration;
    BYTE    btReason;
    BYTE    dummy[3];
};


struct strTime
{
    DWORD dwGlTime;
    TClock clock;
};

struct strCallInfoUnit
{
    DWORD dwID;
    DWORD dwSeizGlTime;       //����� ������ �������
    DWORD dwComoverGlTime;    //����� COMOVERLOAD'�
    DWORD dwBegTalkGlTime;      //����� ������ ���������
    DWORD dwReleaseGlTime;      //����� ����� ���������
    DWORD dwAliveGlTime;      //���������� ����� ALIVE'�

    //����� ����
    TClock  clockSeiz;          //�� ������������
    TClock  AliveClock;         //��������� ALIVE

    //���� ��� �������, �� ���� � ���
    char acSubsNumber[MAX_LEN_NUMBER];//���� SIG_EXT �� ����������� �������, ��� ������ ��� ���� �������
    char acSubsAON[MAX_LEN_NUMBER];//���� SIG_EXT �� ��� ��������
    //��������� ����� � ���
    char acCdPN[MAX_LEN_NUMBER]; //��������� �����
    char acCgPN[MAX_LEN_NUMBER];//���
    //������
    BYTE abReserved[STRCALLINFOUNIT_RESERVED];

    BYTE btSig;
    BYTE btMod;
    BYTE btPcmSlot;
    BYTE btKIPort;
};


struct strCallInfo
{
    strCallInfoUnit InUnit;
    strCallInfoUnit OutUnit;

    DWORD dwNextCombineCallID;
    DWORD dwPrevCombineCallID;

    int iDelta1;                   //InUnit.dwBegGlTime + iDelta = OutUnit.dwBegGlTime 
    int fRadiusAbsentAONInUnit;  //���� ������ ��� � ����� InUnit (��� �������)
    char RadiusAuthUserName[32];  //��� ������������ ��� ����������� ������
    char acRedirBuf[DVO_REDIR_BUFFER];

    //������
    BYTE abReserved[STRCALLINFO_RESERVED];

    strCallInfo* pNextCall;
    strCallInfo* pPrevCall;

    bool bTalk; //��� �������� ��� ���
    bool dummy[3];
} ;
typedef std::list<strCallInfo*> TListPSCallInfo;

//���������� � �������, ����������� ��� ��������� ���������
#ifndef MAX_MOD
#define MAX_MOD 64
#endif

#define STRMODULEINFO_RESERVED      52
struct strModuleInfo
{
    strTime LastAliveTime;      // ��������� ���������� ����� ALIVE'� � ��� �������� �����
    DWORD   dwCounter;          // ������� ������, ���������������� ����� ������������� ����� ��������� �� ����. ����
    DWORD   dwLastCheckComoverCallsGlTime;      //���������� ����� ��� ��������, ���� �� �������� ������ ��-�� COMOVERLOAD'�
    //������
    BYTE abReserved[STRMODULEINFO_RESERVED];

    BYTE    btLastRelTime;      // ������������� ����� ���������� ��������� ������
    BYTE    dummy[3];
};

typedef std::list<char*> TPCharList;

struct strLocalNumbersSettings
{
    strPrefix SPrefix;
    TListInterval lstInterval;
};

#define LOWORD(X) ((WORD)(X))
#define HIWORD(X) ((WORD)((X)>>16))
#define LOBYTE(X) ((BYTE)(WORD)(X))
#define HIBYTE(X) ((BYTE)(((WORD)(X)) >> 8))

//*****************************************************************************
//��� �����-����������
class CCDRBuilder
{
public:

private:
    //����
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

    int m_ModDelta[MAX_MOD][MAX_MOD]; //������� ��������� �����, �� ������� �� ����������� ������� ������
                                      //������ ����� ��������� ����� �������
    //������
public:
    CCDRBuilder();
    ~CCDRBuilder(void);

    //��������� 
    void SetCommonSettings(const strCDRBuilderSettings* pSSettings);
    void SetLocalNumbersSettings(const TListInterval& lstLocalNumbers,  const strPrefix* pSPrefix = NULL);
    void SetJournalSettings(const strJournalSettings* pSett);
    //�������� � �������� �������, ���������� ����� ��������� �����
    void PutResiduaryCalls(const strModuleInfo SModuleInfoArray[MAX_MOD], const TListPSCallInfo& lstCallInfo);
    void GetResiduaryCalls(strModuleInfo SModuleInfoArray[MAX_MOD], TListPSCallInfo& lstCallInfo) const;

    DWORD GetLastError(void);
    //��������� ��������� 
    bool ProcessMessage(CMonMessageEx& mes);
    //��������� ����������
    void GetCDRList(TPCharList& lst);
    void GetJournalList(TPCharList& lst);
    void GetErrorList(TPCharList& lst);
    //������� �������
    void CleanCalls(void);
    void OnComoverload(BYTE btMod);
    static bool TransformMessageToString(CMonMessageEx& mes, char* pBuffer, int iBufferSize);
    static const char* SigTypeToStr(BYTE sig);

private:
    std::string GetFormat164(const char* sNumber, std::string s164Prefix);
    std::string GetFormatBgBilling164(const char* sNumber, std::string s164Prefix);
    std::string GetStringParameterBySample(const char* sSample, const CDRBuilding::strCDR& CDR);
    bool CheckCallsGlTime(strCallInfoUnit& MainCallUnit, strCallInfoUnit& SubsCallUnit, bool bLastCheck);
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
    void OnSpiderGateLost(CMonMessageEx& mes);
    void ReleaseCalls(BYTE btMod);
    void ReleaseCall(strCallInfo* pCall, BYTE btReason);
    void DeleteCallFromList(strCallInfo* pDelCall);
    TClock AddToClock(const TClock& clock, int iDelta);
    void OnSpiderTcpDown(void);
    void OnCombine(CMonMessageEx& mes);
    void MakeCDR(strCDR& CDR);
    void DeleteSpaces(char* pStr);
    void AddJournalString(const char* sStr);
    void AddCDRString(const char* sStr);
    void OnRelease(CMonMessageEx& mes);
    void OnDvoRedirect(CMonMessageEx& mes);
    bool testNumber(const char *stringtotest);
    void FixCDRNumbers(CDRBuilding::strCDR& CDR);
public:
    strCallInfo* GetPBegListCall(void);
    bool IsAcceptedTime(const TClock* pClock);
    bool TransformGlTimeToClock(BYTE btMod, DWORD dwGlTime, TClock& cl);
    strCallInfo* FindCall(DWORD dwID);
    void FreeListMem(TPCharList& lst);

private:
    void WriteAliveTime(strCallInfoUnit* pCallUnit);
    bool IsInnerNumber(char* pNum, bool bIsAon);
    void CopyList(TPCharList& lstDest, const TPCharList& lstSrc);
public:
    bool IsBilling2000Mode(void);
};

}
#endif
