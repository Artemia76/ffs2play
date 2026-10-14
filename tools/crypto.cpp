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

/****************************************************************************
** crypto.cpp is part of FF2Play project
**
** This class provide tools for encryption needs
** It use AES cipher for xml texte based protocol
** it provide RSA encryption for AES key exchange
****************************************************************************/

#include "tools/crypto.h"

#include <memory>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

namespace
{
    const int        AES_KEY_SIZE = 32;     // AES-256
    const int        AES_IV_SIZE = 16;      // AES block size
    const QByteArray IV_SEPARATOR("-[--IV-[-");

    struct EvpCipherCtxDeleter { void operator()(EVP_CIPHER_CTX* p) const { EVP_CIPHER_CTX_free(p); } };
    struct EvpPkeyCtxDeleter { void operator()(EVP_PKEY_CTX* p) const { EVP_PKEY_CTX_free(p); } };
    struct BioDeleter { void operator()(BIO* p) const { BIO_free(p); } };
    struct X509Deleter { void operator()(X509* p) const { X509_free(p); } };

    using EvpCipherCtxPtr = std::unique_ptr<EVP_CIPHER_CTX, EvpCipherCtxDeleter>;
    using EvpPkeyCtxPtr = std::unique_ptr<EVP_PKEY_CTX, EvpPkeyCtxDeleter>;
    using BioPtr = std::unique_ptr<BIO, BioDeleter>;
    using X509Ptr = std::unique_ptr<X509, X509Deleter>;

    ///
    /// \brief aes256Cfb run AES-256-CFB128 (same mode as Crypto++ CFB_Mode<AES>)
    /// \param pEncrypt true to encrypt, false to decrypt
    /// \param pOk set to false on OpenSSL failure
    ///
    QByteArray aes256Cfb(bool pEncrypt, const QByteArray& pKey, const QByteArray& pIV, const QByteArray& pInput, bool& pOk)
    {
        pOk = false;
        EvpCipherCtxPtr ctx(EVP_CIPHER_CTX_new());
        if (!ctx) return QByteArray();
        if (EVP_CipherInit_ex(ctx.get(), EVP_aes_256_cfb128(), nullptr,
                              reinterpret_cast<const unsigned char*>(pKey.constData()),
                              reinterpret_cast<const unsigned char*>(pIV.constData()),
                              pEncrypt ? 1 : 0) != 1)
            return QByteArray();
        // CFB is a stream mode : output size equals input size
        QByteArray output(pInput.size() + AES_IV_SIZE, Qt::Uninitialized);
        int len = 0;
        int total = 0;
        if (EVP_CipherUpdate(ctx.get(),
                             reinterpret_cast<unsigned char*>(output.data()), &len,
                             reinterpret_cast<const unsigned char*>(pInput.constData()),
                             static_cast<int>(pInput.size())) != 1)
            return QByteArray();
        total = len;
        if (EVP_CipherFinal_ex(ctx.get(), reinterpret_cast<unsigned char*>(output.data()) + total, &len) != 1)
            return QByteArray();
        total += len;
        output.resize(total);
        pOk = true;
        return output;
    }
}

///
/// \brief CCrypto::CCrypto
/// \param parent
///
CCrypto::CCrypto(QObject *parent) : QObject(parent)
{
    m_log = CLogger::instance();
    m_key.resize(AES_KEY_SIZE);
    if (RAND_bytes(reinterpret_cast<unsigned char*>(m_key.data()), AES_KEY_SIZE) != 1)
    {
        m_log->log("CCrypto : unable to generate AES key");
    }
}

///
/// \brief CCrypto::EncryptionKey
/// \return
///
const QByteArray& CCrypto::EncryptionKey()
{
    return m_key;
}

///
/// \brief CCrypto::EncryptionKeyString
/// \return
///
QString CCrypto::EncryptionKeyString()
{
    return QString(m_key.toBase64());
}

