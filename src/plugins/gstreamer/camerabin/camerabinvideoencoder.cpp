/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "camerabinvideoencoder.h"
#include "camerabinsession.h"
#include "camerabincontainer.h"

#include <QtCore/qdebug.h>

CameraBinVideoEncoder::CameraBinVideoEncoder(CameraBinSession *session)
    :QVideoEncoderControl(session),
     m_session(session),
     m_codecs(QGstCodecsInfo::VideoEncoder)
{
}

CameraBinVideoEncoder::~CameraBinVideoEncoder()
{
}

QList<QSize> CameraBinVideoEncoder::supportedResolutions(const QVideoEncoderSettings &settings, bool *continuous) const
{
    if (continuous)
        *continuous = false;

    QPair<int,int> rate = rateAsRational(settings.frameRate());

    //select the closest supported rational rate to settings.frameRate()

    return m_session->supportedResolutions(rate, continuous, QCamera::CaptureVideo);
}

QList< qreal > CameraBinVideoEncoder::supportedFrameRates(const QVideoEncoderSettings &settings, bool *continuous) const
{
    if (continuous)
        *continuous = false;

    QList< qreal > res;
    QPair<int,int> rate;

    foreach(rate, m_session->supportedFrameRates(settings.resolution(), continuous)) {
        if (rate.second > 0)
            res << qreal(rate.first)/rate.second;
    }

    return res;
}

QStringList CameraBinVideoEncoder::supportedVideoCodecs() const
{
    return m_codecs.supportedCodecs();
}

QString CameraBinVideoEncoder::videoCodecDescription(const QString &codecName) const
{
    return m_codecs.codecDescription(codecName);
}

QVideoEncoderSettings CameraBinVideoEncoder::videoSettings() const
{
    return m_videoSettings;
}

void CameraBinVideoEncoder::setVideoSettings(const QVideoEncoderSettings &settings)
{
    m_videoSettings = settings;
    m_userSettings = settings;
    emit settingsChanged();
}

void CameraBinVideoEncoder::setActualVideoSettings(const QVideoEncoderSettings &settings)
{
    m_videoSettings = settings;
}

void CameraBinVideoEncoder::resetActualSettings()
{
    m_videoSettings = m_userSettings;
}


QPair<int,int> CameraBinVideoEncoder::rateAsRational(qreal frameRate) const
{
    if (frameRate > 0.001) {
        //convert to rational number
        QList<int> denumCandidates;
        denumCandidates << 1 << 2 << 3 << 5 << 10 << 25 << 30 << 50 << 100 << 1001 << 1000;

        qreal error = 1.0;
        int num = 1;
        int denum = 1;

        foreach (int curDenum, denumCandidates) {
            int curNum = qRound(frameRate*curDenum);
            qreal curError = qAbs(qreal(curNum)/curDenum - frameRate);

            if (curError < error) {
                error = curError;
                num = curNum;
                denum = curDenum;
            }

            if (curError < 1e-8)
                break;
        }

        return QPair<int,int>(num,denum);
    }

    return QPair<int,int>();
}

GstEncodingProfile *CameraBinVideoEncoder::createProfile()
{
    QString codec = m_videoSettings.codec();
    GstCaps *caps;

    if (codec.isEmpty())
        caps = gst_caps_new_any();
    else
        caps = gst_caps_from_string(codec.toLatin1());

    return (GstEncodingProfile *)gst_encoding_video_profile_new(
                caps,
                NULL, //preset
                NULL, //restriction
                0); //presence
}
