
#copy main.xml
function(copyres dest)
configure_file(res/main.xml ${dest}res/main.xml COPYONLY)
configure_file(res/fileExplorer.css ${dest}res/fileExplorer.css COPYONLY)
configure_file(res/Entry.css ${dest}res/Entry.css COPYONLY)
configure_file(res/icons/ie-512x512.png ${dest}res/icons/ie-512x512.png COPYONLY)

configure_file(res/bitmapViewer.glade ${dest}res/bitmapViewer.glade COPYONLY)
configure_file(res/meshes.glade ${dest}res/meshes.glade COPYONLY)
configure_file(res/bsp.glade ${dest}res/bsp.glade COPYONLY)
configure_file(res/matDialog.glade ${dest}res/matDialog.glade COPYONLY)

configure_file(res/hashes.txt ${dest}res/hashes.txt COPYONLY)

configure_file(res/vtx.glsl ${dest}res/vtx.glsl COPYONLY)
configure_file(res/frg.glsl ${dest}res/frg.glsl COPYONLY)
endfunction(copyres)
