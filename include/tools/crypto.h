#ifndef CRYPTO_H
#define CRYPTO_H

#include <QObject>
#include <singleton/clogger.h>
#include <filters.h>
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::StreamTransformationFilter;

#include <aes.h>
using CryptoPP::AES;

#include <modes.h>
using CryptoPP::CBC_Mode;

#include <osrng.h>
using CryptoPP::AutoSeededRandomPool;


#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

class CCrypto : public QObject
{
    QByteArray  m_key;
    CLogger*    Log;
    X509*       cert;
    EVP_PKEY*   pkey;
    RSA* rsa;
    Q_OBJECT
    void GenerateRandomIV(QByteArray& IV);
public:
    explicit CCrypto(QObject *parent = 0);
    const QByteArray& EncryptionKey();
    QString EncryptionKeyString();
    QString EncryptToBase64(const QString& Data);
    QString DecryptFromBase64(const QString& Data);
    void    LoadCertificate(const QString& Cert);
    QString RSA_EncryptToBase64(const QByteArray& Data);
signals:

public slots:
};

#endif // CRYPTO_H
