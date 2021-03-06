  PROJECT( photolayoutseditor )

  #SET(CMAKE_INSTALL_PREFIX /usr/)

  CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

  ADD_SUBDIRECTORY( ./utils/qtpropertybrowser )

  FIND_PACKAGE(KDE4 REQUIRED)
  INCLUDE( KDE4Defaults )

  FIND_PACKAGE(Kipi REQUIRED)

  SET( CMAKE_PACKAGE_QTGUI TRUE )
  SET( QT_USE_QTXML TRUE )

  SET( PhotoLayoutsEditor_SRCS
      plugin/photolayoutseditor.cpp
      widgets/dialogs/CanvasSizeDialog.cpp
      models/LayersModel.cpp
      models/LayersModelItem.cpp
      models/LayersSelectionModel.cpp
      models/AbstractMovableModel.cpp
      widgets/canvas/Scene.cpp
      widgets/canvas/Canvas.cpp
      widgets/layers/LayersTree.cpp
      widgets/layers/LayersTreeDelegate.cpp
      widgets/menus/LayersTreeMenu.cpp
      undo/UndoMoveRowsCommand.cpp
      widgets/tools/ToolsDockWidget.cpp
      widgets/tools/BorderEditTool.cpp
      widgets/items/AbstractPhoto.cpp
      widgets/items/PhotoItem.cpp
      widgets/tools/EffectsEditorTool.cpp
      effects/PhotoEffectsGroup.cpp
      utils/KEditFactory.cpp
      events/UndoCommandEvent.cpp
      events/UndoCommandEventFilter.cpp
      effects/PhotoEffectsLoader.cpp
      utils/global.cpp
      utils/SavingThread.cpp
      widgets/items/TextItem.cpp
      widgets/dialogs/ImageFileDialog.cpp
      settings/PLEConfigDialog.cpp
      settings/PLEConfigViewWidget.cpp
      widgets/tools/TextEditorTool.cpp
      listeners/TextColorChangeListener.cpp
      listeners/TextFontChangeListener.cpp
      widgets/items/AbstractItemInterface.cpp
      borders/BordersGroup.cpp
      borders/BorderDrawersLoader.cpp
      borders/BorderChangeListener.cpp
      widgets/dialogs/GridSetupDialog.cpp
      widgets/dialogs/SavingProgressDialog.cpp
      widgets/canvas/SceneBackground.cpp
      widgets/tools/CanvasEditTool.cpp
      widgets/tools/AbstractTool.cpp
      widgets/tools/AbstractItemsTool.cpp
      widgets/tools/AbstractItemsListViewTool.cpp
      widgets/extra/PatternsComboBox.cpp
      widgets/canvas/RotationWidgetItem.cpp
      widgets/canvas/ScalingWidgetItem.cpp
      widgets/canvas/CropWidgetItem.cpp
      widgets/canvas/MousePressListener.cpp
      widgets/canvas/CanvasSize.cpp
  )

  SET(PhotoLayoutsEditor_RCS
      resources/icons.qrc
  )

  QT4_ADD_RESOURCES( PhotoLayoutsEditor_RC_SRCS ${PhotoLayoutsEditor_RCS} )

  SET( INCLUDE_DIRS
      plugin
      widgets
      widgets/extra
      widgets/canvas
      widgets/dialogs
      widgets/items
      widgets/layers
      widgets/menus
      widgets/tools
      effects
      events
      listeners
      utils
      utils/qtpropertybrowser/src
      models
      undo
      borders
      settings
  )

  INCLUDE_DIRECTORIES(${QT_INCLUDE}
                      ${KDE4_INCLUDES}
                      ${INCLUDE_DIRS}
                      ${KIPI_INCLUDE_DIR})

  ADD_SUBDIRECTORY( ./resources/app )

  KDE4_ADD_KCFG_FILES(PhotoLayoutsEditor_SRCS GENERATE_MOC ./settings/PLEConfigSkeleton.kcfgc)
  INSTALL( FILES ./settings/PLEConfigSkeleton.kcfgc DESTINATION ${KCFG_INSTALL_DIR} )

  # Adjust flag for static lib and 64 bits computers using -fPIC for GCC compiler (B.K.O: #269903)
  FOREACH(_currentfile ${PhotoLayoutsEditor_SRCS})
      IF(NOT MSVC)
          SET_SOURCE_FILES_PROPERTIES(${_currentfile} PROPERTIES COMPILE_FLAGS "-fPIC")
      ENDIF(NOT MSVC)
  ENDFOREACH(_currentfile ${PhotoLayoutsEditor_SRCS})

  KDE4_ADD_LIBRARY( libphotolayoutseditor STATIC ${PhotoLayoutsEditor_SRCS} ${PhotoLayoutsEditor_RC_SRCS} )

  SET_TARGET_PROPERTIES(libphotolayoutseditor PROPERTIES COMPILE_FLAGS "-fPIC")

  TARGET_LINK_LIBRARIES( libphotolayoutseditor ${QT_LIBRARIES} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KIPI_LIBRARIES} PropertyBrowser )

# =======================================================
# PhotoLayoutsEditor plugin

  SET(kipiplugin_PhotoLayoutsEditor_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/plugin/plugin_photolayoutseditor.cpp)

  KDE4_ADD_PLUGIN(kipiplugin_photolayoutseditor ${kipiplugin_PhotoLayoutsEditor_PART_SRCS})

  TARGET_LINK_LIBRARIES(kipiplugin_photolayoutseditor libphotolayoutseditor ${KIPI_LIBRARIES})

  INSTALL(TARGETS kipiplugin_photolayoutseditor DESTINATION ${PLUGIN_INSTALL_DIR})
  INSTALL(FILES plugin/kipiplugin_photolayoutseditor.desktop DESTINATION ${SERVICES_INSTALL_DIR})

# =======================================================
# Stand-alone version of PhotoLayoutsEditor

  SET(PhotoLayoutsEditor_SRCS main.cpp)

  KDE4_ADD_APP_ICON( PhotoLayoutsEditor_SRCS "photolayoutseditor" )

  KDE4_ADD_EXECUTABLE( photolayoutseditor ${PhotoLayoutsEditor_SRCS} )

  TARGET_LINK_LIBRARIES( photolayoutseditor libphotolayoutseditor )
  INSTALL(TARGETS photolayoutseditor ${INSTALL_TARGETS_DEFAULT_ARGS})
  INSTALL(PROGRAMS plugin/photolayoutseditor.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
  INSTALL(FILES plugin/photolayoutseditorui.rc DESTINATION ${DATA_INSTALL_DIR}/photolayoutseditor)
  INSTALL(FILES effects/photolayoutseditoreffectplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
  INSTALL(FILES borders/photolayoutseditorborderplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

  ADD_SUBDIRECTORY( ./effectplugins )
  ADD_SUBDIRECTORY( ./borderplugins )
