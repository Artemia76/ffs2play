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
