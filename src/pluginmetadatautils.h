/*
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2022 Stefano Crocco <stefano.crocco@alice.it>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PLUGINMETADATAUTILS_H
#define PLUGINMETADATAUTILS_H
#endif //PLUGINMETADATAUTILS_H

#include <KPluginMetaData>

#include <QDebug>

using PluginMetaDataVector = QVector<KPluginMetaData>;

KPluginMetaData findPartById(const QString &id);

QDebug operator<<(QDebug debug, const KPluginMetaData &md);

QDebug operator<<(QDebug debug, const PluginMetaDataVector &vec);
