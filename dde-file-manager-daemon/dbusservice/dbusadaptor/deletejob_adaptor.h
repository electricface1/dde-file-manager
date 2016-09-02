/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -i fileoperationjob/deletejob.h -c DeleteJobAdaptor -l DeleteJob -a dbusadaptor/deletejob_adaptor deletejob.xml
 *
 * qdbusxml2cpp is Copyright (C) 2015 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef DELETEJOB_ADAPTOR_H
#define DELETEJOB_ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "fileoperationjob/deletejob.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface com.deepin.filemanager.daemon.DeleteJob
 */
class DeleteJobAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.daemon.DeleteJob")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.filemanager.daemon.DeleteJob\">\n"
"    <method name=\"Execute\"/>\n"
"    <signal name=\"Done\">\n"
"      <arg type=\"s\"/>\n"
"    </signal>\n"
"  </interface>\n"
        "")
public:
    DeleteJobAdaptor(DeleteJob *parent);
    virtual ~DeleteJobAdaptor();

    inline DeleteJob *parent() const
    { return static_cast<DeleteJob *>(QObject::parent()); }

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void Execute();
Q_SIGNALS: // SIGNALS
    void Done(const QString &in0);
};

#endif
