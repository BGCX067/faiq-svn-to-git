find . -type f -name '*.h'
find . -type f -name '*.ui'
find . -type f -name '*.ui' >> tmp
cat tmp
find . -type f -name '*.h' >> tmp
cat tmp
find . -type f -name '*.cpp' >> tmp
cat tmp
wc
man wc
info
info coreutils wc
man info
info -O
cat tmp
wc --help
wc -l --files0-from=tmp
vi tmp
wc -l --files0-from=tmp
vi tmp
wc -l --files0-from=tmp
clear
wc -l --files0-from=tmp
vi tmp
wc -l --files0-from=tmp
wc --help
ps
vi tmp
wc -l --files0-from=tmp
vi tmp
wc -l --files0-from=tmp
wc --help
cat tmp
vi tmp
wc -l `cat tmp`
grep *.ui tmp
grep '*.ui' tmp
grep . tmp
grep ui tmp
grep '\*\.ui' tmp
grep '*\.ui' tmp
grep 'ui$' tmp
grep .ui tmp
grep .ui tmp | wc
grep .ui tmp
grep .ui tmp > tmpui
cat tmpui
vi tmp
wc -l `cat tmp`
man wc
wc
wc --help
wc -m `cat tmp`
cat debug.h
grep ; debug.h
grep ';'debug.h
grep ';' debug.h
grep ';' debug.h | wc
cat debug.h | wc
grep ';' `cat tmp`
grep ';' `cat tmp` > tmpsemi
cat tmpsemi
wc semi
wc tmpsemi
vi mainwindow.ui
wc -l `cat tmp`
wc tmpsemi
rm tmp*
clear
grep "new Presets" *
grep "new Settings" *
mkdir tmp
cp mainwindow.cpp tmp/hack
cd tmp
vi hack
cd ..
grep processMore *
cat /home/shawno/coms
find . -type f -name '*' -exec grep 'processMore' {} \; -print
man find
man prune
man find
find . -type f -name '*' -exec grep 'processMore' {} \; -print -path ./.svn -prune
find . -type f -name '*' -exec grep 'processMore' {} \; -exec test {} = './*/.svn' \; -prune -print 
stat
clear
find . -type f -name '*' -exec grep 'processMore' {} \; -exec test {} = './*/.svn' \; -prune -print 
find . -type f -name '*' ! -exec test {} = './*/.svn' \; -print 
find . -type f -name '*' ! -exec test {} = './*/.svn*' \; -exec grep 'processMore' {} \; -print 
find . -type f -name '*' ! -exec test {} = './*/.svn/*' \; -exec grep 'processMore' {} \; -print 
find . -type f -name '*' ! -exec test {} = '*presets*' \; -exec grep 'processMore' {} \; -print 
find . -type f -name '*' -exec test {} = '*presets*' \; -exec grep 'processMore' {} \; -print 
man test
find . -type f -name '*' ! -exec test {} = '*presets*' \; -exec grep 'processMore' {} \; -print 
find . -type f -name '*' -exec test {} = '*svn*' \; -exec grep 'processMore' {} \; -print 
find . -type f -name '*' ! -exec test {} = '*svn*' \; -exec grep 'processMore' {} \; -print 
find . -type f -name '*' ! -exec test {} = '*.svn*' \; -exec grep 'processMore' {} \; -print 
find . -type f -name '*' -exec test {} = '*.svn*' \; -exec grep 'processMore' {} \; -print 
find . -type f -name '*' -exec test {} = './.*' \; -exec grep 'processMore' {} \; -print 
find . -type f -name '*' ! -exec test {} = './.*' \; -exec grep 'processMore' {} \; -print 
find . -type f -name '*' -exec test {} = './presets/.svn/*/*' \; -prune -exec grep 'processMore' {} \; -print 
find . -type f -name '*' -exec grep 'processMore' {} \; -print | grep -v svn
find . -type f -name '*' -exec test {} = *svn* \; -prune -o \( -exec grep 'processMore' {} \; -print \)
find . -type f -name '*' ! -exec test {} = *svn* \; -prune -o \( -exec grep 'processMore' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'processMore' {} \; -print \)
vi /home/shawno/coms
clear
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'processMore' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'isValid' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'uint2syntax' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'm_checkBoxes' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'CB_BEFORE' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'pui' {} \; -print \)
clear
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'pui' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'sui' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'fui' {} \; -print \)
clear
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'fui' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'filterDirs' {} \; -print \)
clear
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'filterDir' {} \; -print \)
ps
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep '*.h' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep "*.h" {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep *.h {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep "*\.h" {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'limits.h' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'string.h' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep '*.h' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep *.h {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep \*.h {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'newsets' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'hide' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'show' {} \; -print \)
clear
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'show()' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'sui' {} \; -print \)
clear
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'sui' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'cancel' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'cancel()' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'getKey()' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'filterSkip' {} \; -print \)
clear
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'filterSkip' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'set(' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep '^set(' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'set(' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'skipFilter' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'SGUI' {} \; -print \)
rm -fr tmp/
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'getKey' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'target' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'target(' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'list2File' {} \; -print \)
clear
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'SGUI' {} \; -print \)
find . \( -type d -name *svn* -prune \) -o \( -type f -name '*' -exec grep 'on_buttonBox_clicked' {} \; -print \)
vi fnd
chmod +x fnd
fnd
cat fnd
bash fnd
vi fnd
fnd
./fnd
fnd
./fnd
cat fnd
bash fnd
vn* -prune \) -o \( -type f -name '*' -exec grep 'on_buttonBox_clicked' {} \; -
print \)./fnd on_setTypePushButton_clicked
fnd on_setTypePushButton_clicked
bash fnd on_setTypePushButton_clicked
vi /home/shawno/.bash_profile
ps
fnd
echo $PATH
. fnd
ps
fnd
chmod +x fnd
fnd
. fnd
./fnd
bash fnd on_setTypePushButton_clicked
bash fnd createActions 
cat fnd
bash fnd "matches found" 
bash fnd "eek" 
bash fnd "ripGlobalSettings" 
clear
bash fnd "ENTEX" 
bash fnd "threadCount" 
bash fnd "ount" 
bash fnd "eadCount" 
clear
vi fnd
bash fnd "eadCount" 
bash fnd "pool" 
bash fnd "file_encoding" 
bash fnd "layoutWidget" 
bash fnd "add()" 
bash fnd "add(" 
bash fnd "onMoreLineEdit" 
bash fnd "on_MoreLineEdit" 
bash fnd "on_moreLineEdit" 
bash fnd "grepCancelButton" 
bash fnd "elect" 
bash fnd "initGrep" 
bash fnd "SLOT(grep" 
bash fnd "getProgressDialogWasCanceled" 
bash fnd "ueue" 
bash fnd "tabClose" 
bash fnd "delete tm" 
bash fnd "unState" 
bash fnd "new Highlighter" 
bash fnd "limits" 
bash fnd "INT_MAX" 
bash fnd "limits" 
vi fnd
mv fnd fndc
cp fndc fnd
vi fnd
bash fnd "limits" 
bash fnd "INT_MAX" 
bash fnd "resize" 
bash fnd "depth" 
bash fnd "setDepthMax" 
bash fnd "depth" 
bash fnd "initTargetComboBox" 
bash fnd "SG->pm" 
bash fnd "eek" 
bash fnd "next" 
bash fnd "SG->pm" 
bash fndc "set" 
bash fndc "set(" 
cd /cygdrive/m
cd '!repo'
cd branches/shawno
cd *x
fnd
fnd nfoTip
fnd atus
fnd nfo
vi fnd
fnd nfoTip
fnd [tT]ip
fnd shutdown
cd /cygdrive/c
find .
find .|grep ack
cat /user/shawno/coms
cat /home/shawno/coms
find . -type f -exec grep ack {} \; -print
time
man time
time -h
time --help
time --h
time cd
time cat
time home
home
cls
time clear
clear
time clear
time find .
ps
cd /cygdrive/m/Qt
cd *
cd qt
cd src
find .|grep tabwidget
cd gui/widgets
vi qtabwidget.cpp
grep removeWidget *
cd ../..
fndc removeWidget
whois 72.213.46.131
whois ?
whois -h
whois -c 72.213.46.131
whois -z 72.213.46.131
whois --help
whois -h
whois ?
whois -h
whois 72.213.46.131
whois 72.213.46.131 -z
whois -z 72.213.46.131
whois ?
whois /?
whois -h
whois --help
whois 72.213.46.131
?
whois -a 72.213.46.131
whois z 72.213.46.131
whois -a 72.213.46.131
whois -a n 72.213.46.131
whois -c 72.213.46.131
whois 72.213.46.131 z
whois
whois cox.net
whois 72.132.167.23
cls
clear
whois remote.mccinfo.net
whois mccinfo.net
connect
connect remote.mccinfo.net
connect remote.mccinfo.net 80
telnet
ping
ping remote.mccinfo.net
ping info.mccinfo.net
ping info.mccinfo.net &
ping infoproject.mccinfo.net
ping infoproject.mccinfo.net &
nslookup
dns
dnslookup
rpc
cd /
cd bin
ls|more
dnslookup
ind . -name * -type f -exec grep INT_MAX {} \; -print
namei
hostid
hostname
hostname https://remote.mccinfo.net 
 nslookup
