
#copy main.xml
configure_file(res/main.xml res/main.xml COPYONLY)
configure_file(res/fileExplorer.css res/fileExplorer.css COPYONLY)
configure_file(res/Entry.css res/Entry.css COPYONLY)
configure_file(res/icons/ie-512x512.png res/icons/ie-512x512.png COPYONLY)

configure_file(res/bitmapViewer.glade res/bitmapViewer.glade COPYONLY)
configure_file(res/meshes.glade res/meshes.glade COPYONLY)
configure_file(res/bsp.glade res/bsp.glade COPYONLY)
configure_file(res/matDialog.glade res/matDialog.glade COPYONLY)

configure_file(res/hashes.txt res/hashes.txt COPYONLY)

configure_file(res/vtx.glsl res/vtx.glsl COPYONLY)
configure_file(res/frg.glsl res/frg.glsl COPYONLY)