/**
 * @file NexHardware.cpp
 *
 * The implementation of base API for using Nextion. 
 *
 * @author  Wu Pengfei (email:<pengfei.wu@itead.cc>)
 * @date    2015/8/11
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */
#include "NexHardware.h"

#define NEX_RET_CMD_FINISHED                (0x01)

#define NEX_RET_INVALID_CMD                 (0x00)
#define NEX_RET_INVALID_COMPONENT_ID        (0x02)
#define NEX_RET_INVALID_PAGE_ID             (0x03)
#define NEX_RET_INVALID_PICTURE_ID          (0x04)
#define NEX_RET_INVALID_FONT_ID             (0x05)
#define NEX_RET_INVALID_BAUD                (0x11)
#define NEX_RET_INVALID_VARIABLE            (0x1A)
#define NEX_RET_INVALID_OPERATION           (0x1B)

#define NEX_RET_EVENT_TOUCH_HEAD            (0x65)     
#define NEX_RET_EVENT_SLEEP_POSITION_HEAD   (0x68)
#define NEX_RET_EVENT_POSITION_HEAD         (0x67)
#define NEX_RET_CURRENT_PAGE_ID_HEAD        (0x66)
#define NEX_RET_STRING_HEAD                 (0x70)
#define NEX_RET_NUMBER_HEAD                 (0x71)
#define NEX_RET_EVENT_LAUNCHED              (0x88)
#define NEX_RET_EVENT_UPGRADED              (0x89)


#define eventMsgLen 7
#define maxEvents   8
byte eventQ[maxEvents][eventMsgLen];
byte eventQPut      = 0;
byte eventQGet      = 0;
byte eventQCnt      = 0;

#define respMsgLen  4
#define maxResps    1
byte *RespBuffer = NULL;
byte RespBufferCnt = 0;
boolean respMsgAvailable = false;

byte *DataBuffer = NULL;
byte DataBufferLen = 0;
byte DataBufferCnt = 0;
boolean dataMsgAvailable = false;

byte msgTmp[eventMsgLen];
byte msgTmpCnt = 0;

byte FFCnt = 0;


enum parseTargets
{
    PARSE_MSG,
    PARSE_STRING,
    PARSE_NUMBER,
    PARSE_DISCARD
};
int parseTarget = parseTargets::PARSE_MSG;


void dumpMsg(const char *s, byte *p, int len)
{
#ifdef DEBUG_SERIAL_ENABLE    
    dbSerialPrintf("%s: ", s);
    for (int i=0; i<len; i++)
        dbSerialPrintf("%02X ", (int)p[i] & 0xff);
    dbSerialPrintf("\n");
#endif    
}


void readDataMsg(void)
{
    if (DataBuffer)
    {
        while(nexSerial.available())
        {
            int c = nexSerial.read() & 0xff;

            dbSerialPrintf("readDataMsg Read: %02X\n", c);

            if (c == 0xff)
            {
                if (++FFCnt == 3)
                {
                    // Found message end
                    if (parseTarget == PARSE_NUMBER)
                    {
                        if (DataBufferCnt == 4)
                        {
                            dumpMsg("numMsg", DataBuffer, DataBufferCnt);
                            dataMsgAvailable = true;
                        }
                        else
                        {
                            dbSerialPrintf("readDataMsg got bad count: %d\n", DataBufferCnt);
                        }
                    }
                    else if (parseTarget == PARSE_STRING)
                    {
                        dataMsgAvailable = true;
                    }
                    
                    DataBuffer = NULL;
                    DataBufferCnt = 0;
                    parseTarget = parseTargets::PARSE_MSG;
                    break;
                }
            }
            else
            {
                // c != 0xff
                FFCnt = 0;
                if (parseTarget == PARSE_NUMBER)
                {
                    // Clip number if too long
                    if (DataBufferCnt <= DataBufferLen)
                    {
                        DataBuffer[DataBufferCnt++] = (byte)c;
                    }
                }
                else if (parseTarget == PARSE_STRING)
                {
                    // Clip string if too long
                    if (DataBufferCnt < DataBufferLen-1)
                    {
                        DataBuffer[DataBufferCnt++] = (byte)c;
                        DataBuffer[DataBufferCnt] = '\0';
                    }
                }
            }
        }
    }
    else
    {
        DataBufferCnt = 0;
        parseTarget = parseTargets::PARSE_MSG;
    }    
}


