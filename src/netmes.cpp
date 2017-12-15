#include  <netmes.h>
//#include  <include/lib.h>


void CNetMessage::fillReply(CNetMessage *mes, TModule srcModule)
{
    dst = mes->src;

    src.nMod = srcModule;
    src.nVir = char(-1);

    nMessage = mes->nMessage;
}

void CNetMessage::fillWide(TModule srcModule)
{
    dst.nMod = 0xFF;
    dst.nVir = char(-1);

    src.nMod = srcModule;
    src.nVir = char(-1);

    un.multi.set.Full();
}

void CNetMessage::fillNear(TModule srcModule)
{
    dst.nMod = 0xFE;
    dst.nVir = char(-1);

    src.nMod = srcModule;
    src.nVir = char(-1);
}

void CNetSetPoint::Clear()
{
    for (int i = 0; i < MAX_MODULE / 8; i ++)
    {
        bitSet[i] = 0;
    }
}

void CNetSetPoint::Full()
{
    for (int i = 0; i < MAX_MODULE / 8; i ++)
    {
        bitSet[i] = 0xFF;
    }
}

void CNetSetPoint::SetIndex(short ind)
{
    if (ind >= 0 && ind < MAX_MODULE)
    {
        bitSet[ind / 8] |= 1 << (ind % 8);
    }
}

void CNetSetPoint::ResetIndex(short ind)
{
    if (ind >= 0 && ind < MAX_MODULE)
    {
        bitSet[ind / 8] &= (0xFF ^ (1 << (ind % 8)));
    }
}

bool CNetSetPoint::CheckIndex(short ind)
{
    if (ind >= 0 && ind < MAX_MODULE)
    {
        return (bitSet[ind / 8] & (1 << (ind % 8))) != 0;
    }
    else
        return false;
}


#if defined(__GNUC__) || defined(WIN32)

//
const uc* CNetMessage::decode(const uc* stream, int& size)
{
    if (size < 6) 
        return NULL;

    dst.nMod = *stream++;
    dst.nVir = *stream++;
    src.nMod = *stream++;
    src.nVir = *stream++;

    nMessage = *stream++;
    nMessage |= (*stream++ << 8);

    gate     = *stream++;
    size -= 7;

    if (dst.nMod == 0xFF) {
        if (size < (int)sizeof(CNetSetPoint))
            return NULL;

        memcpy(&un.multi.set, stream, sizeof(un.multi.set));
        stream += sizeof(un.multi.set);

        size -= sizeof(un.multi.set);
    }

    return stream;
}
uc* CNetMessage::encode(uc* stream)
{
    *stream++   = dst.nMod;
    *stream++   = dst.nVir;
    *stream++   = src.nMod;
    *stream++   = src.nVir;

    *stream++   = nMessage & 0xFF;
    *stream++   = nMessage >> 8;

    *stream++   = gate;

    if (dst.nMod == 0xFF) {

        memcpy(stream, &un.multi.set, sizeof(un.multi.set));
        stream += sizeof(un.multi.set);
    }

    return stream;
}

const uc* CNetMessageBody::decode(const uc* stream, int size)
{
    stream = CNetMessage::decode(stream, size);

    if (size > Max_NetMessageDataSize || !stream)
        return NULL;

    if(size)
    {
    memcpy(data, stream, size);
    dataSize = size;
    stream += size;
    }

    return stream;    
}
uc* CNetMessageBody::encode(uc* stream)
{
    stream = CNetMessage::encode(stream);

    memcpy(stream, data, dataSize);
    stream += dataSize;

    return stream;    
}

//added by pax for timeupdate
const uc* CNetMessageBody::input(const uc* stream, int size)
{
    if(!stream)
        return NULL;

    if (dataSize + size > Max_NetMessageDataSize)
        return NULL;

    memcpy(data + dataSize, stream, size);
    dataSize += size;
    stream += size;

    return stream;
}
#endif
