/****************************************************************************
**
** Copyright (C) 2017 FSFranceSimulateur team.
** Contact: https://github.com/ffs2/ffs2play
**
** FFS2Play is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** FFS2Play is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** The license is as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this software. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
****************************************************************************/

#ifndef CRYPTO_H
#define CRYPTO_H

#include <QObject>
#include <singleton/clogger.h>
#include <filters.h>

using CryptoPP::StringSink;
using CryptoPP::ArraySink;
using CryptoPP::StringSource;
using CryptoPP::StreamTransformationFilter;

#include <aes.h>
using CryptoPP::AES;

#include <modes.h>
using CryptoPP::CFB_Mode;

#include <osrng.h>
using CryptoPP::AutoSeededRandomPool;

#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

class CCrypto : public QObject
{
    QByteArray  m_key;
    CLogger*    Log;
    Q_OBJECT
public:
    explicit CCrypto(QObject *parent = 0);
    const QByteArray& EncryptionKey();
    QString EncryptionKeyString();
    QString EncryptToBase64(const QString& Data);
    QString DecryptFromBase64(const QString& Data);
    QString RSA_EncryptToBase64(const QString& Cert, const QByteArray& Data);
signals:

public slots:
};

#endif // CRYPTO_H
