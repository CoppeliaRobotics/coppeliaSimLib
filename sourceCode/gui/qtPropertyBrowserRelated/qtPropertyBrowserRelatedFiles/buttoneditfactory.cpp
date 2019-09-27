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

#include "buttoneditfactory.h"
#include "pushbuttonedit.h"

PushButtonEditFactory::~PushButtonEditFactory()
{
    QList<PushButtonEdit *> editors = theEditorToProperty.keys();
    QListIterator<PushButtonEdit *> it(editors);
    while (it.hasNext())
        delete it.next();
}

void PushButtonEditFactory::connectPropertyManager(ButtonEditManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty *, const QString &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QString &)));

}

QWidget *PushButtonEditFactory::createEditor(ButtonEditManager *manager,
        QtProperty *property, QWidget *parent)
{
    PushButtonEdit *editor = new PushButtonEdit(parent);

    editor->setButtonName(manager->value(property));

    theCreatedEditors[property].append(editor);
    theEditorToProperty[editor] = property;

    connect(editor, SIGNAL(buttonClicked()),
                this, SLOT(slotSetValue()));
    connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));
    return editor;
}

void PushButtonEditFactory::disconnectPropertyManager(ButtonEditManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty *, const QString &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QString &)));
    disconnect(manager, SIGNAL(filterChanged(QtProperty *, const QString &)),
                this, SLOT(slotFilterChanged(QtProperty *, const QString &)));
}

void PushButtonEditFactory::slotPropertyChanged(QtProperty *property,
                const QString &value)
{
    if (!theCreatedEditors.contains(property))
        return;

    QList<PushButtonEdit *> editors = theCreatedEditors[property];
    QListIterator<PushButtonEdit *> itEditor(editors);
    while (itEditor.hasNext())
        itEditor.next()->setButtonName(value);
}


void PushButtonEditFactory::slotSetValue()
{
    QObject *object = sender();
    QMap<PushButtonEdit *, QtProperty *>::ConstIterator itEditor =
                theEditorToProperty.constBegin();
    while (itEditor != theEditorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            ButtonEditManager *manager = propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, manager->value(property));
            return;
        }
        itEditor++;
    }
}

void PushButtonEditFactory::slotEditorDestroyed(QObject *object)
{
    QMap<PushButtonEdit *, QtProperty *>::ConstIterator itEditor =
                theEditorToProperty.constBegin();
    while (itEditor != theEditorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            PushButtonEdit *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            theEditorToProperty.remove(editor);
            theCreatedEditors[property].removeAll(editor);
            if (theCreatedEditors[property].isEmpty())
                theCreatedEditors.remove(property);
            return;
        }
        itEditor++;
    }
}

