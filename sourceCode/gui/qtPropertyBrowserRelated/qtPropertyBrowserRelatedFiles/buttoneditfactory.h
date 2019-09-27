/****************************************************************************
**
** Copyright (C) 2006 Trolltech ASA. All rights reserved.
**
** This file is part of the documentation of Qt. It was originally
** published as part of Qt Quarterly.
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation or under the
** terms of the Qt Commercial License Agreement. The respective license
** texts for these are provided with the open source and commercial
** editions of Qt.
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef FILEEDITFACTORY_H
#define FILEEDITFACTORY_H

#include "qtpropertybrowser.h"
#include "buttoneditmanager.h"

class PushButtonEdit;

class PushButtonEditFactory : public QtAbstractEditorFactory<ButtonEditManager>
{
    Q_OBJECT
public:
    PushButtonEditFactory(QObject *parent = 0)
        : QtAbstractEditorFactory<ButtonEditManager>(parent)
            { }
    virtual ~PushButtonEditFactory();
protected:
    virtual void connectPropertyManager(ButtonEditManager *manager);
    virtual QWidget *createEditor(ButtonEditManager *manager, QtProperty *property,
                QWidget *parent);
    virtual void disconnectPropertyManager(ButtonEditManager *manager);
private slots:
    void slotPropertyChanged(QtProperty *property, const QString &value);
    void slotSetValue();
    void slotEditorDestroyed(QObject *object);
private:
    QMap<QtProperty *, QList<PushButtonEdit *> > theCreatedEditors;
    QMap<PushButtonEdit *, QtProperty *> theEditorToProperty;
};

#endif