nslookup -h
ssh info.mccinfo.net
rsh info.mccinfo.net
rlogin remote.mccinfo.net
rlogin info.mccinfo.net
ping remote.mccinfo.net
ssh info4.mccinfo.net
ftp ftp.mccinfo.net
nslookup
ssh 173.190.246.189
ssh -h
rlogin 173.190.246.189
ssh sobrien5@173.190.246.189
ssh mccadcstudent173.190.246.189
ssh mccadcstudent@173.190.246.189
rsh 173.190.246.189
connect 173.190.246.189
connect 173.190.246.189 80
home
. cdmonkey
cd *x
fndc gridLayout_3
svnadmin
--
svnadmin help
svnadmin --help
man svnadmin
svnadmin help
cd /cygdrive/m/users
cd shawno
cd appdata
cd local/temp
dir
rm * -fr
cd /cygdrive/i
dir
cd P*
cd E*
cd B*
cler
clear
cd /cygdrive/m
rm -fr 3*
rmdir -fr 3*
rmdir --help
rmdir --ignore-fail-on-non-empty -p 3*
rmdir -p 3*
rmdir -p `ls -R 3*`
rmdir  -p < ls -R 3*
rmdir -p 3*
bell
nobell
hel
error
err
cd /cygdrive
cd n
l
cd "Program Files (x86)"
home
cd ../..
l
home
l
cd sbin
l
cd ../commands
l
cd ../..
man find
cd ..
cd lib
cd ..
find . -name jre* -type d
cd /cygdrive/n
l
home
source .bash_profile
l
cd ../../cygdrive
cd n
cd "Deployed"
l
cd "Deployed Programs"
l
cd server
cd [server]
l
cd g*
l
cd jdk7
l
ping www.google.co
ping www.google.com
vi tmp.reg
dir
del tmp.reg
rm tmp.reg
cd /cygdrive/c/inetpub/logs/LogFiles/W3SVC1
dir
tail --help
tail -f *401*.log
cls
clear
tail -f *401*.log
tail -h
tail --help
clear
tail -f -v *401*.log
ki
 kikjkk:
