/***************************************************************
Copyright (c) 2022-2030, shisan233@sszc.live.
SPDX-License-Identifier: MIT 
File:        message_factory.h
Version:     1.0
Author:      cjx
start date:
Description: 消息工厂
Version history

[序号]    |   [修改日期]  |   [修改者]   |   [修改内容]
1             2025-9-24      cjx        create

*****************************************************************/

#ifndef AIS_MESSAGE_FACTORY_H
#define AIS_MESSAGE_FACTORY_H

#include "message.h"

#include <memory>

namespace ais
{

class BitBuffer;

class MessageFactory
{
public:
    static std::unique_ptr<AISMessage> createMessage(BitBuffer& bits);
    
private:
    static std::unique_ptr<AISMessage> parseType1(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType2(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType3(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType4(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType5(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType6(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType7(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType8(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType9(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType10(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType11(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType12(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType13(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType14(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType15(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType16(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType17(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType18(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType19(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType20(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType21(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType22(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType23(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType24(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType25(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType26(BitBuffer& bits);
    static std::unique_ptr<AISMessage> parseType27(BitBuffer& bits);
};

} // namespace ais

#endif // AIS_MESSAGE_FACTORY_H