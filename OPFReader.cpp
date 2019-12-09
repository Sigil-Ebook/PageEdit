#include <QString>
#include <QStringList>
#include <QMap>
#include <QDebug>
#include <QXmlStreamReader>
#include <QTextStream>


#include "Utility.h"
#include "OPFReader.h"

#define DBG if(0)

const QStringList IMAGE_MIMETYPES = QStringList() << "image/gif" << "image/jpeg" 
						  << "image/png" << "image/webp" 
						  << "image/tiff" << "image/bmp";

const QStringList SVG_MIMETYPES   = QStringList() << "image/svg+xml";

const QStringList AUDIO_MIMETYPES = QStringList() << "audio/mpeg" << "audio/mp3" 
						  << "audio/ogg" << "audio/mp4";

const QStringList VIDEO_MIMETYPES = QStringList() << "video/mp4" << "video/ogg" 
						  << "video/webm"; 

OPFReader::OPFReader()
  : 
  m_opfDir(QDir()),
  m_ManifestFilePaths(QStringList()),
  m_IDMap(QMap<QString,QString>()),
  m_FileMimeTypes(QMap<QString,QString>()),
  m_SpineFilePaths(QStringList()),
  m_opfpath(QString())
{
}

void OPFReader::parseOPF(const QString& opfpath)
{
    QFileInfo fi(opfpath);
    m_opfpath = fi.absoluteFilePath();
    m_opfpath = Utility::resolveRelativeSegmentsInFilePath(m_opfpath, "/");
    m_opfDir = QFileInfo(m_opfpath).dir();
    QString opf_text = Utility::ReadUnicodeTextFile(opfpath);
    QXmlStreamReader opf_reader(opf_text);
    while (!opf_reader.atEnd()) {
        opf_reader.readNext();
        if (!opf_reader.isStartElement()) {
            continue;
        }
        else if (opf_reader.name() == "item") {
            ReadManifestItemElement(&opf_reader);
        }
        else if (opf_reader.name() == "itemref") {
            ReadSpineItemRef(&opf_reader);
        }
    }
    if (opf_reader.hasError()) {
        const QString error = QString(QObject::tr("Unable to read OPF file.\nLine: %1 Column %2 - %3"))
          .arg(opf_reader.lineNumber())
          .arg(opf_reader.columnNumber())
          .arg(opf_reader.errorString());
        qDebug() << error;
    }
}

void OPFReader::ReadManifestItemElement(QXmlStreamReader *opf_reader)
{
    QString id   = opf_reader->attributes().value("", "id").toString();
    QString href = opf_reader->attributes().value("", "href").toString();
    QString type = opf_reader->attributes().value("", "media-type").toString();
    href = Utility::URLDecodePath(href);
    QString file_path = m_opfDir.absolutePath() + "/" + href;
    DBG qDebug() << "file path as built from opf info: " << file_path;
    file_path = Utility::resolveRelativeSegmentsInFilePath(file_path, "/");
    DBG qDebug() << "file path after resolving relative segments: " << file_path;
    if (!m_ManifestFilePaths.contains(file_path)) {
        m_IDMap[ id ] = file_path;
        m_FileMimeTypes[ id ] = type;
        m_ManifestFilePaths << file_path;
    }
}

void OPFReader::ReadSpineItemRef(QXmlStreamReader *opf_reader)
{
    QString idref = opf_reader->attributes().value("", "idref").toString();
    if (m_IDMap.contains(idref)) {
        if (m_FileMimeTypes[idref] == "application/xhtml+xml") {
            m_SpineFilePaths << m_IDMap[idref];
        }
    } else {
        qDebug() << "Warning idref is missing from Manifest" << idref;
    }
}

QStringList OPFReader::GetSpineFilePathList()
{
    return m_SpineFilePaths;
}

QStringList OPFReader::GetAudioFilePathList()
{
    QStringList res;
    foreach(QString id, m_IDMap.keys()) {
        QString mtype = m_FileMimeTypes.value(id, "");
        if (AUDIO_MIMETYPES.contains(mtype)) {
            res << m_IDMap[id];
	}
    }
    return res;
}

QStringList OPFReader::GetVideoFilePathList()
{
    QStringList res;
    foreach(QString id, m_IDMap.keys()) {
        QString mtype = m_FileMimeTypes.value(id, "");
        if (VIDEO_MIMETYPES.contains(mtype)) {
            res << m_IDMap[id];
	}
    }
    return res;
}

QStringList OPFReader::GetImageFilePathList()
{
    QStringList res;
    foreach(QString id, m_IDMap.keys()) {
        QString mtype = m_FileMimeTypes.value(id, "");
        if (IMAGE_MIMETYPES.contains(mtype)) {
            res << m_IDMap[id];
	}
    }
    return res;
}

QStringList OPFReader::GetSVGFilePathList()
{
    QStringList res;
    foreach(QString id, m_IDMap.keys()) {
        QString mtype = m_FileMimeTypes.value(id, "");
        if (SVG_MIMETYPES.contains(mtype)) {
            res << m_IDMap[id];
	}
    }
    return res;
}
