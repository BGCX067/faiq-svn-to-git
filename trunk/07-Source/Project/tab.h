/**************************************************************************//**
 * NAME:    tab.h
 *
 * PURPOSE: Class definition for tab widget.
 *
 * DATE:    October 24, 2009
*******************************************************************************/
#ifndef TAB_H
#define TAB_H

#include <QDir>
#include <QFileInfoList>
#include <QMessageBox>
#include <QPersistentModelIndex>
#include <QProgressDialog>
#include <QQueue>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QThreadPool>
#include <QTime>
#include <QWidget>

#include "atomicqueue.h"
#include "types.h"

#include "ui_tab.h"

const bool SM_REFRESH = true;

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QEvent;
class QModelIndex;
class QStatusBar;
class QTextEdit;
class QTreeView;
QT_END_NAMESPACE

struct Line;

class FindRun;
class GrepRun;
class Highlighter;
class MainWindow;
class Parms;
class Settings;
class SortFilterProxyModel;
class TabManager;

namespace Ui {
   class Settings;
	class Tab;
}

class Tab : public QWidget {
Q_OBJECT
public:
   Tab(TabManager *tm, QWidget *parent = nullptr);
	virtual ~Tab();
   void autoGrepFirstFile();
   bool busy() const;
   QString contextGetPath();
   void createActions();
   QAbstractItemModel *createFilesModel(QObject *parent);
   void cursorToTop();
   void filter(const Line &line);
   void findInit(bool refresh = false);
   void findKill();
   void grepKill();
   void findProgressShow(bool);
   inline bool findProgressVisible() const { return ui->findProgressBar->isVisible(); }
   inline bool getAbortMatcher() const { return m_abortMatcher; }
   inline int getActiveThreadCount() const { return pool.activeThreadCount(); }
   inline AtomicQueue<WordCount *> *getAtomicQueue() { return &m_aqueue; }
   QString getFilepath(const QModelIndex &index);
   QString getFilepath(QPersistentModelIndex *index);
   inline bool getGrepCancel() const { return m_grepCancel; }
   inline MainWindow * const getMw() const { return mw; }
   inline QStandardItemModel *getModelPtr() const { return model; }
   inline Parms * const getParms() const { return p; }
   inline int getRowCount() const { return model->rowCount(); }
   inline bool getFindState() const { return m_findState; }
   inline Settings *getSettings() const { return settings; }
   QString getTabLabel();
   inline Ui::Tab *getUi() const { return ui; }
   void grepInit(const QModelIndex &index);
   void grepProgressFinish();
   void grepProgressShow(bool);
	void initProxyView(QTreeView* proxyView);
   inline Layout layout() const { return (ui->resultsStackedWidget->currentIndex()==HORIZONTAL)?HORIZONTAL:VERTICAL;}
	void layoutPushButtonSetLabel();
	inline void setIndex(const QModelIndex &index) { m_index = index; }
   void setMainGuiAppearance();
   void setTabGuiAppearance();
   bool setSettings();
   void setTabLabel();
   void setTabToolTip() const;
   void setupStatusBar();
   void showHeader(const QString &filepath);
   void showLine(const Line &line);
   void showLineno(int lineno);
   bool showWarnings();
   void shutdown();
   void stateFind(bool state);
   void statusShowFindFinal(int _numFiles);
   void waitForMatcherThread();
protected:
	void changeEvent(QEvent *e);
	bool event(QEvent *e);

private:
   bool m_abortMatcher;                ///< abort match calculations
   bool m_findCanceled;                ///< find canceled
   bool m_findDone;                    ///< current find thread is done
   bool m_findState;                   ///< *TRUE* if running a find operation
   bool m_grepCancel;                  ///< cancel grep operation
   bool m_grepProgressBarAvail;        ///< unless worker thread using it
   bool m_grepState;                   ///< *TRUE* if running a grep operation
   bool m_matcherDone;                 ///< Matches column has been updated
   int m_numFound;                     ///< total files found
   quint32 m_findMatches;              ///< in case number of find matches is limited
   qint64 m_grepFileSize;              ///< don't show grep progress for small files
   qint64 m_grepProgress;              ///< grep progress bar progress
   AtomicQueue<WordCount *> m_aqueue;  ///< canonical path queue for counting matches
   FindRun *m_findRun;                 ///< find wrapper
   GrepRun *m_grepRun;                 ///< grep wrapper
   Highlighter *m_highlighter;         ///< for grep display
   MainWindow *mw;                     ///< ptr to the main window app
   Parms *p;                           ///< search context
   Settings *settings;                 ///< global settings ptr
   SortFilterProxyModel *proxyModel;   ///< allows custom sorting
   TabManager *tm;                     ///< to check if current tab to send status to main window
   Ui::Settings *sui;                  ///< shortcut ptr to local settings ui
   Ui::Tab *ui;                        ///< ptr to this gui
   QAction *cancelAct;                 ///< cancel context menu
   QAction *copyAct;                   ///< right click to copy a file
   QAction *deleteAct;                 ///< right click to delete a file
   QAction *moveAct;                   ///< right click to move a file
   QAction *openAct;                   ///< right click to open a file
   QAction *separatorAct;              ///< context menu separator
   QAction *separatorAct2;             ///< context menu separator
   QItemSelectionModel *smodel;        ///< gets sigs off the tree view
   QLabel *statusFindLabel;            ///< allows messaging in local status bar
   QLabel *statusGrepLabel;            ///< ""
   QModelIndex m_index;                ///< to reset current item after progress bar mucks it up
   QPersistentModelIndex m_mruIdx;     ///< most recently acquired model index
   QQueue<Line> m_gqueue;              ///< grep queue
   QStandardItemModel *m_showModel;    ///< shortcut to current widget when search began
   QStandardItemModel *model;          ///< data struct for tab contents
   QStatusBar *statusBar;              ///< local status bar for tab
   QString m_curFilepath;              ///< key variable to cancel or switch greps
   QTextEdit *m_lineDisplay;           ///< ptr to widget for highlighted grep output
   QTime m_findTime;                   ///< clock find operation
   QThreadPool pool;                   ///< find and grep threads

signals:
   void enableSearchButtons(bool);
   void grepQueueFull();
   void statusMiddle(const QString &);
   void statusRight(const QString &);
   void statusShowMain(const QString &);

public slots:
	void addFile(QAbstractItemModel *model,const QString &filename,
					 const QString &size, int hits, const QString &type,
					 const QString &modified);
   void boxError(const QString &, const QMessageBox::Icon = QMessageBox::Warning, Error err = ERR_INVALID);
	void cancel();
   void contextCancel();
   bool contextCopy();
   void contextDelete();
   void contextMove();
   void contextOpen();
   void findDone(int);
   void grepDone(int);
   void grepInit(const QItemSelection &, const QItemSelection &);
   void matcherDone(bool);
   void on_findProgressBar_setFormat(QString);
	void on_findProgressBar_setRange(int, int);
   void on_findProgressShow(bool);
   void on_findProgressBar_addValue(int);
   void on_findProgressBar_setValue(int);
	void on_grepProgressBar_setFormat(QString);
	void on_grepProgressBar_setRange(int, int);
	void on_grepProgressBar_setValue(int);
   void showFiles(QFileInfoList files);
   void statusShow(const QString &, Operation op = FIND);
   void storeLines(QList<Line>);
   void updateMatchColumn(const WordCount *wc);

private slots:
   void on_refreshPushButton_clicked();
   void on_settingsPushButton_clicked();
   void on_grepCancelButton_clicked();
   void on_findCancelButton_clicked();
   void on_layoutPushButton_clicked();
   void processGrepQueue();
};

#endif // TAB_H
