/**************************************************************************//**
 * NAME:    tab.cpp
 *
 * PURPOSE: Implementation of Tab widget.
 *
 * DATE:    October 24, 2009
 *
 * NOTES:   Pushes data out to Find object through an atomic queue and gets
 *          data back via Qt signals.  Receives matched data from Grep objects
 *          via signals, which are queued.  Why?  Because the grep display has
 *          to call qApp::processEvents() which led to recursion problems.  The
 *          queue separates signals from their payload and allows us to block
 *          otherwise recursive signals and discard non-current payloads with-
 *          out unwinding stack frames and botching up the dispaly.
*******************************************************************************/
#include "debug.h"

#include <QAction>
#include <QChar>
#include <QDesktopServices>
#include <QFileDialog>
#include <QModelIndex>
#include <QHash>
#include <QHashIterator>
#include <QKeySequence>
#include <QStatusTipEvent>
#include <QString>
#include <QTextCursor>
#include <QTextStream>
#include <QThread>
#include <QUrl>

#include <qtconcurrentmap.h>

#include "atomicqueue.h"
#include "findrun.h"
#include "greprun.h"
#include "highlighter.h"
#include "mainwindow.h"
#include "matcher.h"
#include "parms.h"
#include "settings.h"
#include "settingsglobal.h"
#include "sortproxy.h"
#include "stabwidget.h"
#include "tabmanager.h"
#include "tab.h"
#include "util.h"
#include "presets/folders.h"
#include "presets/presets.h"

#include "ui_settingsglobal.h"
#include "ui_settings.h"
/**************************************************************************//**
 * Tab(): create a new tab.
 ******************************************************************************/
Tab::Tab  (TabManager *_tm, QWidget *_parent)
:
QWidget(_parent),
m_abortMatcher(false),
m_findCanceled(false),
m_findDone(true),
m_findState(false),
m_grepCancel(false),
m_grepProgressBarAvail(false),
m_grepState(false),
m_matcherDone(true),
m_numFound(0L),
m_findMatches(0UL),
m_grepFileSize(0LL),
m_grepProgress(0LL),
m_findRun(nullptr),
m_grepRun(nullptr),
m_highlighter(nullptr),
mw(_tm->getMw()),
p(new Parms(this, mw)),
settings(new Settings(mw->getUi())),
proxyModel(nullptr),
tm(_tm),
sui(settings->getUi()),
ui(new Ui::Tab),
m_index(QModelIndex()),
m_mruIdx(QModelIndex())
{
   ui->setupUi(this);

   createActions();

   settings->setWindowIcon(QIcon(SM_ICON));

   // one for find, one for calculating matches, and one for grep
   pool.setMaxThreadCount(3);

   findProgressShow(false);
   grepProgressShow(false);
   ui->findProgressBar->reset();  // sets value() to -1
   ui->grepProgressBar->reset();  // ""

   setupStatusBar();

   ui->resultsStackedWidget->setCurrentIndex(1);  // start in vertical layout

   ui->layoutPushButton->setVisible(false);

	proxyModel = new SortFilterProxyModel(this);
	proxyModel->setDynamicSortFilter(true);
	proxyModel->setSourceModel(createFilesModel(this));  // instantiates model

	smodel = new QItemSelectionModel(proxyModel);
	connect(smodel, SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
           this, SLOT(grepInit(QItemSelection, QItemSelection)));

   connect(this, SIGNAL(enableSearchButtons(bool)), mw, SLOT(enableSearchButtons(bool)));
   connect(this, SIGNAL(grepQueueFull()), this, SLOT(processGrepQueue()));
   connect(this, SIGNAL(statusMiddle(QString)), mw, SLOT(statusMiddle(QString)));
   connect(this, SIGNAL(statusRight(QString)), mw, SLOT(statusRight(QString)));
   connect(this, SIGNAL(statusShowMain(QString)), mw, SLOT(statusShow(QString)));

   initProxyView(ui->proxyViewH);          // horizontal
   ui->proxyViewH->setModel(proxyModel);
   ui->proxyViewH->setSelectionModel(smodel);
   ui->proxyViewH->setSelectionMode(QAbstractItemView::SingleSelection);
   ui->proxyViewV->setSelectionBehavior(QAbstractItemView::SelectRows);
   ui->proxyViewH->setColumnWidth(0,225);  // path
   ui->proxyViewH->setColumnWidth(1,27);   // size
   ui->proxyViewH->setColumnWidth(2,32);   // matches
   ui->proxyViewH->setColumnWidth(3,25);   // type
   ui->proxyViewH->setColumnWidth(4,50);   // modified
   ui->proxyViewH->setColumnWidth(5,0);    // separator

   initProxyView(ui->proxyViewV);          // vertical
   ui->proxyViewV->setModel(proxyModel);
   ui->proxyViewV->setSelectionModel(smodel);
   ui->proxyViewV->setSelectionMode(QAbstractItemView::SingleSelection);
   ui->proxyViewV->setSelectionBehavior(QAbstractItemView::SelectRows);
   ui->proxyViewV->setColumnWidth(0,450);  // path
   ui->proxyViewV->setColumnWidth(1,54);   // size
   ui->proxyViewV->setColumnWidth(2,64);   // matches
   ui->proxyViewV->setColumnWidth(3,50);   // type
   ui->proxyViewV->setColumnWidth(4,100);  // modified
   ui->proxyViewV->setColumnWidth(5,0);    // separator

   ui->lineDisplayH->setReadOnly(true);  // O.w. bug in setHtml() when
   ui->lineDisplayV->setReadOnly(true);  // switching stacked layout. (v3.0)
   /* v2.x.x
   connect(ui->lineDisplayH->document(), SIGNAL(contentsChanged()), mw,
														 SLOT(documentWasModified()));
   connect(ui->lineDisplayV->document(), SIGNAL(contentsChanged()), mw,
														 SLOT(documentWasModified())); */
}
/**************************************************************************//**
 * Tab(): destroy a tab and kill all concurrent operations.
 ******************************************************************************/
Tab::~Tab  () {
   ENTEX2("Tab::~Tab", 0);

   grepKill();

   findKill();

   m_gqueue.clear();

   delete p;
   p = nullptr;

   delete ui;
   ui = nullptr;
}
/**************************************************************************//**
 * AddFile(): add row to the matched row model.  [slot]
 ******************************************************************************/