///
/// \brief CCrypto::EncryptToBase64
/// \param Data
/// \return base64 of cipher + IV_SEPARATOR + IV
///
QString CCrypto::EncryptToBase64(const QString& Data)
{
    // Generate a random IV
    QByteArray iv(AES_IV_SIZE, '\0');
    if (RAND_bytes(reinterpret_cast<unsigned char*>(iv.data()), AES_IV_SIZE) != 1)
    {
        m_log->log("CCrypto encode error : unable to generate IV");
        return QString("");
    }
    bool ok;
    QByteArray Output = aes256Cfb(true, m_key, iv, Data.toUtf8(), ok);
    if (!ok)
    {
        m_log->log("CCrypto encode error : AES encryption failed");
        return QString("");
    }
    Output.append(IV_SEPARATOR);
    Output.append(iv);
    return QString(Output.toBase64());
}

///
/// \brief CCrypto::DecryptFromBase64
/// \param Data
/// \return
///
QString CCrypto::DecryptFromBase64(const QString& Data)
{
    // Layout is cipher + IV_SEPARATOR + IV (fixed size) : parse from the end,
    // cipher bytes may contain anything
    QByteArray chaine = QByteArray::fromBase64(Data.toLatin1());
    const qsizetype pos = chaine.size() - AES_IV_SIZE - IV_SEPARATOR.size();
    if ((pos < 0) || (QByteArrayView(chaine).sliced(pos, IV_SEPARATOR.size()) != IV_SEPARATOR))
    {
        m_log->log("CCrypto decode error : IV not found");
        return QString("");
    }
    QByteArray iv = chaine.right(AES_IV_SIZE);
    bool ok;
    QByteArray Decoded = aes256Cfb(false, m_key, iv, chaine.left(pos), ok);
    if (!ok)
    {
        m_log->log("CCrypto decode error : AES decryption failed");
        return QString("");
    }
    return QString::fromUtf8(Decoded);
}

///
/// \brief CCrypto::RSA_EncryptToBase64
/// \param Cert PEM X509 certificate holding the RSA public key
/// \param Data
/// \return base64 of RSA PKCS#1 v1.5 encrypted Data
///
QString CCrypto::RSA_EncryptToBase64(const QString& Cert, const QByteArray& Data)
{
    const QByteArray pem = Cert.toLatin1();
    BioPtr bio(BIO_new_mem_buf(pem.constData(), static_cast<int>(pem.size())));
    if (!bio) return QString();
    X509Ptr x509(PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr));
    if (!x509)
    {
        m_log->log("CCrypto RSA error : invalid certificate");
        return QString();
    }
    EVP_PKEY* pkey = X509_get0_pubkey(x509.get());
    if (pkey == nullptr) return QString();
    EvpPkeyCtxPtr ctx(EVP_PKEY_CTX_new_from_pkey(nullptr, pkey, nullptr));
    if (!ctx
        || (EVP_PKEY_encrypt_init(ctx.get()) <= 0)
        || (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_PADDING) <= 0))
    {
        m_log->log("CCrypto RSA error : unable to init encryption");
        return QString();
    }
    size_t outLen = 0;
    const unsigned char* in = reinterpret_cast<const unsigned char*>(Data.constData());
    if (EVP_PKEY_encrypt(ctx.get(), nullptr, &outLen, in, static_cast<size_t>(Data.size())) <= 0)
        return QString();
    QByteArray encrypt(static_cast<qsizetype>(outLen), Qt::Uninitialized);
    if (EVP_PKEY_encrypt(ctx.get(), reinterpret_cast<unsigned char*>(encrypt.data()), &outLen,
                         in, static_cast<size_t>(Data.size())) <= 0)
    {
        m_log->log("CCrypto RSA error : encryption failed");
        return QString();
    }
    encrypt.resize(static_cast<qsizetype>(outLen));
    return QString(encrypt.toBase64());
}
