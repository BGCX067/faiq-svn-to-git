# My Profile:  hacked from anywhere

PATH=./:/home/shawno/bin/apache-ant-1.8.2/bin:/cygdrive/m/!repo/branches/shawno/sbin:/home/shawno/sh:/home/shawno/sbin:$PATH
export PATH

set -o vi

ANT_HOME=/home/shawno/bin/apache-ant-1.8.2
export ANT_HOME

JAVA_HOME=/cygdrive/n/Deployed\ Programs/[server]/glassfish3/jdk7
export JAVA_HOME

#emacs completion stuff 
#set show-all-if-ambiguous on

PS1='$PWD>'

alias l='ls -la --color=never | more'

alias home='cd /home/shawno'

alias pro='. ~/.bash_profile'

export CDPATH=.:~:~/docs:~/src:~/src/ops/docs:/home:/mnt:/usr/src:/usr/lib:/usr/local

export HISTIGNORE="&:ls:ls *:[bf]g:exit"

shopt -s cdspell
shopt -s dotglob 
shopt -s extglob 

#?(pattern-list) 
#Matches zero or one occurrence of the given patterns 
#*(pattern-list) 
#Matches zero or more occurrences of the given patterns 
#+(pattern-list) 
#Matches one or more occurrences of the given patterns 
#@(pattern-list) 
#Matches exactly one of the given patterns 
#!(pattern-list) 
#Matches anything except one of the given patterns 
#Here's an example. Say, you wanted to install all RPMs in a given directory, except those built for the noarch architecture. You might use something like this: 
#rpm -Uvh /usr/src/RPMS/!(*noarch*) 

#These expressions can be nested, too, so if you wanted a directory listing of all non PDF and PostScript files in the current directory, you might do this: 

#ls -lad !(*.p@(df|s))
#---------------------------------#
#  a few Korn/Bash shell aliases  #
#---------------------------------#

alias lc="ls -C"
alias lm="ls -al | more"
alias dirs="ls -al | grep '^d'"     # show the dir's in the current dir
alias h=history                     # show the history of commands issued

alias nu="who|wc -l"                # nu - number of users
alias np="ps -ef|wc -l"             # np - number of processes running
alias p="ps -ef"

# if [ -d /usr/local/bin ]; then
#	pathprepend /usr/local/bin
# fi

# mimick a few DOS commands with these aliases:

alias cd..="cd ../.."
alias cd...="cd ../../.."
alias dir="ls -al"
alias edit=vi
alias help=man
alias path="echo $PATH"

#clear
#cd /home/shawno
#fortune

#
#
#
#$TMOUT 
#If you set this to a value greater than zero, bash will terminate after this number of seconds have elapsed if no input arrives. 

#This setting is useful in root's environment to reduce the potential security risk of someone forgetting to log out as the superuser.
