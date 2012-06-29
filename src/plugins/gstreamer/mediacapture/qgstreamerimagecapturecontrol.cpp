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

#include "qgstreamerimagecapturecontrol.h"
#include <QtCore/QDebug>
#include <QtCore/QDir>

QGstreamerImageCaptureControl::QGstreamerImageCaptureControl(QGstreamerCaptureSession *session)
    :QCameraImageCaptureControl(session), m_session(session), m_ready(false), m_lastId(0)
{
    connect(m_session, SIGNAL(stateChanged(QGstreamerCaptureSession::State)), SLOT(updateState()));
    connect(m_session, SIGNAL(imageExposed(int)), this, SIGNAL(imageExposed(int)));
    connect(m_session, SIGNAL(imageCaptured(int,QImage)), this, SIGNAL(imageCaptured(int,QImage)));
    connect(m_session, SIGNAL(imageSaved(int,QString)), this, SIGNAL(imageSaved(int,QString)));
}

QGstreamerImageCaptureControl::~QGstreamerImageCaptureControl()
{
}

bool QGstreamerImageCaptureControl::isReadyForCapture() const
{
    return m_ready;
}

int QGstreamerImageCaptureControl::capture(const QString &fileName)
{
    m_lastId++;

    //it's allowed to request image capture while camera is starting
    if (m_session->pendingState() == QGstreamerCaptureSession::StoppedState ||
            !(m_session->captureMode() & QGstreamerCaptureSession::Image)) {
        //emit error in the next event loop,
        //so application can associate it with returned request id.
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                  Q_ARG(int, m_lastId),
                                  Q_ARG(int, QCameraImageCapture::NotReadyError),
                                  Q_ARG(QString,tr("Not ready to capture")));

        return m_lastId;
    }

    QString path = fileName;
    if (path.isEmpty()) {
        int lastImage = 0;
        QDir outputDir = QDir::currentPath();
        foreach(QString fileName, outputDir.entryList(QStringList() << "img_*.jpg")) {
            int imgNumber = fileName.mid(4, fileName.size()-8).toInt();
            lastImage = qMax(lastImage, imgNumber);
        }

        path = QString("img_%1.jpg").arg(lastImage+1,
                                         4, //fieldWidth
                                         10,
                                         QLatin1Char('0'));
    }

    m_session->captureImage(m_lastId, path);

    return m_lastId;
}

void QGstreamerImageCaptureControl::cancelCapture()
{

}

void QGstreamerImageCaptureControl::updateState()
{
    bool ready = (m_session->state() == QGstreamerCaptureSession::PreviewState) &&
            (m_session->captureMode() & QGstreamerCaptureSession::Image);

    if (m_ready != ready) {
        emit readyForCaptureChanged(m_ready = ready);
    }
}