cd /cygdrive
dir
cd h
dir
svnadmin dump d:/repositories
svnadmin dump d:/repositories/
svnadmin dump d:/repositories/test
cd d
cd ..
dir
cd h
svnadmin dump y:/repositories/test
svnadmin dump y:/repositories/test|svn load repositories/
svnadmin dump y:/repositories/test|svnadmin load repositories/
svnadmin dump y:/repositories/test|svnadmin/test load repositories/
svnadmin dump y:/repositories/test|svnadmin load repositories/test
svnadmin help
svnadmin create help
dir
svnadmin help create
svnadmin create ./Repositories/test/
svnadmin dump y:/repositories/test|svnadmin load repositories/test
svnadmin create ./Repositories/shawno/
svnadmin dump y:/repositories/shawno|svnadmin load repositories/shawno
svnadmin dump y:/repositories/test|svnadmin load repositories/test
cd repositories
rm -fr test
cd ..
svnadmin dump y:/repositories/test|svnadmin load repositories/test
svnadmin create ./repositories/test/
svnadmin dump y:/repositories/test|svnadmin load repositories/test
dir
if -t hack.dump
if -t hack.dump;
if -t hack.dump echo 1
if -t hack.dump echo 1;
exists
login --help
login -h
bash --help login
bash -c "help login"
bash -c info login
man bash
l
cd .ssh
l
vi config
vi *.pub.pub
file *
vi known_hosts
home
l
ssh loki
