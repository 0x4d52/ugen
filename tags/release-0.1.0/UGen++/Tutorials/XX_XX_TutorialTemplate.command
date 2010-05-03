#!/bin/sh

echo \\n\\n
echo "*****************************************************************************************************"
echo "* This script unzips XX_XX_TutorialTemplate.zip and renames various files based on the script name. *"
echo "*  Make a copy of this script file renaming it (e.g., TEST.command would produce a tutorial called  *"
echo "*  TEST, and the mac project would be called TEST.xcodeproj, the win project TEST.vcproj).          *"
echo "*  Then run the script.                                                                             *"
echo "*****************************************************************************************************"

echo \\n\\n

sleep 2

SCRIPTNAME=`basename $0`
TUTORIALNAME=${SCRIPTNAME%%.*}	
cd `dirname $0`
unzip ./XX_XX_TutorialTemplate.zip
mv XX_XX_TutorialTemplate $TUTORIALNAME
mv ./$TUTORIALNAME/mac_project/XX_XX_TutorialTemplate.xcodeproj $TUTORIALNAME/mac_project/$TUTORIALNAME.xcodeproj
mv ./$TUTORIALNAME/win_project/XX_XX_TutorialTemplate.vcproj $TUTORIALNAME/win_project/$TUTORIALNAME.vcproj


echo \\n\\n

echo Created tutorial: $TUTORIALNAME

echo \\n\\n
