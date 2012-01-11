/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QDebug>
#include <QMediaService>

#include "qdeclarativetorch_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmlclass Torch QDeclarativeTorch
    \brief The Torch element provides simple control over torch functionality

    \ingroup multimedia_qml

    This element is part of the \bold{QtMultimedia 5.0} module.

    In many cases the torch hardware is shared with camera flash functionality,
    and might be automatically controlled by the device.  You have control over
    the power level (of course, higher power levels are brighter but reduce
    battery life significantly).

    \qml
    import QtQuick 2.0
    import QtMultimedia 5.0

    Torch {
        power: 75       // 75% of full power
        enabled: true   // On
    }

    \endqml
*/
QDeclarativeTorch::QDeclarativeTorch(QObject *parent)
    : QObject(parent)
{
    m_camera = new QCamera(this);
    QMediaService *service = m_camera->service();

    m_exposure = service ? service->requestControl<QCameraExposureControl*>() : 0;
    m_flash = service ? service->requestControl<QCameraFlashControl*>() : 0;

    if (m_exposure)
        connect(m_exposure, SIGNAL(exposureParameterChanged(int)), SLOT(parameterChanged(int)));

    // XXX There's no signal for flash mode changed
}

QDeclarativeTorch::~QDeclarativeTorch()
{
}

/*!
    \qmlproperty bool Torch::enabled
    \property QDeclarativeTorch::enabled

    Whether the torch is on.  If the torch functionality is shared
    with the camera flash hardware, the camera will take priority
    over torch settings and the torch may be disabled.
*/
/*!
    Returns whether the torch is on.  If the torch functionality
    is shared with the camera flash hardware, the camera will take
    priority and the torch may be disabled.
 */
bool QDeclarativeTorch::enabled() const
{
    if (!m_flash)
        return false;

    return m_flash->flashMode() & QCameraExposure::FlashTorch;
}


/*!
    If \a on is true, attempt to turn on the torch.
    If the torch hardware is already in use by the camera, this will
    be ignored.
*/
void QDeclarativeTorch::setEnabled(bool on)
{
    if (!m_flash)
        return;

    QCameraExposure::FlashModes mode = m_flash->flashMode();

    if (mode & QCameraExposure::FlashTorch) {
        if (!on) {
            m_flash->setFlashMode(mode & ~QCameraExposure::FlashTorch);
            emit enabledChanged();
        }
    } else {
        if (on) {
            m_flash->setFlashMode(mode | QCameraExposure::FlashTorch);
            emit enabledChanged();
        }
    }
}

/*!
    \qmlproperty int Torch::power
    \property QDeclarativeTorch::power

    The current torch power setting, as a percentage of full power.

    In some cases this setting may change automatically as a result
    of temperature or battery conditions.
*/
/*!
    Returns the current torch power settings, as a percentage of full power.
*/
int QDeclarativeTorch::power() const
{
    if (!m_exposure)
        return 0;

    return m_exposure->exposureParameter(QCameraExposureControl::FlashPower).toInt();
}

/*!
    Sets the current torch power setting to \a power, as a percentage of
    full power.
*/
void QDeclarativeTorch::setPower(int power)
{
    if (!m_exposure)
        return;

    power = qBound(0, power, 100);
    if (this->power() != power) {
        m_exposure->setExposureParameter(QCameraExposureControl::FlashPower, power);
        emit powerChanged();
    }
}

/* Check for changes in flash power */
void QDeclarativeTorch::parameterChanged(int parameter)
{
    if (parameter == QCameraExposureControl::FlashPower) {
        emit powerChanged();
    }
}

QT_END_NAMESPACE