void parseRx(byte *p, int len)
{
    if (p && len)
    {
        DataBuffer = p;
        DataBufferLen = len;
        DataBufferCnt = 0;
        dataMsgAvailable = false;
        msgTmpCnt = 0;
    }

    if ((parseTarget == parseTargets::PARSE_STRING) || 
        (parseTarget == parseTargets::PARSE_NUMBER) ||
        (parseTarget == parseTargets::PARSE_DISCARD))
    {
        readDataMsg();
    }
    else
    {
        while(nexSerial.available())
        {
            int c = nexSerial.read() & 0xff;
            if (c == NEX_RET_STRING_HEAD)
            {
                // Got String message
                msgTmpCnt = 0;
                DataBufferCnt = 0;
                FFCnt = 0;
                if (DataBuffer)
                {
                    // Got String message
                    parseTarget = parseTargets::PARSE_STRING;
                    break;
                }
                else
                {
                    parseTarget = parseTargets::PARSE_DISCARD;
                }
                FFCnt = 0;
                break;
            }
            else if (c == NEX_RET_NUMBER_HEAD)
            {
                // Got Number message
                msgTmpCnt = 0;
                DataBufferCnt = 0;
                FFCnt = 0;
                if (DataBuffer)
                {
                    parseTarget = parseTargets::PARSE_NUMBER;
                }
                else
                {
                    parseTarget = parseTargets::PARSE_DISCARD;
                }
                break;
            }
            else if (c == 0xff)
            {
                msgTmp[msgTmpCnt++] = (byte)c;
                if (++FFCnt == 3)
                {
                    // Found message end
                    if ((msgTmpCnt == 7) && (msgTmp[0] == NEX_RET_EVENT_TOUCH_HEAD))
                    {
                        // got event
                        if (eventQCnt < maxEvents)
                        {
                            memcpy(eventQ[eventQPut], msgTmp, eventMsgLen);
                            eventQPut = (eventQPut + 1) % maxEvents;
                            eventQCnt++;
                        }
                    }
                    else if (msgTmpCnt == 4)
                    {
                        // got response
                        if (RespBuffer)
                        {
                            memcpy(RespBuffer, msgTmp, respMsgLen);
                            respMsgAvailable = true;
                        }
                    }
                    else
                    {
                        // got crap
                    }
                    msgTmpCnt = 0;
                    FFCnt = 0;
                }
            }
            else
            {
                // c != 0xff
                FFCnt = 0;
                if (msgTmpCnt < (sizeof(msgTmp) - 3))
                {
                    msgTmp[msgTmpCnt++] = (byte)c;
                }
                if (msgTmpCnt == eventMsgLen)
                {
                    // msg in msgTmp is too long, so toss first character
                    msgTmpCnt--;
                    memcpy(msgTmp, &msgTmp[1], msgTmpCnt);                
                }
            }
        }
    }
}


boolean getEvent(byte *p)
{
    boolean ret = false;

    parseRx(NULL, 0);
    if (eventQCnt)
    {
        memcpy(p, eventQ[eventQGet], eventMsgLen);
        eventQGet = (eventQGet + 1) % maxEvents;
        eventQCnt--;
        ret = true;
    }
    return ret;
}


boolean getNumber(byte *p, int timeout)
{
    uint32_t t = millis();
    boolean ret = false;

    dataMsgAvailable = false;
    parseRx(p, sizeof(uint32_t));

    while (!dataMsgAvailable)
    {
        if (((millis() - t) > timeout) || (timeout == 0))
        {
            dbSerial.println("getNumber timeout");
            break;
        }

        parseRx(NULL, 0);
    }

    if (dataMsgAvailable)
    {
        dataMsgAvailable = false;
        ret = true;
    }

    return ret;
}


/*
 * Receive uint32_t data. 
 * 
 * @param number - save uint32_t data. 
 * @param timeout - set timeout time. 
 *
 * @retval true - success. 
 * @retval false - failed.
 *
 */
