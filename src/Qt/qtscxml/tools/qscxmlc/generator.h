/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtScxml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef GENERATOR_H
#define GENERATOR_H

#include "moc.h"
#include <QtCore/qhash.h>
#include <QtCore/qvector.h>
#include <QtCore/qiodevice.h>

QT_BEGIN_NAMESPACE

class Generator
{
    QIODevice &out;
    ClassDef *cdef;
    QVector<uint> meta_data;
public:
    Generator(ClassDef *classDef, const QList<QByteArray> &metaTypes, const QHash<QByteArray,
              QByteArray> &knownQObjectClasses, const QHash<QByteArray, QByteArray> &knownGadgets,
              QIODevice &outfile);
    void generateCode();
private:
    bool registerableMetaType(const QByteArray &propertyType);
    void registerClassInfoStrings();
    void generateClassInfos();
    void registerFunctionStrings(const QList<FunctionDef> &list);
    void generateFunctions(const QList<FunctionDef> &list, const char *functype, int type, int &paramsIndex);
    void generateFunctionRevisions(const QList<FunctionDef>& list, const char *functype);
    void generateFunctionParameters(const QList<FunctionDef> &list, const char *functype);
    void generateTypeInfo(const QByteArray &typeName, bool allowEmptyName = false);
    void registerEnumStrings();
    void generateEnums(int index);
    void registerPropertyStrings();
    void generateProperties();
    void generateMetacall();
    void generateStaticMetacall();
    void generateSignal(FunctionDef *def, int index);
//    void generatePluginMetaData();
    QMultiMap<QByteArray, int> automaticPropertyMetaTypesHelper();
    QMap<int, QMultiMap<QByteArray, int> > methodsWithAutomaticTypesHelper(const QList<FunctionDef> &methodList);

    void strreg(const QByteArray &); // registers a string
    int stridx(const QByteArray &); // returns a string's id
    QList<QByteArray> strings;
    QByteArray purestSuperClass;
    QList<QByteArray> metaTypes;
    QHash<QByteArray, QByteArray> knownQObjectClasses;
    QHash<QByteArray, QByteArray> knownGadgets;
};

QT_END_NAMESPACE

#endif // GENERATOR_H
