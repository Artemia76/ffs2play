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
 * crypto.cpp is part of FF2Play project
 *
 * This class provide tools for encryption needs
 * It use AES cipher for xml texte based protocol
 * it provide RSA encryption for AES key exchange
 * **************************************************************************/

#include "tools/crypto.h"

#ifdef Q_OS_WIN32
    #define BYTE CryptoPP::byte
#else
    #define BYTE quint8
#endif

namespace ffs2play
{
    ///
    /// \brief CCrypto::CCrypto
    /// \param parent
    ///
    CCrypto::CCrypto(QObject *parent) : QObject(parent)
    {
        m_log = CLogger::instance();
        m_key.resize(32);
        AutoSeededRandomPool prng;
        prng.GenerateBlock(reinterpret_cast<BYTE*>(m_key.data()),32);
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
    /// \return
    ///
    QString CCrypto::EncryptToBase64(const QString& Data)
    {
        std::string cipher;
        // Generate a random IV
        QByteArray iv (AES::BLOCKSIZE,'\0');
        AutoSeededRandomPool rnd;
        rnd.GenerateBlock(reinterpret_cast<BYTE*>(iv.data()), AES::BLOCKSIZE);
        try
        {
            CFB_Mode<AES>::Encryption cfbEncryption(reinterpret_cast<const BYTE*>(m_key.data()),m_key.size(),reinterpret_cast<const BYTE*>(iv.data()));
            StringSource ss(Data.toStdString(), true,
                        new StreamTransformationFilter ( cfbEncryption,
                            new StringSink(cipher)
                        )
            );
        }
        catch( const CryptoPP::Exception& e)
        {
            m_log->log("CCrypto encode error : " + QString(e.what()));
            return QString("");
        }
        QByteArray Output;
        Output.append(cipher.c_str());
        Output.append(QByteArray("-[--IV-[-"));
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
        QByteArray chaine = QByteArray::fromBase64(Data.toStdString().c_str());
        int pos = chaine.indexOf("-[--IV-[-");
        QByteArray iv = chaine.mid(pos+9);
        std::string ToDecode = chaine.left(pos).toStdString();
        std::string Decoded;
        try
        {
            CFB_Mode<AES>::Decryption cfbDecryption(reinterpret_cast<const BYTE*>(m_key.data()),m_key.size(),reinterpret_cast<const BYTE*>(iv.data()));
            StringSource ss( ToDecode, true,
                        new StreamTransformationFilter( cfbDecryption,
                            new StringSink(Decoded)
                        )
            );
        }
        catch( const CryptoPP::Exception& e)
        {
            m_log->log("CCrypto decode error : " + QString(e.what()));
            return QString("");
        }
        return QString::fromStdString(Decoded);
    }

    ///
    /// \brief CCrypto::RSA_EncryptToBase64
    /// \param Data
    /// \return
    ///
    QString CCrypto::RSA_EncryptToBase64(const QString& Cert, const QByteArray& Data)
    {
        QByteArray encrypt(2048,'\0');
        int Taille =0;
        BIO* bio_mem = BIO_new(BIO_s_mem());
        BIO_puts(bio_mem,QByteArray(Cert.toStdString().c_str()).data());
        EVP_PKEY* pkey = X509_get_pubkey(PEM_read_bio_X509(bio_mem, nullptr, nullptr, nullptr));
        if (pkey != nullptr)
        {
            RSA* rsa = EVP_PKEY_get1_RSA(pkey);
            if (rsa != nullptr)
            {
                Taille = RSA_public_encrypt(Data.size(), reinterpret_cast<const unsigned char*>(Data.data()),  reinterpret_cast<unsigned char*>(encrypt.data()), rsa, RSA_PKCS1_PADDING);
                RSA_free(rsa);
            }
            EVP_PKEY_free(pkey);
        }
        BIO_free(bio_mem);
        encrypt.resize(Taille);
        return QString(encrypt.toBase64());
    }
}
