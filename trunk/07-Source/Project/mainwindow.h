/*****************************************************************************/
/*! \file : mainwindow.h
 *
 * PURPOSE: Main program header file.
 *
 * DATE:    September 28, 2009
*******************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtConcurrentRun>
#include <QMainWindow>

#include <QDir>
#include <QFlags>
#include <QFuture>
#include <QFutureWatcher>
#include <QMenu>
#include <QPoint>
#include <QRunnable>
#include <QThread>
#include <QThreadPool>
#include <QVector>

#include "util.h"

#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
class QAbstractButton;
class QAbstractItemModel;
class QCloseEvent;
class QComboBox;
class QCompleter;
class QDir;
class QDirModel;
class QGroupBox;
class QLabel;
class QRegExpValidator;
class QSplitter;
class QStandardItemModel;
class QTabBar;
class QTabWidget;
class QTextEdit;
class QTreeView;
QT_END_NAMESPACE

class About;
class Hits;
class License;
class Parms;
class Presets;
class SettingsGlobal;
class SortFilterProxyModel;
class StabWidget;
class TabManager;
class Tab;

namespace Ui {
   class MainWindow;
   class SettingsGlobal;
   class Tab;
}

class MainWindow : public QMainWindow {
Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();
	void browseQuick(QLineEdit *);
	Qt::CaseSensitivity caseSensitivity();
	void clear();
   void createActions();
   inline QAction *getLayoutAct() const { return layoutAct; }
   QRegExp::PatternSyntax getSyntax() const;
	inline Ui::MainWindow *getUi() const { return ui; }
	void initLineEditCompleter();
   void initState();
	void readSettings();
	void refresh();
	void resetAdvanced();
   void setupTabWidget();
   void stateDateTime(QString option);
	void stateSize(QString _option);
	void writeSettings();

private:
	bool m_ctorDone;                  ///< true if constructor done
   About *about;                     ///< ptr to the About object
   License *license;                 ///< ptr to the License object
   SortFilterProxyModel *proxyModel; ///< allows sorting tree view by size
	STabWidget *tw;                   ///< shortcut ptr to the tab widget
	TabManager *tm;
   Ui::MainWindow *ui;               ///< ptr to the main window ui
   QAction *closeAct;                ///< close tab
   QAction *exitAct;
	QAction *layoutAct;
	QAction *logHistoryAct;
	QAction *newAct;
	QAction *newTabAct;
	QAction *openAct;
	QAction *stateAct;
	QAction *resultsBrowserAct;
	QAction *settingsAct;
	QCompleter *completer;            ///< filesystem completer for line edits
	QDirModel *dirModel;              ///< basis of filesystem completer
   QLabel *middleLabel;              ///< normal duration status message (for grep)
   QLabel *rightLabel;               ///< status bar message
   QMenu *tabContextMenu;            ///< so each tab has a context menu
   QPoint p;                         ///< local position of tab bar context menu event
   QPushButton *addButton;           ///< add tab corner widget (button)
   QPushButton *closeButton;         ///< close tab corner widget (button)
   QPushButton *addTabButton;        ///< new tab button
   QPushButton *closeTabButton;      ///< close tab button
   QRegExp::PatternSyntax m_syntax;  ///< content match (grep) syntax
   QRegExpValidator *sizeValidator;  ///< validates user size entries
   QSplitter *splitter;
   QTabBar *tb;                      ///< shortcut ptr to tab widget tab bar

signals:
   void stopPreSearchDirCnt();
   void tabFind();

public slots:
	void boxError(const QString &, const QMessageBox::Icon = QMessageBox::Warning, Error err = ERR_INVALID);
	void enableSearchButtons(bool);
   void on_actionHorizontal_toggled(bool);
   void on_actionVertical_toggled(bool);
   //void open(const QModelIndex &index);
	//void open();
   void statusMiddle(const QString &);
   void statusRight(const QString &);
	void statusShow(QAction *);
	void statusShow(const QString &);

private slots:
   void on_actionAbout_Qt_triggered();
   void on_actionAdvanced_toggled(bool);
   void on_action_Close_triggered();
   void on_actionLogHistory_toggled(bool);
   void on_actionQuick_toggled(bool);
	void on_actionRestore_triggered();
	void on_actionResultsBrowser_toggled(bool);
	void on_actionSearch_Dialog_toggled(bool);
   void on_actionSettings_triggered();
	void on_browseToolButton_clicked();
	void on_cancelPushButton_clicked();
	void on_clearPushButton_clicked();
	void on_contentLineEdit_textChanged(QString);
   void on_customContextMenuRequested(QPoint pos);
   void on_dateTimeComboBox_currentIndexChanged(QString);
	void on_directoryLineEdit_textChanged(QString);
   void on_dockWidget_topLevelChanged(bool top);
   void on_dockWidget_visibilityChanged(bool visible);
	void on_fileLineEdit_textChanged(QString);
	void on_lineDisplayH_textChanged();
	void on_lineDisplayV_textChanged();
	void on_logHistoryDockWidget_visibilityChanged(bool visible);
	void on_quickBrowseToolButton_clicked();
	void on_quickCancelPushButton_clicked();
	void on_quickClearPushButton_clicked();
	void on_quickContentLineEdit_textChanged(QString);
	void on_quickFileLineEdit_textChanged(QString);
	void on_quickDirectoryLineEdit_textChanged(QString);
	void on_quickSearchPushButton_clicked();
	void on_resetAdvButton_clicked();
	void on_resultsBrowserDockWidget_visibilityChanged(bool visible);
	void on_resultsTabWidget_currentChanged(int index);
	void on_searchPushButton_clicked();
	void on_sizeComboBox_currentIndexChanged(QString option);
	void on_sizeLineEdit_textEdited(QString);
	void on_sizeLineEdit2_textEdited(QString);
	void on_tbarActionLayout_triggered();
	void on_tbarActionLogHistory_triggered();
	void on_tbarActionResultsBrowser_triggered();
	void on_tbarActionSettings_triggered();
	void on_tbarActionState_triggered();

	void documentWasModified();
   void showAbout();
   void showLicense();
	//void newFile();
	//bool save();
	//bool saveAs();

protected:
   virtual void changeEvent(QEvent *e);
   void closeEvent(QCloseEvent *event);
	void keyPressEvent(QKeyEvent *event);
};

#endif ///< MAINWINDOW_H