bool recvRetNumber(uint32_t *number, uint32_t timeout)
{
    bool ret = false;
    uint8_t temp[4] = { 0, 0, 0, 0 };

    if (number)
    {
        ret = getNumber((byte *)number, timeout);    
    }

    if (!ret) 
    {
        dbSerialPrintf("recvRetNumber err\n");
    }
    
    return ret;
}


int getString(byte *p, int len, int timeout)
{
    uint32_t t = millis();
    byte ret = 0;

    parseRx(p, len);

    dataMsgAvailable = false;
    while (!dataMsgAvailable)
    {
        if (((millis() - t) > timeout) || (timeout == 0))
        {
            dbSerial.println("getString timeout");
            break;
        }

        parseRx(NULL, 0);
    }

    if (dataMsgAvailable)
    {
        dataMsgAvailable = false;
        ret = strlen((const char *)p);
    }

    return ret;
}


/*
 * Receive string data. 
 * 
 * @param buffer - save string data. 
 * @param len - string buffer length. 
 * @param timeout - set timeout time. 
 *
 * @return the length of string buffer.
 *
 */
uint16_t recvRetString(char *buffer, uint16_t len, uint32_t timeout)
{
    uint16_t ret = 0;

    if (buffer && len)
    {
        ret = getString((byte *)buffer, len, timeout);
    }

    if (!ret) 
    {
        dbSerialPrintf("recvRetString err\n");
    }

    return ret;
}


boolean getResp(byte *p, uint32_t timeout)
{
    uint32_t t = millis();
    boolean ret = false;

    RespBuffer = p;
    parseRx(NULL, 0);
    while (!respMsgAvailable)
    {
        if ((millis() - t) > timeout)
        {
            dbSerialPrintf("getResp timeout\n");
            break;
        }

        parseRx(NULL, 0);
    }

    if (respMsgAvailable)
    {
        RespBuffer = NULL;
        respMsgAvailable = false;
        ret = true;
    }

    return ret;
}


/*
 * Command is executed successfully. 
 *
 * @param timeout - set timeout time.
 *
 * @retval true - success.
 * @retval false - failed. 
 *
 */
bool recvRetCommandFinished(uint32_t timeout)
{    
    boolean ret = false;
    byte temp[respMsgLen] = {0};
    
    ret = getResp(temp, timeout);
    if (ret && (temp[0] == NEX_RET_CMD_FINISHED))
    {
        ret = true;
    }
    else
    {
        dbSerialPrintf("recvRetCommandFinished err\n");
    }
    return ret;
}


/*
 * Send command to Nextion.
 *
 * @param cmd - the string of command.
 */
void sendCommand(const char* cmd)
{
//    dbSerialPrintf("[%08ld]sendCommand Sent: |%s| FF FF FF\n", millis(), cmd);    
    nexSerial.print(cmd);
    nexSerial.write(0xFF);
    nexSerial.write(0xFF);
    nexSerial.write(0xFF);
}


/*
 * Send command to Nextion.
 *
 * @param cmd - the string of command.
 */
boolean sendCommandWait(const char* cmd)
{
    sendCommand(cmd);
    return recvRetCommandFinished(100);
}


void nexBAUD(long baud)
{
    // Change Nextion BAUD rate
    nexSerial.print("baud=");
    nexSerial.print(baud);
    nexSerial.write(0xff);
    nexSerial.write(0xff);
    nexSerial.write(0xff);
    delay(10);
    nexSerial.flush();
    nexSerial.begin(baud);
    sendCommand("");
    sendCommand("");
    sendCommand("");
    delay(10);
    nexSerial.flush();
}


bool nexInit(long baud)
{
    bool ret = false;
    
    nexSerial.begin(9600);
    sendCommand("");
    sendCommand("");
    sendCommand("");
    delay(10);
    nexSerial.flush();
    ret = sendCommandWait("bkcmd=1");
    ret &= sendCommandWait("page=0");
    if(baud != 9600)
    {
        nexBAUD(baud);
    }
    return ret;
}

void nexLoop(NexTouch *nex_listen_list[])
{
    byte __buffer[10];

    while (getEvent(__buffer))
    {
        NexTouch::iterate(nex_listen_list, __buffer[1], __buffer[2], (int32_t)__buffer[3]);
    }
}
