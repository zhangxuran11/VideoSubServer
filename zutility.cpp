#include "zutility.h"

ZUtility::ZUtility()
{

}
 QString ZUtility::getVideoUrl(int channel)
{
    if(channel == 9)
        return "http://192.168.1.123:8090/test2.sdp";
    else
        return QString("udp://225.0.0.1:200%1").arg(channel);
}

