/****************************************************************************
**
** Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qgeoroutereplyosm.h"
#include "qgeoroutingmanagerengineosm.h"

QT_BEGIN_NAMESPACE

QGeoRouteReplyOsm::QGeoRouteReplyOsm(QNetworkReply *reply, const QGeoRouteRequest &request,
                                     QObject *parent)
:   QGeoRouteReply(request, parent), m_reply(reply)
{
    connect(m_reply, SIGNAL(finished()), this, SLOT(networkReplyFinished()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(networkReplyError(QNetworkReply::NetworkError)));
}

QGeoRouteReplyOsm::~QGeoRouteReplyOsm()
{
    if (m_reply)
        m_reply->deleteLater();
}

void QGeoRouteReplyOsm::abort()
{
    if (!m_reply)
        return;

    m_reply->abort();

    m_reply->deleteLater();
    m_reply = 0;
}

void QGeoRouteReplyOsm::networkReplyFinished()
{
    if (!m_reply)
        return;

    if (m_reply->error() != QNetworkReply::NoError) {
        setError(QGeoRouteReply::CommunicationError, m_reply->errorString());
        m_reply->deleteLater();
        m_reply = 0;
        return;
    }

    QGeoRoutingManagerEngineOsm *engine = qobject_cast<QGeoRoutingManagerEngineOsm *>(parent());
    const QGeoRouteParser *parser = engine->routeParser();

    QList<QGeoRoute> routes;
    QString errorString;
    QGeoRouteReply::Error error = parser->parseReply(routes, errorString, m_reply->readAll());

    if (error == QGeoRouteReply::NoError) {
        setRoutes(routes.mid(0,1)); // TODO QTBUG-56426
        // setError(QGeoRouteReply::NoError, status);  // can't do this, or NoError is emitted and does damages
        setFinished(true);
    } else {
        setError(error, errorString);
    }

    m_reply->deleteLater();
    m_reply = 0;
}

void QGeoRouteReplyOsm::networkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)

    if (!m_reply)
        return;

    setError(QGeoRouteReply::CommunicationError, m_reply->errorString());

    m_reply->deleteLater();
    m_reply = 0;
}

QT_END_NAMESPACE
