#!/bin/sh


PROJECT_NAME=qmmp-stable
PROJECT_ORGANIZATION=qmmp-development-team
TX_CONFIG="../.tx/config"

echo "[main]" > ${TX_CONFIG}
echo "host = https://www.transifex.com" >> ${TX_CONFIG}
echo "" >> ${TX_CONFIG}

for tr_dir in `find ../src/ -type d -name "translations"`
do

     plug_name=`echo $tr_dir | cut -d '/' -f 5`

     if  [ -z "$plug_name" ]; then
         plug_name=`echo $tr_dir | cut -d '/' -f 3`
     fi

     case $plug_name in
        IncDecVolumeOption)
            plug_name="incdecvolume"
        ;;
        PlayListOption)
            plug_name="playlist"
        ;;
        SeekOption)
            plug_name="seek"
        ;;
        StatusOption)
            plug_name="status"
        ;;
        UninstallOption)
            plug_name="uninstall"
        ;;
        QmmpFileDialog)
            plug_name="qmmp_file_dialog"
        ;;
        TwoPanelFileDialog)
            plug_name="two_panel_file_dialog"
        ;;
        qmmpui)
            plug_name="libqmmpui"
        ;;
        app)
            plug_name="qmmp"
        ;;
     esac

     echo "Updating ${plug_name}"

     file_filter=`echo ${tr_dir} | sed 's/..\///'`

     echo "[o:${PROJECT_ORGANIZATION}:p:${PROJECT_NAME}:r:${plug_name}]" >> ${TX_CONFIG}

     if [ "$plug_name" = "qmmp" ] || [ "$plug_name" = "libqmmpui" ]; then
        echo "file_filter = ${file_filter}/${plug_name}_<lang>.ts" >> ${TX_CONFIG}
        echo "source_lang = en" >> ${TX_CONFIG}
        echo "source_file = ${file_filter}/${plug_name}_en.ts" >> ${TX_CONFIG}
     else
        echo "file_filter = ${file_filter}/${plug_name}_plugin_<lang>.ts" >> ${TX_CONFIG}
        echo "source_lang = en" >> ${TX_CONFIG}
        echo "source_file = ${file_filter}/${plug_name}_plugin_en.ts" >> ${TX_CONFIG}
     fi
     echo "type = QT" >> ${TX_CONFIG}
     echo "" >> ${TX_CONFIG}

done

for RESOURCE_NAME in description authors translators thanks
do
    echo "[o:${PROJECT_ORGANIZATION}:p:${PROJECT_NAME}:r:${RESOURCE_NAME}]" >> ${TX_CONFIG}
    echo "file_filter = src/qmmpui/txt/${RESOURCE_NAME}_<lang>.txt" >> ${TX_CONFIG}
    echo "source_lang = en" >> ${TX_CONFIG}
    echo "source_file = src/qmmpui/txt/${RESOURCE_NAME}.txt" >> ${TX_CONFIG}
    echo "type = TXT" >> ${TX_CONFIG}
    echo "" >> ${TX_CONFIG}
done

for RESOURCE_NAME in qmmp qmmp-dir qmmp-enqueue qmmp-opencda
do
    echo "[o:${PROJECT_ORGANIZATION}:p:${PROJECT_NAME}:r:${RESOURCE_NAME}-desktop]" >> ${TX_CONFIG}
    echo "file_filter = src/app/desktop-translations/${RESOURCE_NAME}_<lang>.desktop.in" >> ${TX_CONFIG}
    echo "source_lang = en" >> ${TX_CONFIG}
    echo "source_file = src/app/${RESOURCE_NAME}.desktop" >> ${TX_CONFIG}
    echo "type = DESKTOP" >> ${TX_CONFIG}
    echo "" >> ${TX_CONFIG}
done