void Tab::addFile  (QAbstractItemModel *model, const QString &filename,
                    const QString &size, int matches, const QString &type,
                    const QString &modified) {
   ENTEX("Tab::addFile");
	int row = model->rowCount();
	model->insertRow(row);
   qd(10) << "adding to model-" << filename;
	model->setData(model->index(row, 0), filename, Qt::DisplayRole);
	model->setData(model->index(row, 1), size, Qt::DisplayRole);
   model->setData(model->index(row, 2), matches, Qt::DisplayRole);
	model->setData(model->index(row, 3), type, Qt::DisplayRole);
	model->setData(model->index(row, 4), modified, Qt::DisplayRole);
}
/**************************************************************************//**
 * AutoGrepFirstFile(): auto grep first file after find and give it focus.
 ******************************************************************************/
 void Tab::autoGrepFirstFile  () {
   if (!smodel->hasSelection()) {
      smodel->setCurrentIndex(model->index(0,0), QItemSelectionModel::Rows | QItemSelectionModel::Current);
      smodel->select(model->index(0,0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
   }
}
/**************************************************************************//**
* BoxError(): pass along a popup error message (GUI event).  [slot]
******************************************************************************/
void Tab::boxError  (const QString & s, const QMessageBox::Icon icon, Error err) {
   ENTEX("Tab::boxError");
   (void) err;
   if (!Singleton<SettingsGlobal>::instance()->getUi()->ignoreCheckBox->isChecked()) {
      ::boxError(s, icon);
   }
}
/**************************************************************************//**
* Busy(): is this tab busy finding, grepping, matching, or printing?
******************************************************************************/
bool Tab::busy  () const {
   ENTEX("Tab::busy");
   return m_findState || m_grepState || !m_matcherDone || !m_gqueue.isEmpty();
}
/**************************************************************************//**
* Cancel(): cancel the current find operation.
******************************************************************************/
void Tab::cancel  () {
   if (!Singleton<SettingsGlobal>::instance()->confirm(CB_CANCEL, getTabLabel()))
      return;
   m_findState = false;
   m_findCanceled = true;
}
/**************************************************************************//**
* ChangeEvent(): process change event.
******************************************************************************/
void Tab::changeEvent  (QEvent *e) {
   QWidget::changeEvent(e);
   switch (e->type()) {
   case QEvent::LanguageChange:
      ui->retranslateUi(this);
      break;
   default:
      break;
   }
}
/**************************************************************************//**
 * ContextCancel(): cancel item context menu.  [slot]
 ******************************************************************************/
void Tab::contextCancel  () {
}
/**************************************************************************//**
 * ContextCopy(): use view context menu to copy file.  [slot]
 ******************************************************************************/
bool Tab::contextCopy  () {
   ENTEX2("Tab::contextCopy", 0);

   QString dir = ::browse(this, CONTEXT);
   qd(0) << "browse() returned dir-" << dir;
   if (dir.isEmpty())
      return false;

   QString source = contextGetPath();
   QFileInfo destination(source);
   QString name = destination.fileName();  // pull filename off source
   qd(0) << "name-" << name;

   QString dst = dir + "/" + name;  // new dir, same filename
   qd(0) << "dst-" << dst;

   if (source == dst) {
      statusShow(tr("Same file...nothing to do"), MAIN);
      return false;
   }

   QFile src(source);

   statusShow(tr("Copying %1 to %2...").arg(name).arg(dst), MAIN);
   qd(0) << "Copying" << name << "to" << dst;

   if (!src.copy(dst)) {
      QFile fdst(dst);
      if (fdst.exists()) {
         if (::boxConfirm("File exists. Overwrite?", "Overwrite Confirmation")) {
            if (fdst.remove()) {
               if (src.copy(dst))
                  goto exit;
            }
            else {
               ::boxError(tr("Could not remove %1. Copy aborted.").arg(dst));
               statusShow(tr("Copy aborted. Remove failed-") + fdst.errorString(), MAIN);
               qd(0) << "QFile::remove() failed[" << fdst.error() << "]:" << fdst.errorString();
               return false;
            }
         }
         else {
            statusShow(tr("Copy canceled on overwrite (%1)").arg(src.errorString()), MAIN);
            return false;
         }
      }

      ::boxError(tr("Could not copy %1 to %2").arg(source).arg(dst));
      statusShow(tr("Copy failed-") + src.errorString(), MAIN);
      qd(0) << "QFile::copy() failed[" << src.error() << "]:" << src.errorString();
      return false;
   }

   exit:

   statusShow(tr("Copy succeeded"), MAIN);
   return true;
}
/**************************************************************************//**
 * ContextDelete(): use view context menu to delete file.  [slot]
 ******************************************************************************/
void Tab::contextDelete  () {
   ENTEX("Tab::contextDelete");

   QString path = contextGetPath(),
           s;

   if (!::fileRemove(path, PROMPT, &s)) {
      statusShow(tr("Remove failed- %1").arg(s), MAIN);
      return;
   }

   statusShow(tr("Deleted %1 from disk").arg(path), MAIN);

   /* if we ever search for folders as well as files...
   if (validDir(path)) {
      QDir dir;
      if (dir.exists(path) && !dir.rmdir(path)) {
         boxError(tr("Could not remove directory %1.").arg(path));
         return;
      }
   } */

   if (!m_mruIdx.isValid()) {
      qd(0) << "invalid mru index, very bad";
      return;
   }
   // same QModelIndex sourceIndex = proxyModel->mapToSource(index);
   // as.. model->removeRow(sourceIndex.row());
   proxyModel->removeRow(m_mruIdx.row());

   /* strikeout only code...
   QStandardItem * si = model->itemFromIndex(sourceIndex);
   QFont font(si->font());
   font.setStrikeOut(true);
   si->setFont(font); */
}
/**************************************************************************//**
 * ContextGetPath(): grab path from current row and save index.
 ******************************************************************************/
QString Tab::contextGetPath  () {
   ENTEX("Tab::contextGetPath");

   m_mruIdx = smodel->selectedIndexes().at(0);  // or ui->proxyView?->currentIndex()

   QString path = getFilepath(&m_mruIdx);

   qd(0) << "target path-" << path << "from selection model index(" << m_mruIdx.row() << "," << m_mruIdx.column() << ")";

   return path;
}
/**************************************************************************//**
 * ContextMove(): use view context menu to move file.  [slot]
 ******************************************************************************/
void Tab::contextMove  () {
   ENTEX("Tab::contextMove");

   QString path = contextGetPath();

   if (!::boxConfirm(QObject::tr("Delete file %1 from disk?").arg(path)))
      return;

   if (!contextCopy()) {
      qd(0) << "aborting move operation (copy failed)...";
      return;
   }

   statusShow(tr("Removing original file..."), MAIN);
   QString s;
   if (!::fileRemove(path, NOPROMPT, &s)) {
      statusShow(tr("Copy OK but remove failed- %1").arg(s), MAIN);
      return;
   }
   statusShow(tr("Move succeeded"), MAIN);

   if (!m_mruIdx.isValid()) {
      qd(0) << "invalid mru index, very bad";
      return;
   }

   proxyModel->removeRow(m_mruIdx.row());
}
/**************************************************************************//**
 * ContextOpen(): open file of item off context menu.  [slot]
 *
 * \todo consider +QWebWidget to embed live pages (Adam has examples in svn)
 ******************************************************************************/
void Tab::contextOpen  () {
   ENTEX("Tab::contextOpen");

   QString path = contextGetPath();
   if (QDesktopServices::openUrl(QUrl::fromLocalFile(path)))  // file:///...
      statusShow(tr("File open succeeded"), MIDDLE);
   else
      statusShow(tr("File open failed"), MIDDLE);
}
/**************************************************************************//**
* CreateActions(): create actions for widgets residing on this tab.
******************************************************************************/
void Tab::createActions  () {
   ENTEX("Tab::createActions");

   cancelAct = new QAction(tr("&Cancel"), this);
   //cancelAct->setShortcuts();
   cancelAct->setStatusTip(tr("Cancel menu"));
   connect(cancelAct, SIGNAL(triggered()), this, SLOT(contextCancel()));

   copyAct = new QAction(tr("&Copy to..."), this);
   copyAct->setShortcuts(QKeySequence::Copy);
   copyAct->setStatusTip(tr("Copy current file"));
   connect(copyAct, SIGNAL(triggered()), this, SLOT(contextCopy()));

   deleteAct = new QAction(tr("&Delete"), this);
   deleteAct->setShortcuts(QKeySequence::Delete);
   deleteAct->setStatusTip(tr("Delete current file"));
   connect(deleteAct, SIGNAL(triggered()), this, SLOT(contextDelete()));

   moveAct = new QAction(tr("&Move to..."), this);
   moveAct->setShortcuts(QKeySequence::MoveToEndOfBlock);
   moveAct->setStatusTip(tr("Move current file"));
   connect(moveAct, SIGNAL(triggered()), this, SLOT(contextMove()));

   openAct = new QAction(tr("&Open"), this);
   openAct->setShortcuts(QKeySequence::Open);
   openAct->setStatusTip(tr("Open existing file"));
   connect(openAct, SIGNAL(triggered()), this, SLOT(contextOpen()));

   separatorAct = new QAction(this);
   separatorAct->setSeparator(true);
   separatorAct2 = new QAction(this);
   separatorAct2->setSeparator(true);
}
/**************************************************************************//**
* CreateFilesModel(): constructor helper.
******************************************************************************/
QAbstractItemModel *Tab::createFilesModel  (QObject *_parent) {
   model = new QStandardItemModel(0, 6, _parent);

   model->setHeaderData(0, Qt::Horizontal, QObject::tr("File Name"), Qt::DisplayRole);
   model->setHeaderData(1, Qt::Horizontal, QObject::tr("Size"), Qt::DisplayRole);
   model->setHeaderData(2, Qt::Horizontal, QObject::tr("Matches"), Qt::DisplayRole);
   model->setHeaderData(3, Qt::Horizontal, QObject::tr("Type"), Qt::DisplayRole);
   model->setHeaderData(4, Qt::Horizontal, QObject::tr("Modified"), Qt::DisplayRole);
   model->setHeaderData(5, Qt::Horizontal, QObject::tr(""), Qt::DisplayRole);  // separator

   return (model);
}
/**************************************************************************//**
 * CursorToTop(): move cursor to the top of the TextEdit.
 ******************************************************************************/
void Tab::cursorToTop  () {
   QTextCursor cursor = m_lineDisplay->textCursor();
   cursor.movePosition(QTextCursor::Start);
   m_lineDisplay->setTextCursor(cursor);
}
/**************************************************************************//**
 * Event(): overload to catch and display status events on local status bar.
 ******************************************************************************/
bool Tab::event  (QEvent *e) {
   ENTEX("Tab::event");

   if (e->type() == QEvent::StatusTip){
		QStatusTipEvent *ev = (QStatusTipEvent*)e;
      statusBar->showMessage(ev->tip(), 3000);
		return true;
   }
   // qd(4) << "got event type-" << e->type();

	return QWidget::event(e);
}
/**************************************************************************//**
 * Filter(): filter and process the grep queue.
 ******************************************************************************/
void Tab::filter  (const Line &line) {
   ENTEX2("Tab::filter", 3);

   if (line.lineno == GREP_BEGIN) {
      qd(0) << "got BEGIN package for-" << line.filepath;

      showHeader(line.filepath);  /// clears TextEdit
      return;
   }
   if (line.lineno == GREP_EOF) {
      qd(1) << "got EOF package for-" << line.filepath;
      return;
   }
   if (line.filepath != m_curFilepath) {  /// clears obsolete Line objects
      return;  /// Qt should deallocate possibly large Line object
   }

   showLineno(line.lineno);

   showLine(line);
}
/**************************************************************************//**
 * FindDone(): signal indicates that find has completed.  [slot]
 ******************************************************************************/
 void Tab::findDone  (int _numFiles) {
   ENTEX("Tab::findDone");

   m_findDone = true;

   stateFind(m_findState = false);  // waits on Matcher thread

   statusShowFindFinal(_numFiles);
}
/**************************************************************************//**
 * FindKill(): kill current find operation because new one started.
 ******************************************************************************/
void Tab::findKill  () {
   ENTEX("Tab::findKill");

   m_findState = false;
   m_abortMatcher = true;

   QTime timer;
   timer.start();
   while (!m_findDone || !m_matcherDone) {
      if (timer.elapsed() > 1000) {
         statusShow(tr("Stopping..."));
         qd(0) << "waiting for both m_matcherDone(" << m_matcherDone << ") AND m_findDone(" << m_findDone << ") to be true...";
         timer.restart();
      }
      qApp->processEvents(QEventLoop::WaitForMoreEvents, 250);
   }
   m_aqueue.clear();
}
/**************************************************************************//**
 * FindInit(): initialize a tab prior to executing a find in it.
 *
 * Notes     : Parms not ripped on refresh, but are if current tab changes.
 ******************************************************************************/
void Tab::findInit  (bool refresh) {
   ENTEX("Tab::findInit");

   if (!setSettings())
      return;

   if (!refresh)
      p->ripContext();

   if (!p->validate())
      return;

   if (!showWarnings())
      return;

   if (m_findState)  // pre-empt previous search
      findKill();

   stateFind(m_findState = true);

   Matcher *matcher = new Matcher(this);
   qd(0) << "starting thread on Matcher object " << matcher;
   pool.start(matcher);
   m_matcherDone = false;

   m_findRun = new FindRun(this);
   qd(0) << "starting thread on FindRun object " << m_findRun;
   pool.start(m_findRun);
   m_findDone = false;

   emit statusRight(tr("Searching..."));

   qd(6) << "active threads:" << pool.activeThreadCount();

}  // findInit()
/**************************************************************************//**
 * FindProgressShow(): show or hide find progress indication widgets.
 ******************************************************************************/
void Tab::findProgressShow  (bool b) {
   on_findProgressShow(b);
}
/**************************************************************************//**
 * GetFilepath(): get filepath from model index.
 ******************************************************************************/
QString Tab::getFilepath  (const QModelIndex &index) {
   if (index.column() == 0)
      return index.data().toString();
   else {
      QModelIndex idx = index.sibling(index.row(), 0);  // click any column
      return idx.data().toString();
   }
}
/**************************************************************************//**
 * GetFilepath(): get filepath, possibly shifting model index.
 ******************************************************************************/
QString Tab::getFilepath  (QPersistentModelIndex *index) {

   if (index->column() != 0)
      *index = index->sibling(index->row(), 0);  // click any column

   return index->data().toString();
}
/**************************************************************************//**
 * GetTabLabel(): get this tab's label minus leading ampersand.
 ******************************************************************************/
QString Tab::getTabLabel  () {

   STabWidget *tw = tm->getTw();  // shortcut

   int idx = tw->indexOf(this);
   QString txt = tw->tabText(idx);

   txt = txt.trimmed();

   if (txt.startsWith('&'))
      txt.remove('&');

   return txt;
}
/**************************************************************************//**
 * GrepDone(): post-grep tasks.  [slot]
 *
 * Parms     : bytes - total size of the lines containing all the matches
 ******************************************************************************/
void Tab::grepDone  (int bytes) {
   ENTEX2("Tab::grepDone", 2);

   m_grepState = false;

   // Progress bar scrambles index and selection so reset them on target index.
	smodel->clearSelection();
	smodel->setCurrentIndex(m_index, QItemSelectionModel::Rows |
												QItemSelectionModel::Current);
	smodel->select(m_index, QItemSelectionModel::Rows |
                           QItemSelectionModel::Select);

   if (m_grepFileSize > GREP_MIN_PROGRESS)
      m_grepProgressBarAvail = true;
   ui->grepProgressBar->setFormat("%p% Printing");
   ui->grepProgressBar->setRange(0, bytes);
}
/**************************************************************************//**
 * GrepInit(): caught off the selection model.  [slot]
 *
 * Parms: is - selected items
 *      :    - deselected items (ignored)
 *
 * Notes: The progress bar on the tab causes this slot to be invoked.  I do not
 *        know exactly why.  I do know that the static variable below causes
 *        this problem to go away.  The previous solution -- disconnecting the
 *        signal that calls this slot and reconnecting it when the grep thread
 *        finishes - has thus been replaced.  Now we may preempt previous grep
 *        operations with new ones at the thread level in a timely manner.
 ******************************************************************************/
void Tab::grepInit  (const QItemSelection &is, const QItemSelection &) {
   ENTEX2("Tab::grepInit", 2);

   if (is.indexes().isEmpty())
      return;

   static QItemSelection curSelection = QItemSelection();

   if (is == curSelection)
      return;

   curSelection = is;

   grepInit(is.indexes().at(0));
}
/**************************************************************************//**
 * GrepInit(): initialize new grep op, canceling any previous one.  [[slot]]
 ******************************************************************************/
void Tab::grepInit  (const QModelIndex &_index) {
   ENTEX2("Tab::grepInit", 2);

   /// Validate.
   qd(2) << "content-" << p->content;
   if (p->content.isEmpty())
      return;
   if (!smodel->hasSelection() || !_index.isValid()) {
      qd(0) << "returning on no selection or invalid index...";
      return;
   }

   m_grepState = true;

   setIndex(_index);  /// to reset current item after progress bar mucks it up

   m_curFilepath = getFilepath(_index);  /// assignment starts blocking showLine() calls
   m_gqueue.clear();                     /// brute force 03/01/11
   m_grepProgress = 0LL;                 /// ""

   QFile file(m_curFilepath);
   if ((m_grepFileSize = file.size()) > GREP_MIN_PROGRESS)  /// prevent "blips" for small files
      grepProgressShow(true);
   m_grepProgressBarAvail = false;

   m_grepCancel = false;

   /// Cancel prev worker grep thread and cleanup
   grepKill();

   /// Instantiate a grep object and start it with a new worker thread.
   m_grepRun = new GrepRun(getFilepath(_index), this);
   m_grepRun->setAutoDelete(false);
   qd(0) << "starting new grep thread...";
   pool.start(m_grepRun);
   qd(6) << "active threads: " << pool.activeThreadCount();

}  // grepInit()
/**************************************************************************//**
 * GrepKill(): stop grep operation and cleanup grepRun and grep.
 ******************************************************************************/
void Tab::grepKill  () {
   if (m_grepRun) {
      m_grepRun->cancel();
      while (!m_grepRun->done());  // wait for thread to cancel
      delete m_grepRun;
      m_grepRun = nullptr;
   }
}
/**************************************************************************//**
 * GrepProgressFinish(): hack to finish off grep progress bar if not maxed.
 ******************************************************************************/
void Tab::grepProgressFinish  () {
   if (m_grepProgressBarAvail && ui->grepProgressBar->isVisible()) {
      ui->grepProgressBar->setValue(::intMax(ui->grepProgressBar->maximum()));
      grepProgressShow(false);
   }
}
/**************************************************************************//**
 * GrepProgressShow(): show or hide grep progress indication widgets.
 ******************************************************************************/
void Tab::grepProgressShow  (bool b) {
   ui->grepLabel->setVisible(b);
   ui->grepProgressBar->setVisible(b);
   ui->grepCancelButton->setVisible(b);
}
/**************************************************************************//**
 * InitProxyView(): initialize both horizontal and vertical proxy views.
 ******************************************************************************/
void Tab::initProxyView  (QTreeView *proxyView) {
	proxyView->setRootIsDecorated(false);
	proxyView->setAlternatingRowColors(true);
	proxyView->setSelectionMode(QAbstractItemView::SingleSelection);
	proxyView->setSelectionBehavior(QAbstractItemView::SelectRows);
	proxyView->setUniformRowHeights(true);
	proxyView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	proxyView->setContextMenuPolicy(Qt::ActionsContextMenu);
   proxyView->addAction(openAct);
   proxyView->addAction(deleteAct);
   proxyView->addAction(separatorAct);
   proxyView->addAction(copyAct);
   proxyView->addAction(moveAct);
   proxyView->addAction(separatorAct2);
   proxyView->addAction(cancelAct);

	// enable to allow double-clicking to open file listed in the view
	//connect(_proxyView, SIGNAL(activated(const QModelIndex &)), this,
   //                    SLOT(opencontext(const QModelIndex &)));

	//connect(proxyView, SIGNAL(clicked(const QModelIndex &)), this,
	//						 SLOT(grep(const QModelIndex &)), Qt::DirectConnection);
}
/**************************************************************************//**
 * LayoutPushButtonSetLabel(): set the label on this push button.
 ******************************************************************************/
void Tab::layoutPushButtonSetLabel  () {
      ui->layoutPushButton->setText(p->layout == HORIZONTAL ? "&Vertical"
																				 : "&Horizontal");
}
/**************************************************************************//**
 * MatcherDone(): indicates that Matcher object is done, or not.  [slot]
 ******************************************************************************/
void Tab::matcherDone  (bool done) {
   m_matcherDone = done;
}
/**************************************************************************//**
 * On_findProgressShow(): show or hide find progress indication widgets. [slot]
 ******************************************************************************/
void Tab::on_findProgressShow  (bool b) {
   ENTEX("Tab::on_findProgressShow");

   ui->findProgressBar->reset();  // sets value() to -1
   ui->findLabel->setVisible(b);
   ui->findProgressBar->setVisible(b);
   ui->findCancelButton->setVisible(b);
}
/**************************************************************************//**
 * processGrepQueue(): process grep queue and keep GUI responsive.  [slot]
 ******************************************************************************/
void Tab::processGrepQueue  () {
   ENTEX2("Tab::processGrepQueue", 3);

   static bool processing = false;

   if (processing)  /// block recursion caused by processEvents()
      return;

   processing = true;

   while (!m_gqueue.isEmpty()) {

      filter(m_gqueue.dequeue());  /// may call processEvents()
   }

   processing = false;

   grepProgressFinish();
}
/**************************************************************************//**
 * SetMainGuiAppearance(): set main window gui appearance based on find state.
 ******************************************************************************/
void Tab::setMainGuiAppearance  () {

   Ui::MainWindow *ui = mw->getUi();  // shortcut

   if (m_findState) {
      if (ui->actionAdvanced->isChecked()) {
         ui->cancelPushButton->setDefault(true);
         ui->cancelPushButton->setFocus();
      }
      else {
         ui->quickCancelPushButton->setDefault(true);
         ui->quickCancelPushButton->setFocus();
      }
   }
   else {
      autoGrepFirstFile();
      this->ui->proxyViewV->setFocus();

      if (ui->actionAdvanced->isChecked()) {
         ui->searchPushButton->setDefault(true);
         //ui->searchPushButton->setFocus();
      }
      else {
         ui->quickSearchPushButton->setDefault(true);
         //ui->quickSearchPushButton->setFocus();
      }
   }
}
/**************************************************************************//**
 * SetSettings(): set the settings for find operation.
 *
 * Returns      : true on success, false otherwise
 ******************************************************************************/
bool Tab::setSettings  () {

   switch (settings->set(&(p->filelist))) {
      case -2:
         statusShow(tr("Options cancelled out...nothing to do"));
         settings->reset();
         return false;
      case -1:
         statusShow(tr("Usage error"));
         settings->reset();
         return false;
      case 0: ;
   }
   settings->getFolders()->set();
   QDir d(p->path);
   if (settings->getFolders()->filter().contains(d.dirName())) {
      statusShow(tr("Current directory is part of exclude list...nothing to do"));
      settings->reset();
      return false;
   }
   return true;
}
/**************************************************************************//**
 * SetTabGuiAppearance(): set tab gui appearance based on find state.
 ******************************************************************************/
void Tab::setTabGuiAppearance  () {

   if (m_findState) {
      ui->lineDisplayH->clear();
      ui->lineDisplayV->clear();

      ui->lineDisplayH->setVisible(!p->content.isEmpty());
      ui->lineDisplayV->setVisible(!p->content.isEmpty());

      ui->proxyViewH->setColumnHidden(2, p->content.isEmpty());
      ui->proxyViewV->setColumnHidden(2, p->content.isEmpty());
   }
   ui->proxyViewH->setSortingEnabled(!m_findState);  // disable before! model populated
   ui->proxyViewV->setSortingEnabled(!m_findState);  // ...

   if (!ui->refreshPushButton->isVisible())
      ui->refreshPushButton->setVisible(true);
   ui->refreshPushButton->setEnabled(!m_findState);
}
/**************************************************************************//**
 * SetTabLabel(): set this tab's label reusing serial number.
 ******************************************************************************/
void Tab::setTabLabel  () {
   ENTEX("Tab::setTabLabel");

   STabWidget *tw = tm->getTw();  // shortcut

   int idx = tw->indexOf(this);

   int num = getTabLabel().section(' ', 0, 0).toInt();

   tw->setTabText(idx, tm->tabLabel(num));
}
/**************************************************************************//**
 * SetTabToolTip(): set the tab tooltip.
 ******************************************************************************/
void Tab::setTabToolTip  () const {
   ENTEX("Tab::setTabToolTip");
   qd(0) << "p->filelist-" << p->filelist;

   QString s;
   for (int i = 0, sz = p->filelist.size(); i < sz; ++i) {
      s += p->filelist[i];
      if (i != (sz - 1))
         s += " ";
   }
   QString tip = QString("%1 (%2) %3").arg(p->path).arg(s).arg(p->content);
   tm->getTw()->setTabToolTip(tm->getTw()->currentIndex(), tip);
}
/**************************************************************************//**
 * SetupStatusBar(): setup status bar.
 ******************************************************************************/
void Tab::setupStatusBar  () {
   //QSizePolicy policy(QSizePolicy::Maximum, QSizePolicy::Maximum, QSizePolicy::Label);
   //QSizePolicy policy2(QSizePolicy::Expanding, QSizePolicy::Expanding, QSizePolicy::Label);

   statusFindLabel = new QLabel("Ready.");
   //statusFindLabel->setSizePolicy(policy);
   //statusFindLabel->setMinimumSize(0,0);

   statusGrepLabel = new QLabel();
   //statusGrepLabel->setSizePolicy(policy2);
   //statusGrepLabel->setMinimumSize(0,0);
   statusGrepLabel->setWordWrap(true);

   statusBar = new QStatusBar;
   //statusBar->setSizePolicy(policy2);
   //statusBar->setMinimumSize(0,0);
   statusBar->addPermanentWidget(statusFindLabel, 0);
   statusBar->addPermanentWidget(statusGrepLabel, 100);

   ui->gridLayout_3->addWidget(statusBar, 3, 0, 1, 3);
}
/**************************************************************************//**
 * ShowFiles(): display a directory worth of files after queueing up the
 *              canonical paths so matches therein may be calculated.  [slot]
 ******************************************************************************/
 void Tab::showFiles  (QFileInfoList fil) {
   ENTEX("Tab::showFiles");

   if (!m_findState)
      return;
   /*
    * We want to add the filepaths to the model first because once they are
    * enqueued the matches column may be updated.
    */
   WordCount *wc = new WordCount;  // delete in updateMatchColumn()

   register int i = 0;
   for (register int sz = fil.size(); i < sz; ++i) {

      if (!p->content.isEmpty()) {
         wc->insert(fil[i].canonicalFilePath(), model->rowCount());  // we'll need this later
         qd(4) << "insert() key-" << fil[i].canonicalFilePath() << "row:" << model->rowCount();
      }

      QDateTime dt = fil[i].lastModified();

      addFile(model, fil[i].canonicalFilePath(), bytes2units(fil[i].size()),
             !p->content.isEmpty() ? 1 : 0, fil[i].suffix(),
              dt.toString("MM.dd.yy hh:mm:ss"));

      if (p->findMatches) {
         if (++m_findMatches >= p->findMatches) {
            m_findMatches = 0UL;
            m_findState = false;
            m_findCanceled = true;
            ++i;
            break;
         }
      }
   }

   m_numFound += i;

   // Need to count the total matches for Matches column.
   if (!p->content.isEmpty()) {
      m_aqueue.enqueue(wc);
      qd(10) << "m_aqueue size:" << m_aqueue.size();
      qd(10) << "just enqueued WordCount object:" << *wc;
   }

}  // showFiles()
/**************************************************************************//**
 * ShowHeader(): prepare TextEdit and Highlighter for new grep output.
 ******************************************************************************/
void Tab::showHeader  (const QString &_filepath) {
   m_lineDisplay = (layout() == HORIZONTAL) ? ui->lineDisplayH : ui->lineDisplayV;
   m_lineDisplay->clear();

   m_lineDisplay->setVisible(true);  // user may enter content after search done

   m_highlighter = new Highlighter(m_lineDisplay->document(), p->getRegExp());
   m_highlighter->init();

   // Display the filepath but change the format of the highlighter in case
   // the filepath contains any content that would otherwise be highlighted.
   m_lineDisplay->setTextColor(COLOR_HYPERLINK);
   m_lineDisplay->setFontWeight(QFont::Bold);

   m_highlighter->changeFormat(QBrush(COLOR_HYPERLINK), QFont::Bold, _filepath.length());
   m_lineDisplay->insertPlainText(_filepath + "\n");

   m_lineDisplay->setFontWeight(QFont::Normal);
   m_lineDisplay->setTextColor(COLOR_TEXT);
}
/**************************************************************************//**
 * ShowLineno(): prepare TextEdit and Highlighter for new grepped line.
 ******************************************************************************/
void Tab::showLineno  (int _lineno) {
   ENTEX("Tab::showLineno");

   m_lineDisplay->insertPlainText("\n");

   if ((*p)(CB_LINENO)) {

      m_lineDisplay->setTextColor(COLOR_HYPERLINK);
      m_lineDisplay->setFontWeight(QFont::Bold);

      QString s = QString("%1>").arg(_lineno);
      m_highlighter->changeFormat(QBrush(COLOR_HYPERLINK), QFont::Bold, s.length());

      m_lineDisplay->insertPlainText(s);

      m_lineDisplay->setFontWeight(QFont::Normal);
      m_lineDisplay->setTextColor(COLOR_TEXT);
   }

   // Despite our best efforts this will highlight the line number
   // making it stand out, so changes were made in highlighter.cpp.
   m_highlighter->changeFormat(QBrush(COLOR_HIGHLIGHT), QFont::Bold, 0);
}
/**************************************************************************//**
 * ShowLine(): display a line of text in the TextEdit.
 *
 * Notes     : We have to stream it in chunks to maintain user responsiveness.
 *             This is the workhorse/bottleneck, which may be canceled by:
 *                a user hitting Cancel button
 *                b application shutdown
 *                c preemption from new grep operation
 ******************************************************************************/
void Tab::showLine  (const Line &_line) {
   ENTEX("Tab::showLine");

   QTextStream in(const_cast<QString *>(&(_line.line)), QIODevice::ReadOnly);
   in.seek(0);

   QTime timer;
   timer.start();
   qd(4) << "inserting plain text-" << _line.line;

   while (!in.atEnd()) {
      if (m_grepCancel)
         break;

      QString s = in.readLine(75LL);  /// \todo optimize
      qd(4) << "read and now inserting-" << s;

      m_lineDisplay->insertPlainText(s);  // == m_lineDisplay->textCursor().insertText(s)

      /*
       * Be careful because this loop may finish before grep thread
       * relinquishes progress bar (making it available).  It will only be
       * available if file size exceeds GREP_MIN_PROGRESS.
       */
      m_grepProgress += s.size();
      qd(4) << "progress: " << m_grepProgress << " , max: " << ui->grepProgressBar->maximum();

      if (m_grepProgressBarAvail)
         ui->grepProgressBar->setValue(::intMax(m_grepProgress));

      if (m_grepProgress >= ui->grepProgressBar->maximum())  /// grep thread sets max to 0 a tad bit before making progress bar available
         grepProgressShow(false);

      //
      // Recursion to this method blocked in processGrepQueue() slot. May
      // start a new grep operation.
      //
      qApp->processEvents();  /// critical for responsiveness

      if (_line.filepath != m_curFilepath) {
         qd(3) << "discarding package for-" << _line.filepath;
         break;
      }
   }
   qd(3) << "inserted+ " << _line.line.size() << " bytes in " << timer.elapsed() << " ms";

}  // showLine()
/**************************************************************************//**
 * ShowWarnings(): warn about conflicting settings, etc.
 *
 * Notes         : popups are annoying so try to avoid adding warnings
 *
 * Returns       : true to start search, false to cancel search
 ******************************************************************************/
bool Tab::showWarnings  () {
   ENTEX("Tab::showWarnings");

   // user wants to search for known binary extension which is being filtered
   if (sui->extensionRadioButton->isChecked()) {
      QStringList sl = listIntersect(BINARY_EXTS, ::starDotRemove(p->filelist));
      int sz = sl.size();
      if (sz == 0)
         return true;
      int gt1 = sz - 1;  // nonzero (true) if list size exceeds 1
      QString s = "Extension";
      s += gt1 ? "s (" : " (";
      for (int i = 0; i < sz; ++i) {
         s += sl.at(i);
         if (i != gt1)
            s += " ";
      }
      s += ") ";
      s += gt1 ? "are " : "is ";
      s += "being filtered (see tab settings).  Proceed?";

      return ::boxConfirm(s);
   }
   return true;
}
/**************************************************************************//**
 * Shutdown(): trip all flags to shutdown this tab.
 ******************************************************************************/
void Tab::shutdown  () {
   if (m_grepRun)
      m_grepRun->cancel();  // cancel grep thread

   m_grepCancel = true;     // cancel grep output

   m_findState = false;     // cancel find thread

   m_abortMatcher = true;   // cancel matcher thread
}
/**************************************************************************//**
 * StoreLines(): store lines in queue and signal when queue is full.  [slot]
 ******************************************************************************/
void Tab::storeLines  (QList<Line> lines) {
   ENTEX2("Tab::storeLines", 3);

   if (lines.size() == 1  &&  lines.at(0).lineno == GREP_BEGIN)
      emit grepQueueFull();

   QListIterator<Line> i(lines);
   while (i.hasNext()) {
      Line line = i.next();

      m_gqueue.enqueue(line);

      if (m_gqueue.size() > 10  ||  line.line.size() > 256)  /// \todo optimize
         emit grepQueueFull();
   }

   if (lines.size() == 1  &&  lines.at(0).lineno == GREP_EOF)
      emit grepQueueFull();
}
/**************************************************************************//**
 * StateFind(): do stuff right befor and right after a find operation.
 ******************************************************************************/
void Tab::stateFind  (bool) {
   ENTEX("Tab::stateFind");

   qd(0) << "m_findState is" << m_findState;

   if (m_findState) {
      m_abortMatcher = false;
      m_numFound = 0;
      model->setRowCount(0);
      setTabToolTip();
      setTabLabel();
      statusGrepLabel->setText("");
      m_findTime.start();
   }
	else {
      if (!p->content.isEmpty())
         waitForMatcherThread();

      emit statusRight(tr("Ready."));

      m_findMatches = 0UL;
   }

   setTabGuiAppearance();

   setMainGuiAppearance();
}
/**************************************************************************//**
 * StatusShow(): if current tab, pass status to assorted status bars.  [slot]
 ******************************************************************************/
void Tab::statusShow  (const QString &s, Operation op) {
   ENTEX("Tab::statusShow");

   if (!tm->currentTab(this))
      return;

   switch (op) {
      case FIND:
         //statusFindLabel->setText(s.section('/', -1, -1));
         if (m_numFound)
            statusFindLabel->setText(tr("%1 file%2 found").arg(m_numFound)
                                                          .arg(m_numFound == 1 ? "" : tr("s")));
         emit statusShowMain(s);
         break;
      case GREP:
         statusGrepLabel->setText(s);
         break;
      case MAIN:
         emit statusShowMain(s);
         break;
      case MIDDLE:
         emit statusMiddle(s);
         break;
      case MATCHER:
         break;
      case TIP:
         setStatusTip(s);  // triggers event, see event()
         break;
      default:
         break;
   }
}
/**************************************************************************//**
 * StatusShowFindFinal(): show final find status when operation ends.
 *
 * Parms                : _numFiles - total number of files searched
 *
 * Notes                : Cases:
 *                         1 canceled and found files = 0     : msg (cancel msg)
 *                         2 canceled and found files > 0     : msg (cancel msg)
 *                         3 not canceled and found files = 0 : msg
 *                         4 not canceled and found files > 0 : msg
 ******************************************************************************/
void Tab::statusShowFindFinal  (int _numFiles) {
   ENTEX("Tab::statusShowFinalFind");

   int percent = 0,
       val = ui->findProgressBar->value() < 0 ? 0 : ui->findProgressBar->value();
   QString cancel,
           s,
           status;

   if (m_findCanceled) {
      int max = ui->findProgressBar->maximum() ? ui->findProgressBar->maximum()
                                               : FIND_PROGRESS_RANGE_INT;

      qd(0) << "val:" << ui->findProgressBar->value() << "max:" << max;

      QChar c = _numFiles != 0 ? '~' : '\0';
      cancel = tr(" (Incomplete traversal %1%2%)").arg(c).arg((val * 100) / max);

      if (_numFiles == 0) {                  // case 1
         status = "Find canceled" + cancel;
         goto exit;
      }
   }
   // case 3 and 4
   s = tr("%1 file") + ::charS(m_numFound != 1) + tr(" found");
   status = s.arg(m_numFound);

   if (_numFiles)
      percent = (m_numFound * 100) / _numFiles;

   s = tr(" (");
   s += tr(percent == 0  &&  m_numFound > 0 ? ">" : "");  // 0 < percent < 1
   s += tr("%1%)");
   status += s.arg(percent);

   status += tr(" in %1").arg(ms2units(m_findTime.elapsed()));

   if (m_findCanceled)  // case 2
      status += cancel;

   exit:
   findProgressShow(false);
   statusShow(status);
   m_findCanceled = false;
}
/**************************************************************************//**
 * UpdateMatchColumn(): update the Matches column.  [slot]
 ******************************************************************************/
void Tab::updateMatchColumn  (const WordCount *_wc) {
   ENTEX("Tab::updateMatchColumn");
   qd(4) << "QMultiHash:" << *_wc;

   QHashIterator<QString, int> i(*_wc);  ///< <filepath, [matches,row]>
   while (i.hasNext()) {  /// not a good place for processEvents() -- app crashes
      i.next();

      qd(4) << "operating on key-" << i.key();  // unique path

      QList<int> lifo = _wc->values(i.key());
      int matches = lifo.at(0);  /// LIFO QMultiHash (see Tab::showFiles())
      int row = lifo.at(1);      /// LIFO QMultiHash (see Matcher::countHits())

      qd(4) << "got matches:" << matches << "row:" << row;

      QStandardItem *sip = model->item(row, 0);  // filepath
      if (sip == nullptr) {
         qd(0) << "No filepath item! Cannot update mathces. row:" << row << "column: 0";
         return;
      }
      /*
       * Just to be sure, make certain we have the correct row.  If for some
       * very bad reason we do not, then do the expensive find.
       */
      if (sip->data(Qt::DisplayRole).toString() == i.key()) {
         qd(4) << "found expected row:" << row << "(matches:" << matches << ")";
      }
      else {
         qd(0) << "did NOT find expected row:" << row << "(matches:" << matches << ") have to call findItems()...";
         QList<QStandardItem *> list = model->findItems(i.key());
         if (list.isEmpty()  ||  list[0] == 0) {
            qd(0) << "missing or invalid model item:  key-" << i.key();
            return;
         }
         row = list[0]->row();
      }

      sip = new QStandardItem();
      sip->setData(QVariant(matches), Qt::DisplayRole);
      if (model->item(row, 2))
         model->setItem(row, 2, sip);  // update (deletes sip)
      else
         qd(0) << "model(" << row << "," << 2 << ") does not exist: could not update!";

      i.next();  // our QMultiHash has two values per key
   }
   delete _wc;  ///< new in showFiles()
   _wc = nullptr;
}
/**************************************************************************//**
 * WaitForMatcherThread(): wait for the Matcher thread to be done.
 ******************************************************************************/
void Tab::waitForMatcherThread  () {
   ENTEX("Tab::waitForMatcherThread");
   QTime timer;
   timer.start();
   while (!m_matcherDone) {
      if (timer.elapsed() > 1000) {
         if (!m_aqueue.isEmpty())
            statusShow(tr("Calculating 'Matches' column..."));
         qd(0) << "waiting for Matcher thread...";
         timer.restart();
      }
      qApp->processEvents(QEventLoop::WaitForMoreEvents, 250);
   }
}
/**************************************************************************//**
 * Function group [slots]: headers generated by or based upon ui file.
 ******************************************************************************/
void Tab::on_findCancelButton_clicked() {
   cancel();
}
void Tab::on_findProgressBar_setFormat(QString s) {
   ui->findProgressBar->setFormat(s);
}
void Tab::on_findProgressBar_setRange(int m, int n) {
   ui->findProgressBar->setRange(m, n);
}
void Tab::on_findProgressBar_addValue(int i) {
   ENTEX("Tab::on_findprogressBar_addValue");
   ui->findProgressBar->setValue(ui->findProgressBar->value() + i);
}
void Tab::on_findProgressBar_setValue(int i) {
   ENTEX("Tab::on_findprogressBar_setValue");
   ui->findProgressBar->setValue(i);
}
void Tab::on_grepCancelButton_clicked() {
   ENTEX("Tab::on_grepCancelButton_clicked");
   qd(2) << "grep cancel button clicked...";
   m_grepCancel = true;  // cancel output
   if (m_grepRun) {
      qd(2) << "cancelling grep thread...";
      m_grepRun->cancel();  // cancel thread
   }
   m_curFilepath = "";
   m_gqueue.clear();  // stops queue processing
   grepProgressShow(false);
}
void Tab::on_grepProgressBar_setFormat(QString s) {
   ui->grepProgressBar->setFormat(s);
}
void Tab::on_grepProgressBar_setRange(int m, int n) {
   ui->grepProgressBar->setRange(m, n);
}
void Tab::on_grepProgressBar_setValue(int i) {
   ui->grepProgressBar->setValue(i);
}
void Tab::on_layoutPushButton_clicked() {
   if (ui->layoutPushButton->text() == "&Horizontal") {
      ui->layoutPushButton->setText("&Vertical");
		mw->getLayoutAct()->setText("&Vertical");
      mw->getUi()->actionHorizontal->setChecked(true);
	}
	else {
      ui->layoutPushButton->setText("&Horizontal");
		mw->getLayoutAct()->setText("&Horizontal");
      mw->getUi()->actionVertical->setChecked(true);
	}
}
void Tab::on_settingsPushButton_clicked() {
   settings->show();
}
void Tab::on_refreshPushButton_clicked() {
   findInit(SM_REFRESH);
}
