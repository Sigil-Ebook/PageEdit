#pragma once
#ifndef OPFREADER_H
#define OPFREADER_H

#include <QString>
#include <QStringList>
#include <QDir>
#include <QMap>

class QXmlStreamReader;
class QDir;

class OPFReader : QObject
{
    Q_OBJECT

public:

  OPFReader();
  ~OPFReader() { };

  void parseOPF(const QString& opfpath);
  QStringList GetSpineFilePathList();

protected:

  void ReadManifestItemElement(QXmlStreamReader *opf_reader);
  void ReadSpineItemRef(QXmlStreamReader *opf_reader);

private:

  QDir m_opfDir;
  QStringList m_ManifestFilePaths;
  QMap<QString,QString> m_IDMap;
  QMap<QString,QString> m_FileMimeTypes;
  QStringList m_SpineFilePaths;
  QString m_opfpath;
};

#endif
