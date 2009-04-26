#ifndef _ROMALYZER_H_
#define _ROMALYZER_H_

#include <QXmlDefaultHandler>
#include <QTreeWidgetItem>
#include <QByteArray>
#include <QStringList>
#include <QTimer>
#include <QTime>
#include "ui_romalyzer.h"

#define QMC2_ROMALYZER_COLUMN_GAME		0
#define QMC2_ROMALYZER_COLUMN_MERGE		1
#define QMC2_ROMALYZER_COLUMN_TYPE		2
#define QMC2_ROMALYZER_COLUMN_EMUSTATUS		3
#define QMC2_ROMALYZER_COLUMN_FILESTATUS	4
#define QMC2_ROMALYZER_COLUMN_SIZE		5
#define QMC2_ROMALYZER_COLUMN_CRC		6
#define QMC2_ROMALYZER_COLUMN_SHA1		7
#define QMC2_ROMALYZER_COLUMN_MD5		8

#define QMC2_ROMALYZER_EMUSTATUS_GOOD		0x00000001
#define QMC2_ROMALYZER_EMUSTATUS_NODUMP		0x00000020
#define QMC2_ROMALYZER_EMUSTATUS_BADDUMP	0x00000400
#define QMC2_ROMALYZER_EMUSTATUS_UNKNOWN	0x00008000

#define QMC2_ROMALYZER_PAUSE_TIMEOUT		250
#define QMC2_ROMALYZER_FLASH_TIME		100

#define QMC2_ROMALYZER_FILE_TOO_BIG		"QMC2_FILE_TOO_BIG"
#define QMC2_ROMALYZER_FILE_ERROR		"QMC2_FILE_ERROR"
#define QMC2_ROMALYZER_FILE_NOT_SUPPORTED	"QMC2_FILE_NOT_SUPPORTED"
#define QMC2_ROMALYZER_FILE_NOT_FOUND		"QMC2_FILE_NOT_FOUND"

#define QMC2_ROMALYZER_ZIP_BUFFER_SIZE		QMC2_ZIP_BUFFER_SIZE
#define QMC2_ROMALYZER_FILE_BUFFER_SIZE		QMC2_FILE_BUFFER_SIZE
#define QMC2_ROMALYZER_PROGRESS_THRESHOLD	QMC2_ONE_MEGABYTE

#define QMC2_CHD_HEADER_TAG_OFFSET		0
#define QMC2_CHD_HEADER_TAG_LENGTH		8
#define QMC2_CHD_HEADER_VERSION_OFFSET		12
#define QMC2_CHD_HEADER_FLAGS_OFFSET		16
#define QMC2_CHD_HEADER_FLAG_HASPARENT		0x00000001
#define QMC2_CHD_HEADER_FLAG_ALLOWSWRITES	0x00000002
#define QMC2_CHD_HEADER_COMPRESSION_OFFSET	20
#define QMC2_CHD_HEADER_COMPRESSION_NONE	0
#define QMC2_CHD_HEADER_COMPRESSION_ZLIB	1
#define QMC2_CHD_HEADER_COMPRESSION_ZLIB_PLUS	2
#define QMC2_CHD_HEADER_COMPRESSION_AV		3
#define QMC2_CHD_HEADER_V3_TOTALHUNKS_OFFSET	24
#define QMC2_CHD_HEADER_V3_LOGICALBYTES_OFFSET	28
#define QMC2_CHD_HEADER_V3_MD5_OFFSET		44
#define QMC2_CHD_HEADER_V3_MD5_LENGTH		16
#define QMC2_CHD_HEADER_V3_PARENTMD5_OFFSET	60
#define QMC2_CHD_HEADER_V3_PARENTMD5_LENGTH	16
#define QMC2_CHD_HEADER_V3_HUNKBYTES_OFFSET	76
#define QMC2_CHD_HEADER_V3_SHA1_OFFSET		80
#define QMC2_CHD_HEADER_V3_SHA1_LENGTH		20
#define QMC2_CHD_HEADER_V3_PARENTSHA1_OFFSET	100
#define QMC2_CHD_HEADER_V3_PARENTSHA1_LENGTH	20
#define QMC2_CHD_HEADER_V3_LENGTH		120
#define QMC2_CHD_HEADER_V4_TOTALHUNKS_OFFSET	24
#define QMC2_CHD_HEADER_V4_LOGICALBYTES_OFFSET	28
#define QMC2_CHD_HEADER_V4_HUNKBYTES_OFFSET	44
#define QMC2_CHD_HEADER_V4_SHA1_OFFSET		48
#define QMC2_CHD_HEADER_V4_SHA1_LENGTH		20
#define QMC2_CHD_HEADER_V4_PARENTSHA1_OFFSET	68
#define QMC2_CHD_HEADER_V4_PARENTSHA1_LENGTH	20
#define QMC2_CHD_HEADER_V4_RAWSHA1_OFFSET	88
#define QMC2_CHD_HEADER_V4_RAWSHA1_LENGTH	20
#define QMC2_CHD_HEADER_V4_LENGTH		108

class ROMAlyzerXmlHandler : public QXmlDefaultHandler
{
  public:
    QString currentText;
    QTreeWidgetItem *parentItem;
    QTreeWidgetItem *childItem;
    QList<QTreeWidgetItem *> childItems;
    bool autoExpand;
    bool autoScroll;
    int emuStatus;
    int fileCounter;
    QBrush redBrush;
    QBrush greenBrush;
    QBrush blueBrush;
    QBrush yellowBrush;
    QBrush brownBrush;
    QBrush greyBrush;

    ROMAlyzerXmlHandler(QTreeWidgetItem *, bool expand = FALSE, bool scroll = FALSE);
    ~ROMAlyzerXmlHandler();

    bool startElement(const QString &, const QString &, const QString &, const QXmlAttributes &);
    bool endElement(const QString &, const QString &, const QString &);
    bool characters(const QString &);
};

class ROMAlyzer : public QDialog, public Ui::ROMAlyzer
{
  Q_OBJECT

  public:
    QTimer animTimer;
    QTime miscTimer;
    int animSeq;
    QStringList romPaths;
    QStringList chdCompressionTypes;

    ROMAlyzer(QWidget *);
    ~ROMAlyzer();

    void saveState() { closeEvent(NULL); }
    void log(QString);
    QString humanReadable(quint64);
    QString &getXmlData(QString);
    QString &getEffectiveFile(QTreeWidgetItem *item, QString, QString, QString, QString, QString, QString,
                              QByteArray *, QString *, QString *, bool *, bool *, int);

  public slots:
    // callback functions
    void on_pushButtonAnalyze_clicked();
    void on_pushButtonPause_clicked();
    void on_pushButtonClose_clicked();
    void on_pushButtonSearchForward_clicked();
    void on_pushButtonSearchBackward_clicked();
    void on_lineEditGames_textChanged(QString);
    void on_treeWidgetChecksums_itemSelectionChanged();
    void on_spinBoxMaxLogSize_valueChanged(int);

    // miscellaneous slots
    void animationTimeout();
    void analyze();
    void selectItem(QString);
    void enableSearchEdit() { lineEditSearchString->setEnabled(TRUE); }

  protected:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);
};

#endif
