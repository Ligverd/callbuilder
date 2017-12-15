
#include  <include/lib.h>

#ifdef ARMLINUX
// ************************

WORD _LRW ( WORD* X )
{
    if (!(((DWORD)X) & 1))
        return *X;
    WORD ret;
    ((BYTE*)&ret)[0] = ((BYTE*)X)[0];
    ((BYTE*)&ret)[1] = ((BYTE*)X)[1];
    return ret;
}

short _LRS ( short* X )
{
    if (!(((DWORD)X) & 1))
        return *X;
    short ret;
    ((BYTE*)&ret)[0] = ((BYTE*)X)[0];
    ((BYTE*)&ret)[1] = ((BYTE*)X)[1];
    return ret;
}

DWORD _LRD ( DWORD* X )
{
    if (!(((DWORD)X) & 3))
        return *X;
    DWORD ret;
    ((BYTE*)&ret)[0] = ((BYTE*)X)[0];
    ((BYTE*)&ret)[1] = ((BYTE*)X)[1];
    ((BYTE*)&ret)[2] = ((BYTE*)X)[2];
    ((BYTE*)&ret)[3] = ((BYTE*)X)[3];
    return ret;
}

int _LRI ( int* X )
{
    if (!(((DWORD)X) & 3))
        return *X;
    int ret;
    ((BYTE*)&ret)[0] = ((BYTE*)X)[0];
    ((BYTE*)&ret)[1] = ((BYTE*)X)[1];
    ((BYTE*)&ret)[2] = ((BYTE*)X)[2];
    ((BYTE*)&ret)[3] = ((BYTE*)X)[3];
    return ret;
}


// ************************

void _LXW ( WORD* X, WORD Y )
{
    if (!(((DWORD)X) & 1))
    {
        *X = Y;
        return;
    }
    ((BYTE*)X)[0] = ((BYTE*)&Y)[0];
    ((BYTE*)X)[1] = ((BYTE*)&Y)[1];
}

void _LXS ( short* X, short Y )
{
    if (!(((DWORD)X) & 1))
    {
        *X = Y;
        return;
    }
    ((BYTE*)X)[0] = ((BYTE*)&Y)[0];
    ((BYTE*)X)[1] = ((BYTE*)&Y)[1];
}

void _LXD ( DWORD* X, DWORD Y )
{
    if (!(((DWORD)X) & 3))
    {
        *X = Y;
        return;
    }
    ((BYTE*)X)[0] = ((BYTE*)&Y)[0];
    ((BYTE*)X)[1] = ((BYTE*)&Y)[1];
    ((BYTE*)X)[2] = ((BYTE*)&Y)[2];
    ((BYTE*)X)[3] = ((BYTE*)&Y)[3];
}

void _LXI ( int* X, int Y )
{
    if (!(((DWORD)X) & 3))
    {
        *X = Y;
        return;
    }
    ((BYTE*)X)[0] = ((BYTE*)&Y)[0];
    ((BYTE*)X)[1] = ((BYTE*)&Y)[1];
    ((BYTE*)X)[2] = ((BYTE*)&Y)[2];
    ((BYTE*)X)[3] = ((BYTE*)&Y)[3];
}
#endif