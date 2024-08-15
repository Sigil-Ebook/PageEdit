#!/bin/sh
echo $(pwd)
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/iconengines/libqsvgicon.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/imageformats/libqgif.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/imageformats/libqicns.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/imageformats/libqico.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/imageformats/libqjpeg.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/imageformats/libqmacheif.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/imageformats/libqmacjp2.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/imageformats/libqpdf.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/imageformats/libqsvg.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/imageformats/libqtga.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/imageformats/libqtiff.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/imageformats/libqwbmp.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/imageformats/libqwebp.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/networkinformation/libqscnetworkreachability.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/platforminputcontexts/libqtvirtualkeyboardplugin.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/platforms/libqcocoa.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/position/libqtposition_cl.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/position/libqtposition_nmea.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/position/libqtposition_positionpoll.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/styles/libqmacstyle.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/tls/libqcertonlybackend.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/tls/libqopensslbackend.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/PageEdit.app/Contents/PlugIns/tls/libqsecuretransportbackend.dylib
